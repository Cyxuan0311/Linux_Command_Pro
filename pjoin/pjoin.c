#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>
#include <glob.h>
#include "../include/common.h"

// 函数声明
char* str_repeat(const char *str, int count);

#define MAX_FILENAME 256
#define MAX_FILES 1000
#define BUFFER_SIZE 8192

// 合并模式枚举
typedef enum {
    JOIN_SIMPLE,        // 简单合并
    JOIN_WITH_HEADER,   // 带文件头合并
    JOIN_WITH_SEPARATOR // 带分隔符合并
} JoinMode;

// 合并配置结构
typedef struct {
    char output_file[MAX_FILENAME];
    char pattern[MAX_FILENAME];
    JoinMode mode;
    char separator[256];
    int verbose;
    int skip_empty;
    int add_line_numbers;
} JoinConfig;

// 文件信息结构
typedef struct {
    char filename[MAX_FILENAME];
    off_t size;
    int line_count;
} FileInfo;

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

// 计算文件行数
int count_lines(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) return 0;
    
    int count = 0;
    char buffer[BUFFER_SIZE];
    
    while (fgets(buffer, sizeof(buffer), file)) {
        count++;
    }
    
    fclose(file);
    return count;
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
    info->line_count = count_lines(filename);
    
    return 1;
}

// 简单合并模式
int join_simple(const char *files[], int file_count, const JoinConfig *config) {
    FILE *output = fopen(config->output_file, "w");
    if (!output) {
        print_error("无法创建输出文件");
        return 1;
    }
    
    printf("%s开始合并文件...%s\n", COLOR_CYAN, COLOR_RESET);
    printf("%s输出文件: %s%s\n", COLOR_YELLOW, config->output_file, COLOR_RESET);
    printf("%s文件数量: %d%s\n", COLOR_YELLOW, file_count, COLOR_RESET);
    
    for (int i = 0; i < file_count; i++) {
        FILE *input = fopen(files[i], "r");
        if (!input) {
            print_warning("无法打开文件，跳过");
            continue;
        }
        
        char buffer[BUFFER_SIZE];
        size_t bytes;
        
        while ((bytes = fread(buffer, 1, sizeof(buffer), input)) > 0) {
            fwrite(buffer, 1, bytes, output);
        }
        
        fclose(input);
        
        if (config->verbose) {
            show_progress(i + 1, file_count, "合并进度");
        }
    }
    
    fclose(output);
    
    if (config->verbose) {
        printf("\n");
    }
    
    printf("%s合并完成！%s\n", COLOR_GREEN, COLOR_RESET);
    return 0;
}

// 带文件头合并模式
int join_with_header(const char *files[], int file_count, const JoinConfig *config) {
    FILE *output = fopen(config->output_file, "w");
    if (!output) {
        print_error("无法创建输出文件");
        return 1;
    }
    
    printf("%s开始合并文件（带文件头）...%s\n", COLOR_CYAN, COLOR_RESET);
    printf("%s输出文件: %s%s\n", COLOR_YELLOW, config->output_file, COLOR_RESET);
    printf("%s文件数量: %d%s\n", COLOR_YELLOW, file_count, COLOR_RESET);
    
    for (int i = 0; i < file_count; i++) {
        FILE *input = fopen(files[i], "r");
        if (!input) {
            print_warning("无法打开文件，跳过");
            continue;
        }
        
        // 写入文件头
        fprintf(output, "%s=== %s ===%s\n", 
                COLOR_CYAN, files[i], COLOR_RESET);
        
        char line[4096];
        int line_num = 1;
        
        while (fgets(line, sizeof(line), input)) {
            if (config->add_line_numbers) {
                fprintf(output, "%s%4d%s | %s", 
                        COLOR_YELLOW, line_num, COLOR_RESET, line);
            } else {
                fputs(line, output);
            }
            line_num++;
        }
        
        fclose(input);
        
        // 添加分隔符
        if (i < file_count - 1) {
            fprintf(output, "\n%s%s%s\n\n", 
                    COLOR_YELLOW, str_repeat("=", 50), COLOR_RESET);
        }
        
        if (config->verbose) {
            show_progress(i + 1, file_count, "合并进度");
        }
    }
    
    fclose(output);
    
    if (config->verbose) {
        printf("\n");
    }
    
    printf("%s合并完成！%s\n", COLOR_GREEN, COLOR_RESET);
    return 0;
}

// 带分隔符合并模式
int join_with_separator(const char *files[], int file_count, const JoinConfig *config) {
    FILE *output = fopen(config->output_file, "w");
    if (!output) {
        print_error("无法创建输出文件");
        return 1;
    }
    
    printf("%s开始合并文件（带分隔符）...%s\n", COLOR_CYAN, COLOR_RESET);
    printf("%s输出文件: %s%s\n", COLOR_YELLOW, config->output_file, COLOR_RESET);
    printf("%s文件数量: %d%s\n", COLOR_YELLOW, file_count, COLOR_RESET);
    printf("%s分隔符: %s%s\n", COLOR_YELLOW, config->separator, COLOR_RESET);
    
    for (int i = 0; i < file_count; i++) {
        FILE *input = fopen(files[i], "r");
        if (!input) {
            print_warning("无法打开文件，跳过");
            continue;
        }
        
        char line[4096];
        int has_content = 0;
        
        while (fgets(line, sizeof(line), input)) {
            if (config->skip_empty && strlen(line) <= 1) {
                continue;
            }
            
            fputs(line, output);
            has_content = 1;
        }
        
        fclose(input);
        
        // 添加分隔符（除了最后一个文件）
        if (i < file_count - 1 && has_content) {
            fputs(config->separator, output);
        }
        
        if (config->verbose) {
            show_progress(i + 1, file_count, "合并进度");
        }
    }
    
    fclose(output);
    
    if (config->verbose) {
        printf("\n");
    }
    
    printf("%s合并完成！%s\n", COLOR_GREEN, COLOR_RESET);
    return 0;
}

// 字符串重复函数
char* str_repeat(const char *str, int count) {
    static char result[1024];
    result[0] = '\0';
    
    for (int i = 0; i < count && strlen(result) + strlen(str) < sizeof(result) - 1; i++) {
        strcat(result, str);
    }
    
    return result;
}

// 展开通配符
int expand_pattern(const char *pattern, char *files[], int max_files) {
    glob_t glob_result;
    int count = 0;
    
    if (glob(pattern, GLOB_NOSORT, NULL, &glob_result) == 0) {
        for (size_t i = 0; i < glob_result.gl_pathc && count < max_files; i++) {
            strncpy(files[count], glob_result.gl_pathv[i], MAX_FILENAME - 1);
            files[count][MAX_FILENAME - 1] = '\0';
            count++;
        }
    }
    
    globfree(&glob_result);
    return count;
}

// 排序文件（按名称）
int compare_files(const void *a, const void *b) {
    return strcmp(*(const char**)a, *(const char**)b);
}

void print_usage(const char *program_name) {
    printf("用法: %s [选项] -o 输出文件 [文件...]\n", program_name);
    printf("优化版的 join 命令，提供多种合并模式和视觉效果\n\n");
    printf("选项:\n");
    printf("  -o, --output=FILE   指定输出文件\n");
    printf("  -p, --pattern=PAT   使用通配符模式匹配文件\n");
    printf("  -m, --mode=MODE     合并模式 (simple|header|separator)\n");
    printf("  -s, --separator=STR 分隔符（separator模式）\n");
    printf("  -n, --line-numbers 添加行号（header模式）\n");
    printf("  -e, --skip-empty   跳过空行\n");
    printf("  -v, --verbose      显示详细进度\n");
    printf("  -h, --help         显示此帮助信息\n");
    printf("  -V, --version      显示版本信息\n");
    printf("\n合并模式:\n");
    printf("  simple              简单合并（默认）\n");
    printf("  header              带文件头合并\n");
    printf("  separator           带分隔符合并\n");
    printf("\n示例:\n");
    printf("  %s -o output.txt file1.txt file2.txt\n", program_name);
    printf("  %s -p \"*.log\" -o combined.log -m header\n", program_name);
    printf("  %s -p \"part.*\" -o full.txt -m separator -s \"\\n---\\n\"\n", program_name);
    printf("  %s -p \"*.txt\" -o all.txt -m header -n -v\n", program_name);
}

int main(int argc, char *argv[]) {
    JoinConfig config = {0};
    config.mode = JOIN_SIMPLE;
    strcpy(config.separator, "\n");
    config.verbose = 0;
    config.skip_empty = 0;
    config.add_line_numbers = 0;
    
    char *files[MAX_FILES];
    int file_count = 0;
    
    // 解析命令行参数
    for (int i = 1; i < argc; i++) {
        if (strncmp(argv[i], "-o", 2) == 0 || strncmp(argv[i], "--output=", 9) == 0) {
            const char *value = strchr(argv[i], '=');
            if (value) {
                strncpy(config.output_file, value + 1, MAX_FILENAME - 1);
            } else if (i + 1 < argc) {
                strncpy(config.output_file, argv[++i], MAX_FILENAME - 1);
            }
        } else if (strncmp(argv[i], "-p", 2) == 0 || strncmp(argv[i], "--pattern=", 10) == 0) {
            const char *value = strchr(argv[i], '=');
            if (value) {
                strncpy(config.pattern, value + 1, MAX_FILENAME - 1);
            } else if (i + 1 < argc) {
                strncpy(config.pattern, argv[++i], MAX_FILENAME - 1);
            }
        } else if (strncmp(argv[i], "-m", 2) == 0 || strncmp(argv[i], "--mode=", 7) == 0) {
            const char *value = strchr(argv[i], '=');
            const char *mode = value ? value + 1 : (i + 1 < argc ? argv[++i] : NULL);
            
            if (strcmp(mode, "simple") == 0) {
                config.mode = JOIN_SIMPLE;
            } else if (strcmp(mode, "header") == 0) {
                config.mode = JOIN_WITH_HEADER;
            } else if (strcmp(mode, "separator") == 0) {
                config.mode = JOIN_WITH_SEPARATOR;
            } else {
                print_error("无效的合并模式");
                return 1;
            }
        } else if (strncmp(argv[i], "-s", 2) == 0 || strncmp(argv[i], "--separator=", 12) == 0) {
            const char *value = strchr(argv[i], '=');
            if (value) {
                strncpy(config.separator, value + 1, sizeof(config.separator) - 1);
            } else if (i + 1 < argc) {
                strncpy(config.separator, argv[++i], sizeof(config.separator) - 1);
            }
        } else if (strcmp(argv[i], "-n") == 0 || strcmp(argv[i], "--line-numbers") == 0) {
            config.add_line_numbers = 1;
        } else if (strcmp(argv[i], "-e") == 0 || strcmp(argv[i], "--skip-empty") == 0) {
            config.skip_empty = 1;
        } else if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--verbose") == 0) {
            config.verbose = 1;
        } else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            print_usage(argv[0]);
            return 0;
        } else if (strcmp(argv[i], "-V") == 0 || strcmp(argv[i], "--version") == 0) {
            printf("pjoin - 优化版 join 命令 v1.0\n");
            return 0;
        } else if (argv[i][0] != '-') {
            if (file_count < MAX_FILES) {
                files[file_count] = argv[i];
                file_count++;
            }
        } else {
            printf("未知选项: %s\n", argv[i]);
            print_usage(argv[0]);
            return 1;
        }
    }
    
    // 检查必需参数
    if (strlen(config.output_file) == 0) {
        print_error("请指定输出文件");
        print_usage(argv[0]);
        return 1;
    }
    
    // 处理文件列表
    if (strlen(config.pattern) > 0) {
        // 使用通配符模式
        file_count = expand_pattern(config.pattern, files, MAX_FILES);
        if (file_count == 0) {
            print_error("没有找到匹配的文件");
            return 1;
        }
    } else if (file_count == 0) {
        print_error("请指定要合并的文件");
        print_usage(argv[0]);
        return 1;
    }
    
    // 排序文件
    qsort(files, file_count, sizeof(char*), compare_files);
    
    // 显示文件信息
    if (config.verbose) {
        printf("%s文件列表:%s\n", COLOR_CYAN, COLOR_RESET);
        for (int i = 0; i < file_count; i++) {
            FileInfo info;
            if (get_file_info(files[i], &info)) {
                printf("  %s %s %s (%s, %d 行)%s\n", 
                       ICON_FILE, files[i], COLOR_WHITE,
                       format_size(info.size), info.line_count, COLOR_RESET);
            }
        }
        printf("\n");
    }
    
    // 根据模式执行合并
    switch (config.mode) {
        case JOIN_SIMPLE:
            return join_simple(files, file_count, &config);
            
        case JOIN_WITH_HEADER:
            return join_with_header(files, file_count, &config);
            
        case JOIN_WITH_SEPARATOR:
            return join_with_separator(files, file_count, &config);
            
        default:
            print_error("未知合并模式");
            return 1;
    }
}
