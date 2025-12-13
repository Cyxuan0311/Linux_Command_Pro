#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>
#include <dirent.h>
#include <time.h>
#include <stdint.h>
#include <zlib.h>
#include "../include/common.h"

#define MAX_FILENAME 256
#define MAX_FILES 1000
#define BUFFER_SIZE 8192
#define COMPRESSION_LEVEL 6

// 操作类型枚举
typedef enum {
    OP_COMPRESS,    // 压缩
    OP_DECOMPRESS,  // 解压
    OP_LIST,        // 列出内容
    OP_TEST         // 测试
} OperationType;

// 压缩配置结构
typedef struct {
    char archive_name[MAX_FILENAME];
    char **files;
    int file_count;
    OperationType operation;
    int compression_level;
    int verbose;
    int force;
    int recursive;
    int preserve_paths;
} ZipConfig;

// ZIP文件头结构
typedef struct {
    uint32_t signature;
    uint16_t version;
    uint16_t flags;
    uint16_t compression;
    uint16_t mod_time;
    uint16_t mod_date;
    uint32_t crc32;
    uint32_t compressed_size;
    uint32_t uncompressed_size;
    uint16_t filename_length;
    uint16_t extra_field_length;
} ZipLocalFileHeader;

// 文件信息结构
typedef struct {
    char filename[MAX_FILENAME];
    off_t size;
    time_t mtime;
    uint32_t crc32;
    int is_directory;
} FileInfo;

// 显示进度条
void show_progress(off_t current, off_t total, const char *operation) {
    if (total <= 0) return;
    
    int percent = (int)((current * 100) / total);
    int bar_length = 50;
    int filled_length = (int)((current * bar_length) / total);
    
    printf("\r%s%s [", COLOR_CYAN, operation);
    for (int i = 0; i < bar_length; i++) {
        if (i < filled_length) {
            printf("%s█%s", COLOR_GREEN, COLOR_RESET);
        } else {
            printf(" ");
        }
    }
    printf("] %d%% (%s/%s)", percent, format_size(current), format_size(total));
    fflush(stdout);
}

// 计算CRC32
uint32_t calculate_crc32(const char *data, size_t length) {
    return crc32(0, (const Bytef*)data, length);
}

// 获取文件信息
int get_file_info(const char *filename, FileInfo *info) {
    struct stat st;
    if (stat(filename, &st) != 0) {
        return 0;
    }
    
    strncpy(info->filename, filename, MAX_FILENAME - 1);
    info->filename[MAX_FILENAME - 1] = '\0';
    info->size = st.st_size;
    info->mtime = st.st_mtime;
    info->is_directory = S_ISDIR(st.st_mode);
    info->crc32 = 0;
    
    if (!info->is_directory) {
        // 计算文件CRC32
        FILE *file = fopen(filename, "rb");
        if (file) {
            char buffer[BUFFER_SIZE];
            size_t bytes;
            uint32_t crc = 0;
            
            while ((bytes = fread(buffer, 1, sizeof(buffer), file)) > 0) {
                crc = crc32(crc, (const Bytef*)buffer, bytes);
            }
            
            info->crc32 = crc;
            fclose(file);
        }
    }
    
    return 1;
}

// 压缩文件
int compress_file(const char *filename, FILE *output, int compression_level) {
    FILE *input = fopen(filename, "rb");
    if (!input) {
        return 0;
    }
    
    // 初始化zlib
    z_stream strm;
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    
    if (deflateInit2(&strm, compression_level, Z_DEFLATED, -MAX_WBITS, 8, Z_DEFAULT_STRATEGY) != Z_OK) {
        fclose(input);
        return 0;
    }
    
    char in_buffer[BUFFER_SIZE];
    char out_buffer[BUFFER_SIZE];
    int ret;
    
    do {
        strm.avail_in = fread(in_buffer, 1, sizeof(in_buffer), input);
        strm.next_in = (Bytef*)in_buffer;
        
        do {
            strm.avail_out = sizeof(out_buffer);
            strm.next_out = (Bytef*)out_buffer;
            
            ret = deflate(&strm, Z_NO_FLUSH);
            if (ret == Z_STREAM_ERROR) {
                deflateEnd(&strm);
                fclose(input);
                return 0;
            }
            
            size_t have = sizeof(out_buffer) - strm.avail_out;
            fwrite(out_buffer, 1, have, output);
        } while (strm.avail_out == 0);
    } while (strm.avail_in != 0);
    
    // 完成压缩
    do {
        strm.avail_out = sizeof(out_buffer);
        strm.next_out = (Bytef*)out_buffer;
        
        ret = deflate(&strm, Z_FINISH);
        if (ret == Z_STREAM_ERROR) {
            deflateEnd(&strm);
            fclose(input);
            return 0;
        }
        
        size_t have = sizeof(out_buffer) - strm.avail_out;
        fwrite(out_buffer, 1, have, output);
    } while (strm.avail_out == 0);
    
    deflateEnd(&strm);
    fclose(input);
    return 1;
}

// 解压文件
int decompress_file(FILE *input, const char *filename, off_t compressed_size) {
    FILE *output = fopen(filename, "wb");
    if (!output) {
        return 0;
    }
    
    // 初始化zlib
    z_stream strm;
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    
    if (inflateInit2(&strm, -MAX_WBITS) != Z_OK) {
        fclose(output);
        return 0;
    }
    
    char in_buffer[BUFFER_SIZE];
    char out_buffer[BUFFER_SIZE];
    int ret;
    off_t bytes_read = 0;
    
    do {
        size_t to_read = (compressed_size - bytes_read > (off_t)sizeof(in_buffer)) ? 
                        sizeof(in_buffer) : (size_t)(compressed_size - bytes_read);
        
        strm.avail_in = fread(in_buffer, 1, to_read, input);
        strm.next_in = (Bytef*)in_buffer;
        bytes_read += strm.avail_in;
        
        do {
            strm.avail_out = sizeof(out_buffer);
            strm.next_out = (Bytef*)out_buffer;
            
            ret = inflate(&strm, Z_NO_FLUSH);
            if (ret == Z_STREAM_ERROR) {
                inflateEnd(&strm);
                fclose(output);
                return 0;
            }
            
            size_t have = sizeof(out_buffer) - strm.avail_out;
            fwrite(out_buffer, 1, have, output);
        } while (strm.avail_out == 0);
    } while (ret != Z_STREAM_END && bytes_read < compressed_size);
    
    inflateEnd(&strm);
    fclose(output);
    return 1;
}

// 创建ZIP文件
int create_zip(const ZipConfig *config) {
    FILE *output = fopen(config->archive_name, "wb");
    if (!output) {
        print_error("无法创建压缩文件");
        return 1;
    }
    
    printf("%s开始压缩文件...%s\n", COLOR_CYAN, COLOR_RESET);
    printf("%s压缩文件: %s%s\n", COLOR_YELLOW, config->archive_name, COLOR_RESET);
    printf("%s文件数量: %d%s\n", COLOR_YELLOW, config->file_count, COLOR_RESET);
    printf("%s压缩级别: %d%s\n", COLOR_YELLOW, config->compression_level, COLOR_RESET);
    
    off_t total_size = 0;
    off_t compressed_size = 0;
    
    for (int i = 0; i < config->file_count; i++) {
        FileInfo info;
        if (!get_file_info(config->files[i], &info)) {
            print_warning("无法获取文件信息，跳过");
            continue;
        }
        
        if (info.is_directory) {
            printf("%s跳过目录: %s%s\n", COLOR_YELLOW, info.filename, COLOR_RESET);
            continue;
        }
        
        total_size += info.size;
        
        // 写入ZIP文件头
        ZipLocalFileHeader header = {0};
        header.signature = 0x04034b50;  // ZIP签名
        header.version = 20;
        header.flags = 0;
        header.compression = 8;  // DEFLATE
        header.mod_time = 0;
        header.mod_date = 0;
        header.crc32 = info.crc32;
        header.uncompressed_size = info.size;
        header.filename_length = strlen(info.filename);
        header.extra_field_length = 0;
        
        fwrite(&header, sizeof(header), 1, output);
        fwrite(info.filename, 1, header.filename_length, output);
        
        // 压缩文件内容
        off_t start_pos = ftell(output);
        if (compress_file(info.filename, output, config->compression_level)) {
            off_t end_pos = ftell(output);
            header.compressed_size = end_pos - start_pos;
            compressed_size += header.compressed_size;
            
            // 更新文件头中的压缩大小
            fseek(output, start_pos - sizeof(header) + offsetof(ZipLocalFileHeader, compressed_size), SEEK_SET);
            fwrite(&header.compressed_size, sizeof(header.compressed_size), 1, output);
            fseek(output, end_pos, SEEK_SET);
            
            if (config->verbose) {
                printf("%s压缩: %s (%s -> %s)%s\n", 
                       COLOR_GREEN, info.filename, 
                       format_size(info.size), format_size(header.compressed_size), COLOR_RESET);
            }
        } else {
            print_warning("压缩失败，跳过");
        }
        
        if (config->verbose) {
            show_progress(compressed_size, total_size, "压缩进度");
        }
    }
    
    fclose(output);
    
    if (config->verbose) {
        printf("\n");
    }
    
    printf("%s压缩完成！%s\n", COLOR_GREEN, COLOR_RESET);
    printf("%s原始大小: %s%s\n", COLOR_CYAN, format_size(total_size), COLOR_RESET);
    printf("%s压缩大小: %s%s\n", COLOR_CYAN, format_size(compressed_size), COLOR_RESET);
    
    if (total_size > 0) {
        int ratio = (int)((compressed_size * 100) / total_size);
        printf("%s压缩率: %d%%%s\n", COLOR_CYAN, ratio, COLOR_RESET);
    }
    
    return 0;
}

// 列出ZIP文件内容
int list_zip(const ZipConfig *config) {
    FILE *input = fopen(config->archive_name, "rb");
    if (!input) {
        print_error("无法打开压缩文件");
        return 1;
    }
    
    printf("%sZIP文件内容: %s%s\n", COLOR_CYAN, config->archive_name, COLOR_RESET);
    printf("%s%s%s\n", COLOR_YELLOW, "=" + strlen(config->archive_name) + 10, COLOR_RESET);
    
    ZipLocalFileHeader header;
    int file_count = 0;
    off_t total_size = 0;
    off_t compressed_size = 0;
    
    while (fread(&header, sizeof(header), 1, input) == 1) {
        if (header.signature != 0x04034b50) {
            break;  // 不是ZIP文件头
        }
        
        char filename[MAX_FILENAME];
        fread(filename, 1, header.filename_length, input);
        filename[header.filename_length] = '\0';
        
        // 跳过额外字段
        fseek(input, header.extra_field_length, SEEK_CUR);
        
        // 跳过压缩数据
        fseek(input, header.compressed_size, SEEK_CUR);
        
        printf("%s %s %s %s %s %s\n",
               ICON_FILE,
               filename,
               COLOR_WHITE,
               format_size(header.uncompressed_size),
               format_size(header.compressed_size),
               COLOR_RESET);
        
        total_size += header.uncompressed_size;
        compressed_size += header.compressed_size;
        file_count++;
    }
    
    fclose(input);
    
    printf("\n%s文件数量: %d%s\n", COLOR_CYAN, file_count, COLOR_RESET);
    printf("%s原始大小: %s%s\n", COLOR_CYAN, format_size(total_size), COLOR_RESET);
    printf("%s压缩大小: %s%s\n", COLOR_CYAN, format_size(compressed_size), COLOR_RESET);
    
    if (total_size > 0) {
        int ratio = (int)((compressed_size * 100) / total_size);
        printf("%s压缩率: %d%%%s\n", COLOR_CYAN, ratio, COLOR_RESET);
    }
    
    return 0;
}

// 解压ZIP文件
int extract_zip(const ZipConfig *config) {
    FILE *input = fopen(config->archive_name, "rb");
    if (!input) {
        print_error("无法打开压缩文件");
        return 1;
    }
    
    printf("%s开始解压文件...%s\n", COLOR_CYAN, COLOR_RESET);
    printf("%s压缩文件: %s%s\n", COLOR_YELLOW, config->archive_name, COLOR_RESET);
    
    ZipLocalFileHeader header;
    int file_count = 0;
    
    while (fread(&header, sizeof(header), 1, input) == 1) {
        if (header.signature != 0x04034b50) {
            break;  // 不是ZIP文件头
        }
        
        char filename[MAX_FILENAME];
        fread(filename, 1, header.filename_length, input);
        filename[header.filename_length] = '\0';
        
        // 跳过额外字段
        fseek(input, header.extra_field_length, SEEK_CUR);
        
        // 解压文件
        if (decompress_file(input, filename, header.compressed_size)) {
            printf("%s解压: %s%s\n", COLOR_GREEN, filename, COLOR_RESET);
            file_count++;
        } else {
            print_warning("解压失败，跳过");
        }
    }
    
    fclose(input);
    
    printf("%s解压完成！%s\n", COLOR_GREEN, COLOR_RESET);
    printf("%s解压了 %d 个文件%s\n", COLOR_CYAN, file_count, COLOR_RESET);
    
    return 0;
}

void print_usage(const char *program_name) {
    printf("用法: %s [选项] 压缩文件 [文件...]\n", program_name);
    printf("优化版的 zip 命令，提供彩色输出和进度显示\n\n");
    printf("选项:\n");
    printf("  -c, --create         创建压缩文件\n");
    printf("  -x, --extract        解压文件\n");
    printf("  -l, --list           列出压缩文件内容\n");
    printf("  -t, --test           测试压缩文件\n");
    printf("  -r, --recursive      递归处理目录\n");
    printf("  -p, --preserve       保留路径结构\n");
    printf("  -f, --force          强制覆盖\n");
    printf("  -v, --verbose        显示详细信息\n");
    printf("  -1..-9              设置压缩级别 (1=最快, 9=最好)\n");
    printf("  -h, --help           显示此帮助信息\n");
    printf("  -V, --version        显示版本信息\n");
    printf("\n示例:\n");
    printf("  %s -c archive.zip file1.txt file2.txt\n", program_name);
    printf("  %s -x archive.zip\n", program_name);
    printf("  %s -l archive.zip\n", program_name);
    printf("  %s -c -r -9 backup.zip /home/user\n", program_name);
}

int main(int argc, char *argv[]) {
    ZipConfig config = {0};
    config.operation = OP_COMPRESS;
    config.compression_level = COMPRESSION_LEVEL;
    config.verbose = 0;
    config.force = 0;
    config.recursive = 0;
    config.preserve_paths = 0;
    config.files = malloc(MAX_FILES * sizeof(char*));
    config.file_count = 0;
    
    // 解析命令行参数
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-c") == 0 || strcmp(argv[i], "--create") == 0) {
            config.operation = OP_COMPRESS;
        } else if (strcmp(argv[i], "-x") == 0 || strcmp(argv[i], "--extract") == 0) {
            config.operation = OP_DECOMPRESS;
        } else if (strcmp(argv[i], "-l") == 0 || strcmp(argv[i], "--list") == 0) {
            config.operation = OP_LIST;
        } else if (strcmp(argv[i], "-t") == 0 || strcmp(argv[i], "--test") == 0) {
            config.operation = OP_TEST;
        } else if (strcmp(argv[i], "-r") == 0 || strcmp(argv[i], "--recursive") == 0) {
            config.recursive = 1;
        } else if (strcmp(argv[i], "-p") == 0 || strcmp(argv[i], "--preserve") == 0) {
            config.preserve_paths = 1;
        } else if (strcmp(argv[i], "-f") == 0 || strcmp(argv[i], "--force") == 0) {
            config.force = 1;
        } else if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--verbose") == 0) {
            config.verbose = 1;
        } else if (argv[i][0] == '-' && argv[i][1] >= '1' && argv[i][1] <= '9') {
            config.compression_level = argv[i][1] - '0';
        } else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            print_usage(argv[0]);
            free(config.files);
            return 0;
        } else if (strcmp(argv[i], "-V") == 0 || strcmp(argv[i], "--version") == 0) {
            printf("pzip - 优化版 zip 命令 v1.0\n");
            free(config.files);
            return 0;
        } else if (argv[i][0] != '-') {
            if (strlen(config.archive_name) == 0) {
                strncpy(config.archive_name, argv[i], MAX_FILENAME - 1);
            } else if (config.file_count < MAX_FILES) {
                config.files[config.file_count] = argv[i];
                config.file_count++;
            }
        } else {
            printf("未知选项: %s\n", argv[i]);
            print_usage(argv[0]);
            free(config.files);
            return 1;
        }
    }
    
    // 检查必需参数
    if (strlen(config.archive_name) == 0) {
        print_error("请指定压缩文件名");
        print_usage(argv[0]);
        free(config.files);
        return 1;
    }
    
    if (config.operation == OP_COMPRESS && config.file_count == 0) {
        print_error("请指定要压缩的文件");
        print_usage(argv[0]);
        free(config.files);
        return 1;
    }
    
    // 执行操作
    int result = 0;
    switch (config.operation) {
        case OP_COMPRESS:
            result = create_zip(&config);
            break;
        case OP_DECOMPRESS:
            result = extract_zip(&config);
            break;
        case OP_LIST:
            result = list_zip(&config);
            break;
        case OP_TEST:
            printf("测试功能暂未实现\n");
            result = 1;
            break;
        default:
            print_error("未知操作类型");
            result = 1;
            break;
    }
    
    free(config.files);
    return result;
}
