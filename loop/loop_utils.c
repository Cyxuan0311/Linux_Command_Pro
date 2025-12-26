#include "loop.h"
#include "../include/common.h"
#include <termios.h>

// 全局变量
volatile int running = 1;
static struct termios old_termios;


// 恢复终端设置
void restore_terminal(void) {
    tcsetattr(STDIN_FILENO, TCSANOW, &old_termios);
}

// 设置终端为原始模式
void set_raw_mode(void) {
    struct termios new_termios;
    tcgetattr(STDIN_FILENO, &old_termios);
    new_termios = old_termios;
    new_termios.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &new_termios);
    atexit(restore_terminal);
}

// 信号处理函数
void signal_handler(int sig) {
    (void)sig;
    running = 0;
}

// 清屏
void clear_screen(void) {
    printf("\033[2J\033[H");
    fflush(stdout);
}

// 获取终端宽度
int get_terminal_width(void) {
    struct winsize w;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == 0) {
        return w.ws_col;
    }
    return 80;  // 默认宽度
}

// 初始化选项
void init_options(LoopOptions *opts) {
    memset(opts, 0, sizeof(LoopOptions));
    strcpy(opts->text, DEFAULT_TEXT);
    opts->anim_type = ANIM_TYPEWRITER;
    opts->color = COLOR_NONE;
    opts->delay = DEFAULT_DELAY;
    opts->interactive = 0;
}

// 解析动画类型
AnimationType parse_animation_type(const char *type_str) {
    if (strcmp(type_str, "typewriter") == 0) return ANIM_TYPEWRITER;
    if (strcmp(type_str, "expand") == 0) return ANIM_EXPAND;
    if (strcmp(type_str, "fade") == 0) return ANIM_FADE;
    if (strcmp(type_str, "wave") == 0) return ANIM_WAVE;
    if (strcmp(type_str, "blink") == 0) return ANIM_BLINK;
    if (strcmp(type_str, "scroll") == 0) return ANIM_SCROLL;
    if (strcmp(type_str, "bounce") == 0) return ANIM_BOUNCE;
    return ANIM_TYPEWRITER;
}

// 解析颜色
ColorEnum parse_color(const char *color_str) {
    if (strcmp(color_str, "red") == 0) return COLOR_RED_ENUM;
    if (strcmp(color_str, "green") == 0) return COLOR_GREEN_ENUM;
    if (strcmp(color_str, "yellow") == 0) return COLOR_YELLOW_ENUM;
    if (strcmp(color_str, "blue") == 0) return COLOR_BLUE_ENUM;
    if (strcmp(color_str, "magenta") == 0) return COLOR_MAGENTA_ENUM;
    if (strcmp(color_str, "cyan") == 0) return COLOR_CYAN_ENUM;
    if (strcmp(color_str, "white") == 0) return COLOR_WHITE_ENUM;
    return COLOR_NONE;
}

// 获取动画类型名称
const char* get_animation_name(AnimationType type) {
    switch (type) {
        case ANIM_TYPEWRITER: return "typewriter";
        case ANIM_EXPAND: return "expand";
        case ANIM_FADE: return "fade";
        case ANIM_WAVE: return "wave";
        case ANIM_BLINK: return "blink";
        case ANIM_SCROLL: return "scroll";
        case ANIM_BOUNCE: return "bounce";
        default: return "unknown";
    }
}

// 获取颜色名称
const char* get_color_name(ColorEnum color) {
    switch (color) {
        case COLOR_RED_ENUM: return "red";
        case COLOR_GREEN_ENUM: return "green";
        case COLOR_YELLOW_ENUM: return "yellow";
        case COLOR_BLUE_ENUM: return "blue";
        case COLOR_MAGENTA_ENUM: return "magenta";
        case COLOR_CYAN_ENUM: return "cyan";
        case COLOR_WHITE_ENUM: return "white";
        default: return "none";
    }
}

