#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <getopt.h>
#include "../include/common.h"

#define MAX_PATH_LENGTH 4096
#define MAX_COMMANDS 100

// which选项结构
typedef struct {
    int show_all;
    int show_version;
    int show_help;
    int show_aliases;
    int show_functions;
    int show_builtins;
    int quiet;
    int verbose;
} WhichOptions;

// 初始化选项
void init_options(WhichOptions *opts) {
    memset(opts, 0, sizeof(WhichOptions));
}

// 显示帮助信息
void print_help(const char *program_name) {
    printf("用法: %s [选项] [命令...]\n", program_name);
    printf("增强版 which 命令，查找命令的完整路径\n\n");
    printf("选项:\n");
    printf("  -a, --all             显示所有匹配的路径\n");
    printf("  -i, --read-alias      读取别名\n");
    printf("  -f, --read-functions  读取函数\n");
    printf("  -b, --read-builtins   读取内置命令\n");
    printf("  -q, --quiet           静默模式\n");
    printf("  -v, --verbose         详细模式\n");
    printf("  -h, --help            显示此帮助信息\n");
    printf("  --version             显示版本信息\n\n");
    printf("示例:\n");
    printf("  %s ls                 # 查找ls命令路径\n", program_name);
    printf("  %s -a gcc             # 显示所有gcc路径\n", program_name);
    printf("  %s -v python          # 详细显示python信息\n", program_name);
    printf("  %s -q bash            # 静默查找bash\n", program_name);
}

// 显示版本信息
void print_version() {
    printf("pwhich - 增强版 which 命令 v1.0\n");
    printf("提供强大的命令查找功能\n");
}

// 检查文件是否存在且可执行
int is_executable(const char *path) {
    struct stat st;
    if (stat(path, &st) == 0) {
        return S_ISREG(st.st_mode) && (st.st_mode & S_IXUSR);
    }
    return 0;
}

// 在PATH中查找命令
int find_command_in_path(const char *command, char *result, int show_all) {
    char *path_env = getenv("PATH");
    if (!path_env) {
        return 0;
    }
    
    char *path_copy = strdup(path_env);
    char *dir = strtok(path_copy, ":");
    int found = 0;
    
    while (dir != NULL) {
        char full_path[MAX_PATH_LENGTH];
        snprintf(full_path, sizeof(full_path), "%s/%s", dir, command);
        
        if (is_executable(full_path)) {
            strcpy(result, full_path);
            found = 1;
            if (!show_all) {
                break;
            }
            if (show_all) {
                printf("%s%s%s\n", COLOR_GREEN, full_path, COLOR_RESET);
            }
        }
        
        dir = strtok(NULL, ":");
    }
    
    free(path_copy);
    return found;
}

// 查找命令
int find_command(const char *command, const WhichOptions *opts) {
    char result[MAX_PATH_LENGTH];
    int found = 0;
    
    if (opts->verbose) {
        printf("%s查找命令: %s%s\n", COLOR_CYAN, command, COLOR_RESET);
    }
    
    // 在PATH中查找
    if (find_command_in_path(command, result, opts->show_all)) {
        if (!opts->show_all) {
            printf("%s%s%s\n", COLOR_GREEN, result, COLOR_RESET);
        }
        found = 1;
    }
    
    if (!found) {
        if (!opts->quiet) {
            printf("%s%s: 未找到命令%s\n", COLOR_RED, command, COLOR_RESET);
        }
        return 1;
    }
    
    return 0;
}

// 解析命令行参数
int parse_arguments(int argc, char *argv[], WhichOptions *opts) {
    int opt;
    static struct option long_options[] = {
        {"all", no_argument, 0, 'a'},
        {"read-alias", no_argument, 0, 'i'},
        {"read-functions", no_argument, 0, 'f'},
        {"read-builtins", no_argument, 0, 'b'},
        {"quiet", no_argument, 0, 'q'},
        {"verbose", no_argument, 0, 'v'},
        {"help", no_argument, 0, 'h'},
        {"version", no_argument, 0, 1},
        {0, 0, 0, 0}
    };

    while ((opt = getopt_long(argc, argv, "aifbqvh", long_options, NULL)) != -1) {
        switch (opt) {
            case 'a':
                opts->show_all = 1;
                break;
            case 'i':
                opts->show_aliases = 1;
                break;
            case 'f':
                opts->show_functions = 1;
                break;
            case 'b':
                opts->show_builtins = 1;
                break;
            case 'q':
                opts->quiet = 1;
                break;
            case 'v':
                opts->verbose = 1;
                break;
            case 'h':
                opts->show_help = 1;
                break;
            case 1: // --version
                opts->show_version = 1;
                break;
            case '?':
                return 1;
            default:
                break;
        }
    }

    return 0;
}

int main(int argc, char *argv[]) {
    WhichOptions opts;
    
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
    
    // 检查是否有命令参数
    if (optind >= argc) {
        if (!opts.quiet) {
            printf("错误: 请指定要查找的命令\n");
            print_help(argv[0]);
        }
        return 1;
    }
    
    int exit_code = 0;
    
    // 处理每个命令
    for (int i = optind; i < argc; i++) {
        if (find_command(argv[i], &opts) != 0) {
            exit_code = 1;
        }
    }
    
    return exit_code;
}
