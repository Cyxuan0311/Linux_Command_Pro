#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <getopt.h>
#include "../include/common.h"

#define MAX_COMMAND_LENGTH 256
#define MAX_SECTION_LENGTH 16

// man选项结构
typedef struct {
    int show_help;
    int show_version;
    int show_all_sections;
    int show_sections;
    int show_apropos;
    int show_whatis;
    int show_whereis;
    int show_help_sections;
    char section[MAX_SECTION_LENGTH];
    char command[MAX_COMMAND_LENGTH];
} ManOptions;

// 初始化选项
void init_options(ManOptions *opts) {
    memset(opts, 0, sizeof(ManOptions));
}

// 显示帮助信息
void print_help(const char *program_name) {
    printf("用法: %s [选项] [命令]\n", program_name);
    printf("增强版 man 命令，提供美观的手册页查看功能\n\n");
    printf("选项:\n");
    printf("  -s, --section=SECTION  指定手册页章节\n");
    printf("  -a, --all              显示所有匹配的手册页\n");
    printf("  -f, --whatis           显示命令的简短描述\n");
    printf("  -k, --apropos          搜索关键词\n");
    printf("  -w, --whereis          显示手册页位置\n");
    printf("  -l, --list-sections    列出所有章节\n");
    printf("  -h, --help             显示此帮助信息\n");
    printf("  -v, --version          显示版本信息\n\n");
    printf("章节说明:\n");
    printf("  1   用户命令\n");
    printf("  2   系统调用\n");
    printf("  3   库函数\n");
    printf("  4   设备文件\n");
    printf("  5   文件格式\n");
    printf("  6   游戏\n");
    printf("  7   杂项\n");
    printf("  8   系统管理命令\n\n");
    printf("示例:\n");
    printf("  %s ls                  # 查看ls命令手册\n", program_name);
    printf("  %s -s 2 open           # 查看open系统调用\n", program_name);
    printf("  %s -f ls               # 显示ls的简短描述\n", program_name);
    printf("  %s -k \"file system\"    # 搜索文件系统相关命令\n", program_name);
    printf("  %s -w ls               # 显示ls手册页位置\n", program_name);
}

// 显示版本信息
void print_version() {
    printf("pman - 增强版 man 命令 v1.0\n");
    printf("提供美观的手册页查看功能\n");
}

// 显示章节说明
void print_sections() {
    printf("手册页章节说明:\n\n");
    printf("  %s1%s   用户命令 - 普通用户可执行的命令\n", COLOR_GREEN, COLOR_RESET);
    printf("  %s2%s   系统调用 - 内核提供的系统调用\n", COLOR_GREEN, COLOR_RESET);
    printf("  %s3%s   库函数 - 标准C库函数\n", COLOR_GREEN, COLOR_RESET);
    printf("  %s4%s   设备文件 - 特殊设备文件\n", COLOR_GREEN, COLOR_RESET);
    printf("  %s5%s   文件格式 - 配置文件格式\n", COLOR_GREEN, COLOR_RESET);
    printf("  %s6%s   游戏 - 游戏和娱乐程序\n", COLOR_GREEN, COLOR_RESET);
    printf("  %s7%s   杂项 - 其他主题\n", COLOR_GREEN, COLOR_RESET);
    printf("  %s8%s   系统管理命令 - 系统管理员命令\n", COLOR_GREEN, COLOR_RESET);
    printf("\n使用示例:\n");
    printf("  %s -s 1 ls%s           # 查看ls用户命令\n", COLOR_CYAN, COLOR_RESET);
    printf("  %s -s 2 open%s         # 查看open系统调用\n", COLOR_CYAN, COLOR_RESET);
    printf("  %s -s 3 printf%s       # 查看printf库函数\n", COLOR_CYAN, COLOR_RESET);
}

// 执行系统命令
int execute_command(const char *command) {
    int status = system(command);
    return WEXITSTATUS(status);
}

// 查看手册页
void view_manual_page(const char *command, const char *section) {
    char command_str[MAX_COMMAND_LENGTH];
    
    if (strlen(section) > 0) {
        snprintf(command_str, sizeof(command_str), "man %s %s", section, command);
    } else {
        snprintf(command_str, sizeof(command_str), "man %s", command);
    }
    
    printf("%s正在查看手册页: %s%s\n", COLOR_CYAN, command, COLOR_RESET);
    if (strlen(section) > 0) {
        printf("%s章节: %s%s\n", COLOR_YELLOW, section, COLOR_RESET);
    }
    printf("%s%s%s\n", COLOR_YELLOW, "================================", COLOR_RESET);
    
    execute_command(command_str);
}

// 显示whatis信息
void show_whatis(const char *command) {
    char command_str[MAX_COMMAND_LENGTH];
    snprintf(command_str, sizeof(command_str), "whatis %s", command);
    
    printf("%s命令描述: %s%s\n", COLOR_CYAN, command, COLOR_RESET);
    printf("%s%s%s\n", COLOR_YELLOW, "================================", COLOR_RESET);
    
    execute_command(command_str);
}

// 搜索apropos
void search_apropos(const char *keyword) {
    char command_str[MAX_COMMAND_LENGTH];
    snprintf(command_str, sizeof(command_str), "apropos %s", keyword);
    
    printf("%s搜索结果: %s%s\n", COLOR_CYAN, keyword, COLOR_RESET);
    printf("%s%s%s\n", COLOR_YELLOW, "================================", COLOR_RESET);
    
    execute_command(command_str);
}

// 显示whereis信息
void show_whereis(const char *command) {
    char command_str[MAX_COMMAND_LENGTH];
    snprintf(command_str, sizeof(command_str), "whereis %s", command);
    
    printf("%s命令位置: %s%s\n", COLOR_CYAN, command, COLOR_RESET);
    printf("%s%s%s\n", COLOR_YELLOW, "================================", COLOR_RESET);
    
    execute_command(command_str);
}

// 列出所有章节
void list_sections() {
    printf("%s手册页章节列表:%s\n", COLOR_CYAN, COLOR_RESET);
    printf("%s%s%s\n", COLOR_YELLOW, "================================", COLOR_RESET);
    
    const char *sections[] = {
        "1", "2", "3", "4", "5", "6", "7", "8"
    };
    
    const char *descriptions[] = {
        "用户命令",
        "系统调用", 
        "库函数",
        "设备文件",
        "文件格式",
        "游戏",
        "杂项",
        "系统管理命令"
    };
    
    for (int i = 0; i < 8; i++) {
        printf("  %s%s%s - %s\n", COLOR_GREEN, sections[i], COLOR_RESET, descriptions[i]);
    }
}

// 解析命令行参数
int parse_arguments(int argc, char *argv[], ManOptions *opts) {
    int opt;
    static struct option long_options[] = {
        {"section", required_argument, 0, 's'},
        {"all", no_argument, 0, 'a'},
        {"whatis", no_argument, 0, 'f'},
        {"apropos", no_argument, 0, 'k'},
        {"whereis", no_argument, 0, 'w'},
        {"list-sections", no_argument, 0, 'l'},
        {"help", no_argument, 0, 'h'},
        {"version", no_argument, 0, 'v'},
        {0, 0, 0, 0}
    };

    while ((opt = getopt_long(argc, argv, "s:afkwlhv", long_options, NULL)) != -1) {
        switch (opt) {
            case 's':
                strncpy(opts->section, optarg, MAX_SECTION_LENGTH - 1);
                opts->section[MAX_SECTION_LENGTH - 1] = '\0';
                break;
            case 'a':
                opts->show_all_sections = 1;
                break;
            case 'f':
                opts->show_whatis = 1;
                break;
            case 'k':
                opts->show_apropos = 1;
                break;
            case 'w':
                opts->show_whereis = 1;
                break;
            case 'l':
                opts->show_sections = 1;
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
        strncpy(opts->command, argv[optind], MAX_COMMAND_LENGTH - 1);
        opts->command[MAX_COMMAND_LENGTH - 1] = '\0';
    }

    return 0;
}

int main(int argc, char *argv[]) {
    ManOptions opts;
    
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
    
    if (opts.show_sections) {
        list_sections();
        return 0;
    }
    
    if (strlen(opts.command) == 0) {
        printf("错误: 请指定要查看的命令\n");
        print_help(argv[0]);
        return 1;
    }
    
    if (opts.show_whatis) {
        show_whatis(opts.command);
    } else if (opts.show_apropos) {
        search_apropos(opts.command);
    } else if (opts.show_whereis) {
        show_whereis(opts.command);
    } else {
        view_manual_page(opts.command, opts.section);
    }
    
    return 0;
}
