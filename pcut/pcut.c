#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <errno.h>
#include <ctype.h>

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

#define MAX_LINE_LENGTH 4096
#define MAX_FIELDS 100

// 全局选项
struct options {
    char *delimiter;
    char *output_delimiter;
    int *fields;
    int field_count;
    int complement;
    int color;
    int show_field_numbers;
    int show_line_numbers;
    int auto_detect_delimiter;
    int verbose;
    char *separator;
};

// 初始化选项
void init_options(struct options *opts) {
    opts->delimiter = "\t";
    opts->output_delimiter = "\t";
    opts->fields = NULL;
    opts->field_count = 0;
    opts->complement = 0;
    opts->color = 1;
    opts->show_field_numbers = 0;
    opts->show_line_numbers = 0;
    opts->auto_detect_delimiter = 0;
    opts->verbose = 0;
    opts->separator = "==>";
}

// 打印帮助信息
void print_help() {
    printf("%spcut - 优化版 cut 命令%s\n", COLOR_CYAN, COLOR_RESET);
    printf("%s==============================================%s\n", COLOR_YELLOW, COLOR_RESET);
    printf("用法: pcut [选项] [文件...]\n\n");
    printf("选项:\n");
    printf("  -d, --delimiter=DELIM    使用DELIM作为字段分隔符 (默认: TAB)\n");
    printf("  -f, --fields=LIST        选择字段 (例如: 1,3,5-7)\n");
    printf("  --complement             选择未指定的字段\n");
    printf("  --output-delimiter=DELIM 使用DELIM作为输出分隔符\n");
    printf("  --auto-detect            自动检测分隔符\n");
    printf("  --color                  启用彩色输出 (默认)\n");
    printf("  --no-color               禁用彩色输出\n");
    printf("  --field-numbers          显示字段号\n");
    printf("  --line-numbers           显示行号\n");
    printf("  --verbose                详细输出\n");
    printf("  --separator=STR          设置文件分隔符 (默认: '==>')\n");
    printf("  -h, --help               显示此帮助信息\n");
    printf("  -V, --version            显示版本信息\n\n");
    printf("示例:\n");
    printf("  pcut -f1,3 file.txt              # 选择第1和第3字段\n");
    printf("  pcut -d: -f1,3 /etc/passwd       # 使用冒号分隔符\n");
    printf("  pcut --complement -f2-4 file.txt # 选择除2-4外的所有字段\n");
    printf("  pcut --auto-detect file.txt      # 自动检测分隔符\n");
    printf("  pcut --field-numbers file.txt    # 显示字段号\n");
}

// 打印版本信息
void print_version() {
    printf("pcut - 优化版 cut 命令 v1.0\n");
    printf("使用C语言实现，添加字段高亮和智能分隔符检测\n");
    printf("作者: pcut team\n");
}

// 解析字段列表
int parse_fields(const char *field_list, int **fields) {
    char *list = strdup(field_list);
    char *token;
    int count = 0;
    int *temp_fields = malloc(MAX_FIELDS * sizeof(int));
    
    if (!temp_fields) {
        free(list);
        return -1;
    }
    
    token = strtok(list, ",");
    while (token != NULL) {
        char *dash = strchr(token, '-');
        if (dash) {
            // 范围字段 (例如: 1-5)
            int start = atoi(token);
            int end = atoi(dash + 1);
            for (int i = start; i <= end; i++) {
                if (count < MAX_FIELDS) {
                    temp_fields[count++] = i;
                }
            }
        } else {
            // 单个字段
            if (count < MAX_FIELDS) {
                temp_fields[count++] = atoi(token);
            }
        }
        token = strtok(NULL, ",");
    }
    
    *fields = malloc(count * sizeof(int));
    if (*fields) {
        memcpy(*fields, temp_fields, count * sizeof(int));
    }
    
    free(temp_fields);
    free(list);
    return count;
}

// 自动检测分隔符
char detect_delimiter(const char *line) {
    int counts[256] = {0};
    int max_count = 0;
    char best_delimiter = '\t';
    
    // 统计各种分隔符的出现次数
    for (const char *p = line; *p; p++) {
        if (*p == '\t' || *p == ',' || *p == ';' || *p == '|' || *p == ':') {
            counts[(unsigned char)*p]++;
        }
    }
    
    // 找到出现次数最多的分隔符
    for (int i = 0; i < 256; i++) {
        if (counts[i] > max_count) {
            max_count = counts[i];
            best_delimiter = (char)i;
        }
    }
    
    return best_delimiter;
}

// 分割字符串
int split_string(const char *line, const char *delimiter, char **fields, int max_fields) {
    char *line_copy = strdup(line);
    char *token;
    int count = 0;
    
    token = strtok(line_copy, delimiter);
    while (token != NULL && count < max_fields) {
        fields[count] = strdup(token);
        count++;
        token = strtok(NULL, delimiter);
    }
    
    free(line_copy);
    return count;
}

// 检查字段是否被选中
int is_field_selected(int field_num, const int *fields, int field_count, int complement) {
    for (int i = 0; i < field_count; i++) {
        if (fields[i] == field_num) {
            return !complement;
        }
    }
    return complement;
}

// 高亮字段
void highlight_field(const char *field, int field_num, int is_selected) {
    if (is_selected) {
        if (field_num % 2 == 0) {
            printf("%s%s%s", COLOR_GREEN, field, COLOR_RESET);
        } else {
            printf("%s%s%s", COLOR_BLUE, field, COLOR_RESET);
        }
    } else {
        printf("%s%s%s", COLOR_WHITE, field, COLOR_RESET);
    }
}

// 处理单行
void process_line(const char *line, const struct options *opts, int line_num) {
    char *fields[MAX_FIELDS];
    char delimiter = opts->delimiter[0];
    
    // 自动检测分隔符
    if (opts->auto_detect_delimiter) {
        delimiter = detect_delimiter(line);
        if (opts->verbose) {
            printf("%s检测到分隔符: '%c'%s\n", COLOR_YELLOW, delimiter, COLOR_RESET);
        }
    }
    
    // 分割字段
    int field_count = split_string(line, &delimiter, fields, MAX_FIELDS);
    
    if (opts->show_line_numbers) {
        printf("%s%4d: %s", COLOR_CYAN, line_num, COLOR_RESET);
    }
    
    // 输出选中的字段
    int output_count = 0;
    for (int i = 1; i <= field_count; i++) {
        if (is_field_selected(i, opts->fields, opts->field_count, opts->complement)) {
            if (output_count > 0) {
                printf("%s", opts->output_delimiter);
            }
            
            if (opts->show_field_numbers) {
                printf("%s$%d=%s", COLOR_YELLOW, i, COLOR_RESET);
            }
            
            if (opts->color) {
                highlight_field(fields[i-1], i, 1);
            } else {
                printf("%s", fields[i-1]);
            }
            output_count++;
        }
    }
    
    printf("\n");
    
    // 清理内存
    for (int i = 0; i < field_count; i++) {
        free(fields[i]);
    }
}

// 处理文件
int process_file(const char *filename, const struct options *opts) {
    FILE *file;
    char line[MAX_LINE_LENGTH];
    int line_num = 0;
    
    if (strcmp(filename, "-") == 0) {
        file = stdin;
        filename = "标准输入";
    } else {
        file = fopen(filename, "r");
        if (!file) {
            fprintf(stderr, "%s错误: 无法打开文件 '%s': %s%s\n", 
                    COLOR_RED, filename, strerror(errno), COLOR_RESET);
            return 1;
        }
    }
    
    // 显示文件名（如果需要）
    if (opts->verbose) {
        printf("%s%s %s%s\n", COLOR_MAGENTA, opts->separator, filename, COLOR_RESET);
    }
    
    while (fgets(line, sizeof(line), file)) {
        line_num++;
        process_line(line, opts, line_num);
    }
    
    if (file != stdin) {
        fclose(file);
    }
    
    return 0;
}

int main(int argc, char *argv[]) {
    struct options opts;
    init_options(&opts);
    
    static struct option long_options[] = {
        {"delimiter", required_argument, 0, 'd'},
        {"fields", required_argument, 0, 'f'},
        {"complement", no_argument, 0, 1},
        {"output-delimiter", required_argument, 0, 2},
        {"auto-detect", no_argument, 0, 3},
        {"color", no_argument, 0, 4},
        {"no-color", no_argument, 0, 5},
        {"field-numbers", no_argument, 0, 6},
        {"line-numbers", no_argument, 0, 7},
        {"verbose", no_argument, 0, 8},
        {"separator", required_argument, 0, 9},
        {"help", no_argument, 0, 'h'},
        {"version", no_argument, 0, 'V'},
        {0, 0, 0, 0}
    };
    
    int opt;
    int option_index = 0;
    
    while ((opt = getopt_long(argc, argv, "d:f:hV", long_options, &option_index)) != -1) {
        switch (opt) {
            case 'd':
                opts.delimiter = optarg;
                break;
            case 'f':
                opts.field_count = parse_fields(optarg, &opts.fields);
                if (opts.field_count < 0) {
                    fprintf(stderr, "%s错误: 无法解析字段列表%s\n", COLOR_RED, COLOR_RESET);
                    return 1;
                }
                break;
            case 1: // --complement
                opts.complement = 1;
                break;
            case 2: // --output-delimiter
                opts.output_delimiter = optarg;
                break;
            case 3: // --auto-detect
                opts.auto_detect_delimiter = 1;
                break;
            case 4: // --color
                opts.color = 1;
                break;
            case 5: // --no-color
                opts.color = 0;
                break;
            case 6: // --field-numbers
                opts.show_field_numbers = 1;
                break;
            case 7: // --line-numbers
                opts.show_line_numbers = 1;
                break;
            case 8: // --verbose
                opts.verbose = 1;
                break;
            case 9: // --separator
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
    
    // 检查是否指定了字段
    if (opts.field_count == 0) {
        fprintf(stderr, "%s错误: 必须指定字段列表 (-f)%s\n", COLOR_RED, COLOR_RESET);
        print_help();
        return 1;
    }
    
    // 处理剩余参数（文件名）
    if (optind >= argc) {
        // 没有文件名，从标准输入读取
        return process_file("-", &opts);
    } else {
        int result = 0;
        
        for (int i = optind; i < argc; i++) {
            if (process_file(argv[i], &opts) != 0) {
                result = 1;
            }
        }
        
        return result;
    }
}
