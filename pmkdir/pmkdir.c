#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
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

// 全局选项
struct options {
    int parents;
    int verbose;
    int mode_set;
    int owner_set;
    int group_set;
    int color;
    int show_progress;
    int dry_run;
    int force;
    char *separator;
    mode_t mode;
    char *owner;
    char *group;
    uid_t uid;
    gid_t gid;
};

// 初始化选项
void init_options(struct options *opts) {
    opts->parents = 0;
    opts->verbose = 0;
    opts->mode_set = 0;
    opts->owner_set = 0;
    opts->group_set = 0;
    opts->color = 1;
    opts->show_progress = 0;
    opts->dry_run = 0;
    opts->force = 0;
    opts->separator = "==>";
    opts->mode = 0755;
    opts->owner = NULL;
    opts->group = NULL;
    opts->uid = 0;
    opts->gid = 0;
}

// 打印帮助信息
void print_help() {
    printf("%spmkdir - 优化版 mkdir 命令%s\n", COLOR_CYAN, COLOR_RESET);
    printf("%s==============================================%s\n", COLOR_YELLOW, COLOR_RESET);
    printf("用法: pmkdir [选项] 目录...\n\n");
    printf("选项:\n");
    printf("  -p, --parents           创建父目录（递归创建）\n");
    printf("  -m, --mode=MODE         设置目录权限 (八进制)\n");
    printf("  -o, --owner=OWNER       设置目录所有者\n");
    printf("  -g, --group=GROUP       设置目录组\n");
    printf("  -v, --verbose           详细输出\n");
    printf("  -f, --force             强制创建（忽略已存在错误）\n");
    printf("  --color                 启用彩色输出 (默认)\n");
    printf("  --no-color              禁用彩色输出\n");
    printf("  --progress              显示进度条\n");
    printf("  --dry-run               模拟运行，不实际创建\n");
    printf("  --separator=STR         设置文件分隔符 (默认: '==>')\n");
    printf("  -h, --help              显示此帮助信息\n");
    printf("  -V, --version           显示版本信息\n\n");
    printf("示例:\n");
    printf("  pmkdir dir1 dir2                    # 创建目录\n");
    printf("  pmkdir -p path/to/dir               # 递归创建目录\n");
    printf("  pmkdir -m 755 dir                   # 设置权限\n");
    printf("  pmkdir -o user -g group dir         # 设置所有者和组\n");
    printf("  pmkdir --verbose dir1 dir2          # 详细输出\n");
}

// 打印版本信息
void print_version() {
    printf("pmkdir - 优化版 mkdir 命令 v1.0\n");
    printf("使用C语言实现，添加递归创建和权限管理\n");
    printf("作者: pmkdir team\n");
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
    printf("%s] %d%% 创建中...", COLOR_CYAN, (int)((double)current / total * 100));
    fflush(stdout);
}

// 解析用户和组
int parse_owner_group(const char *owner_str, const char *group_str, uid_t *uid, gid_t *gid) {
    struct passwd *pw = NULL;
    struct group *gr = NULL;
    
    if (owner_str) {
        // 尝试解析为数字
        char *end;
        long uid_long = strtol(owner_str, &end, 10);
        if (*end == '\0') {
            *uid = (uid_t)uid_long;
        } else {
            // 解析为用户名
            pw = getpwnam(owner_str);
            if (!pw) {
                fprintf(stderr, "%s错误: 用户 '%s' 不存在%s\n", COLOR_RED, owner_str, COLOR_RESET);
                return 1;
            }
            *uid = pw->pw_uid;
        }
    }
    
    if (group_str) {
        // 尝试解析为数字
        char *end;
        long gid_long = strtol(group_str, &end, 10);
        if (*end == '\0') {
            *gid = (gid_t)gid_long;
        } else {
            // 解析为组名
            gr = getgrnam(group_str);
            if (!gr) {
                fprintf(stderr, "%s错误: 组 '%s' 不存在%s\n", COLOR_RED, group_str, COLOR_RESET);
                return 1;
            }
            *gid = gr->gr_gid;
        }
    }
    
    return 0;
}

// 创建单个目录
int create_directory(const char *path, const struct options *opts) {
    struct stat st;
    int exists = (stat(path, &st) == 0);
    
    if (exists) {
        if (opts->force) {
            if (opts->verbose) {
                printf("%s%s %s%s %s\n", COLOR_YELLOW, opts->separator, path, "已存在，跳过", COLOR_RESET);
            }
            return 0;
        } else {
            fprintf(stderr, "%s错误: 目录 '%s' 已存在%s\n", COLOR_RED, path, COLOR_RESET);
            return 1;
        }
    }
    
    if (opts->dry_run) {
        if (opts->verbose) {
            printf("%s[模拟] %s%s %s%s\n", COLOR_YELLOW, opts->separator, path, "将创建", COLOR_RESET);
        }
        return 0;
    }
    
    // 创建目录
    if (mkdir(path, opts->mode) != 0) {
        fprintf(stderr, "%s错误: 无法创建目录 '%s': %s%s\n", 
                COLOR_RED, path, strerror(errno), COLOR_RESET);
        return 1;
    }
    
    // 设置所有者和组
    if (opts->owner_set || opts->group_set) {
        if (chown(path, opts->uid, opts->gid) != 0) {
            fprintf(stderr, "%s警告: 无法设置所有者/组 '%s': %s%s\n", 
                    COLOR_YELLOW, path, strerror(errno), COLOR_RESET);
        }
    }
    
    if (opts->verbose) {
        printf("%s%s %s%s %s\n", COLOR_GREEN, opts->separator, path, "已创建", COLOR_RESET);
    }
    
    return 0;
}

// 递归创建目录
int create_directories_recursive(const char *path, const struct options *opts) {
    char *path_copy = strdup(path);
    char *p = path_copy;
    int result = 0;
    
    // 跳过开头的 '/'
    if (*p == '/') p++;
    
    while (*p) {
        if (*p == '/') {
            *p = '\0';
            if (strlen(path_copy) > 0) {
                if (create_directory(path_copy, opts) != 0) {
                    result = 1;
                    break;
                }
            }
            *p = '/';
        }
        p++;
    }
    
    // 创建最终目录
    if (result == 0) {
        result = create_directory(path, opts);
    }
    
    free(path_copy);
    return result;
}

// 处理单个目录
int process_directory(const char *dirname, const struct options *opts) {
    if (opts->parents) {
        return create_directories_recursive(dirname, opts);
    } else {
        return create_directory(dirname, opts);
    }
}

int main(int argc, char *argv[]) {
    struct options opts;
    int result = 0;
    int dir_count = 0;
    
    init_options(&opts);
    
    static struct option long_options[] = {
        {"parents", no_argument, 0, 'p'},
        {"mode", required_argument, 0, 'm'},
        {"owner", required_argument, 0, 'o'},
        {"group", required_argument, 0, 'g'},
        {"verbose", no_argument, 0, 'v'},
        {"force", no_argument, 0, 'f'},
        {"color", no_argument, 0, 1},
        {"no-color", no_argument, 0, 2},
        {"progress", no_argument, 0, 3},
        {"dry-run", no_argument, 0, 4},
        {"separator", required_argument, 0, 5},
        {"help", no_argument, 0, 'h'},
        {"version", no_argument, 0, 'V'},
        {0, 0, 0, 0}
    };
    
    int opt;
    int option_index = 0;
    
    while ((opt = getopt_long(argc, argv, "pm:o:g:vfhV", long_options, &option_index)) != -1) {
        switch (opt) {
            case 'p':
                opts.parents = 1;
                break;
            case 'm':
                opts.mode = strtol(optarg, NULL, 8);
                opts.mode_set = 1;
                break;
            case 'o':
                opts.owner = optarg;
                opts.owner_set = 1;
                break;
            case 'g':
                opts.group = optarg;
                opts.group_set = 1;
                break;
            case 'v':
                opts.verbose = 1;
                break;
            case 'f':
                opts.force = 1;
                break;
            case 1: // --color
                opts.color = 1;
                break;
            case 2: // --no-color
                opts.color = 0;
                break;
            case 3: // --progress
                opts.show_progress = 1;
                break;
            case 4: // --dry-run
                opts.dry_run = 1;
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
    
    // 检查参数
    if (optind >= argc) {
        fprintf(stderr, "%s错误: 必须指定至少一个目录%s\n", COLOR_RED, COLOR_RESET);
        print_help();
        return 1;
    }
    
    // 解析所有者和组
    if (parse_owner_group(opts.owner, opts.group, &opts.uid, &opts.gid) != 0) {
        return 1;
    }
    
    dir_count = argc - optind;
    
    // 处理目录
    for (int i = optind; i < argc; i++) {
        if (opts.show_progress) {
            show_progress(i - optind + 1, dir_count);
        }
        
        if (process_directory(argv[i], &opts) != 0) {
            result = 1;
        }
    }
    
    if (opts.show_progress) {
        printf("\n");
    }
    
    return result;
}
