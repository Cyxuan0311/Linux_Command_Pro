#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <ctype.h>
#include <pwd.h>
#include <grp.h>
#include "../include/common.h"

#define MAX_PROCESSES 1000
#define MAX_PROCESS_NAME 256

typedef struct {
    pid_t pid;
    char name[MAX_PROCESS_NAME];
    char user[32];
    int signal;
    int confirmed;
} process_info_t;

typedef struct {
    process_info_t processes[MAX_PROCESSES];
    int process_count;
    int signal;
    int interactive;
    int force;
    int show_info;
    char pattern[MAX_PROCESS_NAME];
    int use_regex;
} kill_options_t;

// 获取进程信息
int get_process_info(pid_t pid, process_info_t *proc) {
    char path[256];
    FILE *file;
    char line[1024];
    
    proc->pid = pid;
    
    // 获取进程名
    snprintf(path, sizeof(path), "/proc/%d/comm", pid);
    file = fopen(path, "r");
    if (file) {
        if (fgets(line, sizeof(line), file)) {
            line[strcspn(line, "\n")] = '\0';
            strncpy(proc->name, line, MAX_PROCESS_NAME - 1);
            proc->name[MAX_PROCESS_NAME - 1] = '\0';
        }
        fclose(file);
    }
    
    // 获取用户信息
    snprintf(path, sizeof(path), "/proc/%d/status", pid);
    file = fopen(path, "r");
    if (file) {
        while (fgets(line, sizeof(line), file)) {
            if (strncmp(line, "Uid:", 4) == 0) {
                int uid;
                sscanf(line, "Uid: %d", &uid);
                struct passwd *pw = getpwuid(uid);
                if (pw) {
                    strncpy(proc->user, pw->pw_name, 31);
                    proc->user[31] = '\0';
                } else {
                    snprintf(proc->user, sizeof(proc->user), "%d", uid);
                }
                break;
            }
        }
        fclose(file);
    }
    
    return 1;
}

// 搜索进程
int search_processes(kill_options_t *options) {
    DIR *dir;
    struct dirent *entry;
    int count = 0;
    
    dir = opendir("/proc");
    if (!dir) return 0;
    
    while ((entry = readdir(dir)) != NULL && count < MAX_PROCESSES) {
        if (entry->d_type == DT_DIR && isdigit(entry->d_name[0])) {
            pid_t pid = atoi(entry->d_name);
            process_info_t proc;
            
            if (get_process_info(pid, &proc)) {
                // 检查是否匹配模式
                int matches = 0;
                if (options->use_regex) {
                    // 简单的正则表达式匹配（这里简化处理）
                    matches = (strstr(proc.name, options->pattern) != NULL);
                } else {
                    matches = (strstr(proc.name, options->pattern) != NULL);
                }
                
                if (matches) {
                    options->processes[count] = proc;
                    options->processes[count].signal = options->signal;
                    count++;
                }
            }
        }
    }
    
    closedir(dir);
    options->process_count = count;
    return count;
}

// 发送信号
int send_signal(pid_t pid, int signal) {
    if (kill(pid, signal) == 0) {
        return 1;
    }
    return 0;
}

// 获取信号名称
const char* get_signal_name(int signal) {
    switch (signal) {
        case SIGHUP: return "HUP";
        case SIGINT: return "INT";
        case SIGQUIT: return "QUIT";
        case SIGILL: return "ILL";
        case SIGTRAP: return "TRAP";
        case SIGABRT: return "ABRT";
        case SIGBUS: return "BUS";
        case SIGFPE: return "FPE";
        case SIGKILL: return "KILL";
        case SIGUSR1: return "USR1";
        case SIGSEGV: return "SEGV";
        case SIGUSR2: return "USR2";
        case SIGPIPE: return "PIPE";
        case SIGALRM: return "ALRM";
        case SIGTERM: return "TERM";
        case SIGSTKFLT: return "STKFLT";
        case SIGCHLD: return "CHLD";
        case SIGCONT: return "CONT";
        case SIGSTOP: return "STOP";
        case SIGTSTP: return "TSTP";
        case SIGTTIN: return "TTIN";
        case SIGTTOU: return "TTOU";
        case SIGURG: return "URG";
        case SIGXCPU: return "XCPU";
        case SIGXFSZ: return "XFSZ";
        case SIGVTALRM: return "VTALRM";
        case SIGPROF: return "PROF";
        case SIGWINCH: return "WINCH";
        case SIGIO: return "IO";
        case SIGPWR: return "PWR";
        case SIGSYS: return "SYS";
        default: return "UNKNOWN";
    }
}

// 获取信号颜色
const char* get_signal_color(int signal) {
    switch (signal) {
        case SIGKILL: return COLOR_RED;
        case SIGTERM: return COLOR_YELLOW;
        case SIGINT: return COLOR_CYAN;
        case SIGSTOP: return COLOR_MAGENTA;
        case SIGCONT: return COLOR_GREEN;
        default: return COLOR_WHITE;
    }
}

// 显示进程信息
void display_process(const process_info_t *proc) {
    const char *signal_color = get_signal_color(proc->signal);
    
    printf("%s%-8d %s%-15s%s %s%-12s%s %s%s%s",
           COLOR_CYAN, proc->pid, COLOR_RESET,
           COLOR_GREEN, proc->name, COLOR_RESET,
           COLOR_BLUE, proc->user, COLOR_RESET,
           signal_color, get_signal_name(proc->signal), COLOR_RESET);
    
    if (proc->confirmed) {
        printf(" %s✓%s", COLOR_GREEN, COLOR_RESET);
    } else {
        printf(" %s?%s", COLOR_YELLOW, COLOR_RESET);
    }
    
    printf("\n");
}

// 显示所有匹配的进程
void display_processes(kill_options_t *options) {
    printf("%s进程管理 - 优化版 kill%s\n", COLOR_CYAN, COLOR_RESET);
    printf("%s%s%s\n", COLOR_YELLOW, "================================================", COLOR_RESET);
    
    if (options->process_count == 0) {
        printf("%s没有找到匹配的进程%s\n", COLOR_RED, COLOR_RESET);
        return;
    }
    
    // 表头
    printf("%s%-8s %-15s %-12s %-8s %s\n",
           COLOR_WHITE,
           "PID", "进程名", "用户", "信号", "状态",
           COLOR_RESET);
    printf("%s%s%s\n", COLOR_YELLOW, "------------------------------------------------", COLOR_RESET);
    
    // 显示进程
    for (int i = 0; i < options->process_count; i++) {
        display_process(&options->processes[i]);
    }
    
    printf("%s%s%s\n", COLOR_YELLOW, "================================================", COLOR_RESET);
    printf("%s找到 %d 个匹配的进程%s\n", COLOR_CYAN, options->process_count, COLOR_RESET);
}

// 交互式确认
int confirm_action(kill_options_t *options) {
    if (!options->interactive) {
        return 1;
    }
    
    printf("\n%s确认要发送信号给这些进程吗？ (y/N): %s", COLOR_YELLOW, COLOR_RESET);
    
    char response[10];
    if (fgets(response, sizeof(response), stdin)) {
        response[strcspn(response, "\n")] = '\0';
        return (strcasecmp(response, "y") == 0 || strcasecmp(response, "yes") == 0);
    }
    
    return 0;
}

// 执行kill操作
int execute_kill(kill_options_t *options) {
    int success_count = 0;
    int fail_count = 0;
    
    for (int i = 0; i < options->process_count; i++) {
        process_info_t *proc = &options->processes[i];
        
        if (send_signal(proc->pid, proc->signal)) {
            proc->confirmed = 1;
            success_count++;
            printf("%s✓ 成功发送 %s 信号给进程 %d (%s)%s\n",
                   COLOR_GREEN,
                   get_signal_name(proc->signal),
                   proc->pid,
                   proc->name,
                   COLOR_RESET);
        } else {
            fail_count++;
            printf("%s✗ 无法发送信号给进程 %d (%s): %s%s\n",
                   COLOR_RED,
                   proc->pid,
                   proc->name,
                   strerror(errno),
                   COLOR_RESET);
        }
    }
    
    printf("\n%s操作完成: 成功 %d, 失败 %d%s\n",
           COLOR_CYAN, success_count, fail_count, COLOR_RESET);
    
    return (fail_count == 0) ? 0 : 1;
}

// 解析信号
int parse_signal(const char *signal_str) {
    // 检查是否为数字
    if (isdigit(signal_str[0])) {
        return atoi(signal_str);
    }
    
    // 检查信号名称
    if (strcasecmp(signal_str, "HUP") == 0) return SIGHUP;
    if (strcasecmp(signal_str, "INT") == 0) return SIGINT;
    if (strcasecmp(signal_str, "QUIT") == 0) return SIGQUIT;
    if (strcasecmp(signal_str, "ILL") == 0) return SIGILL;
    if (strcasecmp(signal_str, "TRAP") == 0) return SIGTRAP;
    if (strcasecmp(signal_str, "ABRT") == 0) return SIGABRT;
    if (strcasecmp(signal_str, "BUS") == 0) return SIGBUS;
    if (strcasecmp(signal_str, "FPE") == 0) return SIGFPE;
    if (strcasecmp(signal_str, "KILL") == 0) return SIGKILL;
    if (strcasecmp(signal_str, "USR1") == 0) return SIGUSR1;
    if (strcasecmp(signal_str, "SEGV") == 0) return SIGSEGV;
    if (strcasecmp(signal_str, "USR2") == 0) return SIGUSR2;
    if (strcasecmp(signal_str, "PIPE") == 0) return SIGPIPE;
    if (strcasecmp(signal_str, "ALRM") == 0) return SIGALRM;
    if (strcasecmp(signal_str, "TERM") == 0) return SIGTERM;
    if (strcasecmp(signal_str, "STKFLT") == 0) return SIGSTKFLT;
    if (strcasecmp(signal_str, "CHLD") == 0) return SIGCHLD;
    if (strcasecmp(signal_str, "CONT") == 0) return SIGCONT;
    if (strcasecmp(signal_str, "STOP") == 0) return SIGSTOP;
    if (strcasecmp(signal_str, "TSTP") == 0) return SIGTSTP;
    if (strcasecmp(signal_str, "TTIN") == 0) return SIGTTIN;
    if (strcasecmp(signal_str, "TTOU") == 0) return SIGTTOU;
    if (strcasecmp(signal_str, "URG") == 0) return SIGURG;
    if (strcasecmp(signal_str, "XCPU") == 0) return SIGXCPU;
    if (strcasecmp(signal_str, "XFSZ") == 0) return SIGXFSZ;
    if (strcasecmp(signal_str, "VTALRM") == 0) return SIGVTALRM;
    if (strcasecmp(signal_str, "PROF") == 0) return SIGPROF;
    if (strcasecmp(signal_str, "WINCH") == 0) return SIGWINCH;
    if (strcasecmp(signal_str, "IO") == 0) return SIGIO;
    if (strcasecmp(signal_str, "PWR") == 0) return SIGPWR;
    if (strcasecmp(signal_str, "SYS") == 0) return SIGSYS;
    
    return -1;
}

void print_usage(const char *program_name) {
    printf("用法: %s [选项] 信号 进程名\n", program_name);
    printf("优化版的 kill 命令，提供进程管理功能\n\n");
    printf("选项:\n");
    printf("  -i, --interactive     交互式确认\n");
    printf("  -f, --force          强制操作\n");
    printf("  -s, --signal 信号     指定信号 (默认: TERM)\n");
    printf("  -l, --list           列出所有信号\n");
    printf("  -r, --regex          使用正则表达式匹配\n");
    printf("  -v, --verbose        显示详细信息\n");
    printf("  --help               显示此帮助信息\n");
    printf("  --version            显示版本信息\n");
    printf("\n信号:\n");
    printf("  TERM, INT, QUIT, KILL, STOP, CONT, HUP, USR1, USR2\n");
    printf("\n示例:\n");
    printf("  %s -s TERM firefox    # 终止firefox进程\n", program_name);
    printf("  %s -i -s KILL chrome  # 强制终止chrome进程\n", program_name);
    printf("  %s -l                 # 列出所有信号\n", program_name);
    printf("  %s -r \"^java\"         # 使用正则表达式匹配\n", program_name);
}

void list_signals() {
    printf("%s可用信号列表:%s\n", COLOR_CYAN, COLOR_RESET);
    printf("%s%s%s\n", COLOR_YELLOW, "================================================", COLOR_RESET);
    
    int signals[] = {SIGHUP, SIGINT, SIGQUIT, SIGILL, SIGTRAP, SIGABRT, SIGBUS, SIGFPE, SIGKILL,
                     SIGUSR1, SIGSEGV, SIGUSR2, SIGPIPE, SIGALRM, SIGTERM, SIGSTKFLT, SIGCHLD,
                     SIGCONT, SIGSTOP, SIGTSTP, SIGTTIN, SIGTTOU, SIGURG, SIGXCPU, SIGXFSZ,
                     SIGVTALRM, SIGPROF, SIGWINCH, SIGIO, SIGPWR, SIGSYS};
    
    int num_signals = sizeof(signals) / sizeof(signals[0]);
    
    for (int i = 0; i < num_signals; i++) {
        const char *color = get_signal_color(signals[i]);
        printf("%s%2d%s %s%-8s%s %s\n",
               COLOR_CYAN, signals[i], COLOR_RESET,
               color, get_signal_name(signals[i]), COLOR_RESET,
               (signals[i] == SIGKILL) ? "(强制终止)" : 
               (signals[i] == SIGTERM) ? "(优雅终止)" : "");
    }
}

int main(int argc, char *argv[]) {
    kill_options_t options = {0};
    options.signal = SIGTERM; // 默认信号
    
    // 解析命令行参数
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-i") == 0 || strcmp(argv[i], "--interactive") == 0) {
            options.interactive = 1;
        } else if (strcmp(argv[i], "-f") == 0 || strcmp(argv[i], "--force") == 0) {
            options.force = 1;
            options.signal = SIGKILL;
        } else if (strcmp(argv[i], "-s") == 0 || strcmp(argv[i], "--signal") == 0) {
            if (i + 1 < argc) {
                int signal = parse_signal(argv[++i]);
                if (signal == -1) {
                    print_error("无效的信号");
                    return 1;
                }
                options.signal = signal;
            }
        } else if (strcmp(argv[i], "-l") == 0 || strcmp(argv[i], "--list") == 0) {
            list_signals();
            return 0;
        } else if (strcmp(argv[i], "-r") == 0 || strcmp(argv[i], "--regex") == 0) {
            options.use_regex = 1;
        } else if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--verbose") == 0) {
            options.show_info = 1;
        } else if (strcmp(argv[i], "--help") == 0) {
            print_usage(argv[0]);
            return 0;
        } else if (strcmp(argv[i], "--version") == 0) {
            printf("pkill - 优化版 kill 命令 v1.0\n");
            return 0;
        } else if (argv[i][0] != '-') {
            strncpy(options.pattern, argv[i], sizeof(options.pattern) - 1);
            options.pattern[sizeof(options.pattern) - 1] = '\0';
        }
    }
    
    if (strlen(options.pattern) == 0) {
        print_error("请指定进程名模式");
        print_usage(argv[0]);
        return 1;
    }
    
    // 搜索进程
    if (search_processes(&options) == 0) {
        print_error("没有找到匹配的进程");
        return 1;
    }
    
    // 显示进程信息
    display_processes(&options);
    
    // 确认操作
    if (confirm_action(&options)) {
        return execute_kill(&options);
    } else {
        printf("%s操作已取消%s\n", COLOR_YELLOW, COLOR_RESET);
        return 0;
    }
}
