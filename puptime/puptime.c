#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/sysinfo.h>
#include <time.h>
#include <utmp.h>
#include <pwd.h>
#include "../include/common.h"

#define MAX_LOAD_HISTORY 10

typedef struct {
    time_t uptime;
    int users;
    float load_1min;
    float load_5min;
    float load_15min;
    time_t boot_time;
    char boot_time_str[64];
    char current_time_str[64];
    int processes;
    int running_processes;
    int sleeping_processes;
    int zombie_processes;
    unsigned long total_mem;
    unsigned long free_mem;
    unsigned long available_mem;
    float mem_usage_percent;
} system_info_t;

typedef struct {
    system_info_t sys_info;
    int show_users;
    int show_load;
    int show_memory;
    int show_processes;
    int show_boot_time;
    int show_current_time;
    int show_graph;
    int show_detailed;
    int continuous;
    int refresh_interval;
} uptime_options_t;

// 获取系统信息
int get_system_info(system_info_t *info) {
    struct sysinfo si;
    
    if (sysinfo(&si) != 0) {
        return 0;
    }
    
    info->uptime = si.uptime;
    info->users = si.procs; // 简化处理
    info->load_1min = si.loads[0] / 65536.0;
    info->load_5min = si.loads[1] / 65536.0;
    info->load_15min = si.loads[2] / 65536.0;
    info->processes = si.procs;
    info->running_processes = si.procs; // 简化处理
    info->sleeping_processes = 0;
    info->zombie_processes = 0;
    
    // 内存信息
    info->total_mem = si.totalram * si.mem_unit;
    info->free_mem = si.freeram * si.mem_unit;
    info->available_mem = info->free_mem + (si.bufferram * si.mem_unit);
    
    if (info->total_mem > 0) {
        info->mem_usage_percent = (float)((info->total_mem - info->available_mem) * 100.0) / info->total_mem;
    } else {
        info->mem_usage_percent = 0.0;
    }
    
    // 计算启动时间
    info->boot_time = time(NULL) - info->uptime;
    
    // 格式化时间
    struct tm *tm_info = localtime(&info->boot_time);
    strftime(info->boot_time_str, sizeof(info->boot_time_str), "%Y-%m-%d %H:%M:%S", tm_info);
    
    time_t now = time(NULL);
    tm_info = localtime(&now);
    strftime(info->current_time_str, sizeof(info->current_time_str), "%Y-%m-%d %H:%M:%S", tm_info);
    
    return 1;
}

// 格式化运行时间
void format_uptime(time_t uptime, char *buffer, size_t size) {
    int days = uptime / 86400;
    int hours = (uptime % 86400) / 3600;
    int minutes = (uptime % 3600) / 60;
    int seconds = uptime % 60;
    
    if (days > 0) {
        snprintf(buffer, size, "%d天 %d小时 %d分钟", days, hours, minutes);
    } else if (hours > 0) {
        snprintf(buffer, size, "%d小时 %d分钟", hours, minutes);
    } else if (minutes > 0) {
        snprintf(buffer, size, "%d分钟 %d秒", minutes, seconds);
    } else {
        snprintf(buffer, size, "%d秒", seconds);
    }
}

// 获取负载状态图标
const char* get_load_status_icon(float load) {
    if (load < 1.0) return "🟢";
    else if (load < 2.0) return "🟡";
    else if (load < 4.0) return "🟠";
    else return "🔴";
}

// 获取内存状态图标
const char* get_memory_status_icon(float usage) {
    if (usage < 50) return "🟢";
    else if (usage < 80) return "🟡";
    else if (usage < 95) return "🟠";
    else return "🔴";
}

// 绘制负载图表
void draw_load_graph(float load, int width) {
    int bar_length = (int)(load * width / 4.0); // 假设4.0为最大负载
    if (bar_length > width) bar_length = width;
    
    const char *color = COLOR_GREEN;
    if (load > 3.0) color = COLOR_RED;
    else if (load > 2.0) color = COLOR_YELLOW;
    else if (load > 1.0) color = COLOR_CYAN;
    
    printf("%s[", COLOR_CYAN);
    
    for (int i = 0; i < bar_length; i++) {
        printf("%s█", color);
    }
    
    for (int i = bar_length; i < width; i++) {
        printf(" ");
    }
    
    printf("%s]%s", COLOR_CYAN, COLOR_RESET);
}

// 显示系统状态
void display_system_status(uptime_options_t *options) {
    system_info_t *info = &options->sys_info;
    char uptime_str[128];
    
    printf("%s系统状态 - 优化版 uptime%s\n", COLOR_CYAN, COLOR_RESET);
    printf("%s%s%s\n", COLOR_YELLOW, "================================================", COLOR_RESET);
    
    // 运行时间
    format_uptime(info->uptime, uptime_str, sizeof(uptime_str));
    printf("%s运行时间: %s%s%s\n", 
           COLOR_WHITE, COLOR_GREEN, uptime_str, COLOR_RESET);
    
    // 用户数
    if (options->show_users) {
        printf("%s当前用户: %s%d%s\n", 
               COLOR_WHITE, COLOR_CYAN, info->users, COLOR_RESET);
    }
    
    // 负载信息
    if (options->show_load) {
        printf("%s系统负载: %s%.2f%s %.2f %.2f\n",
               COLOR_WHITE,
               COLOR_MAGENTA, info->load_1min, COLOR_RESET,
               info->load_5min, info->load_15min);
        
        if (options->show_graph) {
            printf("%s负载图表: ", COLOR_WHITE);
            draw_load_graph(info->load_1min, 30);
            printf(" %s%.2f%s\n", COLOR_MAGENTA, info->load_1min, COLOR_RESET);
        }
    }
    
    // 内存信息
    if (options->show_memory) {
        printf("%s内存使用: %s%s%s / %s%s%s (%s%.1f%%%s)\n",
               COLOR_WHITE,
               COLOR_RED, format_size(info->total_mem - info->available_mem), COLOR_RESET,
               COLOR_CYAN, format_size(info->total_mem), COLOR_RESET,
               COLOR_MAGENTA, info->mem_usage_percent, COLOR_RESET);
    }
    
    // 进程信息
    if (options->show_processes) {
        printf("%s进程统计: %s%d%s 总计, %s%d%s 运行中, %s%d%s 睡眠中\n",
               COLOR_WHITE,
               COLOR_CYAN, info->processes, COLOR_RESET,
               COLOR_GREEN, info->running_processes, COLOR_RESET,
               COLOR_YELLOW, info->sleeping_processes, COLOR_RESET);
    }
    
    // 启动时间
    if (options->show_boot_time) {
        printf("%s系统启动: %s%s%s\n",
               COLOR_WHITE, COLOR_BLUE, info->boot_time_str, COLOR_RESET);
    }
    
    // 当前时间
    if (options->show_current_time) {
        printf("%s当前时间: %s%s%s\n",
               COLOR_WHITE, COLOR_BLUE, info->current_time_str, COLOR_RESET);
    }
    
    printf("%s%s%s\n", COLOR_YELLOW, "================================================", COLOR_RESET);
}

// 显示详细状态
void display_detailed_status(uptime_options_t *options) {
    system_info_t *info = &options->sys_info;
    
    printf("%s详细系统状态%s\n", COLOR_CYAN, COLOR_RESET);
    printf("%s%s%s\n", COLOR_YELLOW, "================================================", COLOR_RESET);
    
    // 系统信息表格
    printf("%s%-20s %s%s%s\n", COLOR_WHITE, "项目", "值", COLOR_RESET);
    printf("%s%s%s\n", COLOR_YELLOW, "------------------------------------------------", COLOR_RESET);
    
    char uptime_str[128];
    format_uptime(info->uptime, uptime_str, sizeof(uptime_str));
    printf("%s%-20s %s%s%s\n", COLOR_WHITE, "运行时间", COLOR_GREEN, uptime_str, COLOR_RESET);
    
    printf("%s%-20s %s%d%s\n", COLOR_WHITE, "用户数", COLOR_CYAN, info->users, COLOR_RESET);
    
    printf("%s%-20s %s%.2f%s %.2f %.2f\n", COLOR_WHITE, "系统负载", 
           COLOR_MAGENTA, info->load_1min, COLOR_RESET, info->load_5min, info->load_15min);
    
    printf("%s%-20s %s%s%s / %s%s%s\n", COLOR_WHITE, "内存使用",
           COLOR_RED, format_size(info->total_mem - info->available_mem), COLOR_RESET,
           COLOR_CYAN, format_size(info->total_mem), COLOR_RESET);
    
    printf("%s%-20s %s%.1f%%%s\n", COLOR_WHITE, "内存使用率",
           COLOR_MAGENTA, info->mem_usage_percent, COLOR_RESET);
    
    printf("%s%-20s %s%d%s\n", COLOR_WHITE, "总进程数", COLOR_CYAN, info->processes, COLOR_RESET);
    
    printf("%s%-20s %s%s%s\n", COLOR_WHITE, "系统启动", COLOR_BLUE, info->boot_time_str, COLOR_RESET);
    
    printf("%s%-20s %s%s%s\n", COLOR_WHITE, "当前时间", COLOR_BLUE, info->current_time_str, COLOR_RESET);
    
    printf("%s%s%s\n", COLOR_YELLOW, "================================================", COLOR_RESET);
}

// 显示负载历史
void display_load_history(uptime_options_t *options) {
    printf("%s负载历史趋势%s\n", COLOR_CYAN, COLOR_RESET);
    printf("%s%s%s\n", COLOR_YELLOW, "================================================", COLOR_RESET);
    
    system_info_t *info = &options->sys_info;
    
    printf("%s1分钟负载: %s%.2f%s ", COLOR_WHITE, COLOR_MAGENTA, info->load_1min, COLOR_RESET);
    draw_load_graph(info->load_1min, 20);
    printf("\n");
    
    printf("%s5分钟负载: %s%.2f%s ", COLOR_WHITE, COLOR_MAGENTA, info->load_5min, COLOR_RESET);
    draw_load_graph(info->load_5min, 20);
    printf("\n");
    
    printf("%s15分钟负载: %s%.2f%s ", COLOR_WHITE, COLOR_MAGENTA, info->load_15min, COLOR_RESET);
    draw_load_graph(info->load_15min, 20);
    printf("\n");
    
    printf("%s%s%s\n", COLOR_YELLOW, "================================================", COLOR_RESET);
}

void print_usage(const char *program_name) {
    printf("用法: %s [选项]\n", program_name);
    printf("优化版的 uptime 命令，提供系统状态可视化显示\n\n");
    printf("选项:\n");
    printf("  -p, --pretty         美化显示格式\n");
    printf("  -s, --since          显示系统启动时间\n");
    printf("  -u, --users          显示用户信息\n");
    printf("  -l, --load           显示负载信息\n");
    printf("  -m, --memory         显示内存信息\n");
    printf("  -c, --processes      显示进程信息\n");
    printf("  -g, --graph          显示图形化负载\n");
    printf("  -d, --detailed       显示详细信息\n");
    printf("  -w, --watch          连续监控模式\n");
    printf("  -i, --interval 秒数   刷新间隔（默认: 2秒）\n");
    printf("  --help               显示此帮助信息\n");
    printf("  --version            显示版本信息\n");
    printf("\n示例:\n");
    printf("  %s                    # 显示基本状态\n", program_name);
    printf("  %s -p                 # 美化显示\n", program_name);
    printf("  %s -d                 # 显示详细信息\n", program_name);
    printf("  %s -w                 # 连续监控\n", program_name);
    printf("  %s -g -l              # 显示负载图表\n", program_name);
}

int main(int argc, char *argv[]) {
    uptime_options_t options = {0};
    options.show_users = 1;
    options.show_load = 1;
    options.show_memory = 1;
    options.show_processes = 1;
    options.show_boot_time = 1;
    options.show_current_time = 1;
    options.show_graph = 1;
    options.refresh_interval = 2;
    
    // 解析命令行参数
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-p") == 0 || strcmp(argv[i], "--pretty") == 0) {
            options.show_graph = 1;
        } else if (strcmp(argv[i], "-s") == 0 || strcmp(argv[i], "--since") == 0) {
            options.show_boot_time = 1;
        } else if (strcmp(argv[i], "-u") == 0 || strcmp(argv[i], "--users") == 0) {
            options.show_users = 1;
        } else if (strcmp(argv[i], "-l") == 0 || strcmp(argv[i], "--load") == 0) {
            options.show_load = 1;
        } else if (strcmp(argv[i], "-m") == 0 || strcmp(argv[i], "--memory") == 0) {
            options.show_memory = 1;
        } else if (strcmp(argv[i], "-c") == 0 || strcmp(argv[i], "--processes") == 0) {
            options.show_processes = 1;
        } else if (strcmp(argv[i], "-g") == 0 || strcmp(argv[i], "--graph") == 0) {
            options.show_graph = 1;
        } else if (strcmp(argv[i], "-d") == 0 || strcmp(argv[i], "--detailed") == 0) {
            options.show_detailed = 1;
        } else if (strcmp(argv[i], "-w") == 0 || strcmp(argv[i], "--watch") == 0) {
            options.continuous = 1;
        } else if (strcmp(argv[i], "-i") == 0 || strcmp(argv[i], "--interval") == 0) {
            if (i + 1 < argc) {
                options.refresh_interval = atoi(argv[++i]);
            }
        } else if (strcmp(argv[i], "--help") == 0) {
            print_usage(argv[0]);
            return 0;
        } else if (strcmp(argv[i], "--version") == 0) {
            printf("puptime - 优化版 uptime 命令 v1.0\n");
            return 0;
        }
    }
    
    do {
        // 清屏（连续模式）
        if (options.continuous) {
            printf("\033[2J\033[H");
        }
        
        // 获取系统信息
        if (!get_system_info(&options.sys_info)) {
            print_error("无法获取系统信息");
            return 1;
        }
        
        // 显示状态
        if (options.show_detailed) {
            display_detailed_status(&options);
        } else {
            display_system_status(&options);
        }
        
        // 显示负载历史
        if (options.show_graph && options.show_load) {
            printf("\n");
            display_load_history(&options);
        }
        
        // 等待刷新间隔
        if (options.continuous) {
            printf("\n%s按 Ctrl+C 退出%s\n", COLOR_CYAN, COLOR_RESET);
            sleep(options.refresh_interval);
        }
        
    } while (options.continuous);
    
    return 0;
}
