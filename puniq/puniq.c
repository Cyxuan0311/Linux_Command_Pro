#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <errno.h>
#include <ctype.h>

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

// 全局选项
struct options {
    int count;
    int repeated;
    int unique;
    int all_repeated;
    int ignore_case;
    int skip_fields;
    int skip_chars;
    int color;
    int show_stats;
    int show_progress;
    int verbose;
    char *separator;
};

// 初始化选项
void init_options(struct options *opts) {
    opts->count = 0;
    opts->repeated = 0;
    opts->unique = 0;
    opts->all_repeated = 0;
    opts->ignore_case = 0;
    opts->skip_fields = 0;
    opts->skip_chars = 0;
    opts->color = 1;
    opts->show_stats = 0;
    opts->show_progress = 0;
    opts->verbose = 0;
    opts->separator = "==>";
}

// 打印帮助信息
void print_help() {
    printf("%spuniq - 优化版 uniq 命令%s\n", COLOR_CYAN, COLOR_RESET);
    printf("%s==============================================%s\n", COLOR_YELLOW, COLOR_RESET);
    printf("用法: puniq [选项] [文件...]\n\n");
    printf("选项:\n");
    printf("  -c, --count             在每行前显示重复次数\n");
    printf("  -d, --repeated          只显示重复行\n");
    printf("  -u, --unique            只显示唯一行\n");
    printf("  -D, --all-repeated      显示所有重复行\n");
    printf("  -i, --ignore-case       忽略大小写\n");
    printf("  -f, --skip-fields=N     跳过前N个字段\n");
    printf("  -s, --skip-chars=N      跳过前N个字符\n");
    printf("  --color                 启用彩色输出 (默认)\n");
    printf("  --no-color              禁用彩色输出\n");
    printf("  --stats                 显示统计信息\n");
    printf("  --progress              显示进度条\n");
    printf("  --verbose               详细输出\n");
    printf("  --separator=STR         设置文件分隔符 (默认: '==>')\n");
    printf("  -h, --help              显示此帮助信息\n");
    printf("  -V, --version           显示版本信息\n\n");
    printf("示例:\n");
    printf("  puniq file.txt                    # 基本去重\n");
    printf("  puniq -c file.txt                 # 显示重复次数\n");
    printf("  puniq -d file.txt                 # 只显示重复行\n");
    printf("  puniq -u file.txt                 # 只显示唯一行\n");
    printf("  puniq -i file.txt                 # 忽略大小写\n");
    printf("  puniq --stats file.txt            # 显示统计信息\n");
}

// 打印版本信息
void print_version() {
    printf("puniq - 优化版 uniq 命令 v1.0\n");
    printf("使用C语言实现，添加重复项高亮和统计\n");
    printf("作者: puniq team\n");
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
    printf("%s] %d%% 处理中...", COLOR_CYAN, (int)((double)current / total * 100));
    fflush(stdout);
}

// 比较两行（考虑选项）
int compare_lines(const char *line1, const char *line2, const struct options *opts) {
    const char *p1 = line1;
    const char *p2 = line2;
    
    // 跳过指定数量的字段
    if (opts->skip_fields > 0) {
        int fields_skipped = 0;
        while (*p1 && fields_skipped < opts->skip_fields) {
            if (*p1 == ' ' || *p1 == '\t') {
                fields_skipped++;
                while (*p1 == ' ' || *p1 == '\t') p1++;
            } else {
                p1++;
            }
        }
        fields_skipped = 0;
        while (*p2 && fields_skipped < opts->skip_fields) {
            if (*p2 == ' ' || *p2 == '\t') {
                fields_skipped++;
                while (*p2 == ' ' || *p2 == '\t') p2++;
            } else {
                p2++;
            }
        }
    }
    
    // 跳过指定数量的字符
    if (opts->skip_chars > 0) {
        int chars_skipped = 0;
        while (*p1 && chars_skipped < opts->skip_chars) {
            p1++;
            chars_skipped++;
        }
        chars_skipped = 0;
        while (*p2 && chars_skipped < opts->skip_chars) {
            p2++;
            chars_skipped++;
        }
    }
    
    // 比较字符串
    if (opts->ignore_case) {
        return strcasecmp(p1, p2);
    } else {
        return strcmp(p1, p2);
    }
}

// 高亮行内容
void highlight_line(const char *line, int is_duplicate, int count) {
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
    char prev_line[MAX_LINE_LENGTH] = "";
    int line_count = 0;
    int unique_count = 0;
    int duplicate_count = 0;
    int current_count = 1;
    int total_lines = 0;
    
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
    
    // 第一遍：统计总行数
    if (opts->show_progress) {
        FILE *temp_file = file;
        if (file != stdin) {
            fclose(file);
            temp_file = fopen(filename, "r");
        }
        while (fgets(line, sizeof(line), temp_file)) {
            total_lines++;
        }
        if (temp_file != stdin) {
            fclose(temp_file);
        }
        file = fopen(filename, "r");
        if (!file && strcmp(filename, "标准输入") != 0) {
            file = stdin;
        }
    }
    
    // 处理第一行
    if (fgets(line, sizeof(line), file)) {
        strcpy(prev_line, line);
        line_count++;
        current_count = 1;
    } else {
        // 空文件
        if (file != stdin) fclose(file);
        return 0;
    }
    
    // 处理后续行
    while (fgets(line, sizeof(line), file)) {
        line_count++;
        
        if (opts->show_progress) {
            show_progress(line_count, total_lines);
        }
        
        if (compare_lines(prev_line, line, opts) == 0) {
            // 重复行
            current_count++;
        } else {
            // 新行，输出前一行
            int is_duplicate = (current_count > 1);
            
            if (opts->count) {
                printf("%s%7d %s", COLOR_CYAN, current_count, COLOR_RESET);
            }
            
            if (opts->repeated && !is_duplicate) {
                // 只显示重复行，跳过
            } else if (opts->unique && is_duplicate) {
                // 只显示唯一行，跳过
            } else if (opts->all_repeated && !is_duplicate) {
                // 显示所有重复行，跳过
            } else {
                if (opts->color) {
                    highlight_line(prev_line, is_duplicate, current_count);
                } else {
                    printf("%s", prev_line);
                }
            }
            
            if (is_duplicate) {
                duplicate_count++;
            } else {
                unique_count++;
            }
            
            // 更新状态
            strcpy(prev_line, line);
            current_count = 1;
        }
    }
    
    // 处理最后一行
    int is_duplicate = (current_count > 1);
    
    if (opts->count) {
        printf("%s%7d %s", COLOR_CYAN, current_count, COLOR_RESET);
    }
    
    if (opts->repeated && !is_duplicate) {
        // 只显示重复行，跳过
    } else if (opts->unique && is_duplicate) {
        // 只显示唯一行，跳过
    } else if (opts->all_repeated && !is_duplicate) {
        // 显示所有重复行，跳过
    } else {
        if (opts->color) {
            highlight_line(prev_line, is_duplicate, current_count);
        } else {
            printf("%s", prev_line);
        }
    }
    
    if (is_duplicate) {
        duplicate_count++;
    } else {
        unique_count++;
    }
    
    if (opts->show_progress) {
        printf("\n");
    }
    
    // 显示统计信息
    if (opts->show_stats) {
        printf("%s统计信息:%s\n", COLOR_CYAN, COLOR_RESET);
        printf("  总行数: %d\n", line_count);
        printf("  唯一行数: %d\n", unique_count);
        printf("  重复行数: %d\n", duplicate_count);
        printf("  重复率: %.1f%%\n", (double)duplicate_count / line_count * 100);
    }
    
    if (file != stdin) {
        fclose(file);
    }
    
    return 0;
}

int main(int argc, char *argv[]) {
    struct options opts;
    init_options(&opts);
    
    static struct option long_options[] = {
        {"count", no_argument, 0, 'c'},
        {"repeated", no_argument, 0, 'd'},
        {"unique", no_argument, 0, 'u'},
        {"all-repeated", no_argument, 0, 'D'},
        {"ignore-case", no_argument, 0, 'i'},
        {"skip-fields", required_argument, 0, 'f'},
        {"skip-chars", required_argument, 0, 's'},
        {"color", no_argument, 0, 1},
        {"no-color", no_argument, 0, 2},
        {"stats", no_argument, 0, 3},
        {"progress", no_argument, 0, 4},
        {"verbose", no_argument, 0, 5},
        {"separator", required_argument, 0, 6},
        {"help", no_argument, 0, 'h'},
        {"version", no_argument, 0, 'V'},
        {0, 0, 0, 0}
    };
    
    int opt;
    int option_index = 0;
    
    while ((opt = getopt_long(argc, argv, "cduDif:shV", long_options, &option_index)) != -1) {
        switch (opt) {
            case 'c':
                opts.count = 1;
                break;
            case 'd':
                opts.repeated = 1;
                break;
            case 'u':
                opts.unique = 1;
                break;
            case 'D':
                opts.all_repeated = 1;
                break;
            case 'i':
                opts.ignore_case = 1;
                break;
            case 'f':
                opts.skip_fields = atoi(optarg);
                break;
            case 's':
                opts.skip_chars = atoi(optarg);
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
            case 'h':
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
