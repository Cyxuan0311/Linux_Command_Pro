#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>
#include <dirent.h>
#include <fcntl.h>
#include <utime.h>
#include "../include/common.h"

#define MAX_FILENAME 256
#define MAX_FILES 1000
#define BUFFER_SIZE 8192

// 复制模式枚举
typedef enum {
    COPY_SIMPLE,        // 简单复制
    COPY_RECURSIVE,     // 递归复制
    COPY_PRESERVE,      // 保留属性
    COPY_UPDATE         // 更新模式
} CopyMode;

// 复制配置结构
typedef struct {
    char **sources;
    int source_count;
    char destination[MAX_FILENAME];
    CopyMode mode;
    int verbose;
    int force;
    int preserve_attributes;
    int update_only;
    int interactive;
    int show_progress;
} CopyConfig;

// 文件信息结构
typedef struct {
    char source[MAX_FILENAME];
    char destination[MAX_FILENAME];
    off_t size;
    time_t mtime;
    mode_t mode;
    int is_directory;
} FileInfo;

// 统计信息结构
typedef struct {
    int files_copied;
    int directories_created;
    off_t bytes_copied;
    int errors;
} CopyStats;

// 初始化统计信息
void init_stats(CopyStats *stats) {
    stats->files_copied = 0;
    stats->directories_created = 0;
    stats->bytes_copied = 0;
    stats->errors = 0;
}

// 显示进度条
void show_progress(off_t current, off_t total, const char *filename) {
    if (total <= 0) return;
    
    int percent = (int)((current * 100) / total);
    int bar_length = 50;
    int filled_length = (int)((current * bar_length) / total);
    
    printf("\r%s复制 %s [", COLOR_CYAN, filename);
    for (int i = 0; i < bar_length; i++) {
        if (i < filled_length) {
            printf("%s█%s", COLOR_GREEN, COLOR_RESET);
        } else {
            printf(" ");
        }
    }
    printf("] %d%% (%s/%s)", percent, format_size(current), format_size(total));
    fflush(stdout);
}

// 检查文件是否需要更新
int needs_update(const char *source, const char *dest) {
    struct stat src_st, dest_st;
    
    if (stat(source, &src_st) != 0) {
        return 1;  // 源文件不存在，需要复制
    }
    
    if (stat(dest, &dest_st) != 0) {
        return 1;  // 目标文件不存在，需要复制
    }
    
    // 比较修改时间
    return (src_st.st_mtime > dest_st.st_mtime);
}

// 复制文件
int copy_file(const char *source, const char *dest, const CopyConfig *config) {
    int src_fd = open(source, O_RDONLY);
    if (src_fd == -1) {
        print_error("无法打开源文件");
        return 0;
    }
    
    int dest_fd = open(dest, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (dest_fd == -1) {
        close(src_fd);
        print_error("无法创建目标文件");
        return 0;
    }
    
    // 获取源文件信息
    struct stat src_st;
    fstat(src_fd, &src_st);
    
    char buffer[BUFFER_SIZE];
    ssize_t bytes_read, bytes_written;
    off_t total_copied = 0;
    
    while ((bytes_read = read(src_fd, buffer, sizeof(buffer))) > 0) {
        bytes_written = write(dest_fd, buffer, bytes_read);
        if (bytes_written != bytes_read) {
            close(src_fd);
            close(dest_fd);
            print_error("写入文件失败");
            return 0;
        }
        
        total_copied += bytes_written;
        
        if (config->show_progress) {
            show_progress(total_copied, src_st.st_size, source);
        }
    }
    
    close(src_fd);
    close(dest_fd);
    
    if (config->show_progress) {
        printf("\n");
    }
    
    // 保留文件属性
    if (config->preserve_attributes) {
        struct utimbuf times;
        times.actime = src_st.st_atime;
        times.modtime = src_st.st_mtime;
        utime(dest, &times);
        chmod(dest, src_st.st_mode);
    }
    
    return 1;
}

// 创建目录
int create_directory(const char *path, mode_t mode) {
    if (mkdir(path, mode) == 0) {
        return 1;
    }
    
    if (errno == EEXIST) {
        return 1;  // 目录已存在
    }
    
    return 0;
}

// 递归复制目录
int copy_directory(const char *source, const char *dest, const CopyConfig *config) {
    DIR *dir = opendir(source);
    if (!dir) {
        print_error("无法打开源目录");
        return 0;
    }
    
    struct dirent *entry;
    int success = 1;
    
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }
        
        char src_path[MAX_FILENAME];
        char dest_path[MAX_FILENAME];
        
        snprintf(src_path, sizeof(src_path), "%s/%s", source, entry->d_name);
        snprintf(dest_path, sizeof(dest_path), "%s/%s", dest, entry->d_name);
        
        struct stat st;
        if (stat(src_path, &st) != 0) {
            continue;
        }
        
        if (S_ISDIR(st.st_mode)) {
            // 复制子目录
            if (!create_directory(dest_path, st.st_mode)) {
                print_error("无法创建目录");
                success = 0;
                continue;
            }
            
            if (config->verbose) {
                printf("%s创建目录: %s%s\n", COLOR_GREEN, dest_path, COLOR_RESET);
            }
            
            if (!copy_directory(src_path, dest_path, config)) {
                success = 0;
            }
        } else {
            // 复制文件
            if (config->update_only && !needs_update(src_path, dest_path)) {
                if (config->verbose) {
                    printf("%s跳过（已是最新）: %s%s\n", COLOR_YELLOW, src_path, COLOR_RESET);
                }
                continue;
            }
            
            if (config->interactive) {
                printf("复制 %s 到 %s? (y/n): ", src_path, dest_path);
                char response;
                if (scanf(" %c", &response) != 1 || (response != 'y' && response != 'Y')) {
                    continue;
                }
            }
            
            if (copy_file(src_path, dest_path, config)) {
                if (config->verbose) {
                    printf("%s复制文件: %s -> %s%s\n", 
                           COLOR_GREEN, src_path, dest_path, COLOR_RESET);
                }
            } else {
                print_error("复制文件失败");
                success = 0;
            }
        }
    }
    
    closedir(dir);
    return success;
}

// 执行复制操作
int perform_copy(const CopyConfig *config, CopyStats *stats) {
    init_stats(stats);
    
    printf("%s开始复制文件...%s\n", COLOR_CYAN, COLOR_RESET);
    printf("%s目标: %s%s\n", COLOR_YELLOW, config->destination, COLOR_RESET);
    printf("%s源文件数量: %d%s\n", COLOR_YELLOW, config->source_count, COLOR_RESET);
    
    for (int i = 0; i < config->source_count; i++) {
        const char *source = config->sources[i];
        char dest[MAX_FILENAME];
        
        // 构建目标路径
        struct stat dest_st;
        if (stat(config->destination, &dest_st) == 0 && S_ISDIR(dest_st.st_mode)) {
            // 目标是目录
            const char *basename = strrchr(source, '/');
            basename = basename ? basename + 1 : source;
            snprintf(dest, sizeof(dest), "%s/%s", config->destination, basename);
        } else {
            // 目标是文件
            strncpy(dest, config->destination, MAX_FILENAME - 1);
            dest[MAX_FILENAME - 1] = '\0';
        }
        
        struct stat src_st;
        if (stat(source, &src_st) != 0) {
            print_error("源文件不存在");
            stats->errors++;
            continue;
        }
        
        if (S_ISDIR(src_st.st_mode)) {
            // 复制目录
            if (config->mode == COPY_RECURSIVE || config->mode == COPY_PRESERVE) {
                if (!create_directory(dest, src_st.st_mode)) {
                    print_error("无法创建目标目录");
                    stats->errors++;
                    continue;
                }
                
                if (config->verbose) {
                    printf("%s创建目录: %s%s\n", COLOR_GREEN, dest, COLOR_RESET);
                }
                
                stats->directories_created++;
                
                if (!copy_directory(source, dest, config)) {
                    stats->errors++;
                }
            } else {
                print_warning("跳过目录（使用 -r 选项启用递归复制）");
            }
        } else {
            // 复制文件
            if (config->update_only && !needs_update(source, dest)) {
                if (config->verbose) {
                    printf("%s跳过（已是最新）: %s%s\n", COLOR_YELLOW, source, COLOR_RESET);
                }
                continue;
            }
            
            if (config->interactive) {
                printf("复制 %s 到 %s? (y/n): ", source, dest);
                char response;
                if (scanf(" %c", &response) != 1 || (response != 'y' && response != 'Y')) {
                    continue;
                }
            }
            
            if (copy_file(source, dest, config)) {
                stats->files_copied++;
                stats->bytes_copied += src_st.st_size;
                
                if (config->verbose) {
                    printf("%s复制文件: %s -> %s (%s)%s\n", 
                           COLOR_GREEN, source, dest, format_size(src_st.st_size), COLOR_RESET);
                }
            } else {
                stats->errors++;
            }
        }
    }
    
    return (stats->errors == 0);
}

// 显示统计信息
void show_stats(const CopyStats *stats) {
    printf("\n%s复制统计:%s\n", COLOR_CYAN, COLOR_RESET);
    printf("%s文件复制: %d 个%s\n", COLOR_GREEN, stats->files_copied, COLOR_RESET);
    printf("%s目录创建: %d 个%s\n", COLOR_GREEN, stats->directories_created, COLOR_RESET);
    printf("%s数据复制: %s%s\n", COLOR_CYAN, format_size(stats->bytes_copied), COLOR_RESET);
    
    if (stats->errors > 0) {
        printf("%s错误数量: %d 个%s\n", COLOR_RED, stats->errors, COLOR_RESET);
    } else {
        printf("%s复制完成！%s\n", COLOR_GREEN, COLOR_RESET);
    }
}

void print_usage(const char *program_name) {
    printf("用法: %s [选项] 源文件... 目标\n", program_name);
    printf("优化版的 cp 命令，提供彩色输出和进度显示\n\n");
    printf("选项:\n");
    printf("  -r, --recursive      递归复制目录\n");
    printf("  -p, --preserve       保留文件属性\n");
    printf("  -u, --update         只复制较新的文件\n");
    printf("  -i, --interactive    交互式确认\n");
    printf("  -f, --force          强制覆盖\n");
    printf("  -v, --verbose        显示详细信息\n");
    printf("  --progress           显示进度条\n");
    printf("  -h, --help           显示此帮助信息\n");
    printf("  -V, --version        显示版本信息\n");
    printf("\n示例:\n");
    printf("  %s file.txt backup/\n", program_name);
    printf("  %s -r source/ dest/\n", program_name);
    printf("  %s -p -v file1 file2 backup/\n", program_name);
    printf("  %s -u --progress *.txt archive/\n", program_name);
}

int main(int argc, char *argv[]) {
    CopyConfig config = {0};
    config.mode = COPY_SIMPLE;
    config.verbose = 0;
    config.force = 0;
    config.preserve_attributes = 0;
    config.update_only = 0;
    config.interactive = 0;
    config.show_progress = 0;
    config.sources = malloc(MAX_FILES * sizeof(char*));
    config.source_count = 0;
    
    // 解析命令行参数
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-r") == 0 || strcmp(argv[i], "--recursive") == 0) {
            config.mode = COPY_RECURSIVE;
        } else if (strcmp(argv[i], "-p") == 0 || strcmp(argv[i], "--preserve") == 0) {
            config.mode = COPY_PRESERVE;
            config.preserve_attributes = 1;
        } else if (strcmp(argv[i], "-u") == 0 || strcmp(argv[i], "--update") == 0) {
            config.update_only = 1;
        } else if (strcmp(argv[i], "-i") == 0 || strcmp(argv[i], "--interactive") == 0) {
            config.interactive = 1;
        } else if (strcmp(argv[i], "-f") == 0 || strcmp(argv[i], "--force") == 0) {
            config.force = 1;
        } else if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--verbose") == 0) {
            config.verbose = 1;
        } else if (strcmp(argv[i], "--progress") == 0) {
            config.show_progress = 1;
        } else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            print_usage(argv[0]);
            free(config.sources);
            return 0;
        } else if (strcmp(argv[i], "-V") == 0 || strcmp(argv[i], "--version") == 0) {
            printf("pcp - 优化版 cp 命令 v1.0\n");
            free(config.sources);
            return 0;
        } else if (argv[i][0] != '-') {
            if (config.source_count < MAX_FILES) {
                config.sources[config.source_count] = argv[i];
                config.source_count++;
            }
        } else {
            printf("未知选项: %s\n", argv[i]);
            print_usage(argv[0]);
            free(config.sources);
            return 1;
        }
    }
    
    // 检查必需参数
    if (config.source_count == 0) {
        print_error("请指定源文件");
        print_usage(argv[0]);
        free(config.sources);
        return 1;
    }
    
    if (config.source_count == 1) {
        print_error("请指定目标位置");
        print_usage(argv[0]);
        free(config.sources);
        return 1;
    }
    
    // 最后一个参数是目标
    strncpy(config.destination, config.sources[config.source_count - 1], MAX_FILENAME - 1);
    config.destination[MAX_FILENAME - 1] = '\0';
    config.source_count--;
    
    // 执行复制
    CopyStats stats;
    int success = perform_copy(&config, &stats);
    
    // 显示统计信息
    show_stats(&stats);
    
    free(config.sources);
    return success ? 0 : 1;
}
