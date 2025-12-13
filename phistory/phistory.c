#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <termios.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <sys/wait.h>
#include "../include/common.h"

#define MAX_HISTORY_LINES 10000
#define MAX_LINE_LENGTH 4096
#define DEFAULT_HISTORY_LIMIT 50

// 淡蓝色反白效果
#define LIGHT_BLUE_BG "\033[104m"  // 亮蓝色背景
#define LIGHT_BLUE_FG "\033[94m"    // 亮蓝色前景
#define LIGHT_BLUE_RESET "\033[0m"  // 重置颜色

// 历史命令结构
typedef struct {
    char **lines;
    int count;
    int capacity;
} History;

// 选项结构
typedef struct {
    int limit;
    int show_help;
    int show_version;
} HistoryOptions;

// 初始化选项
void init_options(HistoryOptions *opts) {
    memset(opts, 0, sizeof(HistoryOptions));
    opts->limit = DEFAULT_HISTORY_LIMIT;
}

// 显示帮助信息
void print_help(const char *program_name) {
    printf("用法: %s [选项]\n", program_name);
    printf("增强版 history 命令，支持交互式选择历史命令\n\n");
    printf("选项:\n");
    printf("  -l, --limit=N         显示最近 N 条历史命令 (默认: %d)\n", DEFAULT_HISTORY_LIMIT);
    printf("  -h, --help             显示此帮助信息\n");
    printf("  -v, --version          显示版本信息\n\n");
    printf("交互式操作:\n");
    printf("  使用 ↑/↓ 箭头键选择历史命令\n");
    printf("  按 Enter 键执行选中的命令\n");
    printf("  按 q 或 Esc 键退出\n\n");
    printf("示例:\n");
    printf("  %s                # 显示最近 %d 条历史命令\n", program_name, DEFAULT_HISTORY_LIMIT);
    printf("  %s -l 100         # 显示最近 100 条历史命令\n", program_name);
}

// 显示版本信息
void print_version() {
    printf("phistory - 增强版 history 命令 v1.0\n");
    printf("支持交互式选择历史命令\n");
}

// 初始化历史结构
void init_history(History *hist) {
    hist->capacity = 1000;
    hist->count = 0;
    hist->lines = (char **)malloc(hist->capacity * sizeof(char *));
    if (!hist->lines) {
        fprintf(stderr, "内存分配失败\n");
        exit(1);
    }
}

// 释放历史结构
void free_history(History *hist) {
    for (int i = 0; i < hist->count; i++) {
        free(hist->lines[i]);
    }
    free(hist->lines);
    hist->count = 0;
    hist->capacity = 0;
}

// 添加历史行
void add_history_line(History *hist, const char *line) {
    if (hist->count >= hist->capacity) {
        hist->capacity *= 2;
        hist->lines = (char **)realloc(hist->lines, hist->capacity * sizeof(char *));
        if (!hist->lines) {
            fprintf(stderr, "内存重新分配失败\n");
            exit(1);
        }
    }
    
    hist->lines[hist->count] = (char *)malloc(strlen(line) + 1);
    if (!hist->lines[hist->count]) {
        fprintf(stderr, "内存分配失败\n");
        exit(1);
    }
    strcpy(hist->lines[hist->count], line);
    hist->count++;
}

// 刷新历史文件（将当前会话的历史追加到文件）
void refresh_history_file(const char *filename) {
    // 尝试执行 history -a 来将当前会话的历史追加到文件
    // 这会立即将当前 shell 的历史写入历史文件
    char cmd[512];
    const char *shell = getenv("SHELL");
    int ret;
    
    if (shell && strstr(shell, "bash")) {
        // 对于 bash，使用 history -a 命令
        snprintf(cmd, sizeof(cmd), "history -a %s 2>/dev/null", filename);
        ret = system(cmd);
        (void)ret; // 忽略返回值，刷新失败不影响主流程
    } else if (shell && strstr(shell, "zsh")) {
        // 对于 zsh，使用 fc -W 命令
        snprintf(cmd, sizeof(cmd), "fc -W %s 2>/dev/null", filename);
        ret = system(cmd);
        (void)ret; // 忽略返回值，刷新失败不影响主流程
    } else {
        // 默认尝试 bash 的方式
        snprintf(cmd, sizeof(cmd), "history -a %s 2>/dev/null", filename);
        ret = system(cmd);
        (void)ret; // 忽略返回值，刷新失败不影响主流程
    }
}

// 读取历史文件
int read_history_file(History *hist, const char *filename, int limit) {
    // 先刷新历史文件，确保获取最新的历史
    refresh_history_file(filename);
    
    // 等待一下，确保文件已写入
    usleep(100000); // 100ms
    
    FILE *file = fopen(filename, "r");
    if (!file) {
        return 0;
    }
    
    char line[MAX_LINE_LENGTH];
    int line_count = 0;
    
    // 先读取所有行
    while (fgets(line, sizeof(line), file) != NULL) {
        // 移除换行符
        line[strcspn(line, "\n")] = '\0';
        
        // 跳过空行
        if (strlen(line) == 0) {
            continue;
        }
        
        add_history_line(hist, line);
        line_count++;
    }
    
    fclose(file);
    
    // 只保留最近的 limit 条
    if (hist->count > limit) {
        int start = hist->count - limit;
        for (int i = 0; i < limit; i++) {
            free(hist->lines[i]);
            hist->lines[i] = hist->lines[start + i];
        }
        hist->count = limit;
    }
    
    return 1;
}

// 获取历史文件路径
char *get_history_file_path() {
    char *histfile = getenv("HISTFILE");
    if (histfile && strlen(histfile) > 0) {
        return strdup(histfile);
    }
    
    // 尝试常见的 shell 历史文件
    const char *home = getenv("HOME");
    if (!home) {
        return NULL;
    }
    
    char *path = (char *)malloc(512);
    if (!path) {
        return NULL;
    }
    
    // 尝试 bash_history
    snprintf(path, 512, "%s/.bash_history", home);
    if (access(path, R_OK) == 0) {
        return path;
    }
    
    // 尝试 zsh_history
    snprintf(path, 512, "%s/.zsh_history", home);
    if (access(path, R_OK) == 0) {
        return path;
    }
    
    // 默认使用 bash_history
    snprintf(path, 512, "%s/.bash_history", home);
    return path;
}

// 设置终端为原始模式
struct termios set_raw_mode() {
    struct termios old_termios, new_termios;
    if (tcgetattr(STDIN_FILENO, &old_termios) == -1) {
        perror("tcgetattr");
        exit(1);
    }
    new_termios = old_termios;
    new_termios.c_lflag &= ~(ICANON | ECHO | ECHOE | ECHOK | ECHONL | ISIG);
    new_termios.c_iflag &= ~(IXON | IXOFF | INLCR | IGNCR | ICRNL);
    new_termios.c_oflag &= ~OPOST;
    new_termios.c_cc[VMIN] = 1;
    new_termios.c_cc[VTIME] = 0;
    if (tcsetattr(STDIN_FILENO, TCSANOW, &new_termios) == -1) {
        perror("tcsetattr");
        exit(1);
    }
    return old_termios;
}

// 恢复终端模式
void restore_termios(struct termios old_termios) {
    tcsetattr(STDIN_FILENO, TCSANOW, &old_termios);
}

// 清空当前行
void clear_line(int line_length) {
    printf("\r");
    for (int i = 0; i < line_length; i++) {
        printf(" ");
    }
    printf("\r");
}

// 获取终端宽度
int get_terminal_width() {
    struct winsize w;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == 0) {
        return w.ws_col;
    }
    return 80; // 默认宽度
}

// 计算字符串显示宽度（不考虑ANSI代码）
int display_width(const char *str) {
    int width = 0;
    int in_escape = 0;
    for (int i = 0; str[i] != '\0'; i++) {
        if (str[i] == '\033') {
            in_escape = 1;
        } else if (in_escape) {
            if (str[i] == 'm' || str[i] == 'K' || str[i] == 'H' || str[i] == 'J') {
                in_escape = 0;
            }
        } else {
            width++;
        }
    }
    return width;
}

// 截断字符串以适应终端宽度（确保不包含ANSI代码）
void truncate_string(char *dest, const char *src, int max_width) {
    int len = strlen(src);
    if (len <= max_width) {
        strcpy(dest, src);
    } else {
        // 确保截断后加上省略号不超过最大宽度
        int truncate_len = max_width - 3;
        if (truncate_len < 0) truncate_len = 0;
        strncpy(dest, src, truncate_len);
        dest[truncate_len] = '\0';
        strcat(dest, "...");
    }
}

// 显示历史命令列表
void display_history(History *hist, int selected_index) {
    // 清屏并移动光标到左上角
    // \033[2J - 清屏
    // \033[3J - 清除滚动缓冲区（如果支持）
    // \033[H - 移动光标到左上角
    // \033[0m - 重置所有格式
    printf("\033[2J\033[3J\033[H\033[0m");
    fflush(stdout);
    
    int terminal_width = get_terminal_width();
    // 固定前缀宽度："> " 或 "  " (2字符) + 编号(3字符) + ": " (2字符) = 7字符
    // 为命令文本预留空间，确保不超过终端宽度
    int cmd_text_width = terminal_width - 8;
    if (cmd_text_width < 30) cmd_text_width = 30; // 最小宽度
    
    // 输出标题
    printf("%s历史命令列表 (使用 ↑/↓ 选择, Enter 执行, q 退出):%s\n\n", COLOR_CYAN, COLOR_RESET);
    
    int start = 0;
    int end = hist->count;
    int display_count = 20; // 一次显示20条
    
    // 计算显示范围，确保选中项在可见范围内
    if (hist->count > display_count) {
        if (selected_index < display_count / 2) {
            start = 0;
            end = display_count;
        } else if (selected_index >= hist->count - display_count / 2) {
            start = hist->count - display_count;
            end = hist->count;
        } else {
            start = selected_index - display_count / 2;
            end = selected_index + display_count / 2;
        }
    }
    
    // 显示历史命令，使用固定宽度格式确保严格对齐
    char truncated[MAX_LINE_LENGTH];
    
    for (int i = start; i < end && i < hist->count; i++) {
        // 截断过长的命令
        truncate_string(truncated, hist->lines[i], cmd_text_width);
        
        // 计算实际显示宽度（用于填充对齐）
        int actual_width = display_width(truncated);
        int padding = cmd_text_width - actual_width;
        if (padding < 0) padding = 0;
        
        // 确保从行首开始输出（使用 \r 回到行首）
        if (i == selected_index) {
            // 选中项：使用淡蓝色背景反白效果，固定格式对齐
            // 格式："> " (2字符) + 编号(3字符) + ": " (2字符) + 命令文本 + 填充
            printf("\r%s%s> %3d: %s", LIGHT_BLUE_BG, COLOR_WHITE, i + 1, truncated);
            // 添加填充空格确保对齐（填充部分也需要淡蓝色背景）
            for (int j = 0; j < padding; j++) {
                putchar(' ');
            }
            printf("%s\033[K\n", LIGHT_BLUE_RESET);
        } else {
            // 普通项：使用固定格式对齐（2个空格对齐选中项的 "> "）
            // 格式："  " (2字符) + 编号(3字符) + ": " (2字符) + 命令文本 + 填充
            printf("\r  %3d: %s", i + 1, truncated);
            // 添加填充空格确保对齐
            for (int j = 0; j < padding; j++) {
                putchar(' ');
            }
            printf("\033[K\n");
        }
    }
    
    // 显示统计信息
    if (hist->count > display_count) {
        printf("\n%s[显示 %d-%d / 共 %d 条]%s\n", 
               COLOR_YELLOW, start + 1, end, hist->count, COLOR_RESET);
    } else {
        printf("\n%s[共 %d 条]%s\n", COLOR_YELLOW, hist->count, COLOR_RESET);
    }
    
    fflush(stdout);
}

// 交互式选择历史命令
int interactive_select(History *hist, const char *histfile) {
    if (hist->count == 0) {
        printf("%s没有历史命令可显示%s\n", COLOR_YELLOW, COLOR_RESET);
        return -1;
    }
    
    // 检查是否为终端
    if (!isatty(STDIN_FILENO)) {
        // 非交互模式，直接打印所有历史
        for (int i = 0; i < hist->count; i++) {
            printf("%3d  %s\n", i + 1, hist->lines[i]);
        }
        return -1;
    }
    
    struct termios old_termios = set_raw_mode();
    int selected_index = hist->count - 1; // 默认选择最后一条
    
    // 初始显示
    display_history(hist, selected_index);
    
    while (1) {
        unsigned char c;
        ssize_t n = read(STDIN_FILENO, &c, 1);
        
        if (n <= 0) {
            continue;
        }
        
        // 调试：打印接收到的字符（可以注释掉）
        // fprintf(stderr, "收到字符: %d (0x%02x) '%c'\n", c, c, (c >= 32 && c < 127) ? c : '?');
        
        // 处理 ESC 序列（箭头键）
        if (c == '\033' || c == 27) {
            // 临时设置终端为带超时的模式来读取转义序列
            struct termios temp_termios;
            tcgetattr(STDIN_FILENO, &temp_termios);
            temp_termios.c_cc[VTIME] = 1; // 100ms 超时
            temp_termios.c_cc[VMIN] = 0;
            tcsetattr(STDIN_FILENO, TCSANOW, &temp_termios);
            
            unsigned char seq[3];
            int seq_len = 0;
            
            // 读取转义序列的后续字符（最多3个）
            for (int i = 0; i < 3; i++) {
                ssize_t r = read(STDIN_FILENO, &seq[i], 1);
                if (r > 0) {
                    seq_len++;
                    // 如果读到可打印字符（A-Z），序列结束
                    if (seq[i] >= 64 && seq[i] <= 90) {
                        break;
                    }
                } else {
                    // 超时或错误
                    break;
                }
            }
            
            // 恢复原始模式
            set_raw_mode();
            
            // 处理箭头键：ESC [ A (上) 或 ESC [ B (下)
            if (seq_len >= 2 && seq[0] == '[') {
                if (seq[1] == 'A') { // 上箭头
                    if (selected_index > 0) {
                        selected_index--;
                        display_history(hist, selected_index);
                    }
                    continue;
                } else if (seq[1] == 'B') { // 下箭头
                    if (selected_index < hist->count - 1) {
                        selected_index++;
                        display_history(hist, selected_index);
                    }
                    continue;
                }
            }
            
            // 单独的 ESC 键，退出
            if (seq_len == 0) {
                break;
            }
        } else if (c == '\n' || c == '\r' || c == 13 || c == 10) { // Enter 键（多种可能）
            // 恢复终端模式
            restore_termios(old_termios);
            
            // 获取选中的命令
            const char *cmd = hist->lines[selected_index];
            
            // 确保命令不为空
            if (cmd == NULL || strlen(cmd) == 0) {
                printf("\n%s[错误: 命令为空]%s\n", COLOR_RED, COLOR_RESET);
                return -1;
            }
            
            // 彻底清屏，清除所有内容和滚动缓冲区
            printf("\033[2J\033[3J\033[H\033[0m");
            fflush(stdout);
            
            // 执行命令 - 使用 system() 执行
            // 命令的输出会直接显示在终端上
            int status = system(cmd);
            
            // 执行命令后，刷新历史文件以获取最新历史
            if (histfile) {
                refresh_history_file(histfile);
                usleep(50000); // 等待50ms确保文件已写入
            }
            
            // 执行命令后直接退出，不显示额外信息
            // 命令的输出已经通过 system() 显示在终端上了
            // 如果命令执行失败，返回非零值
            if (status == -1) {
                return -1;
            } else if (WIFEXITED(status)) {
                return (WEXITSTATUS(status) == 0) ? selected_index : -1;
            } else {
                return selected_index;
            }
        } else if (c == 'q' || c == 'Q') { // q 键退出
            break;
        } else if (c == 3 || c == 4) { // Ctrl+C 或 Ctrl+D
            break;
        }
    }
    
    restore_termios(old_termios);
    printf("\n");
    return -1;
}

// 解析命令行参数
int parse_arguments(int argc, char *argv[], HistoryOptions *opts) {
    int opt;
    static struct option long_options[] = {
        {"limit", required_argument, 0, 'l'},
        {"help", no_argument, 0, 'h'},
        {"version", no_argument, 0, 'v'},
        {0, 0, 0, 0}
    };
    
    while ((opt = getopt_long(argc, argv, "l:hv", long_options, NULL)) != -1) {
        switch (opt) {
            case 'l':
                opts->limit = atoi(optarg);
                if (opts->limit <= 0) {
                    fprintf(stderr, "错误: limit 必须大于 0\n");
                    return 1;
                }
                if (opts->limit > MAX_HISTORY_LINES) {
                    fprintf(stderr, "警告: limit 超过最大值 %d，将使用最大值\n", MAX_HISTORY_LINES);
                    opts->limit = MAX_HISTORY_LINES;
                }
                break;
            case 'h':
                opts->show_help = 1;
                break;
            case 'v':
                opts->show_version = 1;
                break;
            case '?':
                return 1;
            default:
                break;
        }
    }
    
    return 0;
}

int main(int argc, char *argv[]) {
    HistoryOptions opts;
    History hist;
    
    init_options(&opts);
    
    if (parse_arguments(argc, argv, &opts) != 0) {
        print_help(argv[0]);
        return 1;
    }
    
    if (opts.show_help) {
        print_help(argv[0]);
        return 0;
    }
    
    if (opts.show_version) {
        print_version();
        return 0;
    }
    
    // 初始化历史结构
    init_history(&hist);
    
    // 获取历史文件路径
    char *histfile = get_history_file_path();
    if (!histfile) {
        fprintf(stderr, "%s错误: 无法找到历史文件%s\n", COLOR_RED, COLOR_RESET);
        free_history(&hist);
        return 1;
    }
    
    // 读取历史文件
    if (!read_history_file(&hist, histfile, opts.limit)) {
        fprintf(stderr, "%s警告: 无法读取历史文件: %s%s\n", COLOR_YELLOW, histfile, COLOR_RESET);
        free(histfile);
        free_history(&hist);
        return 1;
    }
    
    // 保存历史文件路径，以便在执行命令后刷新
    // 交互式选择（传入历史文件路径以便刷新）
    int selected = interactive_select(&hist, histfile);
    
    free(histfile);
    
    // 清理
    free_history(&hist);
    
    return (selected >= 0) ? 0 : 1;
}

