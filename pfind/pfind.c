#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <time.h>
#include <fnmatch.h>
#include <regex.h>
#include "../include/common.h"

#define MAX_PATH 1024
#define MAX_PATTERN 256
#define MAX_RESULTS 10000

typedef struct {
    char path[MAX_PATH];
    struct stat stat_info;
    int match_type; // 0=name, 1=type, 2=size, 3=time
} search_result_t;

typedef struct {
    char name_pattern[MAX_PATTERN];
    char type_filter;
    off_t min_size;
    off_t max_size;
    time_t min_time;
    time_t max_time;
    int use_regex;
    int show_details;
    int max_depth;
    int current_depth;
} search_options_t;

search_result_t results[MAX_RESULTS];
int result_count = 0;

// 检查文件是否匹配搜索条件
int matches_criteria(const char *path, const char *name, struct stat *stat_info, search_options_t *options) {
    // 检查名称模式
    if (strlen(options->name_pattern) > 0) {
        if (options->use_regex) {
            regex_t regex;
            if (regcomp(&regex, options->name_pattern, REG_EXTENDED) != 0) {
                return 0;
            }
            int match = regexec(&regex, name, 0, NULL, 0);
            regfree(&regex);
            if (match != 0) return 0;
        } else {
            if (fnmatch(options->name_pattern, name, 0) != 0) {
                return 0;
            }
        }
    }
    
    // 检查文件类型
    if (options->type_filter != '\0') {
        switch (options->type_filter) {
            case 'f': // 普通文件
                if (!S_ISREG(stat_info->st_mode)) return 0;
                break;
            case 'd': // 目录
                if (!S_ISDIR(stat_info->st_mode)) return 0;
                break;
            case 'l': // 符号链接
                if (!S_ISLNK(stat_info->st_mode)) return 0;
                break;
            case 'b': // 块设备
                if (!S_ISBLK(stat_info->st_mode)) return 0;
                break;
            case 'c': // 字符设备
                if (!S_ISCHR(stat_info->st_mode)) return 0;
                break;
            case 'p': // 管道
                if (!S_ISFIFO(stat_info->st_mode)) return 0;
                break;
            case 's': // 套接字
                if (!S_ISSOCK(stat_info->st_mode)) return 0;
                break;
        }
    }
    
    // 检查文件大小
    if (options->min_size > 0 && stat_info->st_size < options->min_size) {
        return 0;
    }
    if (options->max_size > 0 && stat_info->st_size > options->max_size) {
        return 0;
    }
    
    // 检查修改时间
    if (options->min_time > 0 && stat_info->st_mtime < options->min_time) {
        return 0;
    }
    if (options->max_time > 0 && stat_info->st_mtime > options->max_time) {
        return 0;
    }
    
    return 1;
}

// 递归搜索目录
int search_directory(const char *dir_path, search_options_t *options) {
    DIR *dir;
    struct dirent *entry;
    char full_path[MAX_PATH];
    
    if (options->current_depth > options->max_depth) {
        return 0;
    }
    
    dir = opendir(dir_path);
    if (!dir) {
        return 0;
    }
    
    while ((entry = readdir(dir)) != NULL && result_count < MAX_RESULTS) {
        // 跳过 . 和 ..
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }
        
        snprintf(full_path, sizeof(full_path), "%s/%s", dir_path, entry->d_name);
        
        struct stat stat_info;
        if (stat(full_path, &stat_info) == 0) {
            // 检查是否匹配搜索条件
            if (matches_criteria(full_path, entry->d_name, &stat_info, options)) {
                strncpy(results[result_count].path, full_path, MAX_PATH - 1);
                results[result_count].path[MAX_PATH - 1] = '\0';
                results[result_count].stat_info = stat_info;
                results[result_count].match_type = 0; // name match
                result_count++;
            }
            
            // 如果是目录，递归搜索
            if (S_ISDIR(stat_info.st_mode)) {
                options->current_depth++;
                search_directory(full_path, options);
                options->current_depth--;
            }
        }
    }
    
    closedir(dir);
    return 0;
}

// 显示搜索结果
void display_results(search_options_t *options) {
    printf("%s搜索结果 (%d 个匹配项)%s\n", 
           COLOR_CYAN, result_count, COLOR_RESET);
    printf("%s%s%s\n", COLOR_YELLOW, "=" + 20, COLOR_RESET);
    
    for (int i = 0; i < result_count; i++) {
        const char *icon = get_file_icon(results[i].path, results[i].stat_info.st_mode);
        const char *color = COLOR_WHITE;
        
        // 根据文件类型设置颜色
        if (S_ISDIR(results[i].stat_info.st_mode)) {
            color = COLOR_BLUE;
        } else if (S_ISLNK(results[i].stat_info.st_mode)) {
            color = COLOR_CYAN;
        } else if (is_executable(results[i].path)) {
            color = COLOR_GREEN;
        }
        
        if (options->show_details) {
            printf("%s %s %s %s %s %s\n",
                   icon,
                   color,
                   results[i].path,
                   COLOR_RESET,
                   format_size(results[i].stat_info.st_size),
                   format_time(results[i].stat_info.st_mtime));
        } else {
            printf("%s %s%s%s\n", icon, color, results[i].path, COLOR_RESET);
        }
    }
}

// 解析时间参数
time_t parse_time(const char *time_str) {
    // 简单的时间解析，支持相对时间
    if (time_str[0] == '-') {
        int days = atoi(time_str + 1);
        return time(NULL) - (days * 24 * 60 * 60);
    }
    return 0;
}

void print_usage(const char *program_name) {
    printf("用法: %s [选项] [目录] [表达式]\n", program_name);
    printf("优化版的 find 命令，提供更好的搜索体验和结果展示\n\n");
    printf("选项:\n");
    printf("  -name 模式         按名称搜索（支持通配符）\n");
    printf("  -regex 模式        按正则表达式搜索\n");
    printf("  -type 类型         按文件类型搜索 (f=文件, d=目录, l=链接)\n");
    printf("  -size +大小        按文件大小搜索\n");
    printf("  -mtime -天数       按修改时间搜索\n");
    printf("  -maxdepth 深度     最大搜索深度\n");
    printf("  -ls               显示详细信息\n");
    printf("  -h, --help        显示此帮助信息\n");
    printf("  -v, --version     显示版本信息\n");
    printf("\n示例:\n");
    printf("  %s . -name \"*.c\"           # 查找所有.c文件\n", program_name);
    printf("  %s . -type d               # 查找所有目录\n", program_name);
    printf("  %s . -size +1M             # 查找大于1MB的文件\n", program_name);
    printf("  %s . -mtime -7             # 查找7天内修改的文件\n", program_name);
}

int main(int argc, char *argv[]) {
    search_options_t options = {0};
    char *search_dir = ".";
    int i;
    
    // 设置默认值
    options.max_depth = 10;
    options.current_depth = 0;
    
    // 解析命令行参数
    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-name") == 0 && i + 1 < argc) {
            strncpy(options.name_pattern, argv[++i], MAX_PATTERN - 1);
            options.name_pattern[MAX_PATTERN - 1] = '\0';
        } else if (strcmp(argv[i], "-regex") == 0 && i + 1 < argc) {
            strncpy(options.name_pattern, argv[++i], MAX_PATTERN - 1);
            options.name_pattern[MAX_PATTERN - 1] = '\0';
            options.use_regex = 1;
        } else if (strcmp(argv[i], "-type") == 0 && i + 1 < argc) {
            options.type_filter = argv[++i][0];
        } else if (strcmp(argv[i], "-size") == 0 && i + 1 < argc) {
            char *size_str = argv[++i];
            if (size_str[0] == '+') {
                options.min_size = atoi(size_str + 1) * 1024 * 1024; // 假设MB
            } else if (size_str[0] == '-') {
                options.max_size = atoi(size_str + 1) * 1024 * 1024;
            }
        } else if (strcmp(argv[i], "-mtime") == 0 && i + 1 < argc) {
            options.min_time = parse_time(argv[++i]);
        } else if (strcmp(argv[i], "-maxdepth") == 0 && i + 1 < argc) {
            options.max_depth = atoi(argv[++i]);
        } else if (strcmp(argv[i], "-ls") == 0) {
            options.show_details = 1;
        } else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            print_usage(argv[0]);
            return 0;
        } else if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--version") == 0) {
            printf("pfind - 优化版 find 命令 v1.0\n");
            return 0;
        } else if (argv[i][0] != '-') {
            search_dir = argv[i];
        }
    }
    
    // 开始搜索
    printf("%s开始搜索目录: %s%s\n", COLOR_CYAN, search_dir, COLOR_RESET);
    
    if (search_directory(search_dir, &options) != 0) {
        print_error("搜索过程中发生错误");
        return 1;
    }
    
    // 显示结果
    display_results(&options);
    
    return 0;
}
