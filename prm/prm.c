#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <time.h>
#include <pwd.h>
#include <grp.h>

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

#define MAX_PATH_LENGTH 4096
#define TRASH_DIR "/tmp/trash"

// 全局选项
struct options {
    int recursive;
    int force;
    int interactive;
    int verbose;
    int color;
    int show_progress;
    int dry_run;
    int trash;
    int restore;
    int list_trash;
    int empty_trash;
    char *separator;
    int confirm_all;
    int confirm_none;
};

// 初始化选项
void init_options(struct options *opts) {
    opts->recursive = 0;
    opts->force = 0;
    opts->interactive = 0;
    opts->verbose = 0;
    opts->color = 1;
    opts->show_progress = 0;
    opts->dry_run = 0;
    opts->trash = 0;
    opts->restore = 0;
    opts->list_trash = 0;
    opts->empty_trash = 0;
    opts->separator = "==>";
    opts->confirm_all = 0;
    opts->confirm_none = 0;
}

// 打印帮助信息
void print_help() {
    printf("%sprm - 优化版 rm 命令%s\n", COLOR_CYAN, COLOR_RESET);
    printf("%s==============================================%s\n", COLOR_YELLOW, COLOR_RESET);
    printf("用法: prm [选项] 文件...\n\n");
    printf("选项:\n");
    printf("  -r, -R, --recursive     递归删除目录\n");
    printf("  -f, --force             强制删除，不提示\n");
    printf("  -i, --interactive       交互式删除\n");
    printf("  -v, --verbose           详细输出\n");
    printf("  -t, --trash             移动到回收站\n");
    printf("  --restore               从回收站恢复\n");
    printf("  --list-trash            列出回收站内容\n");
    printf("  --empty-trash           清空回收站\n");
    printf("  --color                 启用彩色输出 (默认)\n");
    printf("  --no-color              禁用彩色输出\n");
    printf("  --progress              显示进度条\n");
    printf("  --dry-run               模拟运行，不实际删除\n");
    printf("  --separator=STR         设置文件分隔符 (默认: '==>')\n");
    printf("  -h, --help              显示此帮助信息\n");
    printf("  -V, --version           显示版本信息\n\n");
    printf("示例:\n");
    printf("  prm file.txt                    # 删除文件\n");
    printf("  prm -r dir/                     # 递归删除目录\n");
    printf("  prm -i file1 file2              # 交互式删除\n");
    printf("  prm -t file.txt                 # 移动到回收站\n");
    printf("  prm --restore file.txt          # 从回收站恢复\n");
    printf("  prm --list-trash                # 列出回收站内容\n");
}

// 打印版本信息
void print_version() {
    printf("prm - 优化版 rm 命令 v1.0\n");
    printf("使用C语言实现，添加安全删除和回收站功能\n");
    printf("作者: prm team\n");
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
    printf("%s] %d%% 删除中...", COLOR_CYAN, (int)((double)current / total * 100));
    fflush(stdout);
}

// 创建回收站目录
int create_trash_dir() {
    struct stat st;
    if (stat(TRASH_DIR, &st) != 0) {
        if (mkdir(TRASH_DIR, 0755) != 0) {
            fprintf(stderr, "%s错误: 无法创建回收站目录 '%s': %s%s\n", 
                    COLOR_RED, TRASH_DIR, strerror(errno), COLOR_RESET);
            return 1;
        }
    }
    return 0;
}

// 生成回收站中的文件名
void generate_trash_name(const char *original_path, char *trash_path, size_t size) {
    time_t now = time(NULL);
    char *basename = strrchr(original_path, '/');
    basename = basename ? basename + 1 : (char*)original_path;
    
    snprintf(trash_path, size, "%s/%ld_%s", TRASH_DIR, now, basename);
}

// 移动到回收站
int move_to_trash(const char *path) {
    char trash_path[MAX_PATH_LENGTH];
    generate_trash_name(path, trash_path, sizeof(trash_path));
    
    if (rename(path, trash_path) != 0) {
        fprintf(stderr, "%s错误: 无法移动到回收站 '%s': %s%s\n", 
                COLOR_RED, path, strerror(errno), COLOR_RESET);
        return 1;
    }
    
    return 0;
}

// 确认删除
int confirm_delete(const char *path, const struct options *opts) {
    if (opts->confirm_all) return 1;
    if (opts->confirm_none) return 0;
    
    printf("%s删除 '%s'? [y/N] ", COLOR_YELLOW, path);
    fflush(stdout);
    
    char response[10];
    if (fgets(response, sizeof(response), stdin)) {
        return (response[0] == 'y' || response[0] == 'Y');
    }
    
    return 0;
}

// 删除文件
int remove_file(const char *path, const struct options *opts) {
    struct stat st;
    
    if (stat(path, &st) != 0) {
        if (opts->force) {
            return 0; // 忽略不存在的文件
        }
        fprintf(stderr, "%s错误: 无法访问 '%s': %s%s\n", 
                COLOR_RED, path, strerror(errno), COLOR_RESET);
        return 1;
    }
    
    // 交互式确认
    if (opts->interactive && !opts->force) {
        if (!confirm_delete(path, opts)) {
            if (opts->verbose) {
                printf("%s%s %s%s %s\n", COLOR_YELLOW, opts->separator, path, "已跳过", COLOR_RESET);
            }
            return 0;
        }
    }
    
    if (opts->dry_run) {
        if (opts->verbose) {
            printf("%s[模拟] %s%s %s%s\n", COLOR_YELLOW, opts->separator, path, "将删除", COLOR_RESET);
        }
        return 0;
    }
    
    // 移动到回收站或直接删除
    int result;
    if (opts->trash) {
        result = move_to_trash(path);
        if (result == 0 && opts->verbose) {
            printf("%s%s %s%s %s\n", COLOR_GREEN, opts->separator, path, "已移动到回收站", COLOR_RESET);
        }
    } else {
        result = unlink(path);
        if (result != 0) {
            fprintf(stderr, "%s错误: 无法删除文件 '%s': %s%s\n", 
                    COLOR_RED, path, strerror(errno), COLOR_RESET);
        } else if (opts->verbose) {
            printf("%s%s %s%s %s\n", COLOR_GREEN, opts->separator, path, "已删除", COLOR_RESET);
        }
    }
    
    return result;
}

// 递归删除目录
int remove_directory_recursive(const char *path, const struct options *opts) {
    DIR *dir;
    struct dirent *entry;
    char full_path[MAX_PATH_LENGTH];
    int result = 0;
    
    dir = opendir(path);
    if (!dir) {
        fprintf(stderr, "%s错误: 无法打开目录 '%s': %s%s\n", 
                COLOR_RED, path, strerror(errno), COLOR_RESET);
        return 1;
    }
    
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }
        
        snprintf(full_path, sizeof(full_path), "%s/%s", path, entry->d_name);
        
        struct stat st;
        if (stat(full_path, &st) == 0) {
            if (S_ISDIR(st.st_mode)) {
                result = remove_directory_recursive(full_path, opts);
            } else {
                result = remove_file(full_path, opts);
            }
            
            if (result != 0) {
                break;
            }
        }
    }
    
    closedir(dir);
    
    // 删除空目录
    if (result == 0) {
        if (opts->trash) {
            result = move_to_trash(path);
        } else {
            result = rmdir(path);
            if (result != 0) {
                fprintf(stderr, "%s错误: 无法删除目录 '%s': %s%s\n", 
                        COLOR_RED, path, strerror(errno), COLOR_RESET);
            }
        }
        
        if (result == 0 && opts->verbose) {
            printf("%s%s %s%s %s\n", COLOR_GREEN, opts->separator, path, 
                   opts->trash ? "已移动到回收站" : "已删除", COLOR_RESET);
        }
    }
    
    return result;
}

// 处理单个文件/目录
int process_path(const char *path, const struct options *opts) {
    struct stat st;
    
    if (stat(path, &st) != 0) {
        if (opts->force) {
            return 0; // 忽略不存在的文件
        }
        fprintf(stderr, "%s错误: 无法访问 '%s': %s%s\n", 
                COLOR_RED, path, strerror(errno), COLOR_RESET);
        return 1;
    }
    
    if (S_ISDIR(st.st_mode)) {
        if (opts->recursive) {
            return remove_directory_recursive(path, opts);
        } else {
            fprintf(stderr, "%s错误: '%s' 是目录，使用 -r 选项递归删除%s\n", 
                    COLOR_RED, path, COLOR_RESET);
            return 1;
        }
    } else {
        return remove_file(path, opts);
    }
}

// 列出回收站内容
int list_trash() {
    DIR *dir;
    struct dirent *entry;
    struct stat st;
    char full_path[MAX_PATH_LENGTH];
    
    if (create_trash_dir() != 0) {
        return 1;
    }
    
    dir = opendir(TRASH_DIR);
    if (!dir) {
        fprintf(stderr, "%s错误: 无法打开回收站目录%s\n", COLOR_RED, COLOR_RESET);
        return 1;
    }
    
    printf("%s回收站内容:%s\n", COLOR_CYAN, COLOR_RESET);
    
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }
        
        snprintf(full_path, sizeof(full_path), "%s/%s", TRASH_DIR, entry->d_name);
        
        if (stat(full_path, &st) == 0) {
            printf("  %s%s%s\n", COLOR_WHITE, entry->d_name, COLOR_RESET);
        }
    }
    
    closedir(dir);
    return 0;
}

// 清空回收站
int empty_trash(const struct options *opts) {
    DIR *dir;
    struct dirent *entry;
    char full_path[MAX_PATH_LENGTH];
    int result = 0;
    
    if (create_trash_dir() != 0) {
        return 1;
    }
    
    dir = opendir(TRASH_DIR);
    if (!dir) {
        fprintf(stderr, "%s错误: 无法打开回收站目录%s\n", COLOR_RED, COLOR_RESET);
        return 1;
    }
    
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }
        
        snprintf(full_path, sizeof(full_path), "%s/%s", TRASH_DIR, entry->d_name);
        
        struct stat st;
        if (stat(full_path, &st) == 0) {
            if (S_ISDIR(st.st_mode)) {
                if (remove_directory_recursive(full_path, opts) != 0) {
                    result = 1;
                }
            } else {
                if (unlink(full_path) != 0) {
                    fprintf(stderr, "%s错误: 无法删除 '%s': %s%s\n", 
                            COLOR_RED, full_path, strerror(errno), COLOR_RESET);
                    result = 1;
                }
            }
        }
    }
    
    closedir(dir);
    
    if (result == 0) {
        printf("%s回收站已清空%s\n", COLOR_GREEN, COLOR_RESET);
    }
    
    return result;
}

int main(int argc, char *argv[]) {
    struct options opts;
    int result = 0;
    int path_count = 0;
    
    init_options(&opts);
    
    static struct option long_options[] = {
        {"recursive", no_argument, 0, 'r'},
        {"force", no_argument, 0, 'f'},
        {"interactive", no_argument, 0, 'i'},
        {"verbose", no_argument, 0, 'v'},
        {"trash", no_argument, 0, 't'},
        {"restore", no_argument, 0, 1},
        {"list-trash", no_argument, 0, 2},
        {"empty-trash", no_argument, 0, 3},
        {"color", no_argument, 0, 4},
        {"no-color", no_argument, 0, 5},
        {"progress", no_argument, 0, 6},
        {"dry-run", no_argument, 0, 7},
        {"separator", required_argument, 0, 8},
        {"help", no_argument, 0, 'h'},
        {"version", no_argument, 0, 'V'},
        {0, 0, 0, 0}
    };
    
    int opt;
    int option_index = 0;
    
    while ((opt = getopt_long(argc, argv, "rfivthV", long_options, &option_index)) != -1) {
        switch (opt) {
            case 'r':
            case 'R':
                opts.recursive = 1;
                break;
            case 'f':
                opts.force = 1;
                break;
            case 'i':
                opts.interactive = 1;
                break;
            case 'v':
                opts.verbose = 1;
                break;
            case 't':
                opts.trash = 1;
                break;
            case 1: // --restore
                opts.restore = 1;
                break;
            case 2: // --list-trash
                opts.list_trash = 1;
                break;
            case 3: // --empty-trash
                opts.empty_trash = 1;
                break;
            case 4: // --color
                opts.color = 1;
                break;
            case 5: // --no-color
                opts.color = 0;
                break;
            case 6: // --progress
                opts.show_progress = 1;
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
    
    // 处理特殊选项
    if (opts.list_trash) {
        return list_trash();
    }
    
    if (opts.empty_trash) {
        return empty_trash(&opts);
    }
    
    // 检查参数
    if (optind >= argc) {
        fprintf(stderr, "%s错误: 必须指定至少一个文件或目录%s\n", COLOR_RED, COLOR_RESET);
        print_help();
        return 1;
    }
    
    path_count = argc - optind;
    
    // 处理文件/目录
    for (int i = optind; i < argc; i++) {
        if (opts.show_progress) {
            show_progress(i - optind + 1, path_count);
        }
        
        if (process_path(argv[i], &opts) != 0) {
            result = 1;
        }
    }
    
    if (opts.show_progress) {
        printf("\n");
    }
    
    return result;
}
