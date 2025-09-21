#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>
#include "../include/common.h"

#define MAX_LINE_LENGTH 1024
#define TAB_WIDTH 4

// 语法高亮颜色定义
#define KEYWORD_COLOR    COLOR_BLUE
#define STRING_COLOR     COLOR_GREEN
#define COMMENT_COLOR    COLOR_YELLOW
#define NUMBER_COLOR     COLOR_MAGENTA
#define FUNCTION_COLOR   COLOR_CYAN
#define TYPE_COLOR       COLOR_RED

// C语言关键字
const char *c_keywords[] = {
    "auto", "break", "case", "char", "const", "continue", "default", "do",
    "double", "else", "enum", "extern", "float", "for", "goto", "if",
    "int", "long", "register", "return", "short", "signed", "sizeof", "static",
    "struct", "switch", "typedef", "union", "unsigned", "void", "volatile", "while",
    "NULL", "true", "false", "NULL"
};

// 检查是否为关键字
int is_keyword(const char *word) {
    for (int i = 0; i < 36; i++) {
        if (strcmp(word, c_keywords[i]) == 0) {
            return 1;
        }
    }
    return 0;
}

// 检查是否为数字
int is_number(const char *str) {
    if (*str == '\0') return 0;
    
    // 检查十六进制
    if (str[0] == '0' && (str[1] == 'x' || str[1] == 'X')) {
        for (int i = 2; str[i]; i++) {
            if (!((str[i] >= '0' && str[i] <= '9') || 
                  (str[i] >= 'a' && str[i] <= 'f') || 
                  (str[i] >= 'A' && str[i] <= 'F'))) {
                return 0;
            }
        }
        return 1;
    }
    
    // 检查普通数字
    for (int i = 0; str[i]; i++) {
        if (str[i] < '0' || str[i] > '9') {
            if (str[i] != '.' && str[i] != 'f' && str[i] != 'L' && str[i] != 'U') {
                return 0;
            }
        }
    }
    return 1;
}

// 检查是否为字符串
int is_string(const char *str) {
    return (str[0] == '"' && str[strlen(str)-1] == '"') ||
           (str[0] == '\'' && str[strlen(str)-1] == '\'');
}

// 检查是否为注释
int is_comment(const char *str) {
    return (str[0] == '/' && str[1] == '/') ||
           (str[0] == '/' && str[1] == '*');
}

// 检查是否为函数调用
int is_function_call(const char *str) {
    return strchr(str, '(') != NULL && strchr(str, ')') != NULL;
}

// 简单的语法高亮
void highlight_line(const char *line, int line_num, int show_numbers) {
    if (show_numbers) {
        printf("%s%4d%s | ", COLOR_CYAN, line_num, COLOR_RESET);
    }
    
    char *line_copy = strdup(line);
    char *token = strtok(line_copy, " \t\n");
    
    while (token != NULL) {
        if (is_comment(token)) {
            printf("%s%s%s ", COMMENT_COLOR, token, COLOR_RESET);
        } else if (is_string(token)) {
            printf("%s%s%s ", STRING_COLOR, token, COLOR_RESET);
        } else if (is_number(token)) {
            printf("%s%s%s ", NUMBER_COLOR, token, COLOR_RESET);
        } else if (is_keyword(token)) {
            printf("%s%s%s ", KEYWORD_COLOR, token, COLOR_RESET);
        } else if (is_function_call(token)) {
            printf("%s%s%s ", FUNCTION_COLOR, token, COLOR_RESET);
        } else {
            printf("%s ", token);
        }
        token = strtok(NULL, " \t\n");
    }
    
    free(line_copy);
    printf("\n");
}

// 显示文件内容
int display_file(const char *filename, int show_numbers, int show_ends) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        print_error("无法打开文件");
        return 1;
    }
    
    char line[MAX_LINE_LENGTH];
    int line_num = 1;
    
    printf("%s文件: %s%s\n", COLOR_CYAN, filename, COLOR_RESET);
    printf("%s%s%s\n", COLOR_YELLOW, "=" + strlen(filename) + 6, COLOR_RESET);
    
    while (fgets(line, sizeof(line), file)) {
        // 移除换行符
        line[strcspn(line, "\n")] = '\0';
        
        if (show_ends) {
            // 显示行结束符
            printf("%s%4d%s | %s$%s\n", 
                   COLOR_CYAN, line_num, COLOR_RESET, 
                   line, COLOR_YELLOW);
        } else {
            highlight_line(line, line_num, show_numbers);
        }
        line_num++;
    }
    
    fclose(file);
    return 0;
}

// 显示多个文件
int display_files(char *filenames[], int count, int show_numbers, int show_ends) {
    for (int i = 0; i < count; i++) {
        if (count > 1) {
            printf("\n");
        }
        if (display_file(filenames[i], show_numbers, show_ends) != 0) {
            return 1;
        }
    }
    return 0;
}

void print_usage(const char *program_name) {
    printf("用法: %s [选项] [文件...]\n", program_name);
    printf("优化版的 cat 命令，提供语法高亮和行号显示\n\n");
    printf("选项:\n");
    printf("  -n, --number        显示行号\n");
    printf("  -E, --show-ends     显示行结束符\n");
    printf("  -h, --help          显示此帮助信息\n");
    printf("  -v, --version       显示版本信息\n");
    printf("\n示例:\n");
    printf("  %s file.txt          # 显示文件内容\n", program_name);
    printf("  %s -n file.txt       # 显示带行号的内容\n", program_name);
    printf("  %s -E file.txt       # 显示行结束符\n", program_name);
    printf("  %s file1 file2       # 显示多个文件\n", program_name);
}

int main(int argc, char *argv[]) {
    int show_numbers = 0;
    int show_ends = 0;
    char *files[argc];
    int file_count = 0;
    
    // 解析命令行参数
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-n") == 0 || strcmp(argv[i], "--number") == 0) {
            show_numbers = 1;
        } else if (strcmp(argv[i], "-E") == 0 || strcmp(argv[i], "--show-ends") == 0) {
            show_ends = 1;
        } else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            print_usage(argv[0]);
            return 0;
        } else if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--version") == 0) {
            printf("pcat - 优化版 cat 命令 v1.0\n");
            return 0;
        } else if (argv[i][0] != '-') {
            files[file_count++] = argv[i];
        } else {
            printf("未知选项: %s\n", argv[i]);
            print_usage(argv[0]);
            return 1;
        }
    }
    
    // 如果没有指定文件，从标准输入读取
    if (file_count == 0) {
        char line[MAX_LINE_LENGTH];
        int line_num = 1;
        
        printf("%s从标准输入读取%s\n", COLOR_CYAN, COLOR_RESET);
        printf("%s%s%s\n", COLOR_YELLOW, "=" + 20, COLOR_RESET);
        
        while (fgets(line, sizeof(line), stdin)) {
            line[strcspn(line, "\n")] = '\0';
            highlight_line(line, line_num, show_numbers);
            line_num++;
        }
        return 0;
    }
    
    return display_files(files, file_count, show_numbers, show_ends);
}
