#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <getopt.h>
#include <time.h>
#include <ctype.h>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>

#include "../include/common.h"

// 网络连接信息结构
typedef struct {
    char protocol[16];
    char local_addr[64];
    char local_port[16];
    char remote_addr[64];
    char remote_port[16];
    char state[16];
    char pid[16];
    char process[64];
    time_t timestamp;
} netstat_info_t;

// 网络统计信息
typedef struct {
    int total_connections;
    int tcp_connections;
    int udp_connections;
    int listening_ports;
    int established_connections;
    int time_wait_connections;
    int close_wait_connections;
} netstat_stats_t;

// 选项结构
typedef struct {
    int show_all;
    int show_listening;
    int show_tcp;
    int show_udp;
    int show_processes;
    int show_numerical;
    int show_timers;
    int show_interface;
    int show_statistics;
    int show_route;
    int show_groups;
    int show_memberships;
    int show_masquerade;
    int show_netlink;
    int show_unix;
    int continuous;
    int wide;
    int verbose;
    int color_output;
    int show_headers;
    char *filter_protocol;
    char *filter_state;
    char *filter_address;
    int filter_port;
} netstat_options_t;

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

// 状态颜色映射
const char* get_state_color(const char* state) {
    if (strcmp(state, "ESTABLISHED") == 0) {
        return COLOR_GREEN;
    } else if (strcmp(state, "LISTEN") == 0) {
        return COLOR_YELLOW;
    } else if (strcmp(state, "TIME_WAIT") == 0) {
        return COLOR_MAGENTA;
    } else if (strcmp(state, "CLOSE_WAIT") == 0) {
        return COLOR_RED;
    } else if (strcmp(state, "SYN_SENT") == 0 || strcmp(state, "SYN_RECV") == 0) {
        return COLOR_CYAN;
    } else {
        return COLOR_WHITE;
    }
}

// 协议图标
const char* get_protocol_icon(const char* protocol) {
    if (strcmp(protocol, "tcp") == 0) {
        return "🔗";
    } else if (strcmp(protocol, "udp") == 0) {
        return "📡";
    } else if (strcmp(protocol, "unix") == 0) {
        return "🔌";
    } else {
        return "🌐";
    }
}

// 读取/proc/net/tcp文件
int read_tcp_connections(netstat_info_t **connections, int *count) {
    FILE *fp = fopen("/proc/net/tcp", "r");
    if (!fp) return -1;
    
    char line[1024];
    int capacity = 100;
    *count = 0;
    *connections = malloc(capacity * sizeof(netstat_info_t));
    
    // 跳过标题行
    if (fgets(line, sizeof(line), fp)) {
        // 标题行
    }
    
    while (fgets(line, sizeof(line), fp)) {
        if (*count >= capacity) {
            capacity *= 2;
            *connections = realloc(*connections, capacity * sizeof(netstat_info_t));
        }
        
        netstat_info_t *conn = &(*connections)[*count];
        memset(conn, 0, sizeof(netstat_info_t));
        
        unsigned long local_addr, local_port, remote_addr, remote_port, state, uid, inode;
        int ret = sscanf(line, "%*d: %lx:%lx %lx:%lx %lx %*x %*x %*x %*d %*d %lu",
                        &local_addr, &local_port, &remote_addr, &remote_port, &state, &uid, &inode);
        
        if (ret >= 6) {
            strcpy(conn->protocol, "tcp");
            
            // 转换地址
            struct in_addr addr;
            addr.s_addr = htonl(local_addr);
            strcpy(conn->local_addr, inet_ntoa(addr));
            sprintf(conn->local_port, "%u", ntohs(local_port));
            
            addr.s_addr = htonl(remote_addr);
            strcpy(conn->remote_addr, inet_ntoa(addr));
            sprintf(conn->remote_port, "%u", ntohs(remote_port));
            
            // 状态映射
            switch (state) {
                case 1: strcpy(conn->state, "ESTABLISHED"); break;
                case 2: strcpy(conn->state, "SYN_SENT"); break;
                case 3: strcpy(conn->state, "SYN_RECV"); break;
                case 4: strcpy(conn->state, "FIN_WAIT1"); break;
                case 5: strcpy(conn->state, "FIN_WAIT2"); break;
                case 6: strcpy(conn->state, "TIME_WAIT"); break;
                case 7: strcpy(conn->state, "CLOSE"); break;
                case 8: strcpy(conn->state, "CLOSE_WAIT"); break;
                case 9: strcpy(conn->state, "LAST_ACK"); break;
                case 10: strcpy(conn->state, "LISTEN"); break;
                case 11: strcpy(conn->state, "CLOSING"); break;
                default: strcpy(conn->state, "UNKNOWN"); break;
            }
            
            conn->timestamp = time(NULL);
            (*count)++;
        }
    }
    
    fclose(fp);
    return 0;
}

// 读取/proc/net/udp文件
int read_udp_connections(netstat_info_t **connections, int *count) {
    FILE *fp = fopen("/proc/net/udp", "r");
    if (!fp) return -1;
    
    char line[1024];
    int capacity = 100;
    *count = 0;
    *connections = malloc(capacity * sizeof(netstat_info_t));
    
    // 跳过标题行
    if (fgets(line, sizeof(line), fp)) {
        // 标题行
    }
    
    while (fgets(line, sizeof(line), fp)) {
        if (*count >= capacity) {
            capacity *= 2;
            *connections = realloc(*connections, capacity * sizeof(netstat_info_t));
        }
        
        netstat_info_t *conn = &(*connections)[*count];
        memset(conn, 0, sizeof(netstat_info_t));
        
        unsigned long local_addr, local_port, remote_addr, remote_port, state, uid, inode;
        int ret = sscanf(line, "%*d: %lx:%lx %lx:%lx %lx %*x %*x %*x %*d %*d %lu",
                        &local_addr, &local_port, &remote_addr, &remote_port, &state, &uid, &inode);
        
        if (ret >= 6) {
            strcpy(conn->protocol, "udp");
            
            // 转换地址
            struct in_addr addr;
            addr.s_addr = htonl(local_addr);
            strcpy(conn->local_addr, inet_ntoa(addr));
            sprintf(conn->local_port, "%u", ntohs(local_port));
            
            addr.s_addr = htonl(remote_addr);
            strcpy(conn->remote_addr, inet_ntoa(addr));
            sprintf(conn->remote_port, "%u", ntohs(remote_port));
            
            strcpy(conn->state, "UNCONN");
            conn->timestamp = time(NULL);
            (*count)++;
        }
    }
    
    fclose(fp);
    return 0;
}

// 获取进程信息
void get_process_info(netstat_info_t *conn) {
    (void)conn; // 避免未使用参数警告
    // 简化实现 - 实际应该通过inode查找进程
    strcpy(conn->process, "unknown");
    strcpy(conn->pid, "0");
}

// 显示网络连接
void display_connections(netstat_info_t *connections, int count, netstat_options_t *options) {
    if (options->show_headers) {
        printf("%s%s网络连接信息 - 优化版 netstat%s\n", COLOR_CYAN, COLOR_BOLD, COLOR_RESET);
        printf("%s%s\n", COLOR_YELLOW, "================================================");
        printf("%s协议  %-20s %-20s %-12s %-16s%s\n", 
               COLOR_WHITE, "本地地址", "远程地址", "状态", "进程", COLOR_RESET);
        printf("%s%s%s\n", COLOR_YELLOW, "================================================", COLOR_RESET);
    }
    
    for (int i = 0; i < count; i++) {
        netstat_info_t *conn = &connections[i];
        
        if (options->filter_protocol && strcmp(conn->protocol, options->filter_protocol) != 0) {
            continue;
        }
        
        if (options->filter_state && strcmp(conn->state, options->filter_state) != 0) {
            continue;
        }
        
        if (options->filter_address && strstr(conn->local_addr, options->filter_address) == NULL &&
            strstr(conn->remote_addr, options->filter_address) == NULL) {
            continue;
        }
        
        if (options->filter_port > 0) {
            int port = atoi(conn->local_port);
            if (port != options->filter_port) {
                continue;
            }
        }
        
        // 协议图标和颜色
        const char *icon = get_protocol_icon(conn->protocol);
        const char *state_color = get_state_color(conn->state);
        
        printf("%s%s %s%-4s%s %s%-20s%s %s%-20s%s %s%-12s%s %s%-16s%s\n",
               COLOR_CYAN, icon, COLOR_RESET,
               conn->protocol,
               COLOR_GREEN, conn->local_addr, COLOR_RESET,
               COLOR_BLUE, conn->remote_addr, COLOR_RESET,
               state_color, conn->state, COLOR_RESET,
               COLOR_MAGENTA, conn->process, COLOR_RESET);
    }
}

// 显示统计信息
void display_statistics(netstat_stats_t *stats) {
    printf("\n%s%s网络统计信息%s\n", COLOR_CYAN, COLOR_BOLD, COLOR_RESET);
    printf("%s%s\n", COLOR_YELLOW, "================================================");
    printf("%s总连接数: %s%d%s\n", COLOR_WHITE, COLOR_GREEN, stats->total_connections, COLOR_RESET);
    printf("%sTCP连接: %s%d%s\n", COLOR_WHITE, COLOR_BLUE, stats->tcp_connections, COLOR_RESET);
    printf("%sUDP连接: %s%d%s\n", COLOR_WHITE, COLOR_YELLOW, stats->udp_connections, COLOR_RESET);
    printf("%s监听端口: %s%d%s\n", COLOR_WHITE, COLOR_MAGENTA, stats->listening_ports, COLOR_RESET);
    printf("%s已建立连接: %s%d%s\n", COLOR_WHITE, COLOR_GREEN, stats->established_connections, COLOR_RESET);
    printf("%sTIME_WAIT: %s%d%s\n", COLOR_WHITE, COLOR_RED, stats->time_wait_connections, COLOR_RESET);
    printf("%s%s\n", COLOR_YELLOW, "================================================");
}

// 计算统计信息
void calculate_statistics(netstat_info_t *tcp_conns, int tcp_count,
                         netstat_info_t *udp_conns, int udp_count,
                         netstat_stats_t *stats) {
    (void)udp_conns; // 避免未使用参数警告
    (void)udp_count;
    memset(stats, 0, sizeof(netstat_stats_t));
    
    stats->tcp_connections = tcp_count;
    stats->udp_connections = udp_count;
    stats->total_connections = tcp_count + udp_count;
    
    for (int i = 0; i < tcp_count; i++) {
        if (strcmp(tcp_conns[i].state, "LISTEN") == 0) {
            stats->listening_ports++;
        } else if (strcmp(tcp_conns[i].state, "ESTABLISHED") == 0) {
            stats->established_connections++;
        } else if (strcmp(tcp_conns[i].state, "TIME_WAIT") == 0) {
            stats->time_wait_connections++;
        } else if (strcmp(tcp_conns[i].state, "CLOSE_WAIT") == 0) {
            stats->close_wait_connections++;
        }
    }
}

// 显示帮助信息
void print_help() {
    printf("%spnetstat - 优化版 netstat 命令%s\n", COLOR_CYAN, COLOR_RESET);
    printf("%s%s\n", COLOR_YELLOW, "================================================");
    printf("用法: pnetstat [选项]\n\n");
    printf("选项:\n");
    printf("  -a, --all           显示所有连接\n");
    printf("  -l, --listening     显示监听端口\n");
    printf("  -t, --tcp           显示TCP连接\n");
    printf("  -u, --udp           显示UDP连接\n");
    printf("  -p, --programs      显示进程信息\n");
    printf("  -n, --numeric       显示数字地址\n");
    printf("  -s, --statistics    显示统计信息\n");
    printf("  -r, --route         显示路由表\n");
    printf("  -i, --interfaces    显示网络接口\n");
    printf("  -g, --groups        显示多播组\n");
    printf("  -M, --masquerade    显示伪装连接\n");
    printf("  -c, --continuous    连续显示\n");
    printf("  -w, --wide          宽输出格式\n");
    printf("  -v, --verbose       详细输出\n");
    printf("  --color             启用彩色输出\n");
    printf("  --no-color          禁用彩色输出\n");
    printf("  --filter-protocol   过滤协议\n");
    printf("  --filter-state      过滤状态\n");
    printf("  --filter-address    过滤地址\n");
    printf("  --filter-port       过滤端口\n");
    printf("  -h, --help          显示此帮助信息\n");
    printf("  -V, --version       显示版本信息\n\n");
    printf("示例:\n");
    printf("  pnetstat -tuln      显示所有TCP和UDP监听端口\n");
    printf("  pnetstat -an         显示所有连接\n");
    printf("  pnetstat -s          显示网络统计信息\n");
    printf("  pnetstat --filter-state ESTABLISHED 显示已建立连接\n");
}

// 显示版本信息
void print_version() {
    printf("pnetstat - 优化版 netstat 命令 v1.0\n");
    printf("使用C语言和彩色输出\n");
}

// 解析命令行参数
void parse_arguments(int argc, char *argv[], netstat_options_t *options) {
    memset(options, 0, sizeof(netstat_options_t));
    options->color_output = 1;
    options->show_headers = 1;
    
    static struct option long_options[] = {
        {"all", no_argument, 0, 'a'},
        {"listening", no_argument, 0, 'l'},
        {"tcp", no_argument, 0, 't'},
        {"udp", no_argument, 0, 'u'},
        {"programs", no_argument, 0, 'p'},
        {"numeric", no_argument, 0, 'n'},
        {"statistics", no_argument, 0, 's'},
        {"route", no_argument, 0, 'r'},
        {"interfaces", no_argument, 0, 'i'},
        {"groups", no_argument, 0, 'g'},
        {"masquerade", no_argument, 0, 'M'},
        {"continuous", no_argument, 0, 'c'},
        {"wide", no_argument, 0, 'w'},
        {"verbose", no_argument, 0, 'v'},
        {"color", no_argument, 0, 'C'},
        {"no-color", no_argument, 0, 'N'},
        {"filter-protocol", required_argument, 0, 'P'},
        {"filter-state", required_argument, 0, 'S'},
        {"filter-address", required_argument, 0, 'A'},
        {"filter-port", required_argument, 0, 'P'},
        {"help", no_argument, 0, 'h'},
        {"version", no_argument, 0, 'V'},
        {0, 0, 0, 0}
    };
    
    int opt;
    int option_index = 0;
    
    while ((opt = getopt_long(argc, argv, "altupnsrigMcwvhV", long_options, &option_index)) != -1) {
        switch (opt) {
            case 'a': options->show_all = 1; break;
            case 'l': options->show_listening = 1; break;
            case 't': options->show_tcp = 1; break;
            case 'u': options->show_udp = 1; break;
            case 'p': options->show_processes = 1; break;
            case 'n': options->show_numerical = 1; break;
            case 's': options->show_statistics = 1; break;
            case 'r': options->show_route = 1; break;
            case 'i': options->show_interface = 1; break;
            case 'g': options->show_groups = 1; break;
            case 'M': options->show_masquerade = 1; break;
            case 'c': options->continuous = 1; break;
            case 'w': options->wide = 1; break;
            case 'v': options->verbose = 1; break;
            case 'C': options->color_output = 1; break;
            case 'N': options->color_output = 0; break;
            case 'P': options->filter_protocol = optarg; break;
            case 'S': options->filter_state = optarg; break;
            case 'A': options->filter_address = optarg; break;
            case 'h': print_help(); exit(0);
            case 'V': print_version(); exit(0);
            case '?': print_help(); exit(1);
            default: break;
        }
    }
    
    // 默认显示所有连接
    if (!options->show_all && !options->show_listening && !options->show_tcp && 
        !options->show_udp && !options->show_statistics) {
        options->show_all = 1;
    }
}

int main(int argc, char *argv[]) {
    netstat_options_t options;
    netstat_info_t *tcp_connections = NULL, *udp_connections = NULL;
    int tcp_count = 0, udp_count = 0;
    netstat_stats_t stats;
    
    parse_arguments(argc, argv, &options);
    
    if (options.show_statistics) {
        // 读取连接数据
        if (options.show_tcp || options.show_all) {
            read_tcp_connections(&tcp_connections, &tcp_count);
        }
        if (options.show_udp || options.show_all) {
            read_udp_connections(&udp_connections, &udp_count);
        }
        
        // 计算统计信息
        calculate_statistics(tcp_connections, tcp_count, udp_connections, udp_count, &stats);
        display_statistics(&stats);
    } else {
        // 显示连接信息
        if (options.show_tcp || options.show_all) {
            read_tcp_connections(&tcp_connections, &tcp_count);
            if (tcp_connections) {
                display_connections(tcp_connections, tcp_count, &options);
            }
        }
        
        if (options.show_udp || options.show_all) {
            read_udp_connections(&udp_connections, &udp_count);
            if (udp_connections) {
                display_connections(udp_connections, udp_count, &options);
            }
        }
    }
    
    // 清理内存
    if (tcp_connections) free(tcp_connections);
    if (udp_connections) free(udp_connections);
    
    return 0;
}
