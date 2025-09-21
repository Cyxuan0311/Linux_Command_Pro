#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <sys/stat.h>
#include <errno.h>

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

// 默认行数
#define DEFAULT_LINES 10

// 全局选项
struct options {
    int lines;
    int bytes;
    int quiet;
    int verbose;
    int color;
    int show_filename;
    int show_line_numbers;
    int show_progress;
    char *separator;
};

// 初始化选项
void init_options(struct options *opts) {
    opts->lines = DEFAULT_LINES;
    opts->bytes = -1;
    opts->quiet = 0;
    opts->verbose = 0;
    opts->color = 1;
    opts->show_filename = 0;
    opts->show_line_numbers = 0;
    opts->show_progress = 0;
    opts->separator = "==>";
}

// 打印帮助信息
void print_help() {
    printf("%sphead - 优化版 head 命令%s\n", COLOR_CYAN, COLOR_RESET);
    printf("%s==============================================%s\n", COLOR_YELLOW, COLOR_RESET);
    printf("用法: phead [选项] [文件...]\n\n");
    printf("选项:\n");
    printf("  -n, --lines=NUM     显示前NUM行 (默认: 10)\n");
    printf("  -c, --bytes=NUM     显示前NUM字节\n");
    printf("  -q, --quiet         不显示文件名\n");
    printf("  -v, --verbose       总是显示文件名\n");
    printf("  --color             启用彩色输出 (默认)\n");
    printf("  --no-color          禁用彩色输出\n");
    printf("  --line-numbers      显示行号\n");
    printf("  --progress          显示进度条\n");
    printf("  --separator=STR     设置文件分隔符 (默认: '==>')\n");
    printf("  -h, --help          显示此帮助信息\n");
    printf("  -V, --version       显示版本信息\n\n");
    printf("示例:\n");
    printf("  phead file.txt                    # 显示前10行\n");
    printf("  phead -n 20 file.txt              # 显示前20行\n");
    printf("  phead -c 100 file.txt             # 显示前100字节\n");
    printf("  phead --line-numbers file.txt     # 显示行号\n");
    printf("  phead --progress large_file.txt   # 显示进度\n");
}

// 打印版本信息
void print_version() {
    printf("phead - 优化版 head 命令 v1.0\n");
    printf("使用C语言实现，添加彩色输出和进度显示\n");
    printf("作者: phead team\n");
}

// 获取文件大小
long get_file_size(const char *filename) {
    struct stat st;
    if (stat(filename, &st) == 0) {
        return st.st_size;
    }
    return -1;
}

// 显示进度条
void show_progress(int current, int total, const char *filename) {
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
    printf("%s] %d%% %s%s", COLOR_CYAN, (int)((double)current / total * 100), 
           COLOR_RESET, filename);
    fflush(stdout);
}

// 高亮行内容
void highlight_line(const char *line, int line_num) {
    if (line_num > 0) {
        printf("%s%4d: %s", COLOR_CYAN, line_num, COLOR_RESET);
    }
    
    // 简单的语法高亮
    const char *p = line;
    while (*p) {
        if (*p == '#' && (p == line || *(p-1) == ' ')) {
            // 注释高亮
            printf("%s", COLOR_GREEN);
            while (*p && *p != '\n') {
                putchar(*p++);
            }
            printf("%s", COLOR_RESET);
        } else if (*p == '"' || *p == '\'') {
            // 字符串高亮
            char quote = *p;
            printf("%s%c", COLOR_YELLOW, *p++);
            while (*p && *p != quote) {
                putchar(*p++);
            }
            if (*p == quote) {
                printf("%c%s", *p++, COLOR_RESET);
            }
        } else if (*p >= '0' && *p <= '9') {
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

// 处理单个文件
int process_file(const char *filename, const struct options *opts) {
    FILE *file;
    char *line = NULL;
    size_t len = 0;
    ssize_t read;
    int line_count = 0;
    int byte_count = 0;
    long file_size = 0;
    
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
        file_size = get_file_size(filename);
    }
    
    // 显示文件名（如果需要）
    if (opts->show_filename || (opts->verbose && strcmp(filename, "标准输入") != 0)) {
        printf("%s%s %s%s\n", COLOR_MAGENTA, opts->separator, filename, COLOR_RESET);
    }
    
    // 按行处理
    if (opts->bytes == -1) {
        while ((read = getline(&line, &len, file)) != -1 && line_count < opts->lines) {
            line_count++;
            
            if (opts->show_progress && file_size > 0) {
                byte_count += read;
                show_progress(byte_count, file_size, filename);
            }
            
            if (opts->color) {
                highlight_line(line, opts->show_line_numbers ? line_count : 0);
            } else {
                if (opts->show_line_numbers) {
                    printf("%4d: %s", line_count, line);
                } else {
                    printf("%s", line);
                }
            }
        }
        
        if (opts->show_progress && file_size > 0) {
            printf("\n");
        }
    } else {
        // 按字节处理
        int c;
        while ((c = fgetc(file)) != EOF && byte_count < opts->bytes) {
            putchar(c);
            byte_count++;
            
            if (opts->show_progress && file_size > 0) {
                show_progress(byte_count, file_size, filename);
            }
        }
        
        if (opts->show_progress && file_size > 0) {
            printf("\n");
        }
    }
    
    if (line) {
        free(line);
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
        {"lines", required_argument, 0, 'n'},
        {"bytes", required_argument, 0, 'c'},
        {"quiet", no_argument, 0, 'q'},
        {"verbose", no_argument, 0, 'v'},
        {"color", no_argument, 0, 1},
        {"no-color", no_argument, 0, 2},
        {"line-numbers", no_argument, 0, 3},
        {"progress", no_argument, 0, 4},
        {"separator", required_argument, 0, 5},
        {"help", no_argument, 0, 'h'},
        {"version", no_argument, 0, 'V'},
        {0, 0, 0, 0}
    };
    
    int opt;
    int option_index = 0;
    
    while ((opt = getopt_long(argc, argv, "n:c:qvhV", long_options, &option_index)) != -1) {
        switch (opt) {
            case 'n':
                opts.lines = atoi(optarg);
                if (opts.lines < 0) {
                    fprintf(stderr, "%s错误: 行数不能为负数%s\n", COLOR_RED, COLOR_RESET);
                    return 1;
                }
                break;
            case 'c':
                opts.bytes = atoi(optarg);
                if (opts.bytes < 0) {
                    fprintf(stderr, "%s错误: 字节数不能为负数%s\n", COLOR_RED, COLOR_RESET);
                    return 1;
                }
                break;
            case 'q':
                opts.quiet = 1;
                opts.verbose = 0;
                break;
            case 'v':
                opts.verbose = 1;
                opts.quiet = 0;
                break;
            case 1: // --color
                opts.color = 1;
                break;
            case 2: // --no-color
                opts.color = 0;
                break;
            case 3: // --line-numbers
                opts.show_line_numbers = 1;
                break;
            case 4: // --progress
                opts.show_progress = 1;
                break;
            case 5: // --separator
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
        int file_count = argc - optind;
        
        for (int i = optind; i < argc; i++) {
            if (file_count > 1 && !opts.quiet) {
                opts.show_filename = 1;
            }
            
            if (process_file(argv[i], &opts) != 0) {
                result = 1;
            }
            
            // 文件间添加分隔符
            if (i < argc - 1 && !opts.quiet) {
                printf("\n");
            }
        }
        
        return result;
    }
}
