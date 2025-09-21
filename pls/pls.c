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
#include <errno.h>
#include "../include/common.h"

#define MAX_FILES 1000
#define MAX_FILENAME 256

typedef struct {
    char name[MAX_FILENAME];
    struct stat stat_info;
} file_entry_t;

// 比较函数用于排序
int compare_files(const void *a, const void *b) {
    const file_entry_t *file_a = (const file_entry_t *)a;
    const file_entry_t *file_b = (const file_entry_t *)b;
    
    // 目录优先
    int a_is_dir = S_ISDIR(file_a->stat_info.st_mode);
    int b_is_dir = S_ISDIR(file_b->stat_info.st_mode);
    
    if (a_is_dir && !b_is_dir) return -1;
    if (!a_is_dir && b_is_dir) return 1;
    
    // 按名称排序
    return strcasecmp(file_a->name, file_b->name);
}

void print_file_info(const file_entry_t *file, int show_details) {
    const char *icon = get_file_icon(file->name, file->stat_info.st_mode);
    const char *color = COLOR_WHITE;
    
    // 根据文件类型设置颜色
    if (S_ISDIR(file->stat_info.st_mode)) {
        color = COLOR_BLUE;
    } else if (S_ISLNK(file->stat_info.st_mode)) {
        color = COLOR_CYAN;
    } else if (is_executable(file->name)) {
        color = COLOR_GREEN;
    } else if (is_archive(file->name)) {
        color = COLOR_RED;
    } else if (is_image(file->name)) {
        color = COLOR_MAGENTA;
    } else if (is_video(file->name)) {
        color = COLOR_YELLOW;
    } else if (is_audio(file->name)) {
        color = COLOR_CYAN;
    } else if (is_document(file->name)) {
        color = COLOR_WHITE;
    } else if (is_code_file(file->name)) {
        color = COLOR_GREEN;
    }
    
    if (show_details) {
        // 详细模式：显示权限、大小、时间等
        struct passwd *pw = getpwuid(file->stat_info.st_uid);
        struct group *gr = getgrgid(file->stat_info.st_gid);
        
        printf("%s %s%s%s%s%s%s%s%s%s%s\n",
               icon,
               (S_ISDIR(file->stat_info.st_mode)) ? "d" : "-",
               (file->stat_info.st_mode & S_IRUSR) ? "r" : "-",
               (file->stat_info.st_mode & S_IWUSR) ? "w" : "-",
               (file->stat_info.st_mode & S_IXUSR) ? "x" : "-",
               (file->stat_info.st_mode & S_IRGRP) ? "r" : "-",
               (file->stat_info.st_mode & S_IWGRP) ? "w" : "-",
               (file->stat_info.st_mode & S_IXGRP) ? "x" : "-",
               (file->stat_info.st_mode & S_IROTH) ? "r" : "-",
               (file->stat_info.st_mode & S_IWOTH) ? "w" : "-",
               (file->stat_info.st_mode & S_IXOTH) ? "x" : "-");
        
        printf("  %s %s %s %s %s\n",
               pw ? pw->pw_name : "unknown",
               gr ? gr->gr_name : "unknown",
               format_size(file->stat_info.st_size),
               format_time(file->stat_info.st_mtime),
               file->name);
    } else {
        // 简单模式：只显示图标和文件名
        printf("%s %s%s%s\n", icon, color, file->name, COLOR_RESET);
    }
}

int list_directory(const char *path, int show_details, int show_hidden) {
    DIR *dir;
    struct dirent *entry;
    file_entry_t files[MAX_FILES];
    int file_count = 0;
    
    dir = opendir(path);
    if (!dir) {
        print_error("无法打开目录");
        return 1;
    }
    
    // 读取目录内容
    while ((entry = readdir(dir)) != NULL && file_count < MAX_FILES) {
        // 跳过隐藏文件（除非指定显示）
        if (!show_hidden && entry->d_name[0] == '.') {
            continue;
        }
        
        char full_path[MAX_FILENAME];
        snprintf(full_path, sizeof(full_path), "%s/%s", path, entry->d_name);
        
        if (stat(full_path, &files[file_count].stat_info) == 0) {
            strncpy(files[file_count].name, entry->d_name, MAX_FILENAME - 1);
            files[file_count].name[MAX_FILENAME - 1] = '\0';
            file_count++;
        }
    }
    closedir(dir);
    
    // 排序文件
    qsort(files, file_count, sizeof(file_entry_t), compare_files);
    
    // 显示文件信息
    printf("%s%s 目录内容 (%d 个文件/目录)%s\n", 
           COLOR_CYAN, path, file_count, COLOR_RESET);
    printf("%s%s%s\n", COLOR_YELLOW, "=" + strlen(path) + 20, COLOR_RESET);
    
    for (int i = 0; i < file_count; i++) {
        print_file_info(&files[i], show_details);
    }
    
    return 0;
}

void print_usage(const char *program_name) {
    printf("用法: %s [选项] [目录]\n", program_name);
    printf("优化版的 ls 命令，提供更好的视觉效果和文件类型识别\n\n");
    printf("选项:\n");
    printf("  -l, --long      显示详细信息\n");
    printf("  -a, --all       显示隐藏文件\n");
    printf("  -h, --help      显示此帮助信息\n");
    printf("  -v, --version   显示版本信息\n");
    printf("\n示例:\n");
    printf("  %s                # 列出当前目录\n", program_name);
    printf("  %s -l             # 详细模式\n", program_name);
    printf("  %s -a             # 显示隐藏文件\n", program_name);
    printf("  %s /home          # 列出指定目录\n", program_name);
}

int main(int argc, char *argv[]) {
    int show_details = 0;
    int show_hidden = 0;
    char *target_dir = ".";
    
    // 解析命令行参数
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-l") == 0 || strcmp(argv[i], "--long") == 0) {
            show_details = 1;
        } else if (strcmp(argv[i], "-a") == 0 || strcmp(argv[i], "--all") == 0) {
            show_hidden = 1;
        } else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            print_usage(argv[0]);
            return 0;
        } else if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--version") == 0) {
            printf("pls - 优化版 ls 命令 v1.0\n");
            return 0;
        } else if (argv[i][0] != '-') {
            target_dir = argv[i];
        } else {
            printf("未知选项: %s\n", argv[i]);
            print_usage(argv[0]);
            return 1;
        }
    }
    
    return list_directory(target_dir, show_details, show_hidden);
}
