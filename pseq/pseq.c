#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include "../include/common.h"

#define MAX_FORMAT_LENGTH 256

// seq选项结构
typedef struct {
    double first;
    double last;
    double increment;
    int show_help;
    int show_version;
    int show_help_format;
    char separator[16];
    char format[MAX_FORMAT_LENGTH];
    int equal_width;
    int show_help_separator;
} SeqOptions;

// 初始化选项
void init_options(SeqOptions *opts) {
    memset(opts, 0, sizeof(SeqOptions));
    opts->first = 1.0;
    opts->last = 1.0;
    opts->increment = 1.0;
    strcpy(opts->separator, "\n");
    strcpy(opts->format, "%g");
}

// 显示帮助信息
void print_help(const char *program_name) {
    printf("用法: %s [选项] 数字\n", program_name);
    printf("增强版 seq 命令，生成数字序列\n\n");
    printf("选项:\n");
    printf("  -f, --format=FORMAT   使用printf格式 (默认: %%g)\n");
    printf("  -s, --separator=STR   使用指定分隔符 (默认: \\n)\n");
    printf("  -w, --equal-width     等宽格式\n");
    printf("  --help-format         显示格式帮助\n");
    printf("  --help-separator      显示分隔符帮助\n");
    printf("  -h, --help            显示此帮助信息\n");
    printf("  -v, --version         显示版本信息\n\n");
    printf("参数:\n");
    printf("  数字    可以是以下格式之一:\n");
    printf("          LAST          从1到LAST\n");
    printf("          FIRST LAST    从FIRST到LAST\n");
    printf("          FIRST INCR LAST 从FIRST到LAST，步长为INCR\n\n");
    printf("示例:\n");
    printf("  %s 5                  # 1 2 3 4 5\n", program_name);
    printf("  %s 2 5                # 2 3 4 5\n", program_name);
    printf("  %s 1 2 10             # 1 3 5 7 9\n", program_name);
    printf("  %s -f \"%%02d\" 1 5      # 01 02 03 04 05\n", program_name);
    printf("  %s -s \",\" 1 5         # 1,2,3,4,5\n", program_name);
    printf("  %s -w 1 100           # 等宽格式\n", program_name);
}

// 显示版本信息
void print_version() {
    printf("pseq - 增强版 seq 命令 v1.0\n");
    printf("提供强大的数字序列生成功能\n");
}

// 显示格式帮助
void print_format_help() {
    printf("格式说明:\n\n");
    printf("  %s%%d%s    整数\n", COLOR_GREEN, COLOR_RESET);
    printf("  %s%%i%s    整数\n", COLOR_GREEN, COLOR_RESET);
    printf("  %s%%o%s    八进制\n", COLOR_GREEN, COLOR_RESET);
    printf("  %s%%u%s    无符号整数\n", COLOR_GREEN, COLOR_RESET);
    printf("  %s%%x%s    十六进制 (小写)\n", COLOR_GREEN, COLOR_RESET);
    printf("  %s%%X%s    十六进制 (大写)\n", COLOR_GREEN, COLOR_RESET);
    printf("  %s%%f%s    浮点数\n", COLOR_GREEN, COLOR_RESET);
    printf("  %s%%e%s    科学计数法 (小写)\n", COLOR_GREEN, COLOR_RESET);
    printf("  %s%%E%s    科学计数法 (大写)\n", COLOR_GREEN, COLOR_RESET);
    printf("  %s%%g%s    自动选择格式\n", COLOR_GREEN, COLOR_RESET);
    printf("  %s%%G%s    自动选择格式 (大写)\n", COLOR_GREEN, COLOR_RESET);
    printf("\n宽度和精度:\n");
    printf("  %s%%5d%s   宽度为5的整数\n", COLOR_CYAN, COLOR_RESET);
    printf("  %s%%05d%s  宽度为5，前导零填充\n", COLOR_CYAN, COLOR_RESET);
    printf("  %s%%.2f%s  保留2位小数的浮点数\n", COLOR_CYAN, COLOR_RESET);
    printf("  %s%%8.2f%s 宽度为8，保留2位小数\n", COLOR_CYAN, COLOR_RESET);
    printf("\n示例:\n");
    printf("  %s -f \"%%02d\" 1 5%s     # 01 02 03 04 05\n", COLOR_CYAN, COLOR_RESET);
    printf("  %s -f \"%%8.2f\" 1 3%s    #     1.00     2.00     3.00\n", COLOR_CYAN, COLOR_RESET);
    printf("  %s -f \"%%x\" 10 15%s     # a b c d e f\n", COLOR_CYAN, COLOR_RESET);
}

// 显示分隔符帮助
void print_separator_help() {
    printf("分隔符说明:\n\n");
    printf("  %s\\n%s     换行符 (默认)\n", COLOR_GREEN, COLOR_RESET);
    printf("  %s\\t%s     制表符\n", COLOR_GREEN, COLOR_RESET);
    printf("  %s\\r%s     回车符\n", COLOR_GREEN, COLOR_RESET);
    printf("  %s\\0%s     空字符\n", COLOR_GREEN, COLOR_RESET);
    printf("  %s\\a%s     响铃\n", COLOR_GREEN, COLOR_RESET);
    printf("  %s\\b%s     退格\n", COLOR_GREEN, COLOR_RESET);
    printf("  %s\\f%s     换页\n", COLOR_GREEN, COLOR_RESET);
    printf("  %s\\v%s     垂直制表符\n", COLOR_GREEN, COLOR_RESET);
    printf("  %s\\\\%s     反斜杠\n", COLOR_GREEN, COLOR_RESET);
    printf("  %s\\\"%s     双引号\n", COLOR_GREEN, COLOR_RESET);
    printf("  %s\\'%s     单引号\n", COLOR_GREEN, COLOR_RESET);
    printf("\n示例:\n");
    printf("  %s -s \",\" 1 5%s         # 1,2,3,4,5\n", COLOR_CYAN, COLOR_RESET);
    printf("  %s -s \" \" 1 5%s         # 1 2 3 4 5\n", COLOR_CYAN, COLOR_RESET);
    printf("  %s -s \"\\t\" 1 5%s       # 1\\t2\\t3\\t4\\t5\n", COLOR_CYAN, COLOR_RESET);
    printf("  %s -s \" | \" 1 5%s       # 1 | 2 | 3 | 4 | 5\n", COLOR_CYAN, COLOR_RESET);
}

// 解析转义字符
void parse_escape_sequences(char *str) {
    char *src = str;
    char *dst = str;
    
    while (*src) {
        if (*src == '\\' && *(src + 1)) {
            src++; // 跳过反斜杠
            switch (*src) {
                case 'n':
                    *dst++ = '\n';
                    break;
                case 't':
                    *dst++ = '\t';
                    break;
                case 'r':
                    *dst++ = '\r';
                    break;
                case '0':
                    *dst++ = '\0';
                    break;
                case 'a':
                    *dst++ = '\a';
                    break;
                case 'b':
                    *dst++ = '\b';
                    break;
                case 'f':
                    *dst++ = '\f';
                    break;
                case 'v':
                    *dst++ = '\v';
                    break;
                case '\\':
                    *dst++ = '\\';
                    break;
                case '"':
                    *dst++ = '"';
                    break;
                case '\'':
                    *dst++ = '\'';
                    break;
                default:
                    *dst++ = '\\';
                    *dst++ = *src;
                    break;
            }
            src++;
        } else {
            *dst++ = *src++;
        }
    }
    *dst = '\0';
}

// 生成序列
void generate_sequence(const SeqOptions *opts) {
    double current = opts->first;
    int count = 0;
    
    // 计算序列长度（用于等宽格式）
    int max_digits = 0;
    if (opts->equal_width) {
        double temp = opts->last;
        while (temp >= 1) {
            max_digits++;
            temp /= 10;
        }
    }
    
    while (1) {
        // 检查是否超出范围
        if (opts->increment > 0 && current > opts->last) {
            break;
        }
        if (opts->increment < 0 && current < opts->last) {
            break;
        }
        
        // 输出数字
        if (opts->equal_width && max_digits > 0) {
            printf(opts->format, (int)current);
        } else {
            printf(opts->format, current);
        }
        
        count++;
        
        // 检查是否到达最后一个数字
        if (opts->increment > 0 && current >= opts->last) {
            break;
        }
        if (opts->increment < 0 && current <= opts->last) {
            break;
        }
        
        // 输出分隔符
        if (opts->increment > 0 && current + opts->increment <= opts->last) {
            printf("%s", opts->separator);
        } else if (opts->increment < 0 && current + opts->increment >= opts->last) {
            printf("%s", opts->separator);
        }
        
        current += opts->increment;
    }
    
    // 如果不是换行符分隔符，最后输出一个换行
    if (strcmp(opts->separator, "\n") != 0) {
        printf("\n");
    }
}

// 解析命令行参数
int parse_arguments(int argc, char *argv[], SeqOptions *opts) {
    int opt;
    static struct option long_options[] = {
        {"format", required_argument, 0, 'f'},
        {"separator", required_argument, 0, 's'},
        {"equal-width", no_argument, 0, 'w'},
        {"help-format", no_argument, 0, 1},
        {"help-separator", no_argument, 0, 2},
        {"help", no_argument, 0, 'h'},
        {"version", no_argument, 0, 'v'},
        {0, 0, 0, 0}
    };

    while ((opt = getopt_long(argc, argv, "f:s:whv", long_options, NULL)) != -1) {
        switch (opt) {
            case 'f':
                strncpy(opts->format, optarg, MAX_FORMAT_LENGTH - 1);
                opts->format[MAX_FORMAT_LENGTH - 1] = '\0';
                break;
            case 's':
                strncpy(opts->separator, optarg, 15);
                opts->separator[15] = '\0';
                parse_escape_sequences(opts->separator);
                break;
            case 'w':
                opts->equal_width = 1;
                break;
            case 1: // --help-format
                opts->show_help_format = 1;
                break;
            case 2: // --help-separator
                opts->show_help_separator = 1;
                break;
            case 'h':
                opts->show_help = 1;
                break;
            case 'v':
                opts->show_version = 1;
                break;
            case '?':
                return 1;
            default:
                break;
        }
    }

    // 处理位置参数
    if (optind < argc) {
        if (optind + 1 == argc) {
            // 一个参数：LAST
            opts->last = atof(argv[optind]);
        } else if (optind + 2 == argc) {
            // 两个参数：FIRST LAST
            opts->first = atof(argv[optind]);
            opts->last = atof(argv[optind + 1]);
        } else if (optind + 3 == argc) {
            // 三个参数：FIRST INCR LAST
            opts->first = atof(argv[optind]);
            opts->increment = atof(argv[optind + 1]);
            opts->last = atof(argv[optind + 2]);
        } else {
            printf("错误: 参数过多\n");
            return 1;
        }
    } else {
        printf("错误: 请指定数字参数\n");
        return 1;
    }

    return 0;
}

int main(int argc, char *argv[]) {
    SeqOptions opts;
    
    init_options(&opts);
    
    if (parse_arguments(argc, argv, &opts) != 0) {
        print_help(argv[0]);
        return 1;
    }
    
    if (opts.show_help) {
        print_help(argv[0]);
        return 0;
    }
    
    if (opts.show_version) {
        print_version();
        return 0;
    }
    
    if (opts.show_help_format) {
        print_format_help();
        return 0;
    }
    
    if (opts.show_help_separator) {
        print_separator_help();
        return 0;
    }
    
    generate_sequence(&opts);
    
    return 0;
}
