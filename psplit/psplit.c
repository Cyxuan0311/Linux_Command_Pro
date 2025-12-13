#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>
#include "../include/common.h"

#define MAX_FILENAME 256
#define DEFAULT_CHUNK_SIZE 1000
#define MAX_CHUNKS 10000

// 分割模式枚举
typedef enum {
    SPLIT_BY_LINES,     // 按行分割
    SPLIT_BY_SIZE,      // 按大小分割
    SPLIT_BY_CHUNKS     // 按块数分割
} SplitMode;

// 分割配置结构
typedef struct {
    char input_file[MAX_FILENAME];
    char output_prefix[MAX_FILENAME];
    SplitMode mode;
    int chunk_size;
    int num_chunks;
    int verbose;
} SplitConfig;

// 显示进度条
void show_progress(int current, int total, const char *operation) {
    if (total <= 0) return;
    
    int percent = (current * 100) / total;
    int bar_length = 50;
    int filled_length = (current * bar_length) / total;
    
    printf("\r%s%s [", COLOR_CYAN, operation);
    for (int i = 0; i < bar_length; i++) {
        if (i < filled_length) {
            printf("%s█%s", COLOR_GREEN, COLOR_RESET);
        } else {
            printf(" ");
        }
    }
    printf("] %d%% (%d/%d)", percent, current, total);
    fflush(stdout);
}

// 按行分割文件
int split_by_lines(const SplitConfig *config) {
    FILE *input = fopen(config->input_file, "r");
    if (!input) {
        print_error("无法打开输入文件");
        return 1;
    }
    
    char line[4096];
    int chunk_num = 0;
    int line_count = 0;
    FILE *output = NULL;
    char output_filename[MAX_FILENAME];
    
    // 计算总行数
    int total_lines = 0;
    while (fgets(line, sizeof(line), input)) {
        total_lines++;
    }
    rewind(input);
    
    printf("%s开始按行分割文件...%s\n", COLOR_CYAN, COLOR_RESET);
    printf("%s文件: %s%s\n", COLOR_YELLOW, config->input_file, COLOR_RESET);
    printf("%s总行数: %d%s\n", COLOR_YELLOW, total_lines, COLOR_RESET);
    printf("%s每块行数: %d%s\n", COLOR_YELLOW, config->chunk_size, COLOR_RESET);
    
    while (fgets(line, sizeof(line), input)) {
        if (line_count % config->chunk_size == 0) {
            if (output) {
                fclose(output);
            }
            
            snprintf(output_filename, sizeof(output_filename), 
                    "%s.%03d", config->output_prefix, chunk_num);
            output = fopen(output_filename, "w");
            if (!output) {
                print_error("无法创建输出文件");
                fclose(input);
                return 1;
            }
            chunk_num++;
        }
        
        fputs(line, output);
        line_count++;
        
        if (config->verbose) {
            show_progress(line_count, total_lines, "分割进度");
        }
    }
    
    if (output) {
        fclose(output);
    }
    fclose(input);
    
    if (config->verbose) {
        printf("\n");
    }
    
    printf("%s分割完成！%s\n", COLOR_GREEN, COLOR_RESET);
    printf("%s生成了 %d 个文件%s\n", COLOR_CYAN, chunk_num, COLOR_RESET);
    
    return 0;
}

// 按大小分割文件
int split_by_size(const SplitConfig *config) {
    FILE *input = fopen(config->input_file, "r");
    if (!input) {
        print_error("无法打开输入文件");
        return 1;
    }
    
    // 获取文件大小
    struct stat st;
    if (stat(config->input_file, &st) != 0) {
        print_error("无法获取文件信息");
        fclose(input);
        return 1;
    }
    
    off_t file_size = st.st_size;
    int chunk_num = 0;
    off_t bytes_read = 0;
    FILE *output = NULL;
    char output_filename[MAX_FILENAME];
    char buffer[8192];
    size_t bytes_to_read;
    
    printf("%s开始按大小分割文件...%s\n", COLOR_CYAN, COLOR_RESET);
    printf("%s文件: %s%s\n", COLOR_YELLOW, config->input_file, COLOR_RESET);
    printf("%s文件大小: %s%s\n", COLOR_YELLOW, format_size(file_size), COLOR_RESET);
    printf("%s每块大小: %s%s\n", COLOR_YELLOW, format_size(config->chunk_size), COLOR_RESET);
    
    while (bytes_read < file_size) {
        if (bytes_read % config->chunk_size == 0) {
            if (output) {
                fclose(output);
            }
            
            snprintf(output_filename, sizeof(output_filename), 
                    "%s.%03d", config->output_prefix, chunk_num);
            output = fopen(output_filename, "w");
            if (!output) {
                print_error("无法创建输出文件");
                fclose(input);
                return 1;
            }
            chunk_num++;
        }
        
        bytes_to_read = (file_size - bytes_read < config->chunk_size) ? 
                       (file_size - bytes_read) : config->chunk_size;
        
        size_t bytes = fread(buffer, 1, bytes_to_read, input);
        if (bytes == 0) break;
        
        fwrite(buffer, 1, bytes, output);
        bytes_read += bytes;
        
        if (config->verbose) {
            show_progress(bytes_read, file_size, "分割进度");
        }
    }
    
    if (output) {
        fclose(output);
    }
    fclose(input);
    
    if (config->verbose) {
        printf("\n");
    }
    
    printf("%s分割完成！%s\n", COLOR_GREEN, COLOR_RESET);
    printf("%s生成了 %d 个文件%s\n", COLOR_CYAN, chunk_num, COLOR_RESET);
    
    return 0;
}

// 按块数分割文件
int split_by_chunks(const SplitConfig *config) {
    FILE *input = fopen(config->input_file, "r");
    if (!input) {
        print_error("无法打开输入文件");
        return 1;
    }
    
    // 获取文件大小
    struct stat st;
    if (stat(config->input_file, &st) != 0) {
        print_error("无法获取文件信息");
        fclose(input);
        return 1;
    }
    
    off_t file_size = st.st_size;
    off_t chunk_size = file_size / config->num_chunks;
    int chunk_num = 0;
    off_t bytes_read = 0;
    FILE *output = NULL;
    char output_filename[MAX_FILENAME];
    char buffer[8192];
    size_t bytes_to_read;
    
    printf("%s开始按块数分割文件...%s\n", COLOR_CYAN, COLOR_RESET);
    printf("%s文件: %s%s\n", COLOR_YELLOW, config->input_file, COLOR_RESET);
    printf("%s文件大小: %s%s\n", COLOR_YELLOW, format_size(file_size), COLOR_RESET);
    printf("%s分割成: %d 块%s\n", COLOR_YELLOW, config->num_chunks, COLOR_RESET);
    printf("%s每块大小: %s%s\n", COLOR_YELLOW, format_size(chunk_size), COLOR_RESET);
    
    for (int i = 0; i < config->num_chunks; i++) {
        snprintf(output_filename, sizeof(output_filename), 
                "%s.%03d", config->output_prefix, i);
        output = fopen(output_filename, "w");
        if (!output) {
            print_error("无法创建输出文件");
            fclose(input);
            return 1;
        }
        
        off_t remaining = (i == config->num_chunks - 1) ? 
                         (file_size - bytes_read) : chunk_size;
        
        while (remaining > 0) {
            bytes_to_read = (remaining > (off_t)sizeof(buffer)) ? 
                           sizeof(buffer) : (size_t)remaining;
            
            size_t bytes = fread(buffer, 1, bytes_to_read, input);
            if (bytes == 0) break;
            
            fwrite(buffer, 1, bytes, output);
            bytes_read += bytes;
            remaining -= bytes;
            
            if (config->verbose) {
                show_progress(bytes_read, file_size, "分割进度");
            }
        }
        
        fclose(output);
    }
    
    fclose(input);
    
    if (config->verbose) {
        printf("\n");
    }
    
    printf("%s分割完成！%s\n", COLOR_GREEN, COLOR_RESET);
    printf("%s生成了 %d 个文件%s\n", COLOR_CYAN, config->num_chunks, COLOR_RESET);
    
    return 0;
}

// 解析大小字符串（如 1K, 2M, 3G）
int parse_size(const char *size_str) {
    if (!size_str) return 0;
    
    int len = strlen(size_str);
    if (len == 0) return 0;
    
    char unit = size_str[len - 1];
    int multiplier = 1;
    
    if (unit == 'K' || unit == 'k') {
        multiplier = 1024;
    } else if (unit == 'M' || unit == 'm') {
        multiplier = 1024 * 1024;
    } else if (unit == 'G' || unit == 'g') {
        multiplier = 1024 * 1024 * 1024;
    } else if (unit >= '0' && unit <= '9') {
        return atoi(size_str);
    } else {
        return 0;
    }
    
    char num_str[32];
    strncpy(num_str, size_str, len - 1);
    num_str[len - 1] = '\0';
    
    return atoi(num_str) * multiplier;
}

void print_usage(const char *program_name) {
    printf("用法: %s [选项] [文件] [前缀]\n", program_name);
    printf("优化版的 split 命令，提供更好的视觉效果和多种分割模式\n\n");
    printf("选项:\n");
    printf("  -l, --lines=N       按行分割，每块N行\n");
    printf("  -b, --bytes=SIZE    按大小分割，每块SIZE字节\n");
    printf("  -n, --number=N      按块数分割，分成N块\n");
    printf("  -v, --verbose       显示详细进度\n");
    printf("  -h, --help          显示此帮助信息\n");
    printf("  -V, --version       显示版本信息\n");
    printf("\n大小单位:\n");
    printf("  K, k                千字节 (1024)\n");
    printf("  M, m                兆字节 (1024*1024)\n");
    printf("  G, g                吉字节 (1024*1024*1024)\n");
    printf("\n示例:\n");
    printf("  %s -l 1000 file.txt part    # 按1000行分割\n", program_name);
    printf("  %s -b 1M file.txt part      # 按1MB分割\n", program_name);
    printf("  %s -n 5 file.txt part       # 分成5块\n", program_name);
    printf("  %s -l 500 -v big.txt parts  # 按500行分割，显示进度\n", program_name);
}

int main(int argc, char *argv[]) {
    SplitConfig config = {0};
    config.mode = SPLIT_BY_LINES;
    config.chunk_size = DEFAULT_CHUNK_SIZE;
    config.num_chunks = 0;
    config.verbose = 0;
    
    // 解析命令行参数
    for (int i = 1; i < argc; i++) {
        if (strncmp(argv[i], "-l", 2) == 0 || strncmp(argv[i], "--lines=", 8) == 0) {
            config.mode = SPLIT_BY_LINES;
            const char *value = strchr(argv[i], '=');
            if (value) {
                config.chunk_size = atoi(value + 1);
            } else if (i + 1 < argc) {
                config.chunk_size = atoi(argv[++i]);
            }
        } else if (strncmp(argv[i], "-b", 2) == 0 || strncmp(argv[i], "--bytes=", 8) == 0) {
            config.mode = SPLIT_BY_SIZE;
            const char *value = strchr(argv[i], '=');
            if (value) {
                config.chunk_size = parse_size(value + 1);
            } else if (i + 1 < argc) {
                config.chunk_size = parse_size(argv[++i]);
            }
        } else if (strncmp(argv[i], "-n", 2) == 0 || strncmp(argv[i], "--number=", 9) == 0) {
            config.mode = SPLIT_BY_CHUNKS;
            const char *value = strchr(argv[i], '=');
            if (value) {
                config.num_chunks = atoi(value + 1);
            } else if (i + 1 < argc) {
                config.num_chunks = atoi(argv[++i]);
            }
        } else if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--verbose") == 0) {
            config.verbose = 1;
        } else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            print_usage(argv[0]);
            return 0;
        } else if (strcmp(argv[i], "-V") == 0 || strcmp(argv[i], "--version") == 0) {
            printf("psplit - 优化版 split 命令 v1.0\n");
            return 0;
        } else if (argv[i][0] != '-') {
            if (strlen(config.input_file) == 0) {
                strncpy(config.input_file, argv[i], MAX_FILENAME - 1);
            } else if (strlen(config.output_prefix) == 0) {
                strncpy(config.output_prefix, argv[i], MAX_FILENAME - 1);
            }
        } else {
            printf("未知选项: %s\n", argv[i]);
            print_usage(argv[0]);
            return 1;
        }
    }
    
    // 检查必需参数
    if (strlen(config.input_file) == 0) {
        print_error("请指定输入文件");
        print_usage(argv[0]);
        return 1;
    }
    
    if (strlen(config.output_prefix) == 0) {
        strcpy(config.output_prefix, "x");
    }
    
    // 检查文件是否存在
    if (access(config.input_file, R_OK) != 0) {
        print_error("无法读取输入文件");
        return 1;
    }
    
    // 根据模式执行分割
    switch (config.mode) {
        case SPLIT_BY_LINES:
            if (config.chunk_size <= 0) {
                print_error("行数必须大于0");
                return 1;
            }
            return split_by_lines(&config);
            
        case SPLIT_BY_SIZE:
            if (config.chunk_size <= 0) {
                print_error("大小必须大于0");
                return 1;
            }
            return split_by_size(&config);
            
        case SPLIT_BY_CHUNKS:
            if (config.num_chunks <= 0) {
                print_error("块数必须大于0");
                return 1;
            }
            return split_by_chunks(&config);
            
        default:
            print_error("未知分割模式");
            return 1;
    }
}
