#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <errno.h>
#include <sys/stat.h>
#include <utime.h>
#include <time.h>
#include <glob.h>

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

// 全局选项
struct options {
    int create_dirs;
    int no_create;
    int reference;
    int time;
    int access_time;
    int modify_time;
    int color;
    int show_progress;
    int verbose;
    int dry_run;
    char *separator;
    char *ref_file;
    time_t custom_time;
    mode_t file_mode;
};

// 初始化选项
void init_options(struct options *opts) {
    opts->create_dirs = 0;
    opts->no_create = 0;
    opts->reference = 0;
    opts->time = 0;
    opts->access_time = 0;
    opts->modify_time = 0;
    opts->color = 1;
    opts->show_progress = 0;
    opts->verbose = 0;
    opts->dry_run = 0;
    opts->separator = "==>";
    opts->ref_file = NULL;
    opts->custom_time = 0;
    opts->file_mode = 0644;
}

// 打印帮助信息
void print_help() {
    printf("%sptouch - 优化版 touch 命令%s\n", COLOR_CYAN, COLOR_RESET);
    printf("%s==============================================%s\n", COLOR_YELLOW, COLOR_RESET);
    printf("用法: ptouch [选项] 文件...\n\n");
    printf("选项:\n");
    printf("  -a                      只更改访问时间\n");
    printf("  -c, --no-create         不创建文件\n");
    printf("  -d, --date=STRING       使用指定日期时间\n");
    printf("  -m                      只更改修改时间\n");
    printf("  -r, --reference=FILE    使用参考文件的时间\n");
    printf("  -t STAMP                使用指定时间戳 ([[CC]YY]MMDDhhmm[.ss])\n");
    printf("  --create-dirs           创建必要的目录\n");
    printf("  --mode=MODE             设置文件权限 (八进制)\n");
    printf("  --color                 启用彩色输出 (默认)\n");
    printf("  --no-color              禁用彩色输出\n");
    printf("  --progress              显示进度条\n");
    printf("  --verbose               详细输出\n");
    printf("  --dry-run               模拟运行，不实际修改\n");
    printf("  --separator=STR         设置文件分隔符 (默认: '==>')\n");
    printf("  -h, --help              显示此帮助信息\n");
    printf("  -V, --version           显示版本信息\n\n");
    printf("示例:\n");
    printf("  ptouch file.txt                    # 创建或更新时间戳\n");
    printf("  ptouch -a file.txt                 # 只更改访问时间\n");
    printf("  ptouch -m file.txt                 # 只更改修改时间\n");
    printf("  ptouch -r ref.txt file.txt         # 使用参考文件时间\n");
    printf("  ptouch -t 202312251200 file.txt    # 使用指定时间\n");
    printf("  ptouch --create-dirs dir/file.txt  # 创建目录\n");
}

// 打印版本信息
void print_version() {
    printf("ptouch - 优化版 touch 命令 v1.0\n");
    printf("使用C语言实现，添加批量操作和进度显示\n");
    printf("作者: ptouch team\n");
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

// 创建目录
int create_directories(const char *path) {
    char *dir_path = strdup(path);
    char *p = dir_path;
    int result = 0;
    
    // 跳过开头的 '/'
    if (*p == '/') p++;
    
    while (*p) {
        if (*p == '/') {
            *p = '\0';
            if (strlen(dir_path) > 0) {
                if (mkdir(dir_path, 0755) != 0 && errno != EEXIST) {
                    fprintf(stderr, "%s错误: 无法创建目录 '%s': %s%s\n", 
                            COLOR_RED, dir_path, strerror(errno), COLOR_RESET);
                    result = 1;
                    break;
                }
            }
            *p = '/';
        }
        p++;
    }
    
    free(dir_path);
    return result;
}

// 解析时间戳
time_t parse_timestamp(const char *timestamp) {
    struct tm tm = {0};
    char *end;
    
    // 尝试解析格式: [[CC]YY]MMDDhhmm[.ss]
    if (strlen(timestamp) >= 10) {
        int len = strlen(timestamp);
        int year_offset = 0;
        
        if (len == 10) {
            // MMDDhhmmss
            year_offset = 0;
        } else if (len == 12) {
            // YYMMDDhhmmss
            year_offset = 2;
        } else if (len == 14) {
            // CCYYMMDDhhmmss
            year_offset = 4;
        } else {
            return 0;
        }
        
        tm.tm_year = atoi(timestamp + year_offset) - 1900;
        if (year_offset == 0) {
            // 假设是当前年份
            time_t now = time(NULL);
            struct tm *now_tm = localtime(&now);
            tm.tm_year = now_tm->tm_year;
        } else if (year_offset == 2) {
            // 两位年份，假设2000-2099
            if (tm.tm_year < 50) tm.tm_year += 100;
        }
        
        tm.tm_mon = atoi(timestamp + year_offset + 2) - 1;
        tm.tm_mday = atoi(timestamp + year_offset + 4);
        tm.tm_hour = atoi(timestamp + year_offset + 6);
        tm.tm_min = atoi(timestamp + year_offset + 8);
        
        if (len > year_offset + 10) {
            tm.tm_sec = atoi(timestamp + year_offset + 10);
        }
        
        return mktime(&tm);
    }
    
    return 0;
}

// 获取参考文件时间
time_t get_reference_time(const char *ref_file) {
    struct stat st;
    if (stat(ref_file, &st) == 0) {
        return st.st_mtime;
    }
    return 0;
}

// 处理单个文件
int process_file(const char *filename, const struct options *opts) {
    struct stat st;
    struct utimbuf ut;
    time_t current_time = time(NULL);
    time_t access_time = current_time;
    time_t modify_time = current_time;
    int file_exists = (stat(filename, &st) == 0);
    
    // 确定时间戳
    if (opts->reference && opts->ref_file) {
        time_t ref_time = get_reference_time(opts->ref_file);
        if (ref_time > 0) {
            access_time = modify_time = ref_time;
        }
    } else if (opts->custom_time > 0) {
        access_time = modify_time = opts->custom_time;
    } else if (file_exists) {
        // 使用现有文件的时间
        access_time = st.st_atime;
        modify_time = st.st_mtime;
    }
    
    // 设置时间
    if (opts->access_time && !opts->modify_time) {
        // 只更改访问时间
        modify_time = file_exists ? st.st_mtime : current_time;
    } else if (opts->modify_time && !opts->access_time) {
        // 只更改修改时间
        access_time = file_exists ? st.st_atime : current_time;
    }
    
    ut.actime = access_time;
    ut.modtime = modify_time;
    
    // 创建目录（如果需要）
    if (opts->create_dirs) {
        char *dir_path = strdup(filename);
        char *last_slash = strrchr(dir_path, '/');
        if (last_slash) {
            *last_slash = '\0';
            if (create_directories(dir_path) != 0) {
                free(dir_path);
                return 1;
            }
        }
        free(dir_path);
    }
    
    // 执行操作
    if (opts->dry_run) {
        if (opts->verbose) {
            printf("%s[模拟] %s%s %s%s\n", COLOR_YELLOW, opts->separator, filename, 
                   file_exists ? "更新" : "创建", COLOR_RESET);
        }
        return 0;
    }
    
    if (file_exists) {
        // 更新现有文件
        if (utime(filename, &ut) != 0) {
            fprintf(stderr, "%s错误: 无法更新时间戳 '%s': %s%s\n", 
                    COLOR_RED, filename, strerror(errno), COLOR_RESET);
            return 1;
        }
        
        if (opts->verbose) {
            printf("%s%s %s%s %s\n", COLOR_GREEN, opts->separator, filename, "已更新", COLOR_RESET);
        }
    } else {
        // 创建新文件
        if (opts->no_create) {
            if (opts->verbose) {
                printf("%s%s %s%s %s\n", COLOR_YELLOW, opts->separator, filename, "不存在，跳过", COLOR_RESET);
            }
            return 0;
        }
        
        FILE *file = fopen(filename, "w");
        if (!file) {
            fprintf(stderr, "%s错误: 无法创建文件 '%s': %s%s\n", 
                    COLOR_RED, filename, strerror(errno), COLOR_RESET);
            return 1;
        }
        fclose(file);
        
        // 设置时间戳
        if (utime(filename, &ut) != 0) {
            fprintf(stderr, "%s警告: 无法设置时间戳 '%s': %s%s\n", 
                    COLOR_YELLOW, filename, strerror(errno), COLOR_RESET);
        }
        
        // 设置权限
        if (chmod(filename, opts->file_mode) != 0) {
            fprintf(stderr, "%s警告: 无法设置权限 '%s': %s%s\n", 
                    COLOR_YELLOW, filename, strerror(errno), COLOR_RESET);
        }
        
        if (opts->verbose) {
            printf("%s%s %s%s %s\n", COLOR_GREEN, opts->separator, filename, "已创建", COLOR_RESET);
        }
    }
    
    return 0;
}

int main(int argc, char *argv[]) {
    struct options opts;
    int result = 0;
    int file_count = 0;
    
    init_options(&opts);
    
    static struct option long_options[] = {
        {"no-create", no_argument, 0, 'c'},
        {"date", required_argument, 0, 'd'},
        {"reference", required_argument, 0, 'r'},
        {"create-dirs", no_argument, 0, 1},
        {"mode", required_argument, 0, 2},
        {"color", no_argument, 0, 3},
        {"no-color", no_argument, 0, 4},
        {"progress", no_argument, 0, 5},
        {"verbose", no_argument, 0, 6},
        {"dry-run", no_argument, 0, 7},
        {"separator", required_argument, 0, 8},
        {"help", no_argument, 0, 'h'},
        {"version", no_argument, 0, 'V'},
        {0, 0, 0, 0}
    };
    
    int opt;
    int option_index = 0;
    
    while ((opt = getopt_long(argc, argv, "acd:mr:t:hV", long_options, &option_index)) != -1) {
        switch (opt) {
            case 'a':
                opts.access_time = 1;
                break;
            case 'c':
                opts.no_create = 1;
                break;
            case 'd': {
                // 解析日期字符串
                struct tm tm = {0};
                if (strptime(optarg, "%Y-%m-%d %H:%M:%S", &tm) ||
                    strptime(optarg, "%Y-%m-%d", &tm) ||
                    strptime(optarg, "%H:%M:%S", &tm)) {
                    opts.custom_time = mktime(&tm);
                } else {
                    fprintf(stderr, "%s错误: 无法解析日期 '%s'%s\n", COLOR_RED, optarg, COLOR_RESET);
                    return 1;
                }
                break;
            }
            case 'm':
                opts.modify_time = 1;
                break;
            case 'r':
                opts.reference = 1;
                opts.ref_file = optarg;
                break;
            case 't': {
                opts.custom_time = parse_timestamp(optarg);
                if (opts.custom_time == 0) {
                    fprintf(stderr, "%s错误: 无法解析时间戳 '%s'%s\n", COLOR_RED, optarg, COLOR_RESET);
                    return 1;
                }
                break;
            }
            case 1: // --create-dirs
                opts.create_dirs = 1;
                break;
            case 2: // --mode
                opts.file_mode = strtol(optarg, NULL, 8);
                break;
            case 3: // --color
                opts.color = 1;
                break;
            case 4: // --no-color
                opts.color = 0;
                break;
            case 5: // --progress
                opts.show_progress = 1;
                break;
            case 6: // --verbose
                opts.verbose = 1;
                break;
            case 7: // --dry-run
                opts.dry_run = 1;
                break;
            case 8: // --separator
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
    
    // 检查参数
    if (optind >= argc) {
        fprintf(stderr, "%s错误: 必须指定至少一个文件%s\n", COLOR_RED, COLOR_RESET);
        print_help();
        return 1;
    }
    
    file_count = argc - optind;
    
    // 处理文件
    for (int i = optind; i < argc; i++) {
        if (opts.show_progress) {
            show_progress(i - optind + 1, file_count);
        }
        
        if (process_file(argv[i], &opts) != 0) {
            result = 1;
        }
    }
    
    if (opts.show_progress) {
        printf("\n");
    }
    
    return result;
}
