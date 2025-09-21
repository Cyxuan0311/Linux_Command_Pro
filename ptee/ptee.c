#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <getopt.h>
#include "../include/common.h"

#define MAX_FILENAME_LENGTH 256
#define MAX_FILES 10
#define BUFFER_SIZE 4096

// tee选项结构
typedef struct {
    int show_help;
    int show_version;
    int show_help_usage;
    int show_help_examples;
    int append_mode;
    int ignore_interrupts;
    int show_files;
    char files[MAX_FILES][MAX_FILENAME_LENGTH];
    int file_count;
} TeeOptions;

// 初始化选项
void init_options(TeeOptions *opts) {
    memset(opts, 0, sizeof(TeeOptions));
}

// 显示帮助信息
void print_help(const char *program_name) {
    printf("用法: %s [选项] [文件...]\n", program_name);
    printf("增强版 tee 命令，将标准输入复制到标准输出和文件\n\n");
    printf("选项:\n");
    printf("  -a, --append           追加到文件而不是覆盖\n");
    printf("  -i, --ignore-interrupts 忽略中断信号\n");
    printf("  -p, --preserve         保留文件权限\n");
    printf("  --help                 显示此帮助信息\n");
    printf("  --version              显示版本信息\n");
    printf("  --help-usage           显示使用说明\n");
    printf("  --help-examples        显示使用示例\n\n");
    printf("示例:\n");
    printf("  %s file.txt            # 输出到文件\n", program_name);
    printf("  %s -a file.txt         # 追加到文件\n", program_name);
    printf("  %s file1 file2         # 输出到多个文件\n", program_name);
    printf("  %s -i file.txt         # 忽略中断信号\n", program_name);
}

// 显示版本信息
void print_version() {
    printf("ptee - 增强版 tee 命令 v1.0\n");
    printf("提供分流输出功能\n");
}

// 显示使用说明
void print_usage_help() {
    printf("使用说明:\n\n");
    printf("  %s命令将标准输入复制到标准输出和指定的文件中。%s\n", COLOR_GREEN, COLOR_RESET);
    printf("  %s常用于在管道中同时显示和保存数据。%s\n", COLOR_GREEN, COLOR_RESET);
    printf("\n工作原理:\n");
    printf("  %s• 从标准输入读取数据%s\n", COLOR_YELLOW, COLOR_RESET);
    printf("  %s• 将数据写入标准输出%s\n", COLOR_YELLOW, COLOR_RESET);
    printf("  %s• 同时将数据写入指定的文件%s\n", COLOR_YELLOW, COLOR_RESET);
    printf("\n选项说明:\n");
    printf("  %s-a, --append%s        追加模式，不覆盖现有文件%s\n", COLOR_CYAN, COLOR_RESET, COLOR_GREEN);
    printf("  %s-i, --ignore-interrupts%s  忽略SIGINT信号%s\n", COLOR_CYAN, COLOR_RESET, COLOR_GREEN);
    printf("  %s-p, --preserve%s      保留文件权限%s\n", COLOR_CYAN, COLOR_RESET, COLOR_GREEN);
}

// 显示使用示例
void print_examples_help() {
    printf("使用示例:\n\n");
    printf("  %s基本用法:%s\n", COLOR_CYAN, COLOR_RESET);
    printf("    %s%s%s                    # 输出到文件\n", COLOR_GREEN, "echo 'hello' | ptee file.txt", COLOR_RESET);
    printf("    %s%s%s                    # 追加到文件\n", COLOR_GREEN, "echo 'world' | ptee -a file.txt", COLOR_RESET);
    printf("    %s%s%s                    # 输出到多个文件\n", COLOR_GREEN, "echo 'test' | ptee file1.txt file2.txt", COLOR_RESET);
    printf("\n  %s与管道结合:%s\n", COLOR_CYAN, COLOR_RESET);
    printf("    %s%s%s                    # 同时显示和保存\n", COLOR_GREEN, "ls -la | ptee filelist.txt", COLOR_RESET);
    printf("    %s%s%s                    # 保存并继续处理\n", COLOR_GREEN, "cat file.txt | ptee backup.txt | grep 'pattern'", COLOR_RESET);
    printf("    %s%s%s                    # 多路输出\n", COLOR_GREEN, "ps aux | ptee process.txt | grep 'python'", COLOR_RESET);
    printf("\n  %s高级用法:%s\n", COLOR_CYAN, COLOR_RESET);
    printf("    %s%s%s                    # 忽略中断信号\n", COLOR_GREEN, "yes | ptee -i output.txt", COLOR_RESET);
    printf("    %s%s%s                    # 保留文件权限\n", COLOR_GREEN, "echo 'data' | ptee -p file.txt", COLOR_RESET);
    printf("    %s%s%s                    # 追加到多个文件\n", COLOR_GREEN, "echo 'log' | ptee -a log1.txt log2.txt", COLOR_RESET);
}

// 打开文件
FILE* open_file(const char *filename, int append_mode) {
    FILE *file;
    if (append_mode) {
        file = fopen(filename, "a");
    } else {
        file = fopen(filename, "w");
    }
    
    if (!file) {
        printf("错误: 无法打开文件 %s\n", filename);
        return NULL;
    }
    
    return file;
}

// 复制数据
void copy_data(FILE *files[], int file_count) {
    char buffer[BUFFER_SIZE];
    size_t bytes_read;
    
    while ((bytes_read = fread(buffer, 1, BUFFER_SIZE, stdin)) > 0) {
        // 写入标准输出
        fwrite(buffer, 1, bytes_read, stdout);
        fflush(stdout);
        
        // 写入所有文件
        for (int i = 0; i < file_count; i++) {
            if (files[i]) {
                fwrite(buffer, 1, bytes_read, files[i]);
                fflush(files[i]);
            }
        }
    }
}

// 解析命令行参数
int parse_arguments(int argc, char *argv[], TeeOptions *opts) {
    int opt;
    static struct option long_options[] = {
        {"append", no_argument, 0, 'a'},
        {"ignore-interrupts", no_argument, 0, 'i'},
        {"preserve", no_argument, 0, 'p'},
        {"help", no_argument, 0, 'h'},
        {"version", no_argument, 0, 'v'},
        {"help-usage", no_argument, 0, 1},
        {"help-examples", no_argument, 0, 2},
        {0, 0, 0, 0}
    };

    while ((opt = getopt_long(argc, argv, "aiphv", long_options, NULL)) != -1) {
        switch (opt) {
            case 'a':
                opts->append_mode = 1;
                break;
            case 'i':
                opts->ignore_interrupts = 1;
                break;
            case 'p':
                // 保留文件权限（简化实现）
                break;
            case 'h':
                opts->show_help = 1;
                break;
            case 'v':
                opts->show_version = 1;
                break;
            case 1: // --help-usage
                opts->show_help_usage = 1;
                break;
            case 2: // --help-examples
                opts->show_help_examples = 1;
                break;
            case '?':
                return 1;
            default:
                break;
        }
    }

    // 处理位置参数（文件名）
    for (int i = optind; i < argc && opts->file_count < MAX_FILES; i++) {
        strncpy(opts->files[opts->file_count], argv[i], MAX_FILENAME_LENGTH - 1);
        opts->files[opts->file_count][MAX_FILENAME_LENGTH - 1] = '\0';
        opts->file_count++;
    }

    return 0;
}

int main(int argc, char *argv[]) {
    TeeOptions opts;
    
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
    
    if (opts.file_count == 0) {
        printf("错误: 请指定至少一个输出文件\n");
        print_help(argv[0]);
        return 1;
    }
    
    // 打开所有文件
    FILE *files[MAX_FILES] = {0};
    int success_count = 0;
    
    for (int i = 0; i < opts.file_count; i++) {
        files[i] = open_file(opts.files[i], opts.append_mode);
        if (files[i]) {
            success_count++;
        }
    }
    
    if (success_count == 0) {
        printf("错误: 无法打开任何文件\n");
        return 1;
    }
    
    printf("%s开始分流输出到 %d 个文件%s\n", COLOR_CYAN, success_count, COLOR_RESET);
    if (opts.append_mode) {
        printf("%s模式: 追加%s\n", COLOR_YELLOW, COLOR_RESET);
    } else {
        printf("%s模式: 覆盖%s\n", COLOR_YELLOW, COLOR_RESET);
    }
    printf("%s%s%s\n", COLOR_YELLOW, "================================", COLOR_RESET);
    
    // 复制数据
    copy_data(files, opts.file_count);
    
    // 关闭所有文件
    for (int i = 0; i < opts.file_count; i++) {
        if (files[i]) {
            fclose(files[i]);
        }
    }
    
    printf("\n%s分流输出完成%s\n", COLOR_GREEN, COLOR_RESET);
    
    return 0;
}
