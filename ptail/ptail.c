#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <sys/stat.h>
#include <errno.h>
#include <signal.h>
#include <time.h>

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
#define MAX_LINE_LENGTH 4096

// 全局选项
struct options {
    int lines;
    int bytes;
    int quiet;
    int verbose;
    int color;
    int show_filename;
    int show_line_numbers;
    int follow;
    int sleep_interval;
    int retry;
    char *separator;
    int timestamp;
};

// 全局变量
static volatile int keep_running = 1;

// 信号处理函数
void signal_handler(int sig) {
    if (sig == SIGINT || sig == SIGTERM) {
        keep_running = 0;
    }
}

// 初始化选项
void init_options(struct options *opts) {
    opts->lines = DEFAULT_LINES;
    opts->bytes = -1;
    opts->quiet = 0;
    opts->verbose = 0;
    opts->color = 1;
    opts->show_filename = 0;
    opts->show_line_numbers = 0;
    opts->follow = 0;
    opts->sleep_interval = 1000000; // 1秒（微秒）
    opts->retry = 1;
    opts->separator = "==>";
    opts->timestamp = 0;
}

// 打印帮助信息
void print_help() {
    printf("%sptail - 优化版 tail 命令%s\n", COLOR_CYAN, COLOR_RESET);
    printf("%s==============================================%s\n", COLOR_YELLOW, COLOR_RESET);
    printf("用法: ptail [选项] [文件...]\n\n");
    printf("选项:\n");
    printf("  -n, --lines=NUM     显示最后NUM行 (默认: 10)\n");
    printf("  -c, --bytes=NUM     显示最后NUM字节\n");
    printf("  -f, --follow        跟踪文件变化\n");
    printf("  -F, --follow=name   跟踪文件变化，即使文件被重命名\n");
    printf("  -s, --sleep-interval=NUM  跟踪模式下的睡眠间隔（秒）\n");
    printf("  --retry             跟踪模式下文件不存在时重试\n");
    printf("  -q, --quiet         不显示文件名\n");
    printf("  -v, --verbose       总是显示文件名\n");
    printf("  --color             启用彩色输出 (默认)\n");
    printf("  --no-color          禁用彩色输出\n");
    printf("  --line-numbers      显示行号\n");
    printf("  --timestamp         显示时间戳\n");
    printf("  --separator=STR     设置文件分隔符 (默认: '==>')\n");
    printf("  -h, --help          显示此帮助信息\n");
    printf("  -V, --version       显示版本信息\n\n");
    printf("示例:\n");
    printf("  ptail file.txt                    # 显示最后10行\n");
    printf("  ptail -n 20 file.txt              # 显示最后20行\n");
    printf("  ptail -f log.txt                  # 跟踪文件变化\n");
    printf("  ptail --line-numbers file.txt     # 显示行号\n");
    printf("  ptail --timestamp -f log.txt      # 带时间戳跟踪\n");
}

// 打印版本信息
void print_version() {
    printf("ptail - 优化版 tail 命令 v1.0\n");
    printf("使用C语言实现，添加彩色输出和实时监控\n");
    printf("作者: ptail team\n");
}

// 获取文件大小
long get_file_size(const char *filename) {
    struct stat st;
    if (stat(filename, &st) == 0) {
        return st.st_size;
    }
    return -1;
}

// 显示时间戳
void print_timestamp() {
    time_t now;
    struct tm *tm_info;
    char buffer[26];
    
    time(&now);
    tm_info = localtime(&now);
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", tm_info);
    printf("%s[%s]%s ", COLOR_GREEN, buffer, COLOR_RESET);
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

// 读取文件的最后N行
int read_last_lines(FILE *file, int n, const struct options *opts) {
    char **lines = malloc(n * sizeof(char*));
    if (!lines) return -1;
    
    for (int i = 0; i < n; i++) {
        lines[i] = malloc(MAX_LINE_LENGTH);
        if (!lines[i]) {
            for (int j = 0; j < i; j++) free(lines[j]);
            free(lines);
            return -1;
        }
    }
    
    int line_count = 0;
    int current_line = 0;
    char line[MAX_LINE_LENGTH];
    
    // 读取所有行
    while (fgets(line, sizeof(line), file)) {
        strcpy(lines[current_line], line);
        current_line = (current_line + 1) % n;
        line_count++;
    }
    
    // 输出最后n行
    int start_line = (line_count < n) ? 0 : current_line;
    int total_lines = (line_count < n) ? line_count : n;
    
    for (int i = 0; i < total_lines; i++) {
        int line_idx = (start_line + i) % n;
        int line_num = line_count - total_lines + i + 1;
        
        if (opts->timestamp) {
            print_timestamp();
        }
        
        if (opts->color) {
            highlight_line(lines[line_idx], opts->show_line_numbers ? line_num : 0);
        } else {
            if (opts->show_line_numbers) {
                printf("%4d: %s", line_num, lines[line_idx]);
            } else {
                printf("%s", lines[line_idx]);
            }
        }
    }
    
    // 清理内存
    for (int i = 0; i < n; i++) {
        free(lines[i]);
    }
    free(lines);
    
    return 0;
}

// 跟踪文件变化
int follow_file(const char *filename, const struct options *opts) {
    FILE *file;
    char line[MAX_LINE_LENGTH];
    long last_pos = 0;
    int line_count = 0;
    
    while (keep_running) {
        file = fopen(filename, "r");
        if (!file) {
            if (opts->retry) {
                usleep(opts->sleep_interval);
                continue;
            } else {
                fprintf(stderr, "%s错误: 无法打开文件 '%s': %s%s\n", 
                        COLOR_RED, filename, strerror(errno), COLOR_RESET);
                return 1;
            }
        }
        
        // 移动到上次读取的位置
        if (fseek(file, last_pos, SEEK_SET) != 0) {
            fclose(file);
            usleep(opts->sleep_interval);
            continue;
        }
        
        // 读取新行
        while (fgets(line, sizeof(line), file) && keep_running) {
            line_count++;
            
            if (opts->timestamp) {
                print_timestamp();
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
        
        last_pos = ftell(file);
        fclose(file);
        
        usleep(opts->sleep_interval);
    }
    
    return 0;
}

// 处理单个文件
int process_file(const char *filename, const struct options *opts) {
    FILE *file;
    
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
    if (opts->show_filename || (opts->verbose && strcmp(filename, "标准输入") != 0)) {
        printf("%s%s %s%s\n", COLOR_MAGENTA, opts->separator, filename, COLOR_RESET);
    }
    
    if (opts->follow) {
        // 跟踪模式
        if (strcmp(filename, "标准输入") == 0) {
            fprintf(stderr, "%s错误: 跟踪模式不支持标准输入%s\n", COLOR_RED, COLOR_RESET);
            return 1;
        }
        return follow_file(filename, opts);
    } else {
        // 普通模式
        if (opts->bytes == -1) {
            // 按行处理
            return read_last_lines(file, opts->lines, opts);
        } else {
            // 按字节处理
            long file_size = get_file_size(filename);
            if (file_size > 0) {
                long start_pos = (file_size > opts->bytes) ? file_size - opts->bytes : 0;
                fseek(file, start_pos, SEEK_SET);
            }
            
            int c;
            while ((c = fgetc(file)) != EOF) {
                putchar(c);
            }
        }
    }
    
    if (file != stdin) {
        fclose(file);
    }
    
    return 0;
}

int main(int argc, char *argv[]) {
    struct options opts;
    init_options(&opts);
    
    // 设置信号处理
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    
    static struct option long_options[] = {
        {"lines", required_argument, 0, 'n'},
        {"bytes", required_argument, 0, 'c'},
        {"follow", no_argument, 0, 'f'},
        {"follow=name", no_argument, 0, 'F'},
        {"sleep-interval", required_argument, 0, 's'},
        {"retry", no_argument, 0, 1},
        {"quiet", no_argument, 0, 'q'},
        {"verbose", no_argument, 0, 'v'},
        {"color", no_argument, 0, 2},
        {"no-color", no_argument, 0, 3},
        {"line-numbers", no_argument, 0, 4},
        {"timestamp", no_argument, 0, 5},
        {"separator", required_argument, 0, 6},
        {"help", no_argument, 0, 'h'},
        {"version", no_argument, 0, 'V'},
        {0, 0, 0, 0}
    };
    
    int opt;
    int option_index = 0;
    
    while ((opt = getopt_long(argc, argv, "n:c:fFs:qvhV", long_options, &option_index)) != -1) {
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
            case 'f':
            case 'F':
                opts.follow = 1;
                break;
            case 's':
                opts.sleep_interval = atoi(optarg) * 1000000; // 转换为微秒
                break;
            case 1: // --retry
                opts.retry = 1;
                break;
            case 'q':
                opts.quiet = 1;
                opts.verbose = 0;
                break;
            case 'v':
                opts.verbose = 1;
                opts.quiet = 0;
                break;
            case 2: // --color
                opts.color = 1;
                break;
            case 3: // --no-color
                opts.color = 0;
                break;
            case 4: // --line-numbers
                opts.show_line_numbers = 1;
                break;
            case 5: // --timestamp
                opts.timestamp = 1;
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
