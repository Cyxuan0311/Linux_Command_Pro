#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <regex.h>
#include <fnmatch.h>
#include <strings.h>
#include "../include/common.h"

#define MAX_LINE_LENGTH 1024
#define MAX_FILENAME 256
#define MAX_MATCHES 1000

typedef struct {
    char filename[MAX_FILENAME];
    int line_number;
    char line_content[MAX_LINE_LENGTH];
    int match_start;
    int match_end;
} match_result_t;

typedef struct {
    char pattern[MAX_LINE_LENGTH];
    int use_regex;
    int case_insensitive;
    int show_line_numbers;
    int show_filenames;
    int show_context;
    int context_before;
    int context_after;
    int count_only;
    int invert_match;
    int whole_word;
    int recursive;
    char file_pattern[MAX_FILENAME];
} grep_options_t;

match_result_t matches[MAX_MATCHES];
int match_count = 0;

// 编译正则表达式
regex_t* compile_regex(const char *pattern, int case_insensitive) {
    regex_t *regex = malloc(sizeof(regex_t));
    int flags = REG_EXTENDED;
    if (case_insensitive) {
        flags |= REG_ICASE;
    }
    
    if (regcomp(regex, pattern, flags) != 0) {
        free(regex);
        return NULL;
    }
    
    return regex;
}

// 检查是否匹配
int is_match(const char *line, const char *pattern, regex_t *regex, grep_options_t *options) {
    if (options->use_regex) {
        if (regex) {
            return regexec(regex, line, 0, NULL, 0) == 0;
        }
        return 0;
    } else {
        if (options->case_insensitive) {
            return strcasestr(line, pattern) != NULL;
        } else {
            return strstr(line, pattern) != NULL;
        }
    }
}

// 查找匹配位置
int find_match_position(const char *line, const char *pattern, int case_insensitive) {
    char *pos;
    if (case_insensitive) {
        pos = strcasestr(line, pattern);
    } else {
        pos = strstr(line, pattern);
    }
    
    if (pos) {
        return (int)(pos - line);
    }
    return -1;
}

// 高亮显示匹配的文本
void highlight_match(const char *line, const char *pattern, int case_insensitive) {
    char *line_copy = strdup(line);
    if (!line_copy) return;
    
    char *pos = line_copy;
    int pattern_len = strlen(pattern);
    int line_len = strlen(line);
    
    while (pos < line_copy + line_len) {
        char *match_pos;
        if (case_insensitive) {
            match_pos = strcasestr(pos, pattern);
        } else {
            match_pos = strstr(pos, pattern);
        }
        
        if (match_pos == NULL) {
            // 没有更多匹配，打印剩余部分
            printf("%s", pos);
            break;
        }
        
        // 打印匹配前的部分
        int offset = match_pos - pos;
        printf("%.*s", offset, pos);
        
        // 打印高亮的匹配部分
        printf("%s%.*s%s", 
               COLOR_RED BOLD,
               pattern_len, match_pos,
               COLOR_RESET);
        
        // 移动到匹配后
        pos = match_pos + pattern_len;
    }
    
    free(line_copy);
}

// 搜索单个文件
int search_file(const char *filename, grep_options_t *options) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        return 0;
    }
    
    char line[MAX_LINE_LENGTH];
    int line_number = 0;
    int match_count_in_file = 0;
    regex_t *regex = NULL;
    
    if (options->use_regex) {
        regex = compile_regex(options->pattern, options->case_insensitive);
        if (!regex) {
            fclose(file);
            return 0;
        }
    }
    
    while (fgets(line, sizeof(line), file) && match_count < MAX_MATCHES) {
        line_number++;
        
        // 移除换行符
        line[strcspn(line, "\n")] = '\0';
        
        int matches_line = is_match(line, options->pattern, regex, options);
        
        if (options->invert_match) {
            matches_line = !matches_line;
        }
        
        if (matches_line) {
            match_count_in_file++;
            
            if (!options->count_only) {
                // 存储匹配结果
                strncpy(matches[match_count].filename, filename, MAX_FILENAME - 1);
                matches[match_count].filename[MAX_FILENAME - 1] = '\0';
                matches[match_count].line_number = line_number;
                strncpy(matches[match_count].line_content, line, MAX_LINE_LENGTH - 1);
                matches[match_count].line_content[MAX_LINE_LENGTH - 1] = '\0';
                
                if (!options->use_regex) {
                    int pos = find_match_position(line, options->pattern, options->case_insensitive);
                    matches[match_count].match_start = pos;
                    matches[match_count].match_end = pos + strlen(options->pattern);
                }
                
                match_count++;
            }
        }
    }
    
    fclose(file);
    
    if (regex) {
        regfree(regex);
        free(regex);
    }
    
    if (options->count_only && match_count_in_file > 0) {
        printf("%s%s%s: %d\n", COLOR_CYAN, filename, COLOR_RESET, match_count_in_file);
    }
    
    return match_count_in_file;
}

// 递归搜索目录
int search_directory(const char *dir_path, grep_options_t *options) {
    DIR *dir;
    struct dirent *entry;
    char full_path[MAX_FILENAME];
    int total_matches = 0;
    
    dir = opendir(dir_path);
    if (!dir) {
        return 0;
    }
    
    while ((entry = readdir(dir)) != NULL) {
        // 跳过 . 和 ..
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }
        
        snprintf(full_path, sizeof(full_path), "%s/%s", dir_path, entry->d_name);
        
        struct stat stat_info;
        if (stat(full_path, &stat_info) == 0) {
            if (S_ISDIR(stat_info.st_mode)) {
                // 递归搜索子目录
                if (options->recursive) {
                    total_matches += search_directory(full_path, options);
                }
            } else if (S_ISREG(stat_info.st_mode)) {
                // 检查文件模式匹配
                if (strlen(options->file_pattern) == 0 || 
                    fnmatch(options->file_pattern, entry->d_name, 0) == 0) {
                    total_matches += search_file(full_path, options);
                }
            }
        }
    }
    
    closedir(dir);
    return total_matches;
}

// 显示搜索结果
void display_results(grep_options_t *options) {
    if (options->count_only) {
        return;
    }
    
    printf("%s搜索结果 (%d 个匹配项)%s\n", 
           COLOR_CYAN, match_count, COLOR_RESET);
    printf("%s%s%s\n", COLOR_YELLOW, "====================", COLOR_RESET);
    
    for (int i = 0; i < match_count; i++) {
        if (options->show_filenames) {
            printf("%s%s%s:", COLOR_GREEN, matches[i].filename, COLOR_RESET);
        }
        
        if (options->show_line_numbers) {
            printf("%s%d%s:", COLOR_CYAN, matches[i].line_number, COLOR_RESET);
        }
        
        if (options->use_regex) {
            printf(" %s\n", matches[i].line_content);
        } else {
            printf(" ");
            highlight_match(matches[i].line_content, options->pattern, options->case_insensitive);
            printf("\n");
        }
    }
}

void print_usage(const char *program_name) {
    printf("用法: %s [选项] 模式 [文件...]\n", program_name);
    printf("优化版的 grep 命令，提供更好的搜索体验和结果高亮\n\n");
    printf("选项:\n");
    printf("  -i, --ignore-case      忽略大小写\n");
    printf("  -n, --line-number      显示行号\n");
    printf("  -r, --recursive        递归搜索目录\n");
    printf("  -c, --count            只显示匹配行数\n");
    printf("  -v, --invert-match     显示不匹配的行\n");
    printf("  -w, --word-regexp      匹配整个单词\n");
    printf("  -E, --extended-regexp  使用扩展正则表达式\n");
    printf("  --include=模式         只搜索匹配模式的文件\n");
    printf("  -h, --help             显示此帮助信息\n");
    printf("  -V, --version          显示版本信息\n");
    printf("\n示例:\n");
    printf("  %s \"hello\" file.txt           # 在文件中搜索hello\n", program_name);
    printf("  %s -i \"hello\" file.txt        # 忽略大小写搜索\n", program_name);
    printf("  %s -r \"hello\" .               # 递归搜索当前目录\n", program_name);
    printf("  %s -n \"hello\" file.txt        # 显示行号\n", program_name);
    printf("  %s -E \"hello|world\" file.txt  # 使用正则表达式\n", program_name);
}

int main(int argc, char *argv[]) {
    grep_options_t options = {0};
    char *pattern = NULL;
    char *files[argc];
    int file_count = 0;
    int i;
    
    // 解析命令行参数
    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-i") == 0 || strcmp(argv[i], "--ignore-case") == 0) {
            options.case_insensitive = 1;
        } else if (strcmp(argv[i], "-n") == 0 || strcmp(argv[i], "--line-number") == 0) {
            options.show_line_numbers = 1;
        } else if (strcmp(argv[i], "-r") == 0 || strcmp(argv[i], "--recursive") == 0) {
            options.recursive = 1;
        } else if (strcmp(argv[i], "-c") == 0 || strcmp(argv[i], "--count") == 0) {
            options.count_only = 1;
        } else if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--invert-match") == 0) {
            options.invert_match = 1;
        } else if (strcmp(argv[i], "-w") == 0 || strcmp(argv[i], "--word-regexp") == 0) {
            options.whole_word = 1;
        } else if (strcmp(argv[i], "-E") == 0 || strcmp(argv[i], "--extended-regexp") == 0) {
            options.use_regex = 1;
        } else if (strncmp(argv[i], "--include=", 10) == 0) {
            strncpy(options.file_pattern, argv[i] + 10, MAX_FILENAME - 1);
            options.file_pattern[MAX_FILENAME - 1] = '\0';
        } else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            print_usage(argv[0]);
            return 0;
        } else if (strcmp(argv[i], "-V") == 0 || strcmp(argv[i], "--version") == 0) {
            printf("pgrep - 优化版 grep 命令 v1.0\n");
            return 0;
        } else if (argv[i][0] != '-') {
            if (!pattern) {
                pattern = argv[i];
            } else {
                files[file_count++] = argv[i];
            }
        }
    }
    
    if (!pattern) {
        print_error("请指定搜索模式");
        print_usage(argv[0]);
        return 1;
    }
    
    strncpy(options.pattern, pattern, MAX_LINE_LENGTH - 1);
    options.pattern[MAX_LINE_LENGTH - 1] = '\0';
    
    // 设置默认选项
    options.show_filenames = (file_count > 1 || options.recursive);
    
    int total_matches = 0;
    
    if (file_count == 0) {
        // 从标准输入读取
        char line[MAX_LINE_LENGTH];
        int line_number = 0;
        
        while (fgets(line, sizeof(line), stdin) && match_count < MAX_MATCHES) {
            line_number++;
            line[strcspn(line, "\n")] = '\0';
            
            int matches_line = is_match(line, options.pattern, NULL, &options);
            if (options.invert_match) {
                matches_line = !matches_line;
            }
            
            if (matches_line) {
                if (!options.count_only) {
                    strcpy(matches[match_count].filename, "(标准输入)");
                    matches[match_count].line_number = line_number;
                    strcpy(matches[match_count].line_content, line);
                    match_count++;
                }
                total_matches++;
            }
        }
    } else {
        // 搜索指定文件
        for (i = 0; i < file_count; i++) {
            struct stat stat_info;
            if (stat(files[i], &stat_info) == 0) {
                if (S_ISDIR(stat_info.st_mode)) {
                    if (options.recursive) {
                        total_matches += search_directory(files[i], &options);
                    }
                } else {
                    total_matches += search_file(files[i], &options);
                }
            }
            match_count = total_matches;
        }
    }
    
    // 显示结果
    display_results(&options);
    
    if (options.count_only) {
        printf("%s总计: %d 个匹配项%s\n", COLOR_CYAN, total_matches, COLOR_RESET);
    }
    
    return 0;
}
