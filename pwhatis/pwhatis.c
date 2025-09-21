#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <getopt.h>
#include "../include/common.h"

#define MAX_COMMAND_LENGTH 256

// whatis选项结构
typedef struct {
    int show_help;
    int show_version;
    int show_verbose;
    int show_wildcard;
    int show_exact;
    char command[MAX_COMMAND_LENGTH];
} WhatisOptions;

// 初始化选项
void init_options(WhatisOptions *opts) {
    memset(opts, 0, sizeof(WhatisOptions));
}

// 显示帮助信息
void print_help(const char *program_name) {
    printf("用法: %s [选项] [命令...]\n", program_name);
    printf("增强版 whatis 命令，显示命令的简短描述\n\n");
    printf("选项:\n");
    printf("  -w, --wildcard        使用通配符匹配\n");
    printf("  -e, --exact           精确匹配\n");
    printf("  -v, --verbose         详细模式\n");
    printf("  -h, --help            显示此帮助信息\n");
    printf("  --version             显示版本信息\n\n");
    printf("示例:\n");
    printf("  %s ls                 # 显示ls的描述\n", program_name);
    printf("  %s -w \"*grep*\"        # 使用通配符搜索\n", program_name);
    printf("  %s -e ls              # 精确匹配ls\n", program_name);
    printf("  %s -v python          # 详细显示python信息\n", program_name);
}

// 显示版本信息
void print_version() {
    printf("pwhatis - 增强版 whatis 命令 v1.0\n");
    printf("提供命令描述查询功能\n");
}

// 执行系统命令
int execute_command(const char *command) {
    int status = system(command);
    return WEXITSTATUS(status);
}

// 显示命令描述
void show_description(const char *command, const WhatisOptions *opts) {
    char command_str[MAX_COMMAND_LENGTH];
    char *whatis_cmd = "whatis";
    
    // 构建whatis命令
    strcpy(command_str, whatis_cmd);
    
    if (opts->show_wildcard) {
        strcat(command_str, " -w");
    }
    if (opts->show_exact) {
        strcat(command_str, " -e");
    }
    
    strcat(command_str, " ");
    strcat(command_str, command);
    
    if (opts->show_verbose) {
        printf("%s命令描述: %s%s\n", COLOR_CYAN, command, COLOR_RESET);
        printf("%s%s%s\n", COLOR_YELLOW, "================================", COLOR_RESET);
    }
    
    execute_command(command_str);
}

// 解析命令行参数
int parse_arguments(int argc, char *argv[], WhatisOptions *opts) {
    int opt;
    static struct option long_options[] = {
        {"wildcard", no_argument, 0, 'w'},
        {"exact", no_argument, 0, 'e'},
        {"verbose", no_argument, 0, 'v'},
        {"help", no_argument, 0, 'h'},
        {"version", no_argument, 0, 1},
        {0, 0, 0, 0}
    };

    while ((opt = getopt_long(argc, argv, "wevh", long_options, NULL)) != -1) {
        switch (opt) {
            case 'w':
                opts->show_wildcard = 1;
                break;
            case 'e':
                opts->show_exact = 1;
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
    WhatisOptions opts;
    
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
        printf("错误: 请指定要查询的命令\n");
        print_help(argv[0]);
        return 1;
    }
    
    int exit_code = 0;
    
    // 处理每个命令
    for (int i = optind; i < argc; i++) {
        show_description(argv[i], &opts);
        if (i < argc - 1) {
            printf("\n");
        }
    }
    
    return exit_code;
}
