#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/statvfs.h>
#include <mntent.h>
#include <sys/types.h>
#include <pwd.h>
#include "../include/common.h"

#define MAX_MOUNTS 100
#define BAR_WIDTH 50

typedef struct {
    char filesystem[256];
    char mount_point[256];
    char type[64];
    unsigned long total_blocks;
    unsigned long free_blocks;
    unsigned long used_blocks;
    unsigned long available_blocks;
    unsigned long block_size;
    float usage_percent;
    char total_size[32];
    char used_size[32];
    char free_size[32];
    char available_size[32];
} mount_info_t;

typedef struct {
    mount_info_t mounts[MAX_MOUNTS];
    int mount_count;
    int show_all;
    int show_inodes;
    int show_human_readable;
    int show_graph;
    int sort_by_usage;
    char filter_type[64];
} df_options_t;

// 获取挂载点信息
int get_mount_info(const char *mount_point, mount_info_t *mount) {
    struct statvfs vfs;
    
    if (statvfs(mount_point, &vfs) != 0) {
        return 0;
    }
    
    strncpy(mount->mount_point, mount_point, sizeof(mount->mount_point) - 1);
    mount->mount_point[sizeof(mount->mount_point) - 1] = '\0';
    
    mount->block_size = vfs.f_frsize;
    mount->total_blocks = vfs.f_blocks;
    mount->free_blocks = vfs.f_bavail;
    mount->used_blocks = vfs.f_blocks - vfs.f_bavail;
    mount->available_blocks = vfs.f_bavail;
    
    if (vfs.f_blocks > 0) {
        mount->usage_percent = (float)(mount->used_blocks * 100.0) / vfs.f_blocks;
    } else {
        mount->usage_percent = 0.0;
    }
    
    // 格式化大小
    strcpy(mount->total_size, format_size(mount->total_blocks * mount->block_size));
    strcpy(mount->used_size, format_size(mount->used_blocks * mount->block_size));
    strcpy(mount->free_size, format_size(mount->free_blocks * mount->block_size));
    strcpy(mount->available_size, format_size(mount->available_blocks * mount->block_size));
    
    return 1;
}

// 获取所有挂载点
int get_all_mounts(df_options_t *options) {
    FILE *mtab;
    struct mntent *entry;
    int count = 0;
    
    mtab = setmntent("/proc/mounts", "r");
    if (!mtab) {
        return 0;
    }
    
    while ((entry = getmntent(mtab)) != NULL && count < MAX_MOUNTS) {
        // 跳过虚拟文件系统（除非指定显示所有）
        if (!options->show_all) {
            if (strcmp(entry->mnt_type, "proc") == 0 ||
                strcmp(entry->mnt_type, "sysfs") == 0 ||
                strcmp(entry->mnt_type, "devtmpfs") == 0 ||
                strcmp(entry->mnt_type, "devpts") == 0 ||
                strcmp(entry->mnt_type, "tmpfs") == 0 ||
                strcmp(entry->mnt_type, "debugfs") == 0 ||
                strcmp(entry->mnt_type, "cgroup") == 0 ||
                strcmp(entry->mnt_type, "cgroup2") == 0 ||
                strcmp(entry->mnt_type, "pstore") == 0 ||
                strcmp(entry->mnt_type, "bpf") == 0 ||
                strcmp(entry->mnt_type, "tracefs") == 0 ||
                strcmp(entry->mnt_type, "hugetlbfs") == 0 ||
                strcmp(entry->mnt_type, "mqueue") == 0 ||
                strcmp(entry->mnt_type, "overlay") == 0) {
                continue;
            }
        }
        
        // 过滤文件系统类型
        if (strlen(options->filter_type) > 0) {
            if (strcmp(entry->mnt_type, options->filter_type) != 0) {
                continue;
            }
        }
        
        mount_info_t *mount = &options->mounts[count];
        
        strncpy(mount->filesystem, entry->mnt_fsname, sizeof(mount->filesystem) - 1);
        mount->filesystem[sizeof(mount->filesystem) - 1] = '\0';
        
        strncpy(mount->type, entry->mnt_type, sizeof(mount->type) - 1);
        mount->type[sizeof(mount->type) - 1] = '\0';
        
        if (get_mount_info(entry->mnt_dir, mount)) {
            count++;
        }
    }
    
    endmntent(mtab);
    options->mount_count = count;
    return count;
}

// 比较函数用于排序
int compare_mounts(const void *a, const void *b) {
    const mount_info_t *mount_a = (const mount_info_t *)a;
    const mount_info_t *mount_b = (const mount_info_t *)b;
    
    // 按使用率排序
    if (mount_a->usage_percent > mount_b->usage_percent) return -1;
    if (mount_a->usage_percent < mount_b->usage_percent) return 1;
    
    return 0;
}

// 绘制使用率进度条
void draw_usage_bar(float usage_percent, int width) {
    int bar_length = (int)(usage_percent * width / 100.0);
    if (bar_length > width) bar_length = width;
    
    const char *color = COLOR_GREEN;
    if (usage_percent > 90) color = COLOR_RED;
    else if (usage_percent > 80) color = COLOR_YELLOW;
    else if (usage_percent > 70) color = COLOR_CYAN;
    
    printf("%s[", COLOR_CYAN);
    
    for (int i = 0; i < bar_length; i++) {
        printf("%s█", color);
    }
    
    for (int i = bar_length; i < width; i++) {
        printf(" ");
    }
    
    printf("%s]%s", COLOR_CYAN, COLOR_RESET);
}

// 获取文件系统图标
const char* get_filesystem_icon(const char *type) {
    if (strcmp(type, "ext4") == 0 || strcmp(type, "ext3") == 0 || strcmp(type, "ext2") == 0) {
        return "📁";
    } else if (strcmp(type, "xfs") == 0) {
        return "💾";
    } else if (strcmp(type, "btrfs") == 0) {
        return "🌳";
    } else if (strcmp(type, "ntfs") == 0 || strcmp(type, "vfat") == 0 || strcmp(type, "fat32") == 0) {
        return "💿";
    } else if (strcmp(type, "nfs") == 0) {
        return "🌐";
    } else if (strcmp(type, "tmpfs") == 0) {
        return "⚡";
    } else if (strcmp(type, "proc") == 0) {
        return "⚙️";
    } else if (strcmp(type, "sysfs") == 0) {
        return "🔧";
    } else {
        return "💽";
    }
}

// 显示挂载点信息
void display_mount(const mount_info_t *mount, df_options_t *options) {
    const char *icon = get_filesystem_icon(mount->type);
    
    // 根据使用率设置颜色
    const char *usage_color = COLOR_WHITE;
    if (mount->usage_percent > 90) usage_color = COLOR_RED;
    else if (mount->usage_percent > 80) usage_color = COLOR_YELLOW;
    else if (mount->usage_percent > 70) usage_color = COLOR_CYAN;
    else if (mount->usage_percent > 50) usage_color = COLOR_GREEN;
    
    printf("%s %s%-20s%s %s%-30s%s %s%-8s%s %s%8s%s %s%8s%s %s%8s%s %s%8s%s %s%6.1f%%%s",
           icon,
           COLOR_BLUE, mount->filesystem, COLOR_RESET,
           COLOR_GREEN, mount->mount_point, COLOR_RESET,
           COLOR_MAGENTA, mount->type, COLOR_RESET,
           COLOR_CYAN, mount->total_size, COLOR_RESET,
           COLOR_YELLOW, mount->used_size, COLOR_RESET,
           COLOR_WHITE, mount->free_size, COLOR_RESET,
           COLOR_CYAN, mount->available_size, COLOR_RESET,
           usage_color, mount->usage_percent, COLOR_RESET);
    
    // 显示进度条
    if (options->show_graph) {
        printf(" ");
        draw_usage_bar(mount->usage_percent, BAR_WIDTH);
    }
    
    printf("\n");
}

// 显示所有挂载点
void display_mounts(df_options_t *options) {
    printf("%s磁盘使用情况 - 优化版 df%s\n", COLOR_CYAN, COLOR_RESET);
    printf("%s%s%s\n", COLOR_YELLOW, "================================================", COLOR_RESET);
    
    if (options->mount_count == 0) {
        printf("%s没有找到挂载点%s\n", COLOR_RED, COLOR_RESET);
        return;
    }
    
    // 排序
    if (options->sort_by_usage) {
        qsort(options->mounts, options->mount_count, sizeof(mount_info_t), compare_mounts);
    }
    
    // 表头
    printf("%s%-20s %-30s %-8s %-8s %-8s %-8s %-8s %-6s%s\n",
           COLOR_WHITE,
           "文件系统", "挂载点", "类型", "总大小", "已使用", "可用", "剩余", "使用率",
           COLOR_RESET);
    printf("%s%s%s\n", COLOR_YELLOW, "------------------------------------------------", COLOR_RESET);
    
    // 显示挂载点
    for (int i = 0; i < options->mount_count; i++) {
        display_mount(&options->mounts[i], options);
    }
    
    printf("%s%s%s\n", COLOR_YELLOW, "================================================", COLOR_RESET);
    
    // 计算总计
    unsigned long total_size = 0, total_used = 0, total_free = 0;
    for (int i = 0; i < options->mount_count; i++) {
        total_size += options->mounts[i].total_blocks * options->mounts[i].block_size;
        total_used += options->mounts[i].used_blocks * options->mounts[i].block_size;
        total_free += options->mounts[i].free_blocks * options->mounts[i].block_size;
    }
    
    printf("%s总计: %s 已使用: %s 可用: %s%s\n",
           COLOR_CYAN,
           format_size(total_size),
           format_size(total_used),
           format_size(total_free),
           COLOR_RESET);
}

void print_usage(const char *program_name) {
    printf("用法: %s [选项] [文件系统...]\n", program_name);
    printf("优化版的 df 命令，提供磁盘空间可视化显示\n\n");
    printf("选项:\n");
    printf("  -a, --all            显示所有文件系统\n");
    printf("  -h, --human-readable 以人类可读的格式显示大小\n");
    printf("  -g, --graph          显示图形化进度条\n");
    printf("  -s, --sort           按使用率排序\n");
    printf("  -t, --type 类型      只显示指定类型的文件系统\n");
    printf("  -i, --inodes         显示inode信息\n");
    printf("  --help               显示此帮助信息\n");
    printf("  --version            显示版本信息\n");
    printf("\n示例:\n");
    printf("  %s                    # 显示磁盘使用情况\n", program_name);
    printf("  %s -a                 # 显示所有文件系统\n", program_name);
    printf("  %s -g                 # 显示图形化进度条\n", program_name);
    printf("  %s -t ext4            # 只显示ext4文件系统\n", program_name);
    printf("  %s -s                 # 按使用率排序\n", program_name);
}

int main(int argc, char *argv[]) {
    df_options_t options = {0};
    char *target_filesystems[argc];
    int fs_count = 0;
    
    // 设置默认值
    options.show_human_readable = 1;
    options.show_graph = 1;
    
    // 解析命令行参数
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-a") == 0 || strcmp(argv[i], "--all") == 0) {
            options.show_all = 1;
        } else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--human-readable") == 0) {
            options.show_human_readable = 1;
        } else if (strcmp(argv[i], "-g") == 0 || strcmp(argv[i], "--graph") == 0) {
            options.show_graph = 1;
        } else if (strcmp(argv[i], "-s") == 0 || strcmp(argv[i], "--sort") == 0) {
            options.sort_by_usage = 1;
        } else if (strcmp(argv[i], "-t") == 0 || strcmp(argv[i], "--type") == 0) {
            if (i + 1 < argc) {
                strncpy(options.filter_type, argv[++i], sizeof(options.filter_type) - 1);
                options.filter_type[sizeof(options.filter_type) - 1] = '\0';
            }
        } else if (strcmp(argv[i], "-i") == 0 || strcmp(argv[i], "--inodes") == 0) {
            options.show_inodes = 1;
        } else if (strcmp(argv[i], "--help") == 0) {
            print_usage(argv[0]);
            return 0;
        } else if (strcmp(argv[i], "--version") == 0) {
            printf("pdf - 优化版 df 命令 v1.0\n");
            return 0;
        } else if (argv[i][0] != '-') {
            target_filesystems[fs_count++] = argv[i];
        }
    }
    
    // 获取挂载点信息
    if (get_all_mounts(&options) == 0) {
        print_error("无法获取挂载点信息");
        return 1;
    }
    
    // 显示结果
    display_mounts(&options);
    
    return 0;
}
