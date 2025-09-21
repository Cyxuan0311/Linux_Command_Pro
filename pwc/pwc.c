#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <errno.h>
#include <ctype.h>
#include <sys/stat.h>

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

// 统计结构
struct stats {
    long lines;
    long words;
    long chars;
    long bytes;
    long max_line_length;
    long empty_lines;
    long non_empty_lines;
};

// 全局选项
struct options {
    int show_lines;
    int show_words;
    int show_chars;
    int show_bytes;
    int show_max_line;
    int show_empty_lines;
    int show_breakdown;
    int color;
    int show_progress;
    int verbose;
    int human_readable;
    char *separator;
};

// 初始化选项
void init_options(struct options *opts) {
    opts->show_lines = 1;
    opts->show_words = 1;
    opts->show_chars = 1;
    opts->show_bytes = 0;
    opts->show_max_line = 0;
    opts->show_empty_lines = 0;
    opts->show_breakdown = 0;
    opts->color = 1;
    opts->show_progress = 0;
    opts->verbose = 0;
    opts->human_readable = 0;
    opts->separator = "==>";
}

// 打印帮助信息
void print_help() {
    printf("%spwc - 优化版 wc 命令%s\n", COLOR_CYAN, COLOR_RESET);
    printf("%s==============================================%s\n", COLOR_YELLOW, COLOR_RESET);
    printf("用法: pwc [选项] [文件...]\n\n");
    printf("选项:\n");
    printf("  -l, --lines             显示行数\n");
    printf("  -w, --words             显示单词数\n");
    printf("  -c, --bytes             显示字节数\n");
    printf("  -m, --chars             显示字符数\n");
    printf("  -L, --max-line-length   显示最长行的长度\n");
    printf("  -e, --empty-lines       显示空行数\n");
    printf("  -b, --breakdown         显示详细分解\n");
    printf("  --human-readable        人类可读格式\n");
    printf("  --color                 启用彩色输出 (默认)\n");
    printf("  --no-color              禁用彩色输出\n");
    printf("  --progress              显示进度条\n");
    printf("  --verbose               详细输出\n");
    printf("  --separator=STR         设置文件分隔符 (默认: '==>')\n");
    printf("  -h, --help              显示此帮助信息\n");
    printf("  -V, --version           显示版本信息\n\n");
    printf("示例:\n");
    printf("  pwc file.txt                    # 基本统计\n");
    printf("  pwc -l file.txt                 # 只显示行数\n");
    printf("  pwc -w file.txt                 # 只显示单词数\n");
    printf("  pwc --breakdown file.txt        # 详细分解\n");
    printf("  pwc --human-readable file.txt   # 人类可读格式\n");
}

// 打印版本信息
void print_version() {
    printf("pwc - 优化版 wc 命令 v1.0\n");
    printf("使用C语言实现，添加彩色统计和进度条\n");
    printf("作者: pwc team\n");
}

// 初始化统计结构
void init_stats(struct stats *s) {
    s->lines = 0;
    s->words = 0;
    s->chars = 0;
    s->bytes = 0;
    s->max_line_length = 0;
    s->empty_lines = 0;
    s->non_empty_lines = 0;
}

// 显示进度条
void show_progress(long current, long total) {
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
    printf("%s] %d%% 统计中...", COLOR_CYAN, (int)((double)current / total * 100));
    fflush(stdout);
}

// 人类可读格式
void format_number(long num, char *buffer, int size) {
    if (num < 1000) {
        snprintf(buffer, size, "%ld", num);
    } else if (num < 1000000) {
        snprintf(buffer, size, "%.1fK", num / 1000.0);
    } else if (num < 1000000000) {
        snprintf(buffer, size, "%.1fM", num / 1000000.0);
    } else {
        snprintf(buffer, size, "%.1fG", num / 1000000000.0);
    }
}

// 处理单行
void process_line(const char *line, struct stats *s) {
    s->lines++;
    s->chars += strlen(line);
    s->bytes += strlen(line);
    
    int line_length = strlen(line);
    if (line_length > s->max_line_length) {
        s->max_line_length = line_length;
    }
    
    // 检查是否为空行
    int is_empty = 1;
    for (const char *p = line; *p; p++) {
        if (!isspace(*p)) {
            is_empty = 0;
            break;
        }
    }
    
    if (is_empty) {
        s->empty_lines++;
    } else {
        s->non_empty_lines++;
    }
    
    // 计算单词数
    int in_word = 0;
    for (const char *p = line; *p; p++) {
        if (isspace(*p)) {
            in_word = 0;
        } else if (!in_word) {
            in_word = 1;
            s->words++;
        }
    }
}

// 处理文件
int process_file(const char *filename, const struct options *opts, struct stats *total_stats) {
    FILE *file;
    char line[MAX_LINE_LENGTH];
    struct stats file_stats;
    long file_size = 0;
    long bytes_read = 0;
    
    init_stats(&file_stats);
    
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
        
        // 获取文件大小
        struct stat st;
        if (stat(filename, &st) == 0) {
            file_size = st.st_size;
        }
    }
    
    // 显示文件名（如果需要）
    if (opts->verbose) {
        printf("%s%s %s%s\n", COLOR_MAGENTA, opts->separator, filename, COLOR_RESET);
    }
    
    // 处理文件内容
    while (fgets(line, sizeof(line), file)) {
        process_line(line, &file_stats);
        bytes_read += strlen(line);
        
        if (opts->show_progress && file_size > 0) {
            show_progress(bytes_read, file_size);
        }
    }
    
    if (opts->show_progress && file_size > 0) {
        printf("\n");
    }
    
    // 更新总统计
    total_stats->lines += file_stats.lines;
    total_stats->words += file_stats.words;
    total_stats->chars += file_stats.chars;
    total_stats->bytes += file_stats.bytes;
    total_stats->empty_lines += file_stats.empty_lines;
    total_stats->non_empty_lines += file_stats.non_empty_lines;
    if (file_stats.max_line_length > total_stats->max_line_length) {
        total_stats->max_line_length = file_stats.max_line_length;
    }
    
    // 输出统计结果
    char line_str[32], word_str[32], char_str[32], byte_str[32];
    char max_line_str[32], empty_str[32];
    
    if (opts->human_readable) {
        format_number(file_stats.lines, line_str, sizeof(line_str));
        format_number(file_stats.words, word_str, sizeof(word_str));
        format_number(file_stats.chars, char_str, sizeof(char_str));
        format_number(file_stats.bytes, byte_str, sizeof(byte_str));
        format_number(file_stats.max_line_length, max_line_str, sizeof(max_line_str));
        format_number(file_stats.empty_lines, empty_str, sizeof(empty_str));
    } else {
        snprintf(line_str, sizeof(line_str), "%ld", file_stats.lines);
        snprintf(word_str, sizeof(word_str), "%ld", file_stats.words);
        snprintf(char_str, sizeof(char_str), "%ld", file_stats.chars);
        snprintf(byte_str, sizeof(byte_str), "%ld", file_stats.bytes);
        snprintf(max_line_str, sizeof(max_line_str), "%ld", file_stats.max_line_length);
        snprintf(empty_str, sizeof(empty_str), "%ld", file_stats.empty_lines);
    }
    
    // 构建输出格式
    char output[512] = "";
    char *p = output;
    
    if (opts->show_lines) {
        if (opts->color) {
            p += sprintf(p, "%s%8s%s ", COLOR_GREEN, line_str, COLOR_RESET);
        } else {
            p += sprintf(p, "%8s ", line_str);
        }
    }
    
    if (opts->show_words) {
        if (opts->color) {
            p += sprintf(p, "%s%8s%s ", COLOR_BLUE, word_str, COLOR_RESET);
        } else {
            p += sprintf(p, "%8s ", word_str);
        }
    }
    
    if (opts->show_chars) {
        if (opts->color) {
            p += sprintf(p, "%s%8s%s ", COLOR_YELLOW, char_str, COLOR_RESET);
        } else {
            p += sprintf(p, "%8s ", char_str);
        }
    }
    
    if (opts->show_bytes) {
        if (opts->color) {
            p += sprintf(p, "%s%8s%s ", COLOR_MAGENTA, byte_str, COLOR_RESET);
        } else {
            p += sprintf(p, "%8s ", byte_str);
        }
    }
    
    if (opts->show_max_line) {
        if (opts->color) {
            p += sprintf(p, "%s%8s%s ", COLOR_CYAN, max_line_str, COLOR_RESET);
        } else {
            p += sprintf(p, "%8s ", max_line_str);
        }
    }
    
    if (opts->show_empty_lines) {
        if (opts->color) {
            p += sprintf(p, "%s%8s%s ", COLOR_RED, empty_str, COLOR_RESET);
        } else {
            p += sprintf(p, "%8s ", empty_str);
        }
    }
    
    printf("%s %s\n", output, filename);
    
    // 显示详细分解
    if (opts->show_breakdown) {
        printf("%s详细分解:%s\n", COLOR_CYAN, COLOR_RESET);
        printf("  总行数: %s%ld%s\n", COLOR_GREEN, file_stats.lines, COLOR_RESET);
        printf("  非空行: %s%ld%s\n", COLOR_GREEN, file_stats.non_empty_lines, COLOR_RESET);
        printf("  空行数: %s%ld%s\n", COLOR_RED, file_stats.empty_lines, COLOR_RESET);
        printf("  单词数: %s%ld%s\n", COLOR_BLUE, file_stats.words, COLOR_RESET);
        printf("  字符数: %s%ld%s\n", COLOR_YELLOW, file_stats.chars, COLOR_RESET);
        printf("  字节数: %s%ld%s\n", COLOR_MAGENTA, file_stats.bytes, COLOR_RESET);
        printf("  最长行: %s%ld%s 字符\n", COLOR_CYAN, file_stats.max_line_length, COLOR_RESET);
        printf("  平均行长度: %.1f 字符\n", file_stats.lines > 0 ? (double)file_stats.chars / file_stats.lines : 0);
        printf("  平均单词长度: %.1f 字符\n", file_stats.words > 0 ? (double)file_stats.chars / file_stats.words : 0);
    }
    
    if (file != stdin) {
        fclose(file);
    }
    
    return 0;
}

int main(int argc, char *argv[]) {
    struct options opts;
    struct stats total_stats;
    int result = 0;
    
    init_options(&opts);
    init_stats(&total_stats);
    
    static struct option long_options[] = {
        {"lines", no_argument, 0, 'l'},
        {"words", no_argument, 0, 'w'},
        {"bytes", no_argument, 0, 'c'},
        {"chars", no_argument, 0, 'm'},
        {"max-line-length", no_argument, 0, 'L'},
        {"empty-lines", no_argument, 0, 'e'},
        {"breakdown", no_argument, 0, 'b'},
        {"human-readable", no_argument, 0, 1},
        {"color", no_argument, 0, 2},
        {"no-color", no_argument, 0, 3},
        {"progress", no_argument, 0, 4},
        {"verbose", no_argument, 0, 5},
        {"separator", required_argument, 0, 6},
        {"help", no_argument, 0, 'h'},
        {"version", no_argument, 0, 'V'},
        {0, 0, 0, 0}
    };
    
    int opt;
    int option_index = 0;
    
    while ((opt = getopt_long(argc, argv, "lwcmLebhV", long_options, &option_index)) != -1) {
        switch (opt) {
            case 'l':
                opts.show_lines = 1;
                opts.show_words = 0;
                opts.show_chars = 0;
                opts.show_bytes = 0;
                break;
            case 'w':
                opts.show_words = 1;
                opts.show_lines = 0;
                opts.show_chars = 0;
                opts.show_bytes = 0;
                break;
            case 'c':
                opts.show_bytes = 1;
                opts.show_lines = 0;
                opts.show_words = 0;
                opts.show_chars = 0;
                break;
            case 'm':
                opts.show_chars = 1;
                opts.show_lines = 0;
                opts.show_words = 0;
                opts.show_bytes = 0;
                break;
            case 'L':
                opts.show_max_line = 1;
                opts.show_lines = 0;
                opts.show_words = 0;
                opts.show_chars = 0;
                opts.show_bytes = 0;
                break;
            case 'e':
                opts.show_empty_lines = 1;
                opts.show_lines = 0;
                opts.show_words = 0;
                opts.show_chars = 0;
                opts.show_bytes = 0;
                break;
            case 'b':
                opts.show_breakdown = 1;
                break;
            case 1: // --human-readable
                opts.human_readable = 1;
                break;
            case 2: // --color
                opts.color = 1;
                break;
            case 3: // --no-color
                opts.color = 0;
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
        result = process_file("-", &opts, &total_stats);
    } else {
        int file_count = argc - optind;
        
        for (int i = optind; i < argc; i++) {
            if (process_file(argv[i], &opts, &total_stats) != 0) {
                result = 1;
            }
        }
        
        // 如果有多个文件，显示总计
        if (file_count > 1) {
            char line_str[32], word_str[32], char_str[32], byte_str[32];
            char max_line_str[32], empty_str[32];
            
            if (opts.human_readable) {
                format_number(total_stats.lines, line_str, sizeof(line_str));
                format_number(total_stats.words, word_str, sizeof(word_str));
                format_number(total_stats.chars, char_str, sizeof(char_str));
                format_number(total_stats.bytes, byte_str, sizeof(byte_str));
                format_number(total_stats.max_line_length, max_line_str, sizeof(max_line_str));
                format_number(total_stats.empty_lines, empty_str, sizeof(empty_str));
            } else {
                snprintf(line_str, sizeof(line_str), "%ld", total_stats.lines);
                snprintf(word_str, sizeof(word_str), "%ld", total_stats.words);
                snprintf(char_str, sizeof(char_str), "%ld", total_stats.chars);
                snprintf(byte_str, sizeof(byte_str), "%ld", total_stats.bytes);
                snprintf(max_line_str, sizeof(max_line_str), "%ld", total_stats.max_line_length);
                snprintf(empty_str, sizeof(empty_str), "%ld", total_stats.empty_lines);
            }
            
            char output[512] = "";
            char *p = output;
            
            if (opts.show_lines) {
                if (opts.color) {
                    p += sprintf(p, "%s%8s%s ", COLOR_GREEN, line_str, COLOR_RESET);
                } else {
                    p += sprintf(p, "%8s ", line_str);
                }
            }
            
            if (opts.show_words) {
                if (opts.color) {
                    p += sprintf(p, "%s%8s%s ", COLOR_BLUE, word_str, COLOR_RESET);
                } else {
                    p += sprintf(p, "%8s ", word_str);
                }
            }
            
            if (opts.show_chars) {
                if (opts.color) {
                    p += sprintf(p, "%s%8s%s ", COLOR_YELLOW, char_str, COLOR_RESET);
                } else {
                    p += sprintf(p, "%8s ", char_str);
                }
            }
            
            if (opts.show_bytes) {
                if (opts.color) {
                    p += sprintf(p, "%s%8s%s ", COLOR_MAGENTA, byte_str, COLOR_RESET);
                } else {
                    p += sprintf(p, "%8s ", byte_str);
                }
            }
            
            if (opts.show_max_line) {
                if (opts.color) {
                    p += sprintf(p, "%s%8s%s ", COLOR_CYAN, max_line_str, COLOR_RESET);
                } else {
                    p += sprintf(p, "%8s ", max_line_str);
                }
            }
            
            if (opts.show_empty_lines) {
                if (opts.color) {
                    p += sprintf(p, "%s%8s%s ", COLOR_RED, empty_str, COLOR_RESET);
                } else {
                    p += sprintf(p, "%8s ", empty_str);
                }
            }
            
            printf("%s %s总计%s\n", output, COLOR_BOLD, COLOR_RESET);
        }
    }
    
    return result;
}
