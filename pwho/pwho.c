#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <utmp.h>
#include <sys/types.h>
#include <pwd.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/sysinfo.h>
#include "../include/common.h"

#define MAX_USERS 100
#define MAX_HOSTNAME 256

typedef struct {
    char username[32];
    char terminal[32];
    char hostname[MAX_HOSTNAME];
    char login_time[64];
    char idle_time[32];
    char from[64];
    pid_t pid;
    int login_type;
    int is_current;
} user_info_t;

typedef struct {
    user_info_t users[MAX_USERS];
    int user_count;
    int show_all;
    int show_headers;
    int show_idle;
    int show_login_time;
    int show_hostname;
    int show_processes;
    int sort_by_login;
    int show_count;
} who_options_t;

// 获取主机名
void get_hostname(char *hostname, size_t size) {
    if (gethostname(hostname, size) != 0) {
        strcpy(hostname, "unknown");
    }
}

// 格式化时间
void format_time_local(time_t time_val, char *buffer, size_t size) {
    struct tm *tm_info = localtime(&time_val);
    strftime(buffer, size, "%Y-%m-%d %H:%M", tm_info);
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

// 获取用户信息
int get_user_info(const struct utmp *ut, user_info_t *user) {
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
    user->login_type = ut->ut_type;
    
    // 格式化登录时间
    format_time_local(ut->ut_tv.tv_sec, user->login_time, sizeof(user->login_time));
    
    // 计算空闲时间（简化处理）
    calculate_idle_time(ut->ut_tv.tv_sec, user->idle_time, sizeof(user->idle_time));
    
    // 设置来源信息
    if (strlen(ut->ut_host) > 0) {
        snprintf(user->from, sizeof(user->from), "(%.*s)", 
                 (int)(sizeof(user->from) - 3), ut->ut_host);
    } else {
        strcpy(user->from, "(local)");
    }
    
    // 检查是否为当前用户
    struct passwd *pw = getpwnam(user->username);
    user->is_current = (pw != NULL && getuid() == pw->pw_uid);
    
    return 1;
}

// 获取所有用户信息
int get_all_users(who_options_t *options) {
    struct utmp *ut;
    int count = 0;
    
    setutent();
    
    while ((ut = getutent()) != NULL && count < MAX_USERS) {
        if (get_user_info(ut, &options->users[count])) {
            count++;
        }
    }
    
    endutent();
    options->user_count = count;
    return count;
}

// 获取用户状态图标
const char* get_user_status_icon(const user_info_t *user) {
    if (user->is_current) {
        return "👤";
    } else if (user->login_type == LOGIN_PROCESS) {
        return "🔐";
    } else {
        return "👥";
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

// 显示用户信息
void display_user(const user_info_t *user, who_options_t *options) {
    const char *user_icon = get_user_status_icon(user);
    // const char *term_icon = get_terminal_icon(user->terminal);
    
    // 根据用户类型设置颜色
    const char *user_color = COLOR_WHITE;
    if (user->is_current) {
        user_color = COLOR_GREEN;
    } else if (user->login_type == LOGIN_PROCESS) {
        user_color = COLOR_YELLOW;
    }
    
    printf("%s %s%-12s%s %s%-8s%s %s%-20s%s",
           user_icon,
           user_color, user->username, COLOR_RESET,
           COLOR_CYAN, user->terminal, COLOR_RESET,
           COLOR_BLUE, user->hostname, COLOR_RESET);
    
    if (options->show_login_time) {
        printf(" %s%-16s%s", COLOR_MAGENTA, user->login_time, COLOR_RESET);
    }
    
    if (options->show_idle) {
        printf(" %s%-8s%s", COLOR_YELLOW, user->idle_time, COLOR_RESET);
    }
    
    if (options->show_processes) {
        printf(" %s%-8d%s", COLOR_CYAN, user->pid, COLOR_RESET);
    }
    
    printf(" %s%s%s", COLOR_WHITE, user->from, COLOR_RESET);
    printf("\n");
}

// 显示所有用户
void display_users(who_options_t *options) {
    printf("%s用户登录信息 - 优化版 who%s\n", COLOR_CYAN, COLOR_RESET);
    printf("%s%s%s\n", COLOR_YELLOW, "================================================", COLOR_RESET);
    
    if (options->user_count == 0) {
        printf("%s没有找到登录用户%s\n", COLOR_RED, COLOR_RESET);
        return;
    }
    
    // 表头
    if (options->show_headers) {
        printf("%s%-12s %-8s %-20s%s", COLOR_WHITE, "用户名", "终端", "主机名", COLOR_RESET);
        
        if (options->show_login_time) {
            printf(" %s%-16s%s", COLOR_WHITE, "登录时间", COLOR_RESET);
        }
        
        if (options->show_idle) {
            printf(" %s%-8s%s", COLOR_WHITE, "空闲时间", COLOR_RESET);
        }
        
        if (options->show_processes) {
            printf(" %s%-8s%s", COLOR_WHITE, "进程ID", COLOR_RESET);
        }
        
        printf(" %s%s%s\n", COLOR_WHITE, "来源", COLOR_RESET);
        printf("%s%s%s\n", COLOR_YELLOW, "------------------------------------------------", COLOR_RESET);
    }
    
    // 显示用户
    for (int i = 0; i < options->user_count; i++) {
        display_user(&options->users[i], options);
    }
    
    printf("%s%s%s\n", COLOR_YELLOW, "================================================", COLOR_RESET);
    
    if (options->show_count) {
        printf("%s总用户数: %d%s\n", COLOR_CYAN, options->user_count, COLOR_RESET);
    }
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
    
    printf("  %s负载: %.2f %.2f %.2f%s\n",
           COLOR_CYAN,
           info.loads[0] / 65536.0,
           info.loads[1] / 65536.0,
           info.loads[2] / 65536.0,
           COLOR_RESET);
    
    printf("  %s进程数: %d%s\n",
           COLOR_CYAN, info.procs, COLOR_RESET);
}

// 显示当前用户信息
void display_current_user() {
    uid_t uid = getuid();
    struct passwd *pw = getpwuid(uid);
    
    if (pw) {
        printf("%s当前用户: %s%s%s (%s)%s\n",
               COLOR_GREEN,
               COLOR_WHITE, pw->pw_name, COLOR_RESET,
               pw->pw_gecos,
               COLOR_RESET);
    }
}

void print_usage(const char *program_name) {
    printf("用法: %s [选项]\n", program_name);
    printf("优化版的 who 命令，提供用户登录信息显示\n\n");
    printf("选项:\n");
    printf("  -a, --all            显示所有信息\n");
    printf("  -b, --boot           显示系统启动时间\n");
    printf("  -d, --dead           显示死进程\n");
    printf("  -H, --heading        显示列标题\n");
    printf("  -i, --idle           显示空闲时间\n");
    printf("  -l, --login          显示登录进程\n");
    printf("  -m, --mesg           显示消息状态\n");
    printf("  -p, --process        显示进程ID\n");
    printf("  -q, --count          只显示用户数和用户名\n");
    printf("  -r, --runlevel       显示运行级别\n");
    printf("  -s, --short          短格式输出\n");
    printf("  -T, --writable       显示终端写入状态\n");
    printf("  -u, --users          显示用户列表\n");
    printf("  -w, --writable       显示终端写入状态\n");
    printf("  --help               显示此帮助信息\n");
    printf("  --version            显示版本信息\n");
    printf("\n示例:\n");
    printf("  %s                    # 显示登录用户\n", program_name);
    printf("  %s -a                 # 显示所有信息\n", program_name);
    printf("  %s -i                 # 显示空闲时间\n", program_name);
    printf("  %s -q                 # 显示用户统计\n", program_name);
}

int main(int argc, char *argv[]) {
    who_options_t options = {0};
    options.show_headers = 1;
    options.show_login_time = 1;
    options.show_count = 1;
    
    // 解析命令行参数
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-a") == 0 || strcmp(argv[i], "--all") == 0) {
            options.show_all = 1;
            options.show_idle = 1;
            options.show_processes = 1;
            options.show_hostname = 1;
        } else if (strcmp(argv[i], "-b") == 0 || strcmp(argv[i], "--boot") == 0) {
            // 显示启动时间
            options.show_all = 1;
        } else if (strcmp(argv[i], "-d") == 0 || strcmp(argv[i], "--dead") == 0) {
            // 显示死进程
            options.show_all = 1;
        } else if (strcmp(argv[i], "-H") == 0 || strcmp(argv[i], "--heading") == 0) {
            options.show_headers = 1;
        } else if (strcmp(argv[i], "-i") == 0 || strcmp(argv[i], "--idle") == 0) {
            options.show_idle = 1;
        } else if (strcmp(argv[i], "-l") == 0 || strcmp(argv[i], "--login") == 0) {
            options.show_all = 1;
        } else if (strcmp(argv[i], "-m") == 0 || strcmp(argv[i], "--mesg") == 0) {
            options.show_all = 1;
        } else if (strcmp(argv[i], "-p") == 0 || strcmp(argv[i], "--process") == 0) {
            options.show_processes = 1;
        } else if (strcmp(argv[i], "-q") == 0 || strcmp(argv[i], "--count") == 0) {
            options.show_headers = 0;
            options.show_login_time = 0;
            options.show_idle = 0;
            options.show_processes = 0;
            options.show_count = 1;
        } else if (strcmp(argv[i], "-r") == 0 || strcmp(argv[i], "--runlevel") == 0) {
            options.show_all = 1;
        } else if (strcmp(argv[i], "-s") == 0 || strcmp(argv[i], "--short") == 0) {
            options.show_headers = 0;
            options.show_login_time = 0;
            options.show_idle = 0;
            options.show_processes = 0;
        } else if (strcmp(argv[i], "-T") == 0 || strcmp(argv[i], "--writable") == 0) {
            options.show_all = 1;
        } else if (strcmp(argv[i], "-u") == 0 || strcmp(argv[i], "--users") == 0) {
            options.show_all = 1;
        } else if (strcmp(argv[i], "-w") == 0 || strcmp(argv[i], "--writable") == 0) {
            options.show_all = 1;
        } else if (strcmp(argv[i], "--help") == 0) {
            print_usage(argv[0]);
            return 0;
        } else if (strcmp(argv[i], "--version") == 0) {
            printf("pwho - 优化版 who 命令 v1.0\n");
            return 0;
        }
    }
    
    // 获取用户信息
    if (get_all_users(&options) == 0) {
        print_error("无法获取用户信息");
        return 1;
    }
    
    // 显示当前用户
    display_current_user();
    printf("\n");
    
    // 显示用户列表
    display_users(&options);
    
    // 显示系统信息
    if (options.show_all) {
        printf("\n");
        display_system_info();
    }
    
    return 0;
}
