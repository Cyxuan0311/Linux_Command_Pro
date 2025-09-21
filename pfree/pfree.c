#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/sysinfo.h>
#include <sys/statvfs.h>
#include "../include/common.h"

#define BAR_WIDTH 50
#define REFRESH_INTERVAL 2

typedef struct {
    unsigned long total_mem;
    unsigned long free_mem;
    unsigned long available_mem;
    unsigned long used_mem;
    unsigned long shared_mem;
    unsigned long buffer_mem;
    unsigned long cached_mem;
    unsigned long total_swap;
    unsigned long free_swap;
    unsigned long used_swap;
    float mem_usage_percent;
    float swap_usage_percent;
} memory_info_t;

typedef struct {
    memory_info_t mem_info;
    int show_human_readable;
    int show_graph;
    int continuous;
    int refresh_interval;
    int show_swap;
    int show_buffers;
    int show_cached;
} free_options_t;

// 获取内存信息
int get_memory_info(memory_info_t *mem_info) {
    struct sysinfo info;
    
    if (sysinfo(&info) != 0) {
        return 0;
    }
    
    mem_info->total_mem = info.totalram * info.mem_unit;
    mem_info->free_mem = info.freeram * info.mem_unit;
    mem_info->shared_mem = info.sharedram * info.mem_unit;
    mem_info->buffer_mem = info.bufferram * info.mem_unit;
    mem_info->total_swap = info.totalswap * info.mem_unit;
    mem_info->free_swap = info.freeswap * info.mem_unit;
    
    // 计算可用内存（包括缓存和缓冲区）
    mem_info->available_mem = mem_info->free_mem + mem_info->buffer_mem;
    
    // 计算已使用内存
    mem_info->used_mem = mem_info->total_mem - mem_info->available_mem;
    
    // 计算已使用交换空间
    mem_info->used_swap = mem_info->total_swap - mem_info->free_swap;
    
    // 计算使用率
    if (mem_info->total_mem > 0) {
        mem_info->mem_usage_percent = (float)(mem_info->used_mem * 100.0) / mem_info->total_mem;
    } else {
        mem_info->mem_usage_percent = 0.0;
    }
    
    if (mem_info->total_swap > 0) {
        mem_info->swap_usage_percent = (float)(mem_info->used_swap * 100.0) / mem_info->total_swap;
    } else {
        mem_info->swap_usage_percent = 0.0;
    }
    
    return 1;
}

// 绘制内存使用率进度条
void draw_memory_bar(float usage_percent, int width) {
    int bar_length = (int)(usage_percent * width / 100.0);
    if (bar_length > width) bar_length = width;
    
    const char *color = COLOR_GREEN;
    if (usage_percent > 90) color = COLOR_RED;
    else if (usage_percent > 80) color = COLOR_YELLOW;
    else if (usage_percent > 70) color = COLOR_CYAN;
    else if (usage_percent > 50) color = COLOR_BLUE;
    
    printf("%s[", COLOR_CYAN);
    
    for (int i = 0; i < bar_length; i++) {
        printf("%s█", color);
    }
    
    for (int i = bar_length; i < width; i++) {
        printf(" ");
    }
    
    printf("%s]%s", COLOR_CYAN, COLOR_RESET);
}

// 获取内存状态图标
const char* get_memory_status_icon(float usage_percent) {
    if (usage_percent > 90) return "🔴";
    else if (usage_percent > 80) return "🟡";
    else if (usage_percent > 70) return "🟠";
    else if (usage_percent > 50) return "🔵";
    else return "🟢";
}

// 显示内存信息
void display_memory_info(memory_info_t *mem_info, free_options_t *options) {
    printf("%s内存使用情况 - 优化版 free%s\n", COLOR_CYAN, COLOR_RESET);
    printf("%s%s%s\n", COLOR_YELLOW, "================================================", COLOR_RESET);
    
    // 内存信息
    printf("%s内存:%s\n", COLOR_WHITE, COLOR_RESET);
    printf("  %s总内存: %s %s%.1f%%%s\n",
           get_memory_status_icon(mem_info->mem_usage_percent),
           format_size(mem_info->total_mem),
           COLOR_MAGENTA, mem_info->mem_usage_percent, COLOR_RESET);
    
    printf("  %s已使用: %s%s\n",
           COLOR_RED, format_size(mem_info->used_mem), COLOR_RESET);
    
    printf("  %s可用: %s%s\n",
           COLOR_GREEN, format_size(mem_info->available_mem), COLOR_RESET);
    
    printf("  %s空闲: %s%s\n",
           COLOR_WHITE, format_size(mem_info->free_mem), COLOR_RESET);
    
    if (options->show_buffers) {
        printf("  %s缓冲区: %s%s\n",
               COLOR_BLUE, format_size(mem_info->buffer_mem), COLOR_RESET);
    }
    
    if (options->show_cached) {
        printf("  %s缓存: %s%s\n",
               COLOR_YELLOW, format_size(mem_info->cached_mem), COLOR_RESET);
    }
    
    if (options->show_swap && mem_info->total_swap > 0) {
        printf("\n%s交换空间:%s\n", COLOR_WHITE, COLOR_RESET);
        printf("  %s总交换: %s %s%.1f%%%s\n",
               get_memory_status_icon(mem_info->swap_usage_percent),
               format_size(mem_info->total_swap),
               COLOR_MAGENTA, mem_info->swap_usage_percent, COLOR_RESET);
        
        printf("  %s已使用: %s%s\n",
               COLOR_RED, format_size(mem_info->used_swap), COLOR_RESET);
        
        printf("  %s空闲: %s%s\n",
               COLOR_GREEN, format_size(mem_info->free_swap), COLOR_RESET);
    }
    
    // 图形化显示
    if (options->show_graph) {
        printf("\n%s内存使用率:%s\n", COLOR_WHITE, COLOR_RESET);
        printf("  ");
        draw_memory_bar(mem_info->mem_usage_percent, BAR_WIDTH);
        printf(" %s%.1f%%%s\n", COLOR_MAGENTA, mem_info->mem_usage_percent, COLOR_RESET);
        
        if (options->show_swap && mem_info->total_swap > 0) {
            printf("\n%s交换使用率:%s\n", COLOR_WHITE, COLOR_RESET);
            printf("  ");
            draw_memory_bar(mem_info->swap_usage_percent, BAR_WIDTH);
            printf(" %s%.1f%%%s\n", COLOR_MAGENTA, mem_info->swap_usage_percent, COLOR_RESET);
        }
    }
    
    printf("%s%s%s\n", COLOR_YELLOW, "================================================", COLOR_RESET);
}

// 显示表格格式
void display_table_format(memory_info_t *mem_info, free_options_t *options) {
    printf("%s内存使用情况 - 优化版 free%s\n", COLOR_CYAN, COLOR_RESET);
    printf("%s%s%s\n", COLOR_YELLOW, "================================================", COLOR_RESET);
    
    // 表头
    printf("%s%-12s %-12s %-12s %-12s %-12s%s\n",
           COLOR_WHITE,
           "类型", "总大小", "已使用", "可用", "使用率",
           COLOR_RESET);
    printf("%s%s%s\n", COLOR_YELLOW, "------------------------------------------------", COLOR_RESET);
    
    // 内存行
    printf("%s%-12s %s%-12s%s %s%-12s%s %s%-12s%s %s%-12s%s\n",
           COLOR_GREEN, "内存",
           COLOR_CYAN, format_size(mem_info->total_mem), COLOR_RESET,
           COLOR_RED, format_size(mem_info->used_mem), COLOR_RESET,
           COLOR_GREEN, format_size(mem_info->available_mem), COLOR_RESET,
           COLOR_MAGENTA, format_size(mem_info->mem_usage_percent), COLOR_RESET);
    
    // 交换空间行
    if (options->show_swap && mem_info->total_swap > 0) {
        printf("%s%-12s %s%-12s%s %s%-12s%s %s%-12s%s %s%-12s%s\n",
               COLOR_BLUE, "交换",
               COLOR_CYAN, format_size(mem_info->total_swap), COLOR_RESET,
               COLOR_RED, format_size(mem_info->used_swap), COLOR_RESET,
               COLOR_GREEN, format_size(mem_info->free_swap), COLOR_RESET,
               COLOR_MAGENTA, format_size(mem_info->swap_usage_percent), COLOR_RESET);
    }
    
    printf("%s%s%s\n", COLOR_YELLOW, "================================================", COLOR_RESET);
}

// 显示系统信息
void display_system_info() {
    struct sysinfo info;
    if (sysinfo(&info) != 0) return;
    
    printf("%s系统信息:%s\n", COLOR_WHITE, COLOR_RESET);
    printf("  %s运行时间: %ld 天 %ld 小时 %ld 分钟%s\n",
           COLOR_CYAN,
           info.uptime / 86400,
           (info.uptime % 86400) / 3600,
           (info.uptime % 3600) / 60,
           COLOR_RESET);
    
    printf("  %s进程数: %d%s\n",
           COLOR_CYAN, info.procs, COLOR_RESET);
    
    printf("  %s负载: %.2f %.2f %.2f%s\n",
           COLOR_CYAN,
           info.loads[0] / 65536.0,
           info.loads[1] / 65536.0,
           info.loads[2] / 65536.0,
           COLOR_RESET);
}

void print_usage(const char *program_name) {
    printf("用法: %s [选项]\n", program_name);
    printf("优化版的 free 命令，提供内存使用可视化显示\n\n");
    printf("选项:\n");
    printf("  -h, --human-readable 以人类可读的格式显示大小\n");
    printf("  -g, --graph          显示图形化进度条\n");
    printf("  -c, --continuous     连续显示（类似top）\n");
    printf("  -s, --seconds 秒数   刷新间隔（默认: 2秒）\n");
    printf("  -w, --swap           显示交换空间信息\n");
    printf("  -b, --buffers        显示缓冲区信息\n");
    printf("  -C, --cached         显示缓存信息\n");
    printf("  -t, --table          表格格式显示\n");
    printf("  --help               显示此帮助信息\n");
    printf("  --version            显示版本信息\n");
    printf("\n示例:\n");
    printf("  %s                    # 显示内存使用情况\n", program_name);
    printf("  %s -g                 # 显示图形化进度条\n", program_name);
    printf("  %s -c                 # 连续监控\n", program_name);
    printf("  %s -w -b -C           # 显示详细信息\n", program_name);
    printf("  %s -t                 # 表格格式显示\n", program_name);
}

int main(int argc, char *argv[]) {
    free_options_t options = {0};
    options.show_human_readable = 1;
    options.show_graph = 1;
    options.refresh_interval = 2;
    
    // 解析命令行参数
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--human-readable") == 0) {
            options.show_human_readable = 1;
        } else if (strcmp(argv[i], "-g") == 0 || strcmp(argv[i], "--graph") == 0) {
            options.show_graph = 1;
        } else if (strcmp(argv[i], "-c") == 0 || strcmp(argv[i], "--continuous") == 0) {
            options.continuous = 1;
        } else if (strcmp(argv[i], "-s") == 0 || strcmp(argv[i], "--seconds") == 0) {
            if (i + 1 < argc) {
                options.refresh_interval = atoi(argv[++i]);
            }
        } else if (strcmp(argv[i], "-w") == 0 || strcmp(argv[i], "--swap") == 0) {
            options.show_swap = 1;
        } else if (strcmp(argv[i], "-b") == 0 || strcmp(argv[i], "--buffers") == 0) {
            options.show_buffers = 1;
        } else if (strcmp(argv[i], "-C") == 0 || strcmp(argv[i], "--cached") == 0) {
            options.show_cached = 1;
        } else if (strcmp(argv[i], "-t") == 0 || strcmp(argv[i], "--table") == 0) {
            options.show_graph = 0;
        } else if (strcmp(argv[i], "--help") == 0) {
            print_usage(argv[0]);
            return 0;
        } else if (strcmp(argv[i], "--version") == 0) {
            printf("pfree - 优化版 free 命令 v1.0\n");
            return 0;
        }
    }
    
    do {
        // 清屏（连续模式）
        if (options.continuous) {
            printf("\033[2J\033[H");
        }
        
        // 获取内存信息
        if (!get_memory_info(&options.mem_info)) {
            print_error("无法获取内存信息");
            return 1;
        }
        
        // 显示信息
        if (options.show_graph) {
            display_memory_info(&options.mem_info, &options);
        } else {
            display_table_format(&options.mem_info, &options);
        }
        
        // 显示系统信息
        if (options.continuous) {
            printf("\n");
            display_system_info();
            printf("\n%s按 Ctrl+C 退出%s\n", COLOR_CYAN, COLOR_RESET);
        }
        
        // 等待刷新间隔
        if (options.continuous) {
            sleep(options.refresh_interval);
        }
        
    } while (options.continuous);
    
    return 0;
}
