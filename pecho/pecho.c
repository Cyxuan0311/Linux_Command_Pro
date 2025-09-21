#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include "../include/common.h"

#define MAX_LINE_LENGTH 4096

// echo选项结构
typedef struct {
    int no_newline;
    int enable_escape;
    int disable_escape;
    int show_help;
    int show_version;
    int show_help_escapes;
} EchoOptions;

// 初始化选项
void init_options(EchoOptions *opts) {
    memset(opts, 0, sizeof(EchoOptions));
    opts->enable_escape = 1; // 默认启用转义
}

// 显示帮助信息
void print_help(const char *program_name) {
    printf("用法: %s [选项] [字符串...]\n", program_name);
    printf("增强版 echo 命令，提供丰富的文本输出功能\n\n");
    printf("选项:\n");
    printf("  -n, --no-newline     不输出换行符\n");
    printf("  -e, --enable-escape   启用转义字符解释\n");
    printf("  -E, --disable-escape  禁用转义字符解释\n");
    printf("  --help-escapes       显示转义字符帮助\n");
    printf("  -h, --help           显示此帮助信息\n");
    printf("  -v, --version        显示版本信息\n\n");
    printf("转义字符:\n");
    printf("  \\\\    反斜杠\n");
    printf("  \\a    响铃 (BEL)\n");
    printf("  \\b    退格\n");
    printf("  \\c    不产生进一步输出\n");
    printf("  \\e    转义字符 (ESC)\n");
    printf("  \\f    换页\n");
    printf("  \\n    换行\n");
    printf("  \\r    回车\n");
    printf("  \\t    水平制表符\n");
    printf("  \\v    垂直制表符\n");
    printf("  \\0NNN 八进制值 NNN (1-3位数字)\n");
    printf("  \\xHH  十六进制值 HH (1-2位数字)\n\n");
    printf("示例:\n");
    printf("  %s Hello World        # 输出文本\n", program_name);
    printf("  %s -n Hello           # 不换行输出\n", program_name);
    printf("  %s -e \"Hello\\nWorld\"  # 解释转义字符\n", program_name);
    printf("  %s --help-escapes     # 显示转义字符帮助\n", program_name);
}

// 显示版本信息
void print_version() {
    printf("pecho - 增强版 echo 命令 v1.0\n");
    printf("支持转义字符和彩色输出\n");
}

// 显示转义字符帮助
void print_escape_help() {
    printf("转义字符说明:\n\n");
    printf("  %s\\\\%s    反斜杠字符\n", COLOR_GREEN, COLOR_RESET);
    printf("  %s\\a%s    响铃 (ASCII 7)\n", COLOR_GREEN, COLOR_RESET);
    printf("  %s\\b%s    退格 (ASCII 8)\n", COLOR_GREEN, COLOR_RESET);
    printf("  %s\\c%s    不产生进一步输出\n", COLOR_GREEN, COLOR_RESET);
    printf("  %s\\e%s    转义字符 (ASCII 27)\n", COLOR_GREEN, COLOR_RESET);
    printf("  %s\\f%s    换页 (ASCII 12)\n", COLOR_GREEN, COLOR_RESET);
    printf("  %s\\n%s    换行 (ASCII 10)\n", COLOR_GREEN, COLOR_RESET);
    printf("  %s\\r%s    回车 (ASCII 13)\n", COLOR_GREEN, COLOR_RESET);
    printf("  %s\\t%s    水平制表符 (ASCII 9)\n", COLOR_GREEN, COLOR_RESET);
    printf("  %s\\v%s    垂直制表符 (ASCII 11)\n", COLOR_GREEN, COLOR_RESET);
    printf("  %s\\0NNN%s 八进制值 NNN (1-3位数字)\n", COLOR_GREEN, COLOR_RESET);
    printf("  %s\\xHH%s  十六进制值 HH (1-2位数字)\n", COLOR_GREEN, COLOR_RESET);
    printf("\n示例:\n");
    printf("  %s -e \"Hello\\nWorld\"%s     # 输出两行\n", COLOR_CYAN, COLOR_RESET);
    printf("  %s -e \"Hello\\tWorld\"%s     # 输出制表符分隔\n", COLOR_CYAN, COLOR_RESET);
    printf("  %s -e \"Hello\\x20World\"%s   # 使用十六进制空格\n", COLOR_CYAN, COLOR_RESET);
    printf("  %s -e \"Hello\\041\"%s        # 使用八进制感叹号\n", COLOR_CYAN, COLOR_RESET);
}

// 解析转义字符
int parse_escape_sequence(const char *input, char *output, int *pos) {
    int i = *pos;
    int j = 0;
    
    if (input[i] != '\\') {
        return 0;
    }
    
    i++; // 跳过反斜杠
    
    switch (input[i]) {
        case '\\':
            output[j++] = '\\';
            break;
        case 'a':
            output[j++] = '\a';
            break;
        case 'b':
            output[j++] = '\b';
            break;
        case 'c':
            // 不产生进一步输出
            *pos = i + 1;
            return -1;
        case 'e':
            output[j++] = '\033';
            break;
        case 'f':
            output[j++] = '\f';
            break;
        case 'n':
            output[j++] = '\n';
            break;
        case 'r':
            output[j++] = '\r';
            break;
        case 't':
            output[j++] = '\t';
            break;
        case 'v':
            output[j++] = '\v';
            break;
        case '0':
            // 八进制
            if (i + 1 < strlen(input) && input[i + 1] >= '0' && input[i + 1] <= '7') {
                int oct = 0;
                int digits = 0;
                while (i + 1 < strlen(input) && input[i + 1] >= '0' && input[i + 1] <= '7' && digits < 3) {
                    oct = oct * 8 + (input[i + 1] - '0');
                    i++;
                    digits++;
                }
                output[j++] = (char)oct;
            } else {
                output[j++] = '\0';
            }
            break;
        case 'x':
            // 十六进制
            if (i + 1 < strlen(input) && 
                ((input[i + 1] >= '0' && input[i + 1] <= '9') ||
                 (input[i + 1] >= 'a' && input[i + 1] <= 'f') ||
                 (input[i + 1] >= 'A' && input[i + 1] <= 'F'))) {
                int hex = 0;
                int digits = 0;
                while (i + 1 < strlen(input) && digits < 2) {
                    char c = input[i + 1];
                    int value = 0;
                    if (c >= '0' && c <= '9') {
                        value = c - '0';
                    } else if (c >= 'a' && c <= 'f') {
                        value = c - 'a' + 10;
                    } else if (c >= 'A' && c <= 'F') {
                        value = c - 'A' + 10;
                    } else {
                        break;
                    }
                    hex = hex * 16 + value;
                    i++;
                    digits++;
                }
                output[j++] = (char)hex;
            } else {
                output[j++] = 'x';
            }
            break;
        default:
            // 未知转义字符，保持原样
            output[j++] = '\\';
            output[j++] = input[i];
            break;
    }
    
    *pos = i + 1;
    return j;
}

// 处理字符串
void process_string(const char *input, const EchoOptions *opts) {
    char output[MAX_LINE_LENGTH];
    int input_len = strlen(input);
    int output_pos = 0;
    int input_pos = 0;
    
    while (input_pos < input_len && output_pos < MAX_LINE_LENGTH - 1) {
        if (opts->enable_escape && input[input_pos] == '\\') {
            char escape_output[16];
            int escape_len = parse_escape_sequence(input, escape_output, &input_pos);
            
            if (escape_len == -1) {
                // \c 特殊处理，不产生进一步输出
                break;
            }
            
            for (int i = 0; i < escape_len && output_pos < MAX_LINE_LENGTH - 1; i++) {
                output[output_pos++] = escape_output[i];
            }
        } else {
            output[output_pos++] = input[input_pos++];
        }
    }
    
    output[output_pos] = '\0';
    
    // 输出结果
    printf("%s", output);
}

// 解析命令行参数
int parse_arguments(int argc, char *argv[], EchoOptions *opts) {
    int opt;
    static struct option long_options[] = {
        {"no-newline", no_argument, 0, 'n'},
        {"enable-escape", no_argument, 0, 'e'},
        {"disable-escape", no_argument, 0, 'E'},
        {"help-escapes", no_argument, 0, 1},
        {"help", no_argument, 0, 'h'},
        {"version", no_argument, 0, 'v'},
        {0, 0, 0, 0}
    };

    while ((opt = getopt_long(argc, argv, "neEhv", long_options, NULL)) != -1) {
        switch (opt) {
            case 'n':
                opts->no_newline = 1;
                break;
            case 'e':
                opts->enable_escape = 1;
                opts->disable_escape = 0;
                break;
            case 'E':
                opts->disable_escape = 1;
                opts->enable_escape = 0;
                break;
            case 1: // --help-escapes
                opts->show_help_escapes = 1;
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

    return 0;
}

int main(int argc, char *argv[]) {
    EchoOptions opts;
    
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
    
    if (opts.show_help_escapes) {
        print_escape_help();
        return 0;
    }
    
    // 处理参数
    int first_arg = 1;
    for (int i = 1; i < argc; i++) {
        if (argv[i][0] == '-' && argv[i][1] != '\0') {
            // 这是选项，跳过
            continue;
        }
        
        if (first_arg) {
            first_arg = 0;
        } else {
            printf(" ");
        }
        
        process_string(argv[i], &opts);
    }
    
    if (!opts.no_newline) {
        printf("\n");
    }
    
    return 0;
}
