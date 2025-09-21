#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <sys/statvfs.h>
#include <mntent.h>
#include <getopt.h>
#include <time.h>
#include <ctype.h>
#include <errno.h>
#include <dirent.h>

#include "../include/common.h"

// 挂载点信息结构
typedef struct {
    char device[256];
    char mountpoint[256];
    char fstype[64];
    char options[512];
    char dump[16];
    char pass[16];
    char usage_percent[16];
    char total_size[32];
    char used_size[32];
    char available_size[32];
    char inodes_total[32];
    char inodes_used[32];
    char inodes_available[32];
    time_t mount_time;
    int is_removable;
    int is_network;
    int is_system;
} mount_info_t;

// 选项结构
typedef struct {
    int show_all;
    int show_fstype;
    int show_options;
    int show_usage;
    int show_inodes;
    int show_removable;
    int show_network;
    int show_system;
    int show_tree;
    int show_summary;
    int verbose;
    int color_output;
    int show_headers;
    char *filter_fstype;
    char *filter_device;
    char *filter_mountpoint;
    int filter_usage_min;
    int filter_usage_max;
} mount_options_t;

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

// 文件系统类型图标
const char* get_fstype_icon(const char* fstype) {
    if (strcmp(fstype, "ext4") == 0 || strcmp(fstype, "ext3") == 0 || strcmp(fstype, "ext2") == 0) {
        return "📁";
    } else if (strcmp(fstype, "xfs") == 0) {
        return "🗂️";
    } else if (strcmp(fstype, "btrfs") == 0) {
        return "🌳";
    } else if (strcmp(fstype, "ntfs") == 0 || strcmp(fstype, "vfat") == 0 || strcmp(fstype, "fat32") == 0) {
        return "💾";
    } else if (strcmp(fstype, "nfs") == 0 || strcmp(fstype, "cifs") == 0) {
        return "🌐";
    } else if (strcmp(fstype, "tmpfs") == 0 || strcmp(fstype, "devtmpfs") == 0) {
        return "⚡";
    } else if (strcmp(fstype, "proc") == 0 || strcmp(fstype, "sysfs") == 0) {
        return "🔧";
    } else if (strcmp(fstype, "devpts") == 0) {
        return "⌨️";
    } else {
        return "📀";
    }
}

// 使用率颜色
const char* get_usage_color(int usage_percent) {
    if (usage_percent >= 90) {
        return COLOR_RED;
    } else if (usage_percent >= 80) {
        return COLOR_YELLOW;
    } else if (usage_percent >= 70) {
        return COLOR_MAGENTA;
    } else {
        return COLOR_GREEN;
    }
}

// 设备类型图标
const char* get_device_icon(const char* device) {
    if (strncmp(device, "/dev/sd", 7) == 0 || strncmp(device, "/dev/nvme", 9) == 0) {
        return "💽";
    } else if (strncmp(device, "/dev/loop", 9) == 0) {
        return "🔄";
    } else if (strncmp(device, "/dev/mapper", 11) == 0) {
        return "🔗";
    } else if (strncmp(device, "tmpfs", 5) == 0) {
        return "⚡";
    } else if (strncmp(device, "proc", 4) == 0 || strncmp(device, "sysfs", 5) == 0) {
        return "🔧";
    } else if (strncmp(device, "//", 2) == 0 || strncmp(device, "192.168.", 8) == 0) {
        return "🌐";
    } else {
        return "📀";
    }
}

// 读取挂载信息
int read_mount_info(mount_info_t **mounts, int *count) {
    FILE *fp = setmntent("/proc/mounts", "r");
    if (!fp) return -1;
    
    int capacity = 100;
    *count = 0;
    *mounts = malloc(capacity * sizeof(mount_info_t));
    
    struct mntent *mnt;
    while ((mnt = getmntent(fp)) != NULL) {
        if (*count >= capacity) {
            capacity *= 2;
            *mounts = realloc(*mounts, capacity * sizeof(mount_info_t));
        }
        
        mount_info_t *mount = &(*mounts)[*count];
        memset(mount, 0, sizeof(mount_info_t));
        
        strncpy(mount->device, mnt->mnt_fsname, sizeof(mount->device) - 1);
        strncpy(mount->mountpoint, mnt->mnt_dir, sizeof(mount->mountpoint) - 1);
        strncpy(mount->fstype, mnt->mnt_type, sizeof(mount->fstype) - 1);
        strncpy(mount->options, mnt->mnt_opts, sizeof(mount->options) - 1);
        sprintf(mount->dump, "%d", mnt->mnt_freq);
        sprintf(mount->pass, "%d", mnt->mnt_passno);
        
        // 获取文件系统统计信息
        struct statvfs vfs;
        if (statvfs(mount->mountpoint, &vfs) == 0) {
            unsigned long total_blocks = vfs.f_blocks;
            unsigned long free_blocks = vfs.f_bavail;
            unsigned long used_blocks = total_blocks - free_blocks;
            unsigned long block_size = vfs.f_frsize;
            
            unsigned long total_bytes = total_blocks * block_size;
            unsigned long used_bytes = used_blocks * block_size;
            unsigned long free_bytes = free_blocks * block_size;
            
            int usage_percent = total_blocks > 0 ? (used_blocks * 100) / total_blocks : 0;
            sprintf(mount->usage_percent, "%d%%", usage_percent);
            
            // 简化大小格式化
            if (total_bytes >= 1024*1024*1024) {
                sprintf(mount->total_size, "%.1fG", total_bytes / (1024.0*1024.0*1024.0));
            } else if (total_bytes >= 1024*1024) {
                sprintf(mount->total_size, "%.1fM", total_bytes / (1024.0*1024.0));
            } else if (total_bytes >= 1024) {
                sprintf(mount->total_size, "%.1fK", total_bytes / 1024.0);
            } else {
                sprintf(mount->total_size, "%luB", total_bytes);
            }
            
            if (used_bytes >= 1024*1024*1024) {
                sprintf(mount->used_size, "%.1fG", used_bytes / (1024.0*1024.0*1024.0));
            } else if (used_bytes >= 1024*1024) {
                sprintf(mount->used_size, "%.1fM", used_bytes / (1024.0*1024.0));
            } else if (used_bytes >= 1024) {
                sprintf(mount->used_size, "%.1fK", used_bytes / 1024.0);
            } else {
                sprintf(mount->used_size, "%luB", used_bytes);
            }
            
            if (free_bytes >= 1024*1024*1024) {
                sprintf(mount->available_size, "%.1fG", free_bytes / (1024.0*1024.0*1024.0));
            } else if (free_bytes >= 1024*1024) {
                sprintf(mount->available_size, "%.1fM", free_bytes / (1024.0*1024.0));
            } else if (free_bytes >= 1024) {
                sprintf(mount->available_size, "%.1fK", free_bytes / 1024.0);
            } else {
                sprintf(mount->available_size, "%luB", free_bytes);
            }
            
            // inode信息
            sprintf(mount->inodes_total, "%lu", vfs.f_files);
            sprintf(mount->inodes_used, "%lu", vfs.f_files - vfs.f_ffree);
            sprintf(mount->inodes_available, "%lu", vfs.f_ffree);
        } else {
            strcpy(mount->usage_percent, "N/A");
            strcpy(mount->total_size, "N/A");
            strcpy(mount->used_size, "N/A");
            strcpy(mount->available_size, "N/A");
            strcpy(mount->inodes_total, "N/A");
            strcpy(mount->inodes_used, "N/A");
            strcpy(mount->inodes_available, "N/A");
        }
        
        // 判断设备类型
        mount->is_removable = (strncmp(mount->device, "/dev/sd", 7) == 0 && 
                              strchr(mount->device, 'a') != NULL);
        mount->is_network = (strncmp(mount->device, "//", 2) == 0 || 
                            strncmp(mount->device, "192.168.", 8) == 0 ||
                            strstr(mount->device, "nfs") != NULL ||
                            strstr(mount->device, "cifs") != NULL);
        mount->is_system = (strcmp(mount->fstype, "proc") == 0 || 
                           strcmp(mount->fstype, "sysfs") == 0 ||
                           strcmp(mount->fstype, "devtmpfs") == 0 ||
                           strcmp(mount->fstype, "devpts") == 0 ||
                           strcmp(mount->fstype, "tmpfs") == 0);
        
        mount->mount_time = time(NULL);
        (*count)++;
    }
    
    endmntent(fp);
    return 0;
}

// 显示挂载信息
void display_mounts(mount_info_t *mounts, int count, mount_options_t *options) {
    if (options->show_headers) {
        printf("%s%s挂载点信息 - 优化版 mount%s\n", COLOR_CYAN, COLOR_BOLD, COLOR_RESET);
        printf("%s%s\n", COLOR_YELLOW, "================================================");
        printf("%s%-20s %-30s %-8s %-12s %-8s %-16s%s\n", 
               COLOR_WHITE, "设备", "挂载点", "类型", "使用率", "大小", "选项", COLOR_RESET);
        printf("%s%s\n", COLOR_YELLOW, "================================================");
    }
    
    for (int i = 0; i < count; i++) {
        mount_info_t *mount = &mounts[i];
        
        // 应用过滤器
        if (options->filter_fstype && strcmp(mount->fstype, options->filter_fstype) != 0) {
            continue;
        }
        
        if (options->filter_device && strstr(mount->device, options->filter_device) == NULL) {
            continue;
        }
        
        if (options->filter_mountpoint && strstr(mount->mountpoint, options->filter_mountpoint) == NULL) {
            continue;
        }
        
        if (options->filter_usage_min > 0 || options->filter_usage_max > 0) {
            int usage = atoi(mount->usage_percent);
            if (options->filter_usage_min > 0 && usage < options->filter_usage_min) {
                continue;
            }
            if (options->filter_usage_max > 0 && usage > options->filter_usage_max) {
                continue;
            }
        }
        
        // 设备图标
        const char *device_icon = get_device_icon(mount->device);
        const char *fstype_icon = get_fstype_icon(mount->fstype);
        
        // 使用率颜色
        int usage_percent = atoi(mount->usage_percent);
        const char *usage_color = get_usage_color(usage_percent);
        
        // 设备类型颜色
        const char *device_color = COLOR_WHITE;
        if (mount->is_network) {
            device_color = COLOR_CYAN;
        } else if (mount->is_removable) {
            device_color = COLOR_YELLOW;
        } else if (mount->is_system) {
            device_color = COLOR_MAGENTA;
        }
        
        printf("%s%s %s%-18s%s %s%-28s%s %s%s %s%-6s%s %s%-10s%s %s%-6s%s %s%-14s%s\n",
               COLOR_CYAN, device_icon, COLOR_RESET,
               device_color, mount->device, COLOR_RESET,
               COLOR_GREEN, mount->mountpoint, COLOR_RESET,
               COLOR_BLUE, fstype_icon, mount->fstype, COLOR_RESET,
               usage_color, mount->usage_percent, COLOR_RESET,
               COLOR_MAGENTA, mount->total_size, COLOR_RESET,
               COLOR_WHITE, mount->options, COLOR_RESET);
        
        if (options->verbose) {
            printf("    %s已用: %s%s  %s可用: %s%s  %sinode: %s/%s%s\n",
                   COLOR_RED, mount->used_size, COLOR_RESET,
                   COLOR_GREEN, mount->available_size, COLOR_RESET,
                   COLOR_CYAN, mount->inodes_used, mount->inodes_total, COLOR_RESET);
        }
    }
}

// 显示树形结构
void display_mount_tree(mount_info_t *mounts, int count) {
    printf("%s%s挂载点树形结构%s\n", COLOR_CYAN, COLOR_BOLD, COLOR_RESET);
    printf("%s%s\n", COLOR_YELLOW, "================================================");
    
    // 按挂载点深度排序（简化实现）
    for (int i = 0; i < count; i++) {
        mount_info_t *mount = &mounts[i];
        if (strcmp(mount->mountpoint, "/") == 0) {
            printf("%s%s %s%s%s\n", COLOR_CYAN, "🌳", COLOR_GREEN, mount->mountpoint, COLOR_RESET);
        }
    }
    
    for (int i = 0; i < count; i++) {
        mount_info_t *mount = &mounts[i];
        if (strcmp(mount->mountpoint, "/") != 0) {
            const char *icon = get_device_icon(mount->device);
            const char *fstype_icon = get_fstype_icon(mount->fstype);
            printf("  %s├─ %s%s %s%s%s %s(%s)%s\n",
                   COLOR_YELLOW, COLOR_CYAN, icon, COLOR_GREEN, mount->mountpoint, COLOR_RESET,
                   COLOR_BLUE, fstype_icon, mount->fstype, COLOR_RESET);
        }
    }
}

// 显示摘要信息
void display_summary(mount_info_t *mounts, int count) {
    int total_mounts = count;
    int removable_mounts = 0;
    int network_mounts = 0;
    int system_mounts = 0;
    int high_usage_mounts = 0;
    
    // 统计信息（简化实现）
    // 这些变量在简化实现中未使用
    
    for (int i = 0; i < count; i++) {
        mount_info_t *mount = &mounts[i];
        
        if (mount->is_removable) removable_mounts++;
        if (mount->is_network) network_mounts++;
        if (mount->is_system) system_mounts++;
        
        int usage = atoi(mount->usage_percent);
        if (usage >= 80) high_usage_mounts++;
        
        // 计算总空间（简化实现）
        if (strcmp(mount->total_size, "N/A") != 0) {
            // 这里需要解析大小字符串，简化处理
        }
    }
    
    printf("\n%s%s挂载点摘要%s\n", COLOR_CYAN, COLOR_BOLD, COLOR_RESET);
    printf("%s%s\n", COLOR_YELLOW, "================================================");
    printf("%s总挂载点: %s%d%s\n", COLOR_WHITE, COLOR_GREEN, total_mounts, COLOR_RESET);
    printf("%s可移动设备: %s%d%s\n", COLOR_WHITE, COLOR_YELLOW, removable_mounts, COLOR_RESET);
    printf("%s网络挂载: %s%d%s\n", COLOR_WHITE, COLOR_CYAN, network_mounts, COLOR_RESET);
    printf("%s系统挂载: %s%d%s\n", COLOR_WHITE, COLOR_MAGENTA, system_mounts, COLOR_RESET);
    printf("%s高使用率(≥80%%): %s%d%s\n", COLOR_WHITE, COLOR_RED, high_usage_mounts, COLOR_RESET);
    printf("%s%s\n", COLOR_YELLOW, "================================================");
}

// 显示帮助信息
void print_help() {
    printf("%spmount - 优化版 mount 命令%s\n", COLOR_CYAN, COLOR_RESET);
    printf("%s%s\n", COLOR_YELLOW, "================================================");
    printf("用法: pmount [选项]\n\n");
    printf("选项:\n");
    printf("  -a, --all           显示所有挂载点\n");
    printf("  -t, --fstype       显示文件系统类型\n");
    printf("  -o, --options       显示挂载选项\n");
    printf("  -u, --usage         显示使用率信息\n");
    printf("  -i, --inodes        显示inode信息\n");
    printf("  -r, --removable     只显示可移动设备\n");
    printf("  -n, --network       只显示网络挂载\n");
    printf("  -s, --system        只显示系统挂载\n");
    printf("  -T, --tree          树形显示\n");
    printf("  -S, --summary       显示摘要信息\n");
    printf("  -v, --verbose       详细输出\n");
    printf("  --color             启用彩色输出\n");
    printf("  --no-color          禁用彩色输出\n");
    printf("  --filter-fstype     过滤文件系统类型\n");
    printf("  --filter-device     过滤设备\n");
    printf("  --filter-mountpoint 过滤挂载点\n");
    printf("  --filter-usage-min  最小使用率过滤\n");
    printf("  --filter-usage-max  最大使用率过滤\n");
    printf("  -h, --help          显示此帮助信息\n");
    printf("  -V, --version       显示版本信息\n\n");
    printf("示例:\n");
    printf("  pmount -a           显示所有挂载点\n");
    printf("  pmount -u           显示使用率信息\n");
    printf("  pmount -T           树形显示\n");
    printf("  pmount --filter-fstype ext4 只显示ext4文件系统\n");
}

// 显示版本信息
void print_version() {
    printf("pmount - 优化版 mount 命令 v1.0\n");
    printf("使用C语言和彩色输出\n");
}

// 解析命令行参数
void parse_arguments(int argc, char *argv[], mount_options_t *options) {
    memset(options, 0, sizeof(mount_options_t));
    options->color_output = 1;
    options->show_headers = 1;
    
    static struct option long_options[] = {
        {"all", no_argument, 0, 'a'},
        {"fstype", no_argument, 0, 't'},
        {"options", no_argument, 0, 'o'},
        {"usage", no_argument, 0, 'u'},
        {"inodes", no_argument, 0, 'i'},
        {"removable", no_argument, 0, 'r'},
        {"network", no_argument, 0, 'n'},
        {"system", no_argument, 0, 's'},
        {"tree", no_argument, 0, 'T'},
        {"summary", no_argument, 0, 'S'},
        {"verbose", no_argument, 0, 'v'},
        {"color", no_argument, 0, 'C'},
        {"no-color", no_argument, 0, 'N'},
        {"filter-fstype", required_argument, 0, 'f'},
        {"filter-device", required_argument, 0, 'd'},
        {"filter-mountpoint", required_argument, 0, 'm'},
        {"filter-usage-min", required_argument, 0, 'M'},
        {"filter-usage-max", required_argument, 0, 'X'},
        {"help", no_argument, 0, 'h'},
        {"version", no_argument, 0, 'V'},
        {0, 0, 0, 0}
    };
    
    int opt;
    int option_index = 0;
    
    while ((opt = getopt_long(argc, argv, "atuoirnsSTvhV", long_options, &option_index)) != -1) {
        switch (opt) {
            case 'a': options->show_all = 1; break;
            case 't': options->show_fstype = 1; break;
            case 'o': options->show_options = 1; break;
            case 'u': options->show_usage = 1; break;
            case 'i': options->show_inodes = 1; break;
            case 'r': options->show_removable = 1; break;
            case 'n': options->show_network = 1; break;
            case 's': options->show_system = 1; break;
            case 'T': options->show_tree = 1; break;
            case 'S': options->show_summary = 1; break;
            case 'v': options->verbose = 1; break;
            case 'C': options->color_output = 1; break;
            case 'N': options->color_output = 0; break;
            case 'f': options->filter_fstype = optarg; break;
            case 'd': options->filter_device = optarg; break;
            case 'm': options->filter_mountpoint = optarg; break;
            case 'M': options->filter_usage_min = atoi(optarg); break;
            case 'X': options->filter_usage_max = atoi(optarg); break;
            case 'h': print_help(); exit(0);
            case 'V': print_version(); exit(0);
            case '?': print_help(); exit(1);
            default: break;
        }
    }
    
    // 默认显示所有挂载点
    if (!options->show_all && !options->show_removable && !options->show_network && 
        !options->show_system && !options->show_tree && !options->show_summary) {
        options->show_all = 1;
    }
}

int main(int argc, char *argv[]) {
    mount_options_t options;
    mount_info_t *mounts = NULL;
    int count = 0;
    
    parse_arguments(argc, argv, &options);
    
    if (read_mount_info(&mounts, &count) != 0) {
        fprintf(stderr, "错误: 无法读取挂载信息\n");
        return 1;
    }
    
    if (options.show_tree) {
        display_mount_tree(mounts, count);
    } else if (options.show_summary) {
        display_summary(mounts, count);
    } else {
        display_mounts(mounts, count, &options);
    }
    
    if (mounts) free(mounts);
    return 0;
}
