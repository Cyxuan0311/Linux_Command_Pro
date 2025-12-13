#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include <time.h>
#include <pwd.h>
#include <grp.h>
#include "../include/common.h"

#define MAX_PATH_LENGTH 1024
#define MAX_ENTRIES 10000
#define TAR_BLOCK_SIZE 512

// tar文件头结构 - 使用packed确保结构体对齐
typedef struct __attribute__((packed)) {
    char name[100];         // 文件名
    char mode[8];           // 文件权限
    char uid[8];            // 用户ID
    char gid[8];            // 组ID
    char size[12];          // 文件大小
    char mtime[12];         // 修改时间
    char chksum[8];         // 校验和
    char typeflag;          // 文件类型
    char linkname[100];     // 链接目标
    char magic[6];          // 魔数
    char version[2];        // 版本
    char uname[32];         // 用户名
    char gname[32];         // 组名
    char devmajor[8];       // 主设备号
    char devminor[8];       // 次设备号
    char prefix[155];       // 路径前缀
    char padding[12];       // 填充
} TarHeader;

// 文件条目结构
typedef struct {
    char path[MAX_PATH_LENGTH];
    char name[256];
    struct stat st;
    int is_dir;
    int is_symlink;
} TarEntry;

// tar操作类型
typedef enum {
    TAR_CREATE,     // 创建归档
    TAR_EXTRACT,    // 提取归档
    TAR_LIST,       // 列出内容
    TAR_APPEND,     // 追加文件
    TAR_UPDATE      // 更新文件
} TarOperation;

// tar配置结构
typedef struct {
    char archive_path[MAX_PATH_LENGTH];
    char *files[MAX_ENTRIES];
    int file_count;
    TarOperation operation;
    int verbose;
    int preserve_permissions;
    int follow_symlinks;
    int exclude_hidden;
    int compression;
    int progress;
} TarConfig;

// 初始化tar配置
void init_tar_config(TarConfig *config) {
    memset(config, 0, sizeof(TarConfig));
    config->operation = TAR_CREATE;
    config->verbose = 0;
    config->preserve_permissions = 1;
    config->follow_symlinks = 0;
    config->exclude_hidden = 0;
    config->compression = 0;
    config->progress = 0;
}

// 将数字转换为八进制字符串
void oct_to_string(char *str, int len, unsigned long num) {
    snprintf(str, len, "%07o", (unsigned int)num);
}

// 将八进制字符串转换为数字
unsigned long string_to_oct(const char *str, int len) {
    (void)len; // 避免未使用参数警告
    char *end;
    return strtoul(str, &end, 8);
}

// 计算tar头校验和
unsigned long calculate_checksum(const TarHeader *header) {
    unsigned long sum = 0;
    const char *p = (const char *)header;
    
    // 计算除校验和字段外的所有字节
    for (int i = 0; i < 148; i++) {
        sum += (unsigned char)p[i];
    }
    for (int i = 156; i < 512; i++) {
        sum += (unsigned char)p[i];
    }
    
    return sum;
}

// 创建tar文件头
void create_tar_header(TarHeader *header, const TarEntry *entry) {
    memset(header, 0, sizeof(TarHeader));
    
    // 设置文件名 - 只使用文件名部分，不包含路径
    const char *filename = strrchr(entry->name, '/');
    if (filename) {
        filename++; // 跳过 '/'
    } else {
        filename = entry->name;
    }
    strncpy(header->name, filename, sizeof(header->name) - 1);
    
    // 设置文件权限
    oct_to_string(header->mode, sizeof(header->mode), entry->st.st_mode & 0777);
    
    // 设置用户ID和组ID
    oct_to_string(header->uid, sizeof(header->uid), entry->st.st_uid);
    oct_to_string(header->gid, sizeof(header->gid), entry->st.st_gid);
    
    // 设置文件大小
    oct_to_string(header->size, sizeof(header->size), entry->st.st_size);
    
    // 设置修改时间
    oct_to_string(header->mtime, sizeof(header->mtime), entry->st.st_mtime);
    
    // 设置文件类型
    if (S_ISREG(entry->st.st_mode)) {
        header->typeflag = '0';
    } else if (S_ISDIR(entry->st.st_mode)) {
        header->typeflag = '5';
    } else if (S_ISLNK(entry->st.st_mode)) {
        header->typeflag = '2';
    } else {
        header->typeflag = '0';
    }
    
    // 设置魔数和版本
    strcpy(header->magic, "ustar");
    strncpy(header->version, "00", sizeof(header->version) - 1);
    
    // 设置用户名和组名
    struct passwd *pw = getpwuid(entry->st.st_uid);
    if (pw) {
        strncpy(header->uname, pw->pw_name, sizeof(header->uname) - 1);
    }
    
    struct group *gr = getgrgid(entry->st.st_gid);
    if (gr) {
        strncpy(header->gname, gr->gr_name, sizeof(header->gname) - 1);
    }
    
    // 计算并设置校验和
    unsigned long checksum = calculate_checksum(header);
    oct_to_string(header->chksum, sizeof(header->chksum), checksum);
}

// 写入tar文件头
int write_tar_header(FILE *file, const TarHeader *header) {
    return fwrite(header, 1, sizeof(TarHeader), file) == sizeof(TarHeader);
}

// 读取tar文件头
int read_tar_header(FILE *file, TarHeader *header) {
    size_t bytes_read = fread(header, 1, sizeof(TarHeader), file);
    if (bytes_read != sizeof(TarHeader)) {
        return 0;
    }
    
    // 检查是否为空块（tar文件结束标志）
    int is_empty = 1;
    for (size_t i = 0; i < sizeof(TarHeader); i++) {
        if (((char *)header)[i] != 0) {
            is_empty = 0;
            break;
        }
    }
    
    if (is_empty) {
        return 0;
    }
    
    // 确保字符串正确终止
    header->name[sizeof(header->name) - 1] = '\0';
    header->linkname[sizeof(header->linkname) - 1] = '\0';
    header->uname[sizeof(header->uname) - 1] = '\0';
    header->gname[sizeof(header->gname) - 1] = '\0';
    header->prefix[sizeof(header->prefix) - 1] = '\0';
    
    return 1;
}

// 添加文件到归档
int add_file_to_archive(FILE *archive, const char *filepath, const TarConfig *config) {
    struct stat st;
    if (lstat(filepath, &st) != 0) {
        print_error("无法获取文件信息");
        return 0;
    }
    
    TarEntry entry;
    strncpy(entry.path, filepath, sizeof(entry.path) - 1);
    strncpy(entry.name, filepath, sizeof(entry.name) - 1);
    entry.st = st;
    entry.is_dir = S_ISDIR(st.st_mode);
    entry.is_symlink = S_ISLNK(st.st_mode);
    
    // 创建tar头
    TarHeader header;
    create_tar_header(&header, &entry);
    
    // 写入头
    if (!write_tar_header(archive, &header)) {
        print_error("写入tar头失败");
        return 0;
    }
    
    // 如果是普通文件，写入内容
    if (S_ISREG(st.st_mode)) {
        FILE *file = fopen(filepath, "rb");
        if (!file) {
            print_error("无法打开文件");
            return 0;
        }
        
        char buffer[TAR_BLOCK_SIZE];
        size_t bytes_read;
        size_t total_written = 0;
        
        while ((bytes_read = fread(buffer, 1, sizeof(buffer), file)) > 0) {
            if (fwrite(buffer, 1, bytes_read, archive) != bytes_read) {
                print_error("写入文件内容失败");
                fclose(file);
                return 0;
            }
            total_written += bytes_read;
        }
        
        fclose(file);
        
        // 填充到块边界
        size_t padding = TAR_BLOCK_SIZE - (total_written % TAR_BLOCK_SIZE);
        if (padding < TAR_BLOCK_SIZE) {
            memset(buffer, 0, padding);
            fwrite(buffer, 1, padding, archive);
        }
    }
    
    if (config->verbose) {
        printf("%s添加: %s%s\n", COLOR_GREEN, filepath, COLOR_RESET);
    }
    
    return 1;
}

// 递归添加目录
int add_directory_to_archive(FILE *archive, const char *dirpath, const TarConfig *config) {
    DIR *dir = opendir(dirpath);
    if (!dir) {
        print_error("无法打开目录");
        return 0;
    }
    
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        // 跳过隐藏文件
        if (config->exclude_hidden && entry->d_name[0] == '.') {
            continue;
        }
        
        // 跳过当前目录和父目录
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }
        
        char full_path[MAX_PATH_LENGTH];
        snprintf(full_path, sizeof(full_path), "%s/%s", dirpath, entry->d_name);
        
        struct stat st;
        if (lstat(full_path, &st) != 0) {
            continue;
        }
        
        if (S_ISDIR(st.st_mode)) {
            // 递归处理子目录
            if (!add_directory_to_archive(archive, full_path, config)) {
                closedir(dir);
                return 0;
            }
        } else {
            // 添加文件
            if (!add_file_to_archive(archive, full_path, config)) {
                closedir(dir);
                return 0;
            }
        }
    }
    
    closedir(dir);
    return 1;
}

// 创建tar归档
int create_archive(const TarConfig *config) {
    FILE *archive = fopen(config->archive_path, "wb");
    if (!archive) {
        print_error("无法创建归档文件");
        return 0;
    }
    
    printf("%s创建归档: %s%s\n", COLOR_CYAN, config->archive_path, COLOR_RESET);
    
    int success = 1;
    for (int i = 0; i < config->file_count && success; i++) {
        struct stat st;
        if (lstat(config->files[i], &st) != 0) {
            print_error("文件不存在");
            continue;
        }
        
        if (S_ISDIR(st.st_mode)) {
            success = add_directory_to_archive(archive, config->files[i], config);
        } else {
            success = add_file_to_archive(archive, config->files[i], config);
        }
    }
    
    // 写入两个空块表示结束
    char empty_block[TAR_BLOCK_SIZE * 2];
    memset(empty_block, 0, sizeof(empty_block));
    fwrite(empty_block, 1, sizeof(empty_block), archive);
    
    fclose(archive);
    
    if (success) {
        print_success("归档创建完成");
    }
    
    return success;
}

// 列出tar归档内容
int list_archive(const TarConfig *config) {
    FILE *archive = fopen(config->archive_path, "rb");
    if (!archive) {
        print_error("无法打开归档文件");
        return 0;
    }
    
    printf("%s归档内容: %s%s\n", COLOR_CYAN, config->archive_path, COLOR_RESET);
    printf("%s%s%s\n", COLOR_YELLOW, "====================================", COLOR_RESET);
    
    TarHeader header;
    int entry_count = 0;
    
    while (read_tar_header(archive, &header)) {
        entry_count++;
        
        // 解析文件信息
        unsigned long size = string_to_oct(header.size, sizeof(header.size));
        unsigned long mode = string_to_oct(header.mode, sizeof(header.mode));
        unsigned long mtime = string_to_oct(header.mtime, sizeof(header.mtime));
        
        // 格式化时间
        char time_str[64];
        struct tm *tm_info = localtime((time_t *)&mtime);
        strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M", tm_info);
        
        // 格式化权限
        char perm_str[10];
        snprintf(perm_str, sizeof(perm_str), "%c%c%c%c%c%c%c%c%c",
                (mode & 0400) ? 'r' : '-',
                (mode & 0200) ? 'w' : '-',
                (mode & 0100) ? 'x' : '-',
                (mode & 0040) ? 'r' : '-',
                (mode & 0020) ? 'w' : '-',
                (mode & 0010) ? 'x' : '-',
                (mode & 0004) ? 'r' : '-',
                (mode & 0002) ? 'w' : '-',
                (mode & 0001) ? 'x' : '-');
        
        // 显示文件信息
        printf("%s%s%s %s%8lu%s %s%s%s %s%s%s\n",
               COLOR_GREEN, perm_str, COLOR_RESET,
               COLOR_CYAN, size, COLOR_RESET,
               COLOR_YELLOW, time_str, COLOR_RESET,
               COLOR_WHITE, header.name, COLOR_RESET);
        
        // 跳过文件内容
        if (header.typeflag == '0' || header.typeflag == '\0') {
            size_t blocks = (size + TAR_BLOCK_SIZE - 1) / TAR_BLOCK_SIZE;
            fseek(archive, blocks * TAR_BLOCK_SIZE, SEEK_CUR);
        }
    }
    
    printf("\n%s总计: %d 个条目%s\n", COLOR_CYAN, entry_count, COLOR_RESET);
    
    fclose(archive);
    return 1;
}

// 提取tar归档
int extract_archive(const TarConfig *config) {
    FILE *archive = fopen(config->archive_path, "rb");
    if (!archive) {
        print_error("无法打开归档文件");
        return 0;
    }
    
    printf("%s提取归档: %s%s\n", COLOR_CYAN, config->archive_path, COLOR_RESET);
    
    TarHeader header;
    int extracted_count = 0;
    
    while (read_tar_header(archive, &header)) {
        unsigned long size = string_to_oct(header.size, sizeof(header.size));
        unsigned long mode = string_to_oct(header.mode, sizeof(header.mode));
        
        if (config->verbose) {
            printf("%s提取: '%s' (长度: %zu, 大小: %lu)%s\n", COLOR_GREEN, header.name, strlen(header.name), size, COLOR_RESET);
        }
        
        // 跳过空文件名
        if (strlen(header.name) == 0) {
            continue;
        }
        
        if (header.typeflag == '0' || header.typeflag == '\0') {
            // 普通文件
            FILE *file = fopen(header.name, "wb");
            if (file) {
                char buffer[TAR_BLOCK_SIZE];
                size_t bytes_to_read = size;
                
                while (bytes_to_read > 0) {
                    size_t chunk = (bytes_to_read > sizeof(buffer)) ? sizeof(buffer) : bytes_to_read;
                    size_t bytes_read = fread(buffer, 1, chunk, archive);
                    if (bytes_read > 0) {
                        fwrite(buffer, 1, bytes_read, file);
                        bytes_to_read -= bytes_read;
                    } else {
                        break;
                    }
                }
                
                fclose(file);
                
                // 设置文件权限
                if (config->preserve_permissions) {
                    chmod(header.name, mode);
                }
                
                extracted_count++;
                
                // 跳过文件内容的填充部分
                size_t blocks = (size + TAR_BLOCK_SIZE - 1) / TAR_BLOCK_SIZE;
                size_t total_read = blocks * TAR_BLOCK_SIZE;
                size_t remaining = total_read - size;
                if (remaining > 0) {
                    fseek(archive, remaining, SEEK_CUR);
                }
            } else {
                // 如果无法创建文件，仍然需要跳过文件内容
                size_t blocks = (size + TAR_BLOCK_SIZE - 1) / TAR_BLOCK_SIZE;
                fseek(archive, blocks * TAR_BLOCK_SIZE, SEEK_CUR);
            }
        } else if (header.typeflag == '5') {
            // 目录
            mkdir(header.name, mode);
            extracted_count++;
        }
    }
    
    printf("%s提取完成: %d 个文件%s\n", COLOR_GREEN, extracted_count, COLOR_RESET);
    
    fclose(archive);
    return 1;
}

void print_usage(const char *program_name) {
    printf("用法: %s [选项] 归档文件 [文件...]\n", program_name);
    printf("优化版的 tar 命令，提供彩色输出和进度显示\n\n");
    printf("操作模式:\n");
    printf("  -c, --create        创建归档\n");
    printf("  -x, --extract       提取归档\n");
    printf("  -t, --list          列出归档内容\n");
    printf("  -r, --append        追加文件到归档\n");
    printf("  -u, --update        更新归档中的文件\n\n");
    printf("选项:\n");
    printf("  -v, --verbose       详细输出\n");
    printf("  -f, --file FILE     指定归档文件名\n");
    printf("  -p, --preserve      保留文件权限\n");
    printf("  -h, --help          显示此帮助信息\n");
    printf("  -V, --version       显示版本信息\n\n");
    printf("示例:\n");
    printf("  %s -cf archive.tar file1 file2\n", program_name);
    printf("  %s -tf archive.tar\n", program_name);
    printf("  %s -xf archive.tar\n", program_name);
    printf("  %s -rf archive.tar newfile\n", program_name);
}

int main(int argc, char *argv[]) {
    TarConfig config;
    init_tar_config(&config);
    
    // 解析命令行参数
    for (int i = 1; i < argc; i++) {
        if (argv[i][0] == '-' && argv[i][1] != '-') {
            // 处理组合选项，如 -cf, -tf 等
            char *options = &argv[i][1];
            for (int j = 0; options[j] != '\0'; j++) {
                switch (options[j]) {
                    case 'c':
                        config.operation = TAR_CREATE;
                        break;
                    case 'x':
                        config.operation = TAR_EXTRACT;
                        break;
                    case 't':
                        config.operation = TAR_LIST;
                        break;
                    case 'r':
                        config.operation = TAR_APPEND;
                        break;
                    case 'u':
                        config.operation = TAR_UPDATE;
                        break;
                    case 'v':
                        config.verbose = 1;
                        break;
                    case 'f':
                        if (i + 1 < argc) {
                            strncpy(config.archive_path, argv[++i], sizeof(config.archive_path) - 1);
                        } else {
                            print_error("缺少文件名参数");
                            return 1;
                        }
                        break;
                    case 'p':
                        config.preserve_permissions = 1;
                        break;
                    default:
                        printf("未知选项: -%c\n", options[j]);
                        print_usage(argv[0]);
                        return 1;
                }
            }
        } else if (strcmp(argv[i], "--create") == 0) {
            config.operation = TAR_CREATE;
        } else if (strcmp(argv[i], "--extract") == 0) {
            config.operation = TAR_EXTRACT;
        } else if (strcmp(argv[i], "--list") == 0) {
            config.operation = TAR_LIST;
        } else if (strcmp(argv[i], "--append") == 0) {
            config.operation = TAR_APPEND;
        } else if (strcmp(argv[i], "--update") == 0) {
            config.operation = TAR_UPDATE;
        } else if (strcmp(argv[i], "--verbose") == 0) {
            config.verbose = 1;
        } else if (strcmp(argv[i], "--file") == 0) {
            if (i + 1 < argc) {
                strncpy(config.archive_path, argv[++i], sizeof(config.archive_path) - 1);
            } else {
                print_error("缺少文件名参数");
                return 1;
            }
        } else if (strcmp(argv[i], "--preserve") == 0) {
            config.preserve_permissions = 1;
        } else if (strcmp(argv[i], "--help") == 0) {
            print_usage(argv[0]);
            return 0;
        } else if (strcmp(argv[i], "--version") == 0) {
            printf("ptar - 优化版 tar 命令 v1.0\n");
            return 0;
        } else if (argv[i][0] != '-') {
            if (strlen(config.archive_path) == 0) {
                strncpy(config.archive_path, argv[i], sizeof(config.archive_path) - 1);
            } else {
                config.files[config.file_count] = argv[i];
                config.file_count++;
            }
        } else {
            printf("未知选项: %s\n", argv[i]);
            print_usage(argv[0]);
            return 1;
        }
    }
    
    // 检查必需参数
    if (strlen(config.archive_path) == 0) {
        print_error("请指定归档文件名");
        print_usage(argv[0]);
        return 1;
    }
    
    if (config.operation == TAR_CREATE && config.file_count == 0) {
        print_error("创建归档时需要指定文件");
        print_usage(argv[0]);
        return 1;
    }
    
    // 执行操作
    int success = 0;
    switch (config.operation) {
        case TAR_CREATE:
            success = create_archive(&config);
            break;
        case TAR_EXTRACT:
            success = extract_archive(&config);
            break;
        case TAR_LIST:
            success = list_archive(&config);
            break;
        case TAR_APPEND:
            print_warning("追加功能暂未实现");
            break;
        case TAR_UPDATE:
            print_warning("更新功能暂未实现");
            break;
        default:
            print_error("未知操作");
            break;
    }
    
    return success ? 0 : 1;
}
