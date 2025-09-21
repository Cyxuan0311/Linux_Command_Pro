#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <utmp.h>
#include <sys/types.h>
#include <pwd.h>
#include <time.h>
#include <sys/sysinfo.h>
#include <sys/stat.h>
#include <dirent.h>
#include "../include/common.h"

#define MAX_USERS 100
#define MAX_PROCESSES 1000
#define MAX_COMMAND 256

typedef struct {
    char username[32];
    char terminal[32];
    char hostname[256];
    char login_time[64];
    char idle_time[32];
    char jcpu[16];
    char pcpu[16];
    char what[256];
    pid_t pid;
    int is_current;
    time_t last_activity;
} user_activity_t;

typedef struct {
    user_activity_t users[MAX_USERS];
    int user_count;
    int show_headers;
    int show_idle;
    int show_cpu;
    int show_command;
    int show_processes;
    int show_hostname;
    int show_login_time;
    int sort_by_idle;
    int show_summary;
} w_options_t;

// 获取进程信息
int get_process_info(pid_t pid, char *command, size_t size) {
    char path[256];
    FILE *file;
    char line[1024];
    
    snprintf(path, sizeof(path), "/proc/%d/cmdline", pid);
    file = fopen(path, "r");
    if (file) {
        if (fgets(line, sizeof(line), file)) {
            // 处理cmdline中的null字符
            for (int i = 0; i < (int)strlen(line) && i < (int)size - 1; i++) {
                if (line[i] == '\0') {
                    line[i] = ' ';
                }
            }
            strncpy(command, line, size - 1);
            command[size - 1] = '\0';
        }
        fclose(file);
        return 1;
    }
    return 0;
}

// 计算空闲时间
void calculate_idle_time(time_t last_activity, char *buffer, size_t size) {
    time_t now = time(NULL);
    time_t idle = now - last_activity;
    
    if (idle < 60) {
        snprintf(buffer, size, "%lds", idle);
    } else if (idle < 3600) {
        snprintf(buffer, size, "%ldm", idle / 60);
    } else if (idle < 86400) {
        snprintf(buffer, size, "%ldh", idle / 3600);
    } else {
        snprintf(buffer, size, "%ldd", idle / 86400);
    }
}

// 格式化时间
void format_time_local(time_t time_val, char *buffer, size_t size) {
    struct tm *tm_info = localtime(&time_val);
    strftime(buffer, size, "%H:%M", tm_info);
}

// 获取用户活动信息
int get_user_activity(const struct utmp *ut, user_activity_t *user) {
    if (ut->ut_type != USER_PROCESS && ut->ut_type != LOGIN_PROCESS) {
        return 0;
    }
    
    strncpy(user->username, ut->ut_user, sizeof(user->username) - 1);
    user->username[sizeof(user->username) - 1] = '\0';
    
    strncpy(user->terminal, ut->ut_line, sizeof(user->terminal) - 1);
    user->terminal[sizeof(user->terminal) - 1] = '\0';
    
    strncpy(user->hostname, ut->ut_host, sizeof(user->hostname) - 1);
    user->hostname[sizeof(user->hostname) - 1] = '\0';
    
    user->pid = ut->ut_pid;
    user->last_activity = ut->ut_tv.tv_sec;
    
    // 格式化登录时间
    format_time_local(ut->ut_tv.tv_sec, user->login_time, sizeof(user->login_time));
    
    // 计算空闲时间
    calculate_idle_time(ut->ut_tv.tv_sec, user->idle_time, sizeof(user->idle_time));
    
    // 获取进程信息
    get_process_info(ut->ut_pid, user->what, sizeof(user->what));
    
    // 简化的CPU使用率（这里用固定值模拟）
    strcpy(user->jcpu, "0.00s");
    strcpy(user->pcpu, "0.00s");
    
    // 检查是否为当前用户
    struct passwd *pw = getpwnam(user->username);
    user->is_current = (pw != NULL && getuid() == pw->pw_uid);
    
    return 1;
}

// 获取所有用户活动
int get_all_user_activity(w_options_t *options) {
    struct utmp *ut;
    int count = 0;
    
    setutent();
    
    while ((ut = getutent()) != NULL && count < MAX_USERS) {
        if (get_user_activity(ut, &options->users[count])) {
            count++;
        }
    }
    
    endutent();
    options->user_count = count;
    return count;
}

// 获取用户状态图标
const char* get_user_status_icon(const user_activity_t *user) {
    if (user->is_current) {
        return "👤";
    } else if (strlen(user->hostname) > 0) {
        return "🌐";
    } else {
        return "💻";
    }
}

// 获取终端类型图标
const char* get_terminal_icon(const char *terminal) {
    if (strstr(terminal, "pts") != NULL) {
        return "🖥️";
    } else if (strstr(terminal, "tty") != NULL) {
        return "💻";
    } else if (strstr(terminal, "console") != NULL) {
        return "🖥️";
    } else {
        return "📱";
    }
}

// 获取空闲时间颜色
const char* get_idle_color(const char *idle_time) {
    if (strstr(idle_time, "d") != NULL) {
        return COLOR_RED;
    } else if (strstr(idle_time, "h") != NULL) {
        return COLOR_YELLOW;
    } else if (strstr(idle_time, "m") != NULL) {
        return COLOR_CYAN;
    } else {
        return COLOR_GREEN;
    }
}

// 显示用户活动
void display_user_activity(const user_activity_t *user, w_options_t *options) {
    const char *user_icon = get_user_status_icon(user);
    const char *term_icon = get_terminal_icon(user->terminal);
    const char *idle_color = get_idle_color(user->idle_time);
    
    // 根据用户类型设置颜色
    const char *user_color = COLOR_WHITE;
    if (user->is_current) {
        user_color = COLOR_GREEN;
    } else if (strlen(user->hostname) > 0) {
        user_color = COLOR_BLUE;
    }
    
    printf("%s %s%-12s%s %s%-8s%s",
           user_icon,
           user_color, user->username, COLOR_RESET,
           COLOR_CYAN, user->terminal, COLOR_RESET);
    
    if (options->show_hostname) {
        printf(" %s%-20s%s", COLOR_BLUE, user->hostname, COLOR_RESET);
    }
    
    if (options->show_login_time) {
        printf(" %s%-8s%s", COLOR_MAGENTA, user->login_time, COLOR_RESET);
    }
    
    if (options->show_idle) {
        printf(" %s%-8s%s", idle_color, user->idle_time, COLOR_RESET);
    }
    
    if (options->show_cpu) {
        printf(" %s%-8s%s %s%-8s%s", 
               COLOR_YELLOW, user->jcpu, COLOR_RESET,
               COLOR_YELLOW, user->pcpu, COLOR_RESET);
    }
    
    if (options->show_command) {
        printf(" %s%s%s", COLOR_WHITE, user->what, COLOR_RESET);
    }
    
    printf("\n");
}

// 显示所有用户活动
void display_all_user_activity(w_options_t *options) {
    printf("%s用户活动监控 - 优化版 w%s\n", COLOR_CYAN, COLOR_RESET);
    printf("%s%s%s\n", COLOR_YELLOW, "================================================", COLOR_RESET);
    
    if (options->user_count == 0) {
        printf("%s没有找到活跃用户%s\n", COLOR_RED, COLOR_RESET);
        return;
    }
    
    // 表头
    if (options->show_headers) {
        printf("%s%-12s %-8s%s", COLOR_WHITE, "用户", "终端", COLOR_RESET);
        
        if (options->show_hostname) {
            printf(" %s%-20s%s", COLOR_WHITE, "主机名", COLOR_RESET);
        }
        
        if (options->show_login_time) {
            printf(" %s%-8s%s", COLOR_WHITE, "登录时间", COLOR_RESET);
        }
        
        if (options->show_idle) {
            printf(" %s%-8s%s", COLOR_WHITE, "空闲时间", COLOR_RESET);
        }
        
        if (options->show_cpu) {
            printf(" %s%-8s%s %s%-8s%s", COLOR_WHITE, "JCPU", COLOR_RESET, COLOR_WHITE, "PCPU", COLOR_RESET);
        }
        
        if (options->show_command) {
            printf(" %s%s%s", COLOR_WHITE, "命令", COLOR_RESET);
        }
        
        printf("\n");
        printf("%s%s%s\n", COLOR_YELLOW, "------------------------------------------------", COLOR_RESET);
    }
    
    // 显示用户
    for (int i = 0; i < options->user_count; i++) {
        display_user_activity(&options->users[i], options);
    }
    
    printf("%s%s%s\n", COLOR_YELLOW, "================================================", COLOR_RESET);
    
    if (options->show_summary) {
        printf("%s活跃用户数: %d%s\n", COLOR_CYAN, options->user_count, COLOR_RESET);
    }
}

// 显示系统负载
void display_load_average() {
    FILE *file = fopen("/proc/loadavg", "r");
    if (file) {
        float load_1min, load_5min, load_15min;
        int running, total;
        
        if (fscanf(file, "%f %f %f %d/%d", &load_1min, &load_5min, &load_15min, &running, &total) == 5) {
            printf("%s系统负载: %s%.2f%s %.2f %.2f (运行: %d/%d)%s\n",
                   COLOR_WHITE,
                   COLOR_MAGENTA, load_1min, COLOR_RESET,
                   load_5min, load_15min,
                   running, total,
                   COLOR_RESET);
        }
        fclose(file);
    }
}

// 显示系统时间
void display_system_time() {
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    char time_str[64];
    
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", tm_info);
    printf("%s当前时间: %s%s%s\n", COLOR_WHITE, COLOR_BLUE, time_str, COLOR_RESET);
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
}

void print_usage(const char *program_name) {
    printf("用法: %s [选项]\n", program_name);
    printf("优化版的 w 命令，提供用户活动监控\n\n");
    printf("选项:\n");
    printf("  -h, --no-header      不显示列标题\n");
    printf("  -s, --short          短格式输出\n");
    printf("  -f, --from           显示来源主机\n");
    printf("  -i, --idle           显示空闲时间\n");
    printf("  -u, --user           显示用户信息\n");
    printf("  -w, --writable       显示终端写入状态\n");
    printf("  -o, --old-format     旧格式输出\n");
    printf("  -c, --command        显示命令信息\n");
    printf("  -p, --process        显示进程信息\n");
    printf("  --help               显示此帮助信息\n");
    printf("  --version            显示版本信息\n");
    printf("\n示例:\n");
    printf("  %s                    # 显示用户活动\n", program_name);
    printf("  %s -i                 # 显示空闲时间\n", program_name);
    printf("  %s -c                 # 显示命令信息\n", program_name);
    printf("  %s -f                 # 显示来源主机\n", program_name);
}

int main(int argc, char *argv[]) {
    w_options_t options = {0};
    options.show_headers = 1;
    options.show_idle = 1;
    options.show_cpu = 1;
    options.show_command = 1;
    options.show_hostname = 1;
    options.show_login_time = 1;
    options.show_summary = 1;
    
    // 解析命令行参数
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--no-header") == 0) {
            options.show_headers = 0;
        } else if (strcmp(argv[i], "-s") == 0 || strcmp(argv[i], "--short") == 0) {
            options.show_headers = 0;
            options.show_cpu = 0;
            options.show_command = 0;
        } else if (strcmp(argv[i], "-f") == 0 || strcmp(argv[i], "--from") == 0) {
            options.show_hostname = 1;
        } else if (strcmp(argv[i], "-i") == 0 || strcmp(argv[i], "--idle") == 0) {
            options.show_idle = 1;
        } else if (strcmp(argv[i], "-u") == 0 || strcmp(argv[i], "--user") == 0) {
            options.show_summary = 1;
        } else if (strcmp(argv[i], "-w") == 0 || strcmp(argv[i], "--writable") == 0) {
            options.show_headers = 1;
        } else if (strcmp(argv[i], "-o") == 0 || strcmp(argv[i], "--old-format") == 0) {
            options.show_headers = 0;
        } else if (strcmp(argv[i], "-c") == 0 || strcmp(argv[i], "--command") == 0) {
            options.show_command = 1;
        } else if (strcmp(argv[i], "-p") == 0 || strcmp(argv[i], "--process") == 0) {
            options.show_processes = 1;
        } else if (strcmp(argv[i], "--help") == 0) {
            print_usage(argv[0]);
            return 0;
        } else if (strcmp(argv[i], "--version") == 0) {
            printf("pw - 优化版 w 命令 v1.0\n");
            return 0;
        }
    }
    
    // 显示系统时间
    display_system_time();
    
    // 显示系统负载
    display_load_average();
    
    // 获取用户活动信息
    if (get_all_user_activity(&options) == 0) {
        print_error("无法获取用户活动信息");
        return 1;
    }
    
    // 显示用户活动
    display_all_user_activity(&options);
    
    // 显示系统信息
    printf("\n");
    display_system_info();
    
    return 0;
}
