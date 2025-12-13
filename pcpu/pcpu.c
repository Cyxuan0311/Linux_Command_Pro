/**
 * pcpu - 实时CPU监控工具
 * 显示CPU使用率、负载、温度等信息
 * 
 * 使用方法: pcpu [选项]
 * 
 * 作者: Linux Command Pro Team
 * 版本: 1.0.0
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/sysinfo.h>
#include <time.h>
#include <signal.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <math.h>
#include "../include/common.h"

#define MAX_CPUS 256
#define REFRESH_INTERVAL 1
#define MAX_HISTORY 60

typedef struct {
    unsigned long long user;
    unsigned long long nice;
    unsigned long long system;
    unsigned long long idle;
    unsigned long long iowait;
    unsigned long long irq;
    unsigned long long softirq;
    unsigned long long steal;
    unsigned long long total;
} cpu_stats_t;

typedef struct {
    cpu_stats_t cpus[MAX_CPUS];
    int cpu_count;
    float cpu_usage[MAX_CPUS];
    float load_1min;
    float load_5min;
    float load_15min;
    float cpu_temp;
    int refresh_interval;
    int show_graph;
    int show_per_cpu;
    int show_temp;
    int show_load;
    int continuous;
    int running;
} cpu_monitor_t;

static cpu_monitor_t monitor = {0};
static cpu_stats_t prev_stats[MAX_CPUS] = {0};

// 信号处理函数
void signal_handler(int sig) {
    if (sig == SIGINT || sig == SIGTERM) {
        monitor.running = 0;
    }
}

// 获取CPU核心数
int get_cpu_count() {
    FILE *file = fopen("/proc/cpuinfo", "r");
    if (!file) return 1;
    
    int count = 0;
    char line[256];
    while (fgets(line, sizeof(line), file)) {
        if (strncmp(line, "processor", 9) == 0) {
            count++;
        }
    }
    fclose(file);
    return count > 0 ? count : 1;
}

// 读取CPU统计信息
int read_cpu_stats(cpu_stats_t *stats, int cpu_index) {
    FILE *file = fopen("/proc/stat", "r");
    if (!file) return 0;
    
    char line[512];
    char cpu_label[16];
    int found = 0;
    
    if (cpu_index == -1) {
        snprintf(cpu_label, sizeof(cpu_label), "cpu ");
    } else {
        snprintf(cpu_label, sizeof(cpu_label), "cpu%d ", cpu_index);
    }
    
    while (fgets(line, sizeof(line), file)) {
        if (strncmp(line, cpu_label, strlen(cpu_label)) == 0) {
            sscanf(line, "%*s %llu %llu %llu %llu %llu %llu %llu %llu",
                   &stats->user, &stats->nice, &stats->system, &stats->idle,
                   &stats->iowait, &stats->irq, &stats->softirq, &stats->steal);
            stats->total = stats->user + stats->nice + stats->system + 
                          stats->idle + stats->iowait + stats->irq + 
                          stats->softirq + stats->steal;
            found = 1;
            break;
        }
    }
    fclose(file);
    return found;
}

// 计算CPU使用率
float calculate_cpu_usage(cpu_stats_t *current, cpu_stats_t *previous) {
    if (previous->total == 0) return 0.0;
    
    unsigned long long total_diff = current->total - previous->total;
    unsigned long long idle_diff = current->idle - previous->idle;
    
    if (total_diff == 0) return 0.0;
    
    float usage = 100.0 * (total_diff - idle_diff) / total_diff;
    return usage > 100.0 ? 100.0 : (usage < 0.0 ? 0.0 : usage);
}

// 获取系统负载
int get_load_average(float *load_1min, float *load_5min, float *load_15min) {
    FILE *file = fopen("/proc/loadavg", "r");
    if (!file) return 0;
    
    if (fscanf(file, "%f %f %f", load_1min, load_5min, load_15min) != 3) {
        fclose(file);
        return 0;
    }
    fclose(file);
    return 1;
}

// 获取CPU温度（如果可用）
float get_cpu_temperature() {
    FILE *file;
    float temp = 0.0;
    char line[256];
    
    // 尝试多个可能的温度文件路径
    const char *temp_paths[] = {
        "/sys/class/thermal/thermal_zone0/temp",
        "/sys/class/hwmon/hwmon0/temp1_input",
        "/sys/class/hwmon/hwmon1/temp1_input",
        "/sys/devices/virtual/thermal/thermal_zone0/temp",
        NULL
    };
    
    for (int i = 0; temp_paths[i]; i++) {
        file = fopen(temp_paths[i], "r");
        if (file) {
            if (fgets(line, sizeof(line), file)) {
                temp = atof(line) / 1000.0; // 转换为摄氏度
                fclose(file);
                return temp;
            }
            fclose(file);
        }
    }
    
    return -1.0; // 无法获取温度
}

// 更新CPU信息
void update_cpu_info() {
    // 读取总体CPU使用率
    cpu_stats_t current_total;
    if (read_cpu_stats(&current_total, -1)) {
        monitor.cpu_usage[0] = calculate_cpu_usage(&current_total, &prev_stats[0]);
        prev_stats[0] = current_total;
    }
    
    // 读取每个CPU核心的使用率
    if (monitor.show_per_cpu) {
        for (int i = 0; i < monitor.cpu_count; i++) {
            cpu_stats_t current;
            if (read_cpu_stats(&current, i)) {
                monitor.cpu_usage[i + 1] = calculate_cpu_usage(&current, &prev_stats[i + 1]);
                prev_stats[i + 1] = current;
            }
        }
    }
    
    // 获取负载
    if (monitor.show_load) {
        get_load_average(&monitor.load_1min, &monitor.load_5min, &monitor.load_15min);
    }
    
    // 获取温度
    if (monitor.show_temp) {
        monitor.cpu_temp = get_cpu_temperature();
    }
}

// 绘制进度条
void draw_progress_bar(float value, int width, const char *color) {
    int filled = (int)(value * width / 100.0);
    if (filled > width) filled = width;
    if (filled < 0) filled = 0;
    
    printf("%s", color);
    for (int i = 0; i < filled; i++) {
        printf("█");
    }
    printf("%s", COLOR_RESET);
    for (int i = filled; i < width; i++) {
        printf("░");
    }
}

// 获取颜色根据使用率
const char* get_usage_color(float usage) {
    if (usage >= 80.0) return COLOR_RED;
    if (usage >= 60.0) return COLOR_YELLOW;
    if (usage >= 40.0) return COLOR_CYAN;
    return COLOR_GREEN;
}

// 格式化温度显示
void format_temperature(float temp, char *buffer, size_t size) {
    if (temp < 0) {
        snprintf(buffer, size, "N/A");
    } else {
        snprintf(buffer, size, "%.1f°C", temp);
    }
}

// 显示CPU信息
void display_cpu_info() {
    // 清屏
    printf("\033[2J\033[H");
    
    // 标题
    printf("%s╔══════════════════════════════════════════════════════════════╗%s\n", 
           COLOR_CYAN, COLOR_RESET);
    printf("%s║%s  🖥️  实时CPU监控工具 (按 Ctrl+C 退出)                        %s║%s\n",
           COLOR_CYAN, COLOR_WHITE, COLOR_CYAN, COLOR_RESET);
    printf("%s╚══════════════════════════════════════════════════════════════╝%s\n\n",
           COLOR_CYAN, COLOR_RESET);
    
    // 总体CPU使用率
    printf("%s%s总体CPU使用率:%s\n", BOLD, COLOR_WHITE, COLOR_RESET);
    printf("  %s%.2f%%%s ", get_usage_color(monitor.cpu_usage[0]), 
           monitor.cpu_usage[0], COLOR_RESET);
    draw_progress_bar(monitor.cpu_usage[0], 50, get_usage_color(monitor.cpu_usage[0]));
    printf("\n\n");
    
    // 每个CPU核心的使用率
    if (monitor.show_per_cpu) {
        printf("%s%s各CPU核心使用率:%s\n", BOLD, COLOR_WHITE, COLOR_RESET);
        for (int i = 0; i < monitor.cpu_count; i++) {
            float usage = monitor.cpu_usage[i + 1];
            printf("  %sCPU%d:%s %s%.2f%%%s ", 
                   COLOR_YELLOW, i, COLOR_RESET,
                   get_usage_color(usage), usage, COLOR_RESET);
            draw_progress_bar(usage, 45, get_usage_color(usage));
            printf("\n");
        }
        printf("\n");
    }
    
    // 系统负载
    if (monitor.show_load) {
        printf("%s%s系统负载:%s\n", BOLD, COLOR_WHITE, COLOR_RESET);
        printf("  1分钟: %s%.2f%s  ", 
               get_usage_color(monitor.load_1min * 10), monitor.load_1min, COLOR_RESET);
        printf("5分钟: %s%.2f%s  ", 
               get_usage_color(monitor.load_5min * 10), monitor.load_5min, COLOR_RESET);
        printf("15分钟: %s%.2f%s\n", 
               get_usage_color(monitor.load_15min * 10), monitor.load_15min, COLOR_RESET);
        printf("\n");
    }
    
    // CPU温度
    if (monitor.show_temp) {
        char temp_str[32];
        format_temperature(monitor.cpu_temp, temp_str, sizeof(temp_str));
        const char *temp_color = COLOR_GREEN;
        if (monitor.cpu_temp > 0) {
            if (monitor.cpu_temp >= 80) temp_color = COLOR_RED;
            else if (monitor.cpu_temp >= 60) temp_color = COLOR_YELLOW;
        }
        printf("%s%sCPU温度:%s %s%s%s\n\n", 
               BOLD, COLOR_WHITE, COLOR_RESET, temp_color, temp_str, COLOR_RESET);
    }
    
    // 时间戳
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    char time_str[64];
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", tm_info);
    printf("%s更新时间: %s%s\n", COLOR_CYAN, time_str, COLOR_RESET);
    
    fflush(stdout);
}

// 显示帮助信息
void print_help(const char *program_name) {
    printf("🐧 pcpu - 实时CPU监控工具\n");
    printf("========================\n\n");
    printf("使用方法: %s [选项]\n\n", program_name);
    printf("选项:\n");
    printf("  -h, --help          显示此帮助信息\n");
    printf("  -v, --version       显示版本信息\n");
    printf("  -c, --continuous    持续监控模式（默认）\n");
    printf("  -n, --once          只显示一次\n");
    printf("  -i, --interval N    刷新间隔（秒，默认: %d）\n", REFRESH_INTERVAL);
    printf("  -p, --per-cpu       显示每个CPU核心的使用率\n");
    printf("  -l, --load          显示系统负载\n");
    printf("  -t, --temperature   显示CPU温度\n");
    printf("  -g, --graph         显示图形化进度条（默认）\n");
    printf("\n");
    printf("示例:\n");
    printf("  %s                  # 实时监控CPU使用率\n", program_name);
    printf("  %s -p -l -t          # 显示所有信息\n", program_name);
    printf("  %s -n                # 只显示一次\n", program_name);
    printf("  %s -i 2              # 每2秒刷新一次\n", program_name);
}

// 显示版本信息
void print_version() {
    printf("pcpu version 1.0.0\n");
    printf("Copyright (c) 2025 Linux Command Pro Team\n");
    printf("MIT License\n");
}

int main(int argc, char *argv[]) {
    // 初始化
    monitor.cpu_count = get_cpu_count();
    monitor.refresh_interval = REFRESH_INTERVAL;
    monitor.show_graph = 1;
    monitor.show_per_cpu = 0;
    monitor.show_temp = 0;
    monitor.show_load = 0;
    monitor.continuous = 1;
    monitor.running = 1;
    
    // 解析命令行参数
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            print_help(argv[0]);
            return 0;
        } else if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--version") == 0) {
            print_version();
            return 0;
        } else if (strcmp(argv[i], "-c") == 0 || strcmp(argv[i], "--continuous") == 0) {
            monitor.continuous = 1;
        } else if (strcmp(argv[i], "-n") == 0 || strcmp(argv[i], "--once") == 0) {
            monitor.continuous = 0;
        } else if (strcmp(argv[i], "-p") == 0 || strcmp(argv[i], "--per-cpu") == 0) {
            monitor.show_per_cpu = 1;
        } else if (strcmp(argv[i], "-l") == 0 || strcmp(argv[i], "--load") == 0) {
            monitor.show_load = 1;
        } else if (strcmp(argv[i], "-t") == 0 || strcmp(argv[i], "--temperature") == 0) {
            monitor.show_temp = 1;
        } else if (strcmp(argv[i], "-g") == 0 || strcmp(argv[i], "--graph") == 0) {
            monitor.show_graph = 1;
        } else if (strcmp(argv[i], "-i") == 0 || strcmp(argv[i], "--interval") == 0) {
            if (i + 1 < argc) {
                monitor.refresh_interval = atoi(argv[++i]);
                if (monitor.refresh_interval <= 0) {
                    monitor.refresh_interval = REFRESH_INTERVAL;
                }
            }
        } else {
            fprintf(stderr, "❌ 错误: 未知选项 '%s'\n", argv[i]);
            fprintf(stderr, "使用 '%s --help' 查看帮助信息\n", argv[0]);
            return 1;
        }
    }
    
    // 设置信号处理
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    
    // 首次读取（用于初始化）
    update_cpu_info();
    sleep(1);
    
    // 主循环
    do {
        update_cpu_info();
        display_cpu_info();
        
        if (monitor.continuous) {
            sleep(monitor.refresh_interval);
        } else {
            break;
        }
    } while (monitor.running);
    
    // 恢复终端
    printf("\033[0m");
    
    return 0;
}

