#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <getopt.h>
#include "../include/common.h"

#define MAX_LINE_LENGTH 4096
#define MAX_LINES 1000

// shuf选项结构
typedef struct {
    int show_help;
    int show_version;
    int echo_mode;
    int input_range;
    int head_count;
    int range_start;
    int range_end;
} ShufOptions;

// 初始化选项
void init_options(ShufOptions *opts) {
    memset(opts, 0, sizeof(ShufOptions));
    opts->head_count = -1;
}

// 显示帮助信息
void print_help(const char *program_name) {
    printf("用法: %s [选项] [文件]\n", program_name);
    printf("增强版 shuf 命令，随机打乱输入行\n\n");
    printf("选项:\n");
    printf("  -e, --echo            将每个ARG视为输入行\n");
    printf("  -i, --input-range=LO-HI  将LO到HI的每个数字视为输入行\n");
    printf("  -n, --head-count=COUNT   最多输出COUNT行\n");
    printf("  -h, --help             显示此帮助信息\n");
    printf("  -v, --version          显示版本信息\n\n");
    printf("示例:\n");
    printf("  %s file.txt            # 打乱文件行\n", program_name);
    printf("  %s -e A B C            # 打乱指定字符串\n", program_name);
    printf("  %s -i 1-10             # 打乱数字1-10\n", program_name);
    printf("  %s -n 5 file.txt       # 随机选择5行\n", program_name);
}

// 显示版本信息
void print_version() {
    printf("pshuf - 增强版 shuf 命令 v1.0\n");
    printf("提供随机排序和选择功能\n");
}

// 交换两个字符串
void swap_strings(char *a, char *b) {
    char temp[MAX_LINE_LENGTH];
    strcpy(temp, a);
    strcpy(a, b);
    strcpy(b, temp);
}

// Fisher-Yates洗牌算法
void shuffle_array(char lines[][MAX_LINE_LENGTH], int count) {
    srand(time(NULL));
    
    for (int i = count - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        swap_strings(lines[i], lines[j]);
    }
}

// 从文件读取行
int read_lines_from_file(const char *filename, char lines[][MAX_LINE_LENGTH]) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        printf("错误: 无法打开文件 %s\n", filename);
        return 0;
    }
    
    int count = 0;
    while (fgets(lines[count], MAX_LINE_LENGTH, file) && count < MAX_LINES) {
        // 移除换行符
        lines[count][strcspn(lines[count], "\n")] = '\0';
        count++;
    }
    
    fclose(file);
    return count;
}

// 从标准输入读取行
int read_lines_from_stdin(char lines[][MAX_LINE_LENGTH]) {
    int count = 0;
    while (fgets(lines[count], MAX_LINE_LENGTH, stdin) && count < MAX_LINES) {
        // 移除换行符
        lines[count][strcspn(lines[count], "\n")] = '\0';
        count++;
    }
    return count;
}

// 生成数字范围
int generate_number_range(int start, int end, char lines[][MAX_LINE_LENGTH]) {
    int count = 0;
    for (int i = start; i <= end && count < MAX_LINES; i++) {
        snprintf(lines[count], MAX_LINE_LENGTH, "%d", i);
        count++;
    }
    return count;
}

// 随机选择并输出
void random_select_and_output(char lines[][MAX_LINE_LENGTH], int count, const ShufOptions *opts) {
    shuffle_array(lines, count);
    
    int output_count = count;
    if (opts->head_count > 0 && opts->head_count < count) {
        output_count = opts->head_count;
    }
    
    for (int i = 0; i < output_count; i++) {
        printf("%s\n", lines[i]);
    }
}

// 解析命令行参数
int parse_arguments(int argc, char *argv[], ShufOptions *opts) {
    int opt;
    static struct option long_options[] = {
        {"echo", no_argument, 0, 'e'},
        {"input-range", required_argument, 0, 'i'},
        {"head-count", required_argument, 0, 'n'},
        {"help", no_argument, 0, 'h'},
        {"version", no_argument, 0, 'v'},
        {0, 0, 0, 0}
    };

    while ((opt = getopt_long(argc, argv, "ei:n:hv", long_options, NULL)) != -1) {
        switch (opt) {
            case 'e':
                opts->echo_mode = 1;
                break;
            case 'i':
                opts->input_range = 1;
                if (sscanf(optarg, "%d-%d", &opts->range_start, &opts->range_end) != 2) {
                    printf("错误: 无效的范围格式 %s\n", optarg);
                    return 1;
                }
                break;
            case 'n':
                opts->head_count = atoi(optarg);
                if (opts->head_count <= 0) {
                    printf("错误: 行数必须大于0\n");
                    return 1;
                }
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
    ShufOptions opts;
    
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
    
    char lines[MAX_LINES][MAX_LINE_LENGTH];
    int count = 0;
    
    if (opts.echo_mode) {
        // echo模式：从命令行参数读取
        for (int i = optind; i < argc; i++) {
            strncpy(lines[count], argv[i], MAX_LINE_LENGTH - 1);
            lines[count][MAX_LINE_LENGTH - 1] = '\0';
            count++;
        }
    } else if (opts.input_range) {
        // 数字范围模式
        count = generate_number_range(opts.range_start, opts.range_end, lines);
    } else if (optind < argc) {
        // 文件模式
        count = read_lines_from_file(argv[optind], lines);
    } else {
        // 标准输入模式
        count = read_lines_from_stdin(lines);
    }
    
    if (count == 0) {
        printf("错误: 没有输入数据\n");
        return 1;
    }
    
    random_select_and_output(lines, count, &opts);
    
    return 0;
}