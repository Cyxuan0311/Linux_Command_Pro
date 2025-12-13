#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>
#include "../include/common.h"

#define MAX_LINE_LENGTH 1024
#define MAX_LINES 1000
#define CONTEXT_LINES 3

// 差异类型枚举
typedef enum {
    DIFF_EQUAL,     // 相同
    DIFF_ADD,       // 添加
    DIFF_DELETE,    // 删除
    DIFF_MODIFY     // 修改
} DiffType;

// 差异行结构
typedef struct {
    int line_num1;      // 文件1行号
    int line_num2;      // 文件2行号
    DiffType type;      // 差异类型
    char content[MAX_LINE_LENGTH];  // 行内容
} DiffLine;

// 文件行结构
typedef struct {
    char content[MAX_LINE_LENGTH];
    int line_num;
} FileLine;

// 比较配置结构
typedef struct {
    char file1[MAX_LINE_LENGTH];
    char file2[MAX_LINE_LENGTH];
    int context_lines;
    int ignore_case;
    int ignore_whitespace;
    int show_line_numbers;
    int unified_format;
    int side_by_side;
    int color_output;
    int verbose;
} DiffConfig;

// 统计信息结构
typedef struct {
    int added_lines;
    int deleted_lines;
    int modified_lines;
    int equal_lines;
} DiffStats;

// 初始化统计信息
void init_stats(DiffStats *stats) {
    stats->added_lines = 0;
    stats->deleted_lines = 0;
    stats->modified_lines = 0;
    stats->equal_lines = 0;
}

// 读取文件到行数组
int read_file_lines(const char *filename, FileLine *lines, int max_lines) {
    if (!filename || !lines || max_lines <= 0) {
        return -1;
    }
    
    FILE *file = fopen(filename, "r");
    if (!file) {
        return -1;
    }
    
    int count = 0;
    char buffer[MAX_LINE_LENGTH];
    
    while (fgets(buffer, sizeof(buffer), file) && count < max_lines) {
        // 确保不会越界
        if (count >= max_lines) {
            break;
        }
        
        strncpy(lines[count].content, buffer, MAX_LINE_LENGTH - 1);
        lines[count].content[MAX_LINE_LENGTH - 1] = '\0';
        lines[count].line_num = count + 1;
        count++;
    }
    
    fclose(file);
    return count;
}

// 比较两行是否相同
int lines_equal(const char *line1, const char *line2, const DiffConfig *config) {
    if (!line1 || !line2) {
        return 0;
    }
    
    char *l1 = strdup(line1);
    char *l2 = strdup(line2);
    
    if (!l1 || !l2) {
        if (l1) free(l1);
        if (l2) free(l2);
        return 0;
    }
    
    if (config->ignore_whitespace) {
        // 移除首尾空白字符
        char *start1 = l1;
        char *end1 = l1 + strlen(l1) - 1;
        while (end1 > start1 && (*end1 == ' ' || *end1 == '\t' || *end1 == '\n' || *end1 == '\r')) {
            end1--;
        }
        *(end1 + 1) = '\0';
        
        char *start2 = l2;
        char *end2 = l2 + strlen(l2) - 1;
        while (end2 > start2 && (*end2 == ' ' || *end2 == '\t' || *end2 == '\n' || *end2 == '\r')) {
            end2--;
        }
        *(end2 + 1) = '\0';
    }
    
    int result;
    if (config->ignore_case) {
        result = (strcasecmp(l1, l2) == 0);
    } else {
        result = (strcmp(l1, l2) == 0);
    }
    
    free(l1);
    free(l2);
    return result;
}

// 简单的LCS算法计算差异
void compute_diff(FileLine *lines1, int count1, FileLine *lines2, int count2, 
                  DiffLine *diffs, int *diff_count, const DiffConfig *config) {
    if (!lines1 || !lines2 || !diffs || !diff_count || count1 < 0 || count2 < 0) {
        if (diff_count) *diff_count = 0;
        return;
    }
    
    *diff_count = 0;
    const int MAX_DIFFS = MAX_LINES; // 限制最大差异行数
    
    int i = 0, j = 0;
    
    while ((i < count1 || j < count2) && *diff_count < MAX_DIFFS) {
        if (i >= count1) {
            // 文件1结束，剩余都是添加
            while (j < count2 && *diff_count < MAX_DIFFS) {
                diffs[*diff_count].line_num1 = 0;
                diffs[*diff_count].line_num2 = lines2[j].line_num;
                diffs[*diff_count].type = DIFF_ADD;
                strncpy(diffs[*diff_count].content, lines2[j].content, MAX_LINE_LENGTH - 1);
                diffs[*diff_count].content[MAX_LINE_LENGTH - 1] = '\0';
                (*diff_count)++;
                j++;
            }
            break;
        }
        
        if (j >= count2) {
            // 文件2结束，剩余都是删除
            while (i < count1 && *diff_count < MAX_DIFFS) {
                diffs[*diff_count].line_num1 = lines1[i].line_num;
                diffs[*diff_count].line_num2 = 0;
                diffs[*diff_count].type = DIFF_DELETE;
                strncpy(diffs[*diff_count].content, lines1[i].content, MAX_LINE_LENGTH - 1);
                diffs[*diff_count].content[MAX_LINE_LENGTH - 1] = '\0';
                (*diff_count)++;
                i++;
            }
            break;
        }
        
        if (lines_equal(lines1[i].content, lines2[j].content, config)) {
            // 行相同
            diffs[*diff_count].line_num1 = lines1[i].line_num;
            diffs[*diff_count].line_num2 = lines2[j].line_num;
            diffs[*diff_count].type = DIFF_EQUAL;
            strncpy(diffs[*diff_count].content, lines1[i].content, MAX_LINE_LENGTH - 1);
            diffs[*diff_count].content[MAX_LINE_LENGTH - 1] = '\0';
            (*diff_count)++;
            i++;
            j++;
        } else {
            // 查找下一行匹配
            int found = 0;
            for (int k = j + 1; k < count2 && k < j + 10; k++) {
                if (lines_equal(lines1[i].content, lines2[k].content, config)) {
                    // 找到匹配，中间的行是添加
                    while (j < k && *diff_count < MAX_DIFFS) {
                        diffs[*diff_count].line_num1 = 0;
                        diffs[*diff_count].line_num2 = lines2[j].line_num;
                        diffs[*diff_count].type = DIFF_ADD;
                        strncpy(diffs[*diff_count].content, lines2[j].content, MAX_LINE_LENGTH - 1);
                        diffs[*diff_count].content[MAX_LINE_LENGTH - 1] = '\0';
                        (*diff_count)++;
                        j++;
                    }
                    found = 1;
                    break;
                }
            }
            
            if (!found && *diff_count < MAX_DIFFS) {
                // 没找到匹配，标记为删除
                diffs[*diff_count].line_num1 = lines1[i].line_num;
                diffs[*diff_count].line_num2 = 0;
                diffs[*diff_count].type = DIFF_DELETE;
                strncpy(diffs[*diff_count].content, lines1[i].content, MAX_LINE_LENGTH - 1);
                diffs[*diff_count].content[MAX_LINE_LENGTH - 1] = '\0';
                (*diff_count)++;
                i++;
            }
        }
    }
}

// 显示统一格式差异
void show_unified_diff(const DiffLine *diffs, int diff_count, const DiffConfig *config) {
    printf("%s--- %s%s\n", COLOR_RED, config->file1, COLOR_RESET);
    printf("%s+++ %s%s\n", COLOR_GREEN, config->file2, COLOR_RESET);
    
    int i = 0;
    while (i < diff_count) {
        // 查找下一个差异块
        int block_start = i;
        while (i < diff_count && diffs[i].type == DIFF_EQUAL) {
            i++;
        }
        
        if (i >= diff_count) break;
        
        // 计算块范围
        int block_end = i;
        while (block_end < diff_count && diffs[block_end].type != DIFF_EQUAL) {
            block_end++;
        }
        
        // 显示块头
        int start1 = diffs[block_start].line_num1;
        int start2 = diffs[block_start].line_num2;
        int count1 = 0, count2 = 0;
        
        for (int j = block_start; j < block_end; j++) {
            if (diffs[j].type == DIFF_DELETE || diffs[j].type == DIFF_EQUAL) count1++;
            if (diffs[j].type == DIFF_ADD || diffs[j].type == DIFF_EQUAL) count2++;
        }
        
        printf("%s@@ -%d,%d +%d,%d @@%s\n", 
               COLOR_CYAN, start1, count1, start2, count2, COLOR_RESET);
        
        // 显示块内容
        for (int j = block_start; j < block_end; j++) {
            const char *prefix = " ";
            const char *color = COLOR_RESET;
            
            switch (diffs[j].type) {
                case DIFF_ADD:
                    prefix = "+";
                    color = COLOR_GREEN;
                    break;
                case DIFF_DELETE:
                    prefix = "-";
                    color = COLOR_RED;
                    break;
                case DIFF_EQUAL:
                    prefix = " ";
                    color = COLOR_RESET;
                    break;
                case DIFF_MODIFY:
                    prefix = "~";
                    color = COLOR_YELLOW;
                    break;
            }
            
            printf("%s%s%s%s", color, prefix, diffs[j].content, COLOR_RESET);
        }
        
        i = block_end;
    }
}

// 显示并排格式差异
void show_side_by_side_diff(const DiffLine *diffs, int diff_count, const DiffConfig *config) {
    printf("%s文件比较: %s vs %s%s\n", COLOR_CYAN, config->file1, config->file2, COLOR_RESET);
    printf("%s%s%s\n", COLOR_YELLOW, "=" + strlen(config->file1) + strlen(config->file2) + 10, COLOR_RESET);
    
    for (int i = 0; i < diff_count; i++) {
        const char *color = COLOR_RESET;
        const char *symbol = " ";
        
        switch (diffs[i].type) {
            case DIFF_ADD:
                color = COLOR_GREEN;
                symbol = "+";
                break;
            case DIFF_DELETE:
                color = COLOR_RED;
                symbol = "-";
                break;
            case DIFF_EQUAL:
                color = COLOR_RESET;
                symbol = " ";
                break;
            case DIFF_MODIFY:
                color = COLOR_YELLOW;
                symbol = "~";
                break;
        }
        
        if (config->show_line_numbers) {
            printf("%s%4d%s | %s%4d%s | %s%s%s%s\n",
                   COLOR_CYAN, diffs[i].line_num1, COLOR_RESET,
                   COLOR_CYAN, diffs[i].line_num2, COLOR_RESET,
                   color, symbol, diffs[i].content, COLOR_RESET);
        } else {
            printf("%s%s%s%s", color, symbol, diffs[i].content, COLOR_RESET);
        }
    }
}

// 计算统计信息
void calculate_stats(const DiffLine *diffs, int diff_count, DiffStats *stats) {
    init_stats(stats);
    
    for (int i = 0; i < diff_count; i++) {
        switch (diffs[i].type) {
            case DIFF_ADD:
                stats->added_lines++;
                break;
            case DIFF_DELETE:
                stats->deleted_lines++;
                break;
            case DIFF_MODIFY:
                stats->modified_lines++;
                break;
            case DIFF_EQUAL:
                stats->equal_lines++;
                break;
        }
    }
}

// 显示统计信息
void show_stats(const DiffStats *stats) {
    printf("\n%s差异统计:%s\n", COLOR_CYAN, COLOR_RESET);
    printf("%s添加: %d 行%s\n", COLOR_GREEN, stats->added_lines, COLOR_RESET);
    printf("%s删除: %d 行%s\n", COLOR_RED, stats->deleted_lines, COLOR_RESET);
    printf("%s修改: %d 行%s\n", COLOR_YELLOW, stats->modified_lines, COLOR_RESET);
    printf("%s相同: %d 行%s\n", COLOR_WHITE, stats->equal_lines, COLOR_RESET);
    
    int total_changes = stats->added_lines + stats->deleted_lines + stats->modified_lines;
    if (total_changes == 0) {
        printf("%s文件完全相同！%s\n", COLOR_GREEN, COLOR_RESET);
    } else {
        printf("%s总计变化: %d 行%s\n", COLOR_CYAN, total_changes, COLOR_RESET);
    }
}

void print_usage(const char *program_name) {
    printf("用法: %s [选项] 文件1 文件2\n", program_name);
    printf("优化版的 diff 命令，提供彩色输出和多种显示格式\n\n");
    printf("选项:\n");
    printf("  -u, --unified        统一格式显示\n");
    printf("  -s, --side-by-side   并排格式显示\n");
    printf("  -c, --context=N      显示上下文行数（默认3）\n");
    printf("  -i, --ignore-case    忽略大小写\n");
    printf("  -w, --ignore-space   忽略空白字符\n");
    printf("  -n, --line-numbers   显示行号\n");
    printf("  --no-color           禁用彩色输出\n");
    printf("  -v, --verbose        显示详细信息\n");
    printf("  -h, --help           显示此帮助信息\n");
    printf("  -V, --version        显示版本信息\n");
    printf("\n示例:\n");
    printf("  %s file1.txt file2.txt\n", program_name);
    printf("  %s -u -c 5 old.c new.c\n", program_name);
    printf("  %s -s -i config1.conf config2.conf\n", program_name);
    printf("  %s -w -n old.log new.log\n", program_name);
}

int main(int argc, char *argv[]) {
    DiffConfig config = {0};
    config.context_lines = CONTEXT_LINES;
    config.ignore_case = 0;
    config.ignore_whitespace = 0;
    config.show_line_numbers = 0;
    config.unified_format = 0;
    config.side_by_side = 0;
    config.color_output = 1;
    config.verbose = 0;
    
    // 解析命令行参数
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-u") == 0 || strcmp(argv[i], "--unified") == 0) {
            config.unified_format = 1;
        } else if (strcmp(argv[i], "-s") == 0 || strcmp(argv[i], "--side-by-side") == 0) {
            config.side_by_side = 1;
        } else if (strncmp(argv[i], "-c", 2) == 0 || strncmp(argv[i], "--context=", 10) == 0) {
            const char *value = strchr(argv[i], '=');
            if (value) {
                config.context_lines = atoi(value + 1);
            } else if (i + 1 < argc) {
                config.context_lines = atoi(argv[++i]);
            }
        } else if (strcmp(argv[i], "-i") == 0 || strcmp(argv[i], "--ignore-case") == 0) {
            config.ignore_case = 1;
        } else if (strcmp(argv[i], "-w") == 0 || strcmp(argv[i], "--ignore-space") == 0) {
            config.ignore_whitespace = 1;
        } else if (strcmp(argv[i], "-n") == 0 || strcmp(argv[i], "--line-numbers") == 0) {
            config.show_line_numbers = 1;
        } else if (strcmp(argv[i], "--no-color") == 0) {
            config.color_output = 0;
        } else if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--verbose") == 0) {
            config.verbose = 1;
        } else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            print_usage(argv[0]);
            return 0;
        } else if (strcmp(argv[i], "-V") == 0 || strcmp(argv[i], "--version") == 0) {
            printf("pdiff - 优化版 diff 命令 v1.0\n");
            return 0;
        } else if (argv[i][0] != '-') {
            if (strlen(config.file1) == 0) {
                strncpy(config.file1, argv[i], MAX_LINE_LENGTH - 1);
            } else if (strlen(config.file2) == 0) {
                strncpy(config.file2, argv[i], MAX_LINE_LENGTH - 1);
            }
        } else {
            printf("未知选项: %s\n", argv[i]);
            print_usage(argv[0]);
            return 1;
        }
    }
    
    // 检查必需参数
    if (strlen(config.file1) == 0 || strlen(config.file2) == 0) {
        print_error("请指定两个要比较的文件");
        print_usage(argv[0]);
        return 1;
    }
    
    // 检查文件是否存在
    if (access(config.file1, R_OK) != 0) {
        print_error("无法读取文件1");
        return 1;
    }
    
    if (access(config.file2, R_OK) != 0) {
        print_error("无法读取文件2");
        return 1;
    }
    
    // 动态分配内存避免栈溢出
    FileLine *lines1 = malloc(MAX_LINES * sizeof(FileLine));
    FileLine *lines2 = malloc(MAX_LINES * sizeof(FileLine));
    DiffLine *diffs = malloc(MAX_LINES * sizeof(DiffLine));
    
    if (!lines1 || !lines2 || !diffs) {
        print_error("内存分配失败");
        if (lines1) free(lines1);
        if (lines2) free(lines2);
        if (diffs) free(diffs);
        return 1;
    }
    
    // 读取文件内容
    int count1 = read_file_lines(config.file1, lines1, MAX_LINES);
    int count2 = read_file_lines(config.file2, lines2, MAX_LINES);
    
    if (count1 < 0) {
        print_error("无法读取文件1");
        free(lines1);
        free(lines2);
        free(diffs);
        return 1;
    }
    
    if (count2 < 0) {
        print_error("无法读取文件2");
        free(lines1);
        free(lines2);
        free(diffs);
        return 1;
    }
    
    if (config.verbose) {
        printf("%s文件1: %s (%d 行)%s\n", COLOR_YELLOW, config.file1, count1, COLOR_RESET);
        printf("%s文件2: %s (%d 行)%s\n", COLOR_YELLOW, config.file2, count2, COLOR_RESET);
        printf("\n");
    }
    
    // 计算差异
    int diff_count;
    compute_diff(lines1, count1, lines2, count2, diffs, &diff_count, &config);
    
    // 显示差异
    if (config.side_by_side) {
        show_side_by_side_diff(diffs, diff_count, &config);
    } else if (config.unified_format) {
        show_unified_diff(diffs, diff_count, &config);
    } else {
        // 默认格式
        show_side_by_side_diff(diffs, diff_count, &config);
    }
    
    // 显示统计信息
    DiffStats stats;
    calculate_stats(diffs, diff_count, &stats);
    show_stats(&stats);
    
    // 释放内存
    free(lines1);
    free(lines2);
    free(diffs);
    
    return 0;
}
