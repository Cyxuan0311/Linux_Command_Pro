#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <getopt.h>
#include "../include/common.h"

#define MAX_COMMAND_LENGTH 256
#define MAX_PATH_LENGTH 4096

// whereis选项结构
typedef struct {
    int show_help;
    int show_version;
    int show_binaries;
    int show_manuals;
    int show_sources;
    int show_all;
    int show_verbose;
    char command[MAX_COMMAND_LENGTH];
} WhereisOptions;

// 初始化选项
void init_options(WhereisOptions *opts) {
    memset(opts, 0, sizeof(WhereisOptions));
    opts->show_binaries = 1;
    opts->show_manuals = 1;
    opts->show_sources = 1;
}

// 显示帮助信息
void print_help(const char *program_name) {
    printf("用法: %s [选项] [命令...]\n", program_name);
    printf("增强版 whereis 命令，查找命令的二进制文件、手册页和源代码\n\n");
    printf("选项:\n");
    printf("  -b, --binaries        只查找二进制文件\n");
    printf("  -m, --manuals         只查找手册页\n");
    printf("  -s, --sources         只查找源代码\n");
    printf("  -a, --all             查找所有类型\n");
    printf("  -v, --verbose         详细模式\n");
    printf("  -h, --help            显示此帮助信息\n");
    printf("  --version             显示版本信息\n\n");
    printf("示例:\n");
    printf("  %s ls                 # 查找ls的所有文件\n", program_name);
    printf("  %s -b gcc             # 只查找gcc的二进制文件\n", program_name);
    printf("  %s -m ls              # 只查找ls的手册页\n", program_name);
    printf("  %s -s gcc             # 只查找gcc的源代码\n", program_name);
    printf("  %s -v python          # 详细显示python信息\n", program_name);
}

// 显示版本信息
void print_version() {
    printf("pwhereis - 增强版 whereis 命令 v1.0\n");
    printf("提供强大的文件定位功能\n");
}

// 执行系统命令
int execute_command(const char *command) {
    int status = system(command);
    return WEXITSTATUS(status);
}

// 查找文件
void find_files(const char *command, const WhereisOptions *opts) {
    char command_str[MAX_COMMAND_LENGTH];
    char *whereis_cmd = "whereis";
    
    // 构建whereis命令
    strcpy(command_str, whereis_cmd);
    
    if (opts->show_binaries && !opts->show_manuals && !opts->show_sources) {
        strcat(command_str, " -b");
    } else if (opts->show_manuals && !opts->show_binaries && !opts->show_sources) {
        strcat(command_str, " -m");
    } else if (opts->show_sources && !opts->show_binaries && !opts->show_manuals) {
        strcat(command_str, " -s");
    }
    
    strcat(command_str, " ");
    strcat(command_str, command);
    
    if (opts->show_verbose) {
        printf("%s查找命令: %s%s\n", COLOR_CYAN, command, COLOR_RESET);
        printf("%s%s%s\n", COLOR_YELLOW, "================================", COLOR_RESET);
    }
    
    execute_command(command_str);
}

// 解析命令行参数
int parse_arguments(int argc, char *argv[], WhereisOptions *opts) {
    int opt;
    static struct option long_options[] = {
        {"binaries", no_argument, 0, 'b'},
        {"manuals", no_argument, 0, 'm'},
        {"sources", no_argument, 0, 's'},
        {"all", no_argument, 0, 'a'},
        {"verbose", no_argument, 0, 'v'},
        {"help", no_argument, 0, 'h'},
        {"version", no_argument, 0, 1},
        {0, 0, 0, 0}
    };

    while ((opt = getopt_long(argc, argv, "bmsavh", long_options, NULL)) != -1) {
        switch (opt) {
            case 'b':
                opts->show_binaries = 1;
                opts->show_manuals = 0;
                opts->show_sources = 0;
                break;
            case 'm':
                opts->show_manuals = 1;
                opts->show_binaries = 0;
                opts->show_sources = 0;
                break;
            case 's':
                opts->show_sources = 1;
                opts->show_binaries = 0;
                opts->show_manuals = 0;
                break;
            case 'a':
                opts->show_all = 1;
                break;
            case 'v':
                opts->show_verbose = 1;
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
    WhereisOptions opts;
    
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
        printf("错误: 请指定要查找的命令\n");
        print_help(argv[0]);
        return 1;
    }
    
    int exit_code = 0;
    
    // 处理每个命令
    for (int i = optind; i < argc; i++) {
        find_files(argv[i], &opts);
        if (i < argc - 1) {
            printf("\n");
        }
    }
    
    return exit_code;
}
