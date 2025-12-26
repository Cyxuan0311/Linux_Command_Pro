#include "loop.h"
#include "../include/common.h"
#include <stdlib.h>

// 内部辅助函数：获取颜色代码
static const char* get_color_code(ColorEnum color) {
    const char* colors[] = {
        COLOR_RESET,
        COLOR_RED,
        COLOR_GREEN,
        COLOR_YELLOW,
        COLOR_BLUE,
        COLOR_MAGENTA,
        COLOR_CYAN,
        COLOR_WHITE
    };
    if (color >= 0 && color <= COLOR_WHITE_ENUM) {
        return colors[color];
    }
    return COLOR_RESET;
}

// 打字效果
void animate_typewriter(const char *text, ColorEnum color, int delay) {
    const char *color_code = get_color_code(color);
    int len = strlen(text);
    
    for (int i = 0; i < len && running; i++) {
        clear_screen();
        printf("%s", color_code);
        for (int j = 0; j <= i; j++) {
            printf("%c", text[j]);
        }
        printf("%s", COLOR_RESET);
        fflush(stdout);
        usleep(delay);
    }
}

// 展开效果
void animate_expand(const char *text, ColorEnum color, int delay) {
    const char *color_code = get_color_code(color);
    int len = strlen(text);
    int center = len / 2;
    
    for (int i = 0; i <= center && running; i++) {
        clear_screen();
        printf("%s", color_code);
        // 从中心向两边展开
        for (int j = 0; j < len; j++) {
            int dist_from_center = abs(j - center);
            if (dist_from_center <= i) {
                printf("%c", text[j]);
            } else {
                printf(" ");
            }
        }
        printf("%s", COLOR_RESET);
        fflush(stdout);
        usleep(delay);
    }
}

// 淡入效果
void animate_fade(const char *text, ColorEnum color, int delay) {
    const char *color_code = get_color_code(color);
    int len = strlen(text);
    
    for (int i = 0; i < len && running; i++) {
        clear_screen();
        for (int j = 0; j < len; j++) {
            if (j <= i) {
                printf("%s%c%s", color_code, text[j], COLOR_RESET);
            } else {
                printf(" ");
            }
        }
        fflush(stdout);
        usleep(delay);
    }
}

// 波浪效果
void animate_wave(const char *text, ColorEnum color, int delay) {
    const char *color_code = get_color_code(color);
    int len = strlen(text);
    
    for (int offset = 0; offset < len * 2 && running; offset++) {
        clear_screen();
        printf("%s", color_code);
        for (int i = 0; i < len; i++) {
            int wave_pos = (i + offset) % (len * 2);
            if (wave_pos < len) {
                printf("%c", text[i]);
            } else {
                printf(" ");
            }
        }
        printf("%s", COLOR_RESET);
        fflush(stdout);
        usleep(delay);
    }
}

// 闪烁效果
void animate_blink(const char *text, ColorEnum color, int delay) {
    const char *color_code = get_color_code(color);
    
    while (running) {
        clear_screen();
        printf("%s%s%s", color_code, text, COLOR_RESET);
        fflush(stdout);
        usleep(delay);
        
        clear_screen();
        printf("%s", text);
        fflush(stdout);
        usleep(delay);
    }
}

// 滚动效果
void animate_scroll(const char *text, ColorEnum color, int delay) {
    const char *color_code = get_color_code(color);
    int len = strlen(text);
    int width = get_terminal_width();
    
    for (int offset = 0; offset < len + width && running; offset++) {
        clear_screen();
        printf("%s", color_code);
        for (int i = 0; i < width; i++) {
            int text_pos = i - offset + width;
            if (text_pos >= 0 && text_pos < len) {
                printf("%c", text[text_pos]);
            } else {
                printf(" ");
            }
        }
        printf("%s", COLOR_RESET);
        fflush(stdout);
        usleep(delay);
    }
}

// 弹跳效果
void animate_bounce(const char *text, ColorEnum color, int delay) {
    const char *color_code = get_color_code(color);
    int len = strlen(text);
    int max_height = 5;
    
    for (int cycle = 0; running; cycle++) {
        for (int frame = 0; frame < max_height * 2 && running; frame++) {
            clear_screen();
            int height = max_height - abs(frame - max_height);
            
            // 打印空行
            for (int h = 0; h < max_height - height; h++) {
                printf("\n");
            }
            
            printf("%s", color_code);
            for (int i = 0; i < len; i++) {
                int char_height = height - (i % 3);
                if (char_height < 0) char_height = 0;
                
                if (char_height > 0) {
                    printf("%c", text[i]);
                } else {
                    printf(" ");
                }
            }
            printf("%s", COLOR_RESET);
            fflush(stdout);
            usleep(delay);
        }
    }
}

