#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <time.h>
#include <pwd.h>
#include <grp.h>
#include "../include/common.h"

#define MAX_ENTRIES 1000
#define MAX_PATH_LENGTH 1024
#define BAR_WIDTH 50

typedef struct {
    char path[MAX_PATH_LENGTH];
    off_t size;
    int is_directory;
    int level;
    char owner[32];
    char group[32];
    time_t mtime;
} du_entry_t;

typedef struct {
    du_entry_t entries[MAX_ENTRIES];
    int entry_count;
    int max_depth;
    int show_human_readable;
    int show_owner;
    int show_graph;
    int sort_by_size;
    int show_hidden;
} du_options_t;

// 获取目录大小
off_t get_directory_size(const char *path, int level, int max_depth) {
    DIR *dir;
    struct dirent *entry;
    struct stat stat_info;
    off_t total_size = 0;
    char full_path[MAX_PATH_LENGTH];
    
    if (level > max_depth) return 0;
    
    dir = opendir(path);
    if (!dir) return 0;
    
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }
        
        snprintf(full_path, sizeof(full_path), "%s/%s", path, entry->d_name);
        
        if (stat(full_path, &stat_info) == 0) {
            if (S_ISDIR(stat_info.st_mode)) {
                total_size += get_directory_size(full_path, level + 1, max_depth);
            } else {
                total_size += stat_info.st_size;
            }
        }
    }
    
    closedir(dir);
    return total_size;
}

// 扫描目录
int scan_directory(const char *path, du_options_t *options, int level) {
    DIR *dir;
    struct dirent *entry;
    struct stat stat_info;
    char full_path[MAX_PATH_LENGTH];
    int count = 0;
    
    if (level > options->max_depth || options->entry_count >= MAX_ENTRIES) {
        return 0;
    }
    
    dir = opendir(path);
    if (!dir) return 0;
    
    while ((entry = readdir(dir)) != NULL && options->entry_count < MAX_ENTRIES) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }
        
        // 跳过隐藏文件（除非指定显示）
        if (!options->show_hidden && entry->d_name[0] == '.') {
            continue;
        }
        
        snprintf(full_path, sizeof(full_path), "%s/%s", path, entry->d_name);
        
        if (stat(full_path, &stat_info) == 0) {
            du_entry_t *entry = &options->entries[options->entry_count];
            
            strncpy(entry->path, full_path, MAX_PATH_LENGTH - 1);
            entry->path[MAX_PATH_LENGTH - 1] = '\0';
            entry->level = level;
            entry->is_directory = S_ISDIR(stat_info.st_mode);
            entry->mtime = stat_info.st_mtime;
            
            if (S_ISDIR(stat_info.st_mode)) {
                entry->size = get_directory_size(full_path, level + 1, options->max_depth);
                // 递归扫描子目录
                scan_directory(full_path, options, level + 1);
            } else {
                entry->size = stat_info.st_size;
            }
            
            // 获取所有者信息
            if (options->show_owner) {
                struct passwd *pw = getpwuid(stat_info.st_uid);
                struct group *gr = getgrgid(stat_info.st_gid);
                
                if (pw) {
                    strncpy(entry->owner, pw->pw_name, 31);
                    entry->owner[31] = '\0';
                } else {
                    snprintf(entry->owner, sizeof(entry->owner), "%d", stat_info.st_uid);
                }
                
                if (gr) {
                    strncpy(entry->group, gr->gr_name, 31);
                    entry->group[31] = '\0';
                } else {
                    snprintf(entry->group, sizeof(entry->group), "%d", stat_info.st_gid);
                }
            }
            
            options->entry_count++;
            count++;
        }
    }
    
    closedir(dir);
    return count;
}

// 比较函数用于排序
int compare_entries(const void *a, const void *b) {
    const du_entry_t *entry_a = (const du_entry_t *)a;
    const du_entry_t *entry_b = (const du_entry_t *)b;
    
    // 目录优先
    if (entry_a->is_directory && !entry_b->is_directory) return -1;
    if (!entry_a->is_directory && entry_b->is_directory) return 1;
    
    // 按大小排序
    if (entry_a->size > entry_b->size) return -1;
    if (entry_a->size < entry_b->size) return 1;
    
    return 0;
}

// 绘制进度条
void draw_progress_bar(off_t size, off_t max_size, int width) {
    if (max_size == 0) return;
    
    int bar_length = (int)((double)size * width / max_size);
    if (bar_length > width) bar_length = width;
    
    printf("%s[", COLOR_CYAN);
    
    for (int i = 0; i < bar_length; i++) {
        if (i < width * 0.3) {
            printf("%s█", COLOR_GREEN);
        } else if (i < width * 0.7) {
            printf("%s█", COLOR_YELLOW);
        } else {
            printf("%s█", COLOR_RED);
        }
    }
    
    for (int i = bar_length; i < width; i++) {
        printf(" ");
    }
    
    printf("%s]%s", COLOR_CYAN, COLOR_RESET);
}

// 显示条目
void display_entry(const du_entry_t *entry, du_options_t *options, off_t max_size) {
    const char *icon = entry->is_directory ? ICON_DIRECTORY : ICON_FILE;
    const char *color = entry->is_directory ? COLOR_BLUE : COLOR_WHITE;
    
    // 缩进
    for (int i = 0; i < entry->level; i++) {
        printf("  ");
    }
    
    // 图标和路径
    printf("%s %s%s%s", icon, color, entry->path, COLOR_RESET);
    
    // 大小
    printf(" %s%s%s", COLOR_MAGENTA, format_size(entry->size), COLOR_RESET);
    
    // 所有者信息
    if (options->show_owner) {
        printf(" %s%s%s:%s%s%s", 
               COLOR_GREEN, entry->owner, COLOR_RESET,
               COLOR_CYAN, entry->group, COLOR_RESET);
    }
    
    // 修改时间
    printf(" %s%s%s", COLOR_YELLOW, format_time(entry->mtime), COLOR_RESET);
    
    // 进度条
    if (options->show_graph && max_size > 0) {
        printf(" ");
        draw_progress_bar(entry->size, max_size, BAR_WIDTH);
    }
    
    printf("\n");
}

// 显示结果
void display_results(du_options_t *options) {
    printf("%s磁盘使用情况分析%s\n", COLOR_CYAN, COLOR_RESET);
    printf("%s%s%s\n", COLOR_YELLOW, "================================================", COLOR_RESET);
    
    if (options->entry_count == 0) {
        printf("%s没有找到文件或目录%s\n", COLOR_RED, COLOR_RESET);
        return;
    }
    
    // 排序
    if (options->sort_by_size) {
        qsort(options->entries, options->entry_count, sizeof(du_entry_t), compare_entries);
    }
    
    // 找到最大大小用于进度条
    off_t max_size = 0;
    if (options->show_graph) {
        for (int i = 0; i < options->entry_count; i++) {
            if (options->entries[i].size > max_size) {
                max_size = options->entries[i].size;
            }
        }
    }
    
    // 显示条目
    for (int i = 0; i < options->entry_count; i++) {
        display_entry(&options->entries[i], options, max_size);
    }
    
    // 计算总大小
    off_t total_size = 0;
    for (int i = 0; i < options->entry_count; i++) {
        total_size += options->entries[i].size;
    }
    
    printf("%s%s%s\n", COLOR_YELLOW, "================================================", COLOR_RESET);
    printf("%s总大小: %s%s\n", COLOR_CYAN, format_size(total_size), COLOR_RESET);
    printf("%s文件/目录数量: %d%s\n", COLOR_CYAN, options->entry_count, COLOR_RESET);
}

void print_usage(const char *program_name) {
    printf("用法: %s [选项] [目录...]\n", program_name);
    printf("优化版的 du 命令，提供图形化磁盘使用显示\n\n");
    printf("选项:\n");
    printf("  -h, --human-readable  以人类可读的格式显示大小\n");
    printf("  -s, --summarize       只显示总计\n");
    printf("  -a, --all            显示所有文件，不仅仅是目录\n");
    printf("  -d, --max-depth 深度  限制目录深度\n");
    printf("  -o, --owner           显示所有者信息\n");
    printf("  -g, --graph           显示图形化进度条\n");
    printf("  -S, --sort            按大小排序\n");
    printf("  -H, --hidden          显示隐藏文件\n");
    printf("  --help                显示此帮助信息\n");
    printf("  --version             显示版本信息\n");
    printf("\n示例:\n");
    printf("  %s                     # 分析当前目录\n", program_name);
    printf("  %s -h /home            # 分析/home目录\n", program_name);
    printf("  %s -d 2 -g             # 限制深度并显示图形\n", program_name);
    printf("  %s -a -o               # 显示所有文件和所有者\n", program_name);
}

int main(int argc, char *argv[]) {
    du_options_t options = {0};
    char *target_dirs[argc];
    int dir_count = 0;
    
    // 设置默认值
    options.max_depth = 10;
    options.show_human_readable = 1;
    options.show_graph = 1;
    options.sort_by_size = 1;
    
    // 解析命令行参数
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--human-readable") == 0) {
            options.show_human_readable = 1;
        } else if (strcmp(argv[i], "-s") == 0 || strcmp(argv[i], "--summarize") == 0) {
            options.max_depth = 0;
        } else if (strcmp(argv[i], "-a") == 0 || strcmp(argv[i], "--all") == 0) {
            // 显示所有文件
        } else if (strcmp(argv[i], "-d") == 0 || strcmp(argv[i], "--max-depth") == 0) {
            if (i + 1 < argc) {
                options.max_depth = atoi(argv[++i]);
            }
        } else if (strcmp(argv[i], "-o") == 0 || strcmp(argv[i], "--owner") == 0) {
            options.show_owner = 1;
        } else if (strcmp(argv[i], "-g") == 0 || strcmp(argv[i], "--graph") == 0) {
            options.show_graph = 1;
        } else if (strcmp(argv[i], "-S") == 0 || strcmp(argv[i], "--sort") == 0) {
            options.sort_by_size = 1;
        } else if (strcmp(argv[i], "-H") == 0 || strcmp(argv[i], "--hidden") == 0) {
            options.show_hidden = 1;
        } else if (strcmp(argv[i], "--help") == 0) {
            print_usage(argv[0]);
            return 0;
        } else if (strcmp(argv[i], "--version") == 0) {
            printf("pdu - 优化版 du 命令 v1.0\n");
            return 0;
        } else if (argv[i][0] != '-') {
            target_dirs[dir_count++] = argv[i];
        }
    }
    
    // 如果没有指定目录，使用当前目录
    if (dir_count == 0) {
        target_dirs[dir_count++] = ".";
    }
    
    // 扫描每个目录
    for (int i = 0; i < dir_count; i++) {
        printf("%s分析目录: %s%s\n", COLOR_CYAN, target_dirs[i], COLOR_RESET);
        scan_directory(target_dirs[i], &options, 0);
    }
    
    // 显示结果
    display_results(&options);
    
    return 0;
}
