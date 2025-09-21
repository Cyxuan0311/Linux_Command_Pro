#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <getopt.h>
#include "../include/common.h"

#define MAX_LINE_LENGTH 4096
#define DEFAULT_MESSAGE "y"

// yes选项结构
typedef struct {
    int show_help;
    int show_version;
    int show_help_usage;
    int show_help_examples;
    int show_help_signals;
    int version_info;
    int help_info;
    char message[MAX_LINE_LENGTH];
} YesOptions;

// 全局变量用于信号处理
static volatile int running = 1;

// 初始化选项
void init_options(YesOptions *opts) {
    memset(opts, 0, sizeof(YesOptions));
    strcpy(opts->message, DEFAULT_MESSAGE);
}

// 显示帮助信息
void print_help(const char *program_name) {
    printf("用法: %s [选项] [字符串]\n", program_name);
    printf("增强版 yes 命令，重复输出字符串\n\n");
    printf("选项:\n");
    printf("  -v, --version          显示版本信息\n");
    printf("  -h, --help             显示此帮助信息\n");
    printf("  --help-usage           显示使用说明\n");
    printf("  --help-examples        显示使用示例\n");
    printf("  --help-signals         显示信号处理说明\n");
    printf("\n参数:\n");
    printf("  字符串    要重复输出的字符串 (默认: y)\n\n");
    printf("示例:\n");
    printf("  %s                     # 重复输出 y\n", program_name);
    printf("  %s hello               # 重复输出 hello\n", program_name);
    printf("  %s \"hello world\"       # 重复输出 hello world\n", program_name);
}

// 显示版本信息
void print_version() {
    printf("pyes - 增强版 yes 命令 v1.0\n");
    printf("提供重复输出功能\n");
}

// 显示使用说明
void print_usage_help() {
    printf("使用说明:\n\n");
    printf("  %s命令会无限重复输出指定的字符串，直到被中断。%s\n", COLOR_GREEN, COLOR_RESET);
    printf("  %s默认输出 'y'，常用于自动回答命令的确认提示。%s\n", COLOR_GREEN, COLOR_RESET);
    printf("\n常见用法:\n");
    printf("  %s%s%s                 # 自动回答 rm -i 的确认\n", COLOR_CYAN, "yes | rm -i file", COLOR_RESET);
    printf("  %s%s%s               # 自动回答 cp -i 的确认\n", COLOR_CYAN, "yes | cp -i src dest", COLOR_RESET);
    printf("  %s%s%s              # 自动回答 find -ok 的确认\n", COLOR_CYAN, "yes | find . -ok rm {}", COLOR_RESET);
    printf("\n注意事项:\n");
    printf("  %s• 使用 Ctrl+C 停止输出%s\n", COLOR_YELLOW, COLOR_RESET);
    printf("  %s• 输出会持续到程序被终止%s\n", COLOR_YELLOW, COLOR_RESET);
    printf("  %s• 可以通过管道重定向输出%s\n", COLOR_YELLOW, COLOR_RESET);
}

// 显示使用示例
void print_examples_help() {
    printf("使用示例:\n\n");
    printf("  %s基本用法:%s\n", COLOR_CYAN, COLOR_RESET);
    printf("    %s%s%s                    # 重复输出 y\n", COLOR_GREEN, "pyes", COLOR_RESET);
    printf("    %s%s%s                    # 重复输出 hello\n", COLOR_GREEN, "pyes hello", COLOR_RESET);
    printf("    %s%s%s                    # 重复输出 hello world\n", COLOR_GREEN, "pyes \"hello world\"", COLOR_RESET);
    printf("\n  %s与管道结合:%s\n", COLOR_CYAN, COLOR_RESET);
    printf("    %s%s%s                    # 自动回答确认\n", COLOR_GREEN, "pyes | rm -i file", COLOR_RESET);
    printf("    %s%s%s                    # 自动回答复制确认\n", COLOR_GREEN, "pyes | cp -i src dest", COLOR_RESET);
    printf("    %s%s%s                    # 自动回答查找确认\n", COLOR_GREEN, "pyes | find . -ok rm {}", COLOR_RESET);
    printf("\n  %s重定向输出:%s\n", COLOR_CYAN, COLOR_RESET);
    printf("    %s%s%s                    # 输出到文件\n", COLOR_GREEN, "pyes hello > output.txt", COLOR_RESET);
    printf("    %s%s%s                    # 限制输出行数\n", COLOR_GREEN, "pyes | head -n 100", COLOR_RESET);
    printf("\n  %s特殊用法:%s\n", COLOR_CYAN, COLOR_RESET);
    printf("    %s%s%s                    # 生成测试数据\n", COLOR_GREEN, "pyes \"test line\" | head -n 1000", COLOR_RESET);
    printf("    %s%s%s                    # 填充文件\n", COLOR_GREEN, "pyes \"x\" | head -c 1M > file", COLOR_RESET);
}

// 显示信号处理说明
void print_signals_help() {
    printf("信号处理说明:\n\n");
    printf("  %sSIGINT (Ctrl+C)%s     停止输出并退出程序\n", COLOR_GREEN, COLOR_RESET);
    printf("  %sSIGTERM%s             停止输出并退出程序\n", COLOR_GREEN, COLOR_RESET);
    printf("  %sSIGPIPE%s             当管道断开时退出程序\n", COLOR_GREEN, COLOR_RESET);
    printf("\n信号处理行为:\n");
    printf("  %s• 程序会优雅地处理中断信号%s\n", COLOR_YELLOW, COLOR_RESET);
    printf("  %s• 不会产生不完整的输出行%s\n", COLOR_YELLOW, COLOR_RESET);
    printf("  %s• 退出码为0表示正常退出%s\n", COLOR_YELLOW, COLOR_RESET);
    printf("\n示例:\n");
    printf("  %s%s%s                    # 按Ctrl+C停止\n", COLOR_CYAN, "pyes hello", COLOR_RESET);
    printf("  %s%s%s                    # 管道断开时自动停止\n", COLOR_CYAN, "pyes | head -n 10", COLOR_RESET);
}

// 信号处理函数
void signal_handler(int sig) {
    running = 0;
}

// 重复输出字符串
void repeat_output(const char *message) {
    // 设置信号处理
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    signal(SIGPIPE, signal_handler);
    
    printf("%s开始重复输出: %s%s\n", COLOR_CYAN, message, COLOR_RESET);
    printf("%s%s%s\n", COLOR_YELLOW, "================================", COLOR_RESET);
    
    while (running) {
        printf("%s\n", message);
        fflush(stdout);
        
        // 检查是否被信号中断
        if (!running) {
            break;
        }
    }
    
    printf("\n%s输出已停止%s\n", COLOR_YELLOW, COLOR_RESET);
}

// 解析命令行参数
int parse_arguments(int argc, char *argv[], YesOptions *opts) {
    int opt;
    static struct option long_options[] = {
        {"version", no_argument, 0, 'v'},
        {"help", no_argument, 0, 'h'},
        {"help-usage", no_argument, 0, 1},
        {"help-examples", no_argument, 0, 2},
        {"help-signals", no_argument, 0, 3},
        {0, 0, 0, 0}
    };

    while ((opt = getopt_long(argc, argv, "vh", long_options, NULL)) != -1) {
        switch (opt) {
            case 'v':
                opts->show_version = 1;
                break;
            case 'h':
                opts->show_help = 1;
                break;
            case 1: // --help-usage
                opts->show_help_usage = 1;
                break;
            case 2: // --help-examples
                opts->show_help_examples = 1;
                break;
            case 3: // --help-signals
                opts->show_help_signals = 1;
                break;
            case '?':
                return 1;
            default:
                break;
        }
    }

    // 处理位置参数
    if (optind < argc) {
        strncpy(opts->message, argv[optind], MAX_LINE_LENGTH - 1);
        opts->message[MAX_LINE_LENGTH - 1] = '\0';
    }

    return 0;
}

int main(int argc, char *argv[]) {
    YesOptions opts;
    
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
    
    if (opts.show_help_usage) {
        print_usage_help();
        return 0;
    }
    
    if (opts.show_help_examples) {
        print_examples_help();
        return 0;
    }
    
    if (opts.show_help_signals) {
        print_signals_help();
        return 0;
    }
    
    repeat_output(opts.message);
    
    return 0;
}
