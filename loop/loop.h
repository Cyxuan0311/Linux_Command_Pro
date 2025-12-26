#ifndef LOOP_H
#define LOOP_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <termios.h>
#include <sys/ioctl.h>

#define MAX_TEXT_LENGTH 1024
#define DEFAULT_DELAY 100000  // 微秒，默认100ms
#define DEFAULT_TEXT "Hello World"

// 动画类型枚举
typedef enum {
    ANIM_TYPEWRITER,  // 打字效果
    ANIM_EXPAND,      // 展开效果
    ANIM_FADE,        // 淡入效果
    ANIM_WAVE,        // 波浪效果
    ANIM_BLINK,       // 闪烁效果
    ANIM_SCROLL,      // 滚动效果
    ANIM_BOUNCE       // 弹跳效果
} AnimationType;

// 颜色枚举
typedef enum {
    COLOR_NONE = 0,
    COLOR_RED_ENUM,
    COLOR_GREEN_ENUM,
    COLOR_YELLOW_ENUM,
    COLOR_BLUE_ENUM,
    COLOR_MAGENTA_ENUM,
    COLOR_CYAN_ENUM,
    COLOR_WHITE_ENUM
} ColorEnum;

// 选项结构
typedef struct {
    int show_help;
    int show_version;
    char text[MAX_TEXT_LENGTH];
    AnimationType anim_type;
    ColorEnum color;
    int delay;  // 微秒
    int interactive;  // 交互模式
} LoopOptions;

// 全局变量声明
extern volatile int running;

// 工具函数
void restore_terminal(void);
void set_raw_mode(void);
void clear_screen(void);
int get_terminal_width(void);

// 选项管理
void init_options(LoopOptions *opts);
int parse_arguments(int argc, char *argv[], LoopOptions *opts);

// 帮助信息
void print_help(const char *program_name);
void print_version(void);

// 动画类型和颜色解析
AnimationType parse_animation_type(const char *type_str);
ColorEnum parse_color(const char *color_str);
const char* get_animation_name(AnimationType type);
const char* get_color_name(ColorEnum color);

// 动画效果函数
void animate_typewriter(const char *text, ColorEnum color, int delay);
void animate_expand(const char *text, ColorEnum color, int delay);
void animate_fade(const char *text, ColorEnum color, int delay);
void animate_wave(const char *text, ColorEnum color, int delay);
void animate_blink(const char *text, ColorEnum color, int delay);
void animate_scroll(const char *text, ColorEnum color, int delay);
void animate_bounce(const char *text, ColorEnum color, int delay);

// 主动画循环
void run_animation(LoopOptions *opts);

// 信号处理
void signal_handler(int sig);

#endif // LOOP_H

