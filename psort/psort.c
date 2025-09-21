#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <errno.h>
#include <ctype.h>
#include <locale.h>

// 颜色定义
#define COLOR_RED     "\033[31m"
#define COLOR_GREEN   "\033[32m"
#define COLOR_YELLOW  "\033[33m"
#define COLOR_BLUE    "\033[34m"
#define COLOR_MAGENTA "\033[35m"
#define COLOR_CYAN    "\033[36m"
#define COLOR_WHITE   "\033[37m"
#define COLOR_RESET   "\033[0m"
#define COLOR_BOLD    "\033[1m"

#define MAX_LINE_LENGTH 4096
#define INITIAL_CAPACITY 1000

// 全局选项
struct options {
    int reverse;
    int numeric;
    int human_numeric;
    int ignore_case;
    int unique;
    int stable;
    int color;
    int show_stats;
    int show_progress;
    int field_start;
    int field_end;
    char *field_separator;
    int verbose;
    char *separator;
};

// 行结构
struct line {
    char *content;
    char *key;  // 用于排序的键
    int original_index;
};

// 初始化选项
void init_options(struct options *opts) {
    opts->reverse = 0;
    opts->numeric = 0;
    opts->human_numeric = 0;
    opts->ignore_case = 0;
    opts->unique = 0;
    opts->stable = 0;
    opts->color = 1;
    opts->show_stats = 0;
    opts->show_progress = 0;
    opts->field_start = 0;
    opts->field_end = 0;
    opts->field_separator = " \t";
    opts->verbose = 0;
    opts->separator = "==>";
}

// 打印帮助信息
void print_help() {
    printf("%spsort - 优化版 sort 命令%s\n", COLOR_CYAN, COLOR_RESET);
    printf("%s==============================================%s\n", COLOR_YELLOW, COLOR_RESET);
    printf("用法: psort [选项] [文件...]\n\n");
    printf("选项:\n");
    printf("  -r, --reverse           反向排序\n");
    printf("  -n, --numeric           按数值排序\n");
    printf("  -h, --human-numeric     按人类可读数值排序 (K, M, G)\n");
    printf("  -f, --ignore-case       忽略大小写\n");
    printf("  -u, --unique            去除重复行\n");
    printf("  -s, --stable            稳定排序\n");
    printf("  -k, --key=POS1[,POS2]   指定排序键位置\n");
    printf("  -t, --field-separator=C 指定字段分隔符\n");
    printf("  --color                 启用彩色输出 (默认)\n");
    printf("  --no-color              禁用彩色输出\n");
    printf("  --stats                 显示排序统计信息\n");
    printf("  --progress              显示进度条\n");
    printf("  --verbose               详细输出\n");
    printf("  --separator=STR         设置文件分隔符 (默认: '==>')\n");
    printf("  -h, --help              显示此帮助信息\n");
    printf("  -V, --version           显示版本信息\n\n");
    printf("示例:\n");
    printf("  psort file.txt                    # 基本排序\n");
    printf("  psort -n file.txt                 # 数值排序\n");
    printf("  psort -r file.txt                 # 反向排序\n");
    printf("  psort -u file.txt                 # 去重排序\n");
    printf("  psort -k2,3 file.txt              # 按第2-3字段排序\n");
    printf("  psort --stats file.txt            # 显示统计信息\n");
}

// 打印版本信息
void print_version() {
    printf("psort - 优化版 sort 命令 v1.0\n");
    printf("使用C语言实现，添加彩色排序和统计信息\n");
    printf("作者: psort team\n");
}

// 解析键位置
void parse_key(const char *key_str, int *start, int *end) {
    char *comma = strchr(key_str, ',');
    if (comma) {
        *start = atoi(key_str);
        *end = atoi(comma + 1);
    } else {
        *start = atoi(key_str);
        *end = *start;
    }
}

// 提取排序键
char* extract_key(const char *line, const struct options *opts) {
    if (opts->field_start == 0) {
        // 使用整行作为键
        char *key = malloc(strlen(line) + 1);
        strcpy(key, line);
        return key;
    }
    
    // 按字段分隔符分割
    char *line_copy = strdup(line);
    char *token;
    int field_num = 1;
    char *result = NULL;
    
    token = strtok(line_copy, opts->field_separator);
    while (token != NULL) {
        if (field_num >= opts->field_start && field_num <= opts->field_end) {
            if (result == NULL) {
                result = strdup(token);
            } else {
                char *temp = malloc(strlen(result) + strlen(token) + 2);
                sprintf(temp, "%s %s", result, token);
                free(result);
                result = temp;
            }
        }
        token = strtok(NULL, opts->field_separator);
        field_num++;
    }
    
    free(line_copy);
    return result ? result : strdup("");
}

// 比较函数
int compare_lines(const void *a, const void *b) {
    const struct line *line_a = (const struct line *)a;
    const struct line *line_b = (const struct line *)b;
    
    int result = 0;
    
    if (line_a->key && line_b->key) {
        if (strcmp(line_a->key, line_b->key) == 0) {
            // 如果键相同且需要稳定排序，按原始索引排序
            result = line_a->original_index - line_b->original_index;
        } else {
            result = strcmp(line_a->key, line_b->key);
        }
    }
    
    return result;
}

// 数值比较
int compare_numeric(const void *a, const void *b) {
    const struct line *line_a = (const struct line *)a;
    const struct line *line_b = (const struct line *)b;
    
    double num_a = atof(line_a->key ? line_a->key : line_a->content);
    double num_b = atof(line_b->key ? line_b->key : line_b->content);
    
    if (num_a < num_b) return -1;
    if (num_a > num_b) return 1;
    
    // 数值相同时按原始索引排序（稳定排序）
    return line_a->original_index - line_b->original_index;
}

// 人类可读数值比较
int compare_human_numeric(const void *a, const void *b) {
    const struct line *line_a = (const struct line *)a;
    const struct line *line_b = (const struct line *)b;
    
    const char *str_a = line_a->key ? line_a->key : line_a->content;
    const char *str_b = line_b->key ? line_b->key : line_b->content;
    
    // 简单的K, M, G后缀处理
    double num_a = atof(str_a);
    double num_b = atof(str_b);
    
    // 检查后缀
    if (strstr(str_a, "K") || strstr(str_a, "k")) num_a *= 1024;
    else if (strstr(str_a, "M") || strstr(str_a, "m")) num_a *= 1024 * 1024;
    else if (strstr(str_a, "G") || strstr(str_a, "g")) num_a *= 1024 * 1024 * 1024;
    
    if (strstr(str_b, "K") || strstr(str_b, "k")) num_b *= 1024;
    else if (strstr(str_b, "M") || strstr(str_b, "m")) num_b *= 1024 * 1024;
    else if (strstr(str_b, "G") || strstr(str_b, "g")) num_b *= 1024 * 1024 * 1024;
    
    if (num_a < num_b) return -1;
    if (num_a > num_b) return 1;
    
    return line_a->original_index - line_b->original_index;
}

// 显示进度条
void show_progress(int current, int total) {
    if (total <= 0) return;
    
    int bar_width = 50;
    int pos = (int)((double)current / total * bar_width);
    
    printf("\r%s[", COLOR_CYAN);
    for (int i = 0; i < bar_width; i++) {
        if (i < pos) {
            printf("%s█", COLOR_GREEN);
        } else {
            printf(" ");
        }
    }
    printf("%s] %d%% 排序中...", COLOR_CYAN, (int)((double)current / total * 100));
    fflush(stdout);
}

// 高亮行内容
void highlight_line(const char *line, int is_duplicate) {
    if (is_duplicate) {
        printf("%s%s%s", COLOR_RED, line, COLOR_RESET);
    } else {
        // 简单的语法高亮
        const char *p = line;
        while (*p) {
            if (*p >= '0' && *p <= '9') {
                // 数字高亮
                printf("%s", COLOR_BLUE);
                while (*p && *p >= '0' && *p <= '9') {
                    putchar(*p++);
                }
                printf("%s", COLOR_RESET);
                p--; // 回退一个字符
            } else {
                putchar(*p);
            }
            p++;
        }
    }
}

// 处理文件
int process_file(const char *filename, const struct options *opts) {
    FILE *file;
    char line[MAX_LINE_LENGTH];
    struct line *lines = NULL;
    int capacity = INITIAL_CAPACITY;
    int count = 0;
    int line_num = 0;
    
    if (strcmp(filename, "-") == 0) {
        file = stdin;
        filename = "标准输入";
    } else {
        file = fopen(filename, "r");
        if (!file) {
            fprintf(stderr, "%s错误: 无法打开文件 '%s': %s%s\n", 
                    COLOR_RED, filename, strerror(errno), COLOR_RESET);
            return 1;
        }
    }
    
    // 显示文件名（如果需要）
    if (opts->verbose) {
        printf("%s%s %s%s\n", COLOR_MAGENTA, opts->separator, filename, COLOR_RESET);
    }
    
    // 分配初始内存
    lines = malloc(capacity * sizeof(struct line));
    if (!lines) {
        fprintf(stderr, "%s错误: 内存分配失败%s\n", COLOR_RED, COLOR_RESET);
        if (file != stdin) fclose(file);
        return 1;
    }
    
    // 读取所有行
    while (fgets(line, sizeof(line), file)) {
        if (count >= capacity) {
            capacity *= 2;
            lines = realloc(lines, capacity * sizeof(struct line));
            if (!lines) {
                fprintf(stderr, "%s错误: 内存重新分配失败%s\n", COLOR_RED, COLOR_RESET);
                if (file != stdin) fclose(file);
                return 1;
            }
        }
        
        lines[count].content = strdup(line);
        lines[count].key = extract_key(line, opts);
        lines[count].original_index = count;
        count++;
        
        if (opts->show_progress && count % 100 == 0) {
            show_progress(count, count + 100); // 估算进度
        }
    }
    
    if (file != stdin) {
        fclose(file);
    }
    
    if (opts->show_progress) {
        printf("\n");
    }
    
    // 选择比较函数
    int (*compare_func)(const void *, const void *) = compare_lines;
    if (opts->numeric) {
        compare_func = compare_numeric;
    } else if (opts->human_numeric) {
        compare_func = compare_human_numeric;
    }
    
    // 排序
    qsort(lines, count, sizeof(struct line), compare_func);
    
    // 去重（如果需要）
    if (opts->unique) {
        int write_pos = 0;
        for (int i = 1; i < count; i++) {
            if (strcmp(lines[i].content, lines[write_pos].content) != 0) {
                write_pos++;
                if (write_pos != i) {
                    lines[write_pos] = lines[i];
                }
            } else {
                // 标记为重复
                lines[i].original_index = -1;
            }
        }
        count = write_pos + 1;
    }
    
    // 输出结果
    for (int i = 0; i < count; i++) {
        if (opts->reverse) {
            int idx = count - 1 - i;
            if (opts->color) {
                highlight_line(lines[idx].content, lines[idx].original_index == -1);
            } else {
                printf("%s", lines[idx].content);
            }
        } else {
            if (opts->color) {
                highlight_line(lines[i].content, lines[i].original_index == -1);
            } else {
                printf("%s", lines[i].content);
            }
        }
    }
    
    // 显示统计信息
    if (opts->show_stats) {
        printf("%s统计信息:%s\n", COLOR_CYAN, COLOR_RESET);
        printf("  总行数: %d\n", count);
        if (opts->unique) {
            int duplicates = 0;
            for (int i = 0; i < count; i++) {
                if (lines[i].original_index == -1) duplicates++;
            }
            printf("  重复行数: %d\n", duplicates);
        }
    }
    
    // 清理内存
    for (int i = 0; i < count; i++) {
        free(lines[i].content);
        free(lines[i].key);
    }
    free(lines);
    
    return 0;
}

int main(int argc, char *argv[]) {
    struct options opts;
    init_options(&opts);
    
    static struct option long_options[] = {
        {"reverse", no_argument, 0, 'r'},
        {"numeric", no_argument, 0, 'n'},
        {"human-numeric", no_argument, 0, 'h'},
        {"ignore-case", no_argument, 0, 'f'},
        {"unique", no_argument, 0, 'u'},
        {"stable", no_argument, 0, 's'},
        {"key", required_argument, 0, 'k'},
        {"field-separator", required_argument, 0, 't'},
        {"color", no_argument, 0, 1},
        {"no-color", no_argument, 0, 2},
        {"stats", no_argument, 0, 3},
        {"progress", no_argument, 0, 4},
        {"verbose", no_argument, 0, 5},
        {"separator", required_argument, 0, 6},
        {"help", no_argument, 0, 'H'},
        {"version", no_argument, 0, 'V'},
        {0, 0, 0, 0}
    };
    
    int opt;
    int option_index = 0;
    
    while ((opt = getopt_long(argc, argv, "rnhfusk:t:HV", long_options, &option_index)) != -1) {
        switch (opt) {
            case 'r':
                opts.reverse = 1;
                break;
            case 'n':
                opts.numeric = 1;
                break;
            case 'h':
                opts.human_numeric = 1;
                break;
            case 'f':
                opts.ignore_case = 1;
                break;
            case 'u':
                opts.unique = 1;
                break;
            case 's':
                opts.stable = 1;
                break;
            case 'k':
                parse_key(optarg, &opts.field_start, &opts.field_end);
                break;
            case 't':
                opts.field_separator = optarg;
                break;
            case 1: // --color
                opts.color = 1;
                break;
            case 2: // --no-color
                opts.color = 0;
                break;
            case 3: // --stats
                opts.show_stats = 1;
                break;
            case 4: // --progress
                opts.show_progress = 1;
                break;
            case 5: // --verbose
                opts.verbose = 1;
                break;
            case 6: // --separator
                opts.separator = optarg;
                break;
            case 'H':
                print_help();
                return 0;
            case 'V':
                print_version();
                return 0;
            case '?':
                print_help();
                return 1;
            default:
                break;
        }
    }
    
    // 处理剩余参数（文件名）
    if (optind >= argc) {
        // 没有文件名，从标准输入读取
        return process_file("-", &opts);
    } else {
        int result = 0;
        
        for (int i = optind; i < argc; i++) {
            if (process_file(argv[i], &opts) != 0) {
                result = 1;
            }
        }
        
        return result;
    }
}
