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

// 移动模式枚举
typedef enum {
    MOVE_SIMPLE,        // 简单移动
    MOVE_RECURSIVE,     // 递归移动
    MOVE_PRESERVE,      // 保留属性
    MOVE_BACKUP         // 备份模式
} MoveMode;

// 移动配置结构
typedef struct {
    char **sources;
    int source_count;
    char destination[MAX_FILENAME];
    MoveMode mode;
    int verbose;
    int force;
    int interactive;
    int backup;
    int preserve_attributes;
} MoveConfig;

// 统计信息结构
typedef struct {
    int files_moved;
    int directories_moved;
    off_t bytes_moved;
    int errors;
} MoveStats;

// 初始化统计信息
void init_stats(MoveStats *stats) {
    stats->files_moved = 0;
    stats->directories_moved = 0;
    stats->bytes_moved = 0;
    stats->errors = 0;
}

// 检查文件是否存在
int file_exists(const char *path) {
    return (access(path, F_OK) == 0);
}

// 创建备份文件
int create_backup(const char *filepath) {
    char backup_path[MAX_FILENAME];
    snprintf(backup_path, sizeof(backup_path), "%s~", filepath);
    
    if (rename(filepath, backup_path) == 0) {
        return 1;
    }
    return 0;
}

// 移动文件
int move_file(const char *source, const char *dest, const MoveConfig *config) {
    // 检查目标文件是否存在
    if (file_exists(dest)) {
        if (config->interactive) {
            printf("目标文件已存在: %s\n", dest);
            printf("是否覆盖? (y/n): ");
            char response;
            if (scanf(" %c", &response) != 1 || (response != 'y' && response != 'Y')) {
                printf("跳过: %s\n", source);
                return 1;
            }
        } else if (!config->force) {
            print_error("目标文件已存在，使用 -f 强制覆盖");
            return 0;
        }
        
        if (config->backup) {
            if (!create_backup(dest)) {
                print_warning("无法创建备份文件");
            } else if (config->verbose) {
                printf("%s创建备份: %s~%s\n", COLOR_YELLOW, dest, COLOR_RESET);
            }
        }
    }
    
    // 获取源文件信息
    struct stat src_st;
    if (stat(source, &src_st) != 0) {
        print_error("无法获取源文件信息");
        return 0;
    }
    
    // 尝试重命名（同一文件系统内最快）
    if (rename(source, dest) == 0) {
        if (config->verbose) {
            printf("%s移动文件: %s -> %s%s\n", 
                   COLOR_GREEN, source, dest, COLOR_RESET);
        }
        return 1;
    }
    
    // 重命名失败，可能是跨文件系统，需要复制后删除
    if (errno == EXDEV) {
        // 复制文件
        int src_fd = open(source, O_RDONLY);
        if (src_fd == -1) {
            print_error("无法打开源文件");
            return 0;
        }
        
        int dest_fd = open(dest, O_WRONLY | O_CREAT | O_TRUNC, src_st.st_mode);
        if (dest_fd == -1) {
            close(src_fd);
            print_error("无法创建目标文件");
            return 0;
        }
        
        char buffer[8192];
        ssize_t bytes_read, bytes_written;
        
        while ((bytes_read = read(src_fd, buffer, sizeof(buffer))) > 0) {
            bytes_written = write(dest_fd, buffer, bytes_read);
            if (bytes_written != bytes_read) {
                close(src_fd);
                close(dest_fd);
                print_error("写入文件失败");
                return 0;
            }
        }
        
        close(src_fd);
        close(dest_fd);
        
        // 保留文件属性
        if (config->preserve_attributes) {
            struct utimbuf times;
            times.actime = src_st.st_atime;
            times.modtime = src_st.st_mtime;
            utime(dest, &times);
            chmod(dest, src_st.st_mode);
        }
        
        // 删除源文件
        if (unlink(source) != 0) {
            print_warning("无法删除源文件");
        }
        
        if (config->verbose) {
            printf("%s移动文件: %s -> %s (跨文件系统)%s\n", 
                   COLOR_GREEN, source, dest, COLOR_RESET);
        }
        
        return 1;
    }
    
    print_error("移动文件失败");
    return 0;
}

// 移动目录
int move_directory(const char *source, const char *dest, const MoveConfig *config) {
    // 检查目标目录是否存在
    if (file_exists(dest)) {
        if (config->interactive) {
            printf("目标目录已存在: %s\n", dest);
            printf("是否覆盖? (y/n): ");
            char response;
            if (scanf(" %c", &response) != 1 || (response != 'y' && response != 'Y')) {
                printf("跳过: %s\n", source);
                return 1;
            }
        } else if (!config->force) {
            print_error("目标目录已存在，使用 -f 强制覆盖");
            return 0;
        }
    }
    
    // 尝试重命名目录
    if (rename(source, dest) == 0) {
        if (config->verbose) {
            printf("%s移动目录: %s -> %s%s\n", 
                   COLOR_GREEN, source, dest, COLOR_RESET);
        }
        return 1;
    }
    
    // 重命名失败，需要递归移动
    if (errno == EXDEV) {
        // 创建目标目录
        struct stat src_st;
        if (stat(source, &src_st) != 0) {
            print_error("无法获取源目录信息");
            return 0;
        }
        
        if (mkdir(dest, src_st.st_mode) != 0 && errno != EEXIST) {
            print_error("无法创建目标目录");
            return 0;
        }
        
        // 递归移动目录内容
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
                if (!move_directory(src_path, dest_path, config)) {
                    success = 0;
                }
            } else {
                if (!move_file(src_path, dest_path, config)) {
                    success = 0;
                }
            }
        }
        
        closedir(dir);
        
        // 删除空源目录
        if (success && rmdir(source) != 0) {
            print_warning("无法删除源目录");
        }
        
        if (config->verbose) {
            printf("%s移动目录: %s -> %s (跨文件系统)%s\n", 
                   COLOR_GREEN, source, dest, COLOR_RESET);
        }
        
        return success;
    }
    
    print_error("移动目录失败");
    return 0;
}

// 执行移动操作
int perform_move(const MoveConfig *config, MoveStats *stats) {
    init_stats(stats);
    
    printf("%s开始移动文件...%s\n", COLOR_CYAN, COLOR_RESET);
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
            // 移动目录
            if (config->mode == MOVE_RECURSIVE || config->mode == MOVE_PRESERVE) {
                if (move_directory(source, dest, config)) {
                    stats->directories_moved++;
                    stats->bytes_moved += src_st.st_size;
                } else {
                    stats->errors++;
                }
            } else {
                print_warning("跳过目录（使用 -r 选项启用递归移动）");
            }
        } else {
            // 移动文件
            if (config->interactive) {
                printf("移动 %s 到 %s? (y/n): ", source, dest);
                char response;
                if (scanf(" %c", &response) != 1 || (response != 'y' && response != 'Y')) {
                    continue;
                }
            }
            
            if (move_file(source, dest, config)) {
                stats->files_moved++;
                stats->bytes_moved += src_st.st_size;
            } else {
                stats->errors++;
            }
        }
    }
    
    return (stats->errors == 0);
}

// 显示统计信息
void show_stats(const MoveStats *stats) {
    printf("\n%s移动统计:%s\n", COLOR_CYAN, COLOR_RESET);
    printf("%s文件移动: %d 个%s\n", COLOR_GREEN, stats->files_moved, COLOR_RESET);
    printf("%s目录移动: %d 个%s\n", COLOR_GREEN, stats->directories_moved, COLOR_RESET);
    printf("%s数据移动: %s%s\n", COLOR_CYAN, format_size(stats->bytes_moved), COLOR_RESET);
    
    if (stats->errors > 0) {
        printf("%s错误数量: %d 个%s\n", COLOR_RED, stats->errors, COLOR_RESET);
    } else {
        printf("%s移动完成！%s\n", COLOR_GREEN, COLOR_RESET);
    }
}

void print_usage(const char *program_name) {
    printf("用法: %s [选项] 源文件... 目标\n", program_name);
    printf("优化版的 mv 命令，提供彩色输出和交互式操作\n\n");
    printf("选项:\n");
    printf("  -r, --recursive      递归移动目录\n");
    printf("  -p, --preserve       保留文件属性\n");
    printf("  -b, --backup         创建备份文件\n");
    printf("  -i, --interactive    交互式确认\n");
    printf("  -f, --force          强制覆盖\n");
    printf("  -v, --verbose        显示详细信息\n");
    printf("  -h, --help           显示此帮助信息\n");
    printf("  -V, --version        显示版本信息\n");
    printf("\n示例:\n");
    printf("  %s file.txt backup/\n", program_name);
    printf("  %s -r source/ dest/\n", program_name);
    printf("  %s -i -b old.txt new.txt\n", program_name);
    printf("  %s -v *.txt archive/\n", program_name);
}

int main(int argc, char *argv[]) {
    MoveConfig config = {0};
    config.mode = MOVE_SIMPLE;
    config.verbose = 0;
    config.force = 0;
    config.interactive = 0;
    config.backup = 0;
    config.preserve_attributes = 0;
    config.sources = malloc(MAX_FILES * sizeof(char*));
    config.source_count = 0;
    
    // 解析命令行参数
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-r") == 0 || strcmp(argv[i], "--recursive") == 0) {
            config.mode = MOVE_RECURSIVE;
        } else if (strcmp(argv[i], "-p") == 0 || strcmp(argv[i], "--preserve") == 0) {
            config.mode = MOVE_PRESERVE;
            config.preserve_attributes = 1;
        } else if (strcmp(argv[i], "-b") == 0 || strcmp(argv[i], "--backup") == 0) {
            config.backup = 1;
        } else if (strcmp(argv[i], "-i") == 0 || strcmp(argv[i], "--interactive") == 0) {
            config.interactive = 1;
        } else if (strcmp(argv[i], "-f") == 0 || strcmp(argv[i], "--force") == 0) {
            config.force = 1;
        } else if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--verbose") == 0) {
            config.verbose = 1;
        } else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            print_usage(argv[0]);
            free(config.sources);
            return 0;
        } else if (strcmp(argv[i], "-V") == 0 || strcmp(argv[i], "--version") == 0) {
            printf("pmv - 优化版 mv 命令 v1.0\n");
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
    
    // 执行移动
    MoveStats stats;
    int success = perform_move(&config, &stats);
    
    // 显示统计信息
    show_stats(&stats);
    
    free(config.sources);
    return success ? 0 : 1;
}
