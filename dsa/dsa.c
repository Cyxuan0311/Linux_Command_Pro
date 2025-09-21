/**
 * dsa - 终端图片查看器
 * 在终端中显示JPEG/PNG图片的ASCII艺术版本
 * 
 * 使用方法: ./dsa image.jpg [width]
 * 
 * 作者: Linux Command Pro Team
 * 版本: 1.0.0
 */

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

// Unicode方块字符集，按亮度从暗到亮排列，提供更好的视觉效果
static const char UNICODE_CHARS[] = "█▓▒░";

// 颜色代码
#define RESET   "\033[0m"
#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define BLUE    "\033[34m"
#define MAGENTA "\033[35m"
#define CYAN    "\033[36m"
#define WHITE   "\033[37m"

// 默认宽度
#define DEFAULT_WIDTH 120

// 默认启用颜色
#define DEFAULT_COLOR 1

// 帮助信息
void print_help(const char *program_name) {
    printf("🐧 dsa - 终端图片查看器\n");
    printf("========================\n\n");
    printf("使用方法: %s <图片文件> [宽度]\n\n", program_name);
    printf("参数:\n");
    printf("  图片文件    要显示的图片文件路径 (支持JPG, PNG格式)\n");
    printf("  宽度        可选，ASCII图片的宽度 (默认: %d)\n\n", DEFAULT_WIDTH);
    printf("选项:\n");
    printf("  -h, --help     显示此帮助信息\n");
    printf("  -v, --version  显示版本信息\n");
    printf("  -c, --color    启用颜色显示 (默认)\n");
    printf("  -n, --no-color 禁用颜色显示\n");
    printf("  -w, --width    指定宽度\n\n");
    printf("示例:\n");
    printf("  %s image.jpg\n", program_name);
    printf("  %s image.png 120\n", program_name);
    printf("  %s -c image.jpg\n", program_name);
    printf("  %s -n image.jpg\n", program_name);
    printf("  %s --width 100 image.png\n", program_name);
}

// 版本信息
void print_version() {
    printf("dsa version 1.0.0\n");
    printf("Copyright (c) 2025 Linux Command Pro Team\n");
    printf("MIT License\n");
}

// 将RGB值转换为灰度值
unsigned char rgb_to_gray(unsigned char r, unsigned char g, unsigned char b) {
    return (unsigned char)(0.299 * r + 0.587 * g + 0.114 * b);
}

// 获取Unicode字符
char* get_unicode_char(unsigned char gray_value) {
    static char result[8]; // 支持多字节Unicode字符
    int index = (gray_value * 3) / 255; // 4个字符，索引0-3
    
    // 确保索引在有效范围内
    if (index > 3) index = 3;
    
    // 根据索引返回对应的Unicode字符
    switch(index) {
        case 0: strcpy(result, "░"); break; // 最亮
        case 1: strcpy(result, "▒"); break;
        case 2: strcpy(result, "▓"); break;
        case 3: strcpy(result, "█"); break; // 最暗
        default: strcpy(result, "░"); break;
    }
    return result;
}

// 获取颜色代码 - 改进的颜色映射
const char* get_color_code(unsigned char r, unsigned char g, unsigned char b) {
    // 计算亮度和饱和度
    int brightness = (r + g + b) / 3;
    int max_val = (r > g) ? ((r > b) ? r : b) : ((g > b) ? g : b);
    int min_val = (r < g) ? ((r < b) ? r : b) : ((g < b) ? g : b);
    int saturation = max_val - min_val;
    
    // 如果饱和度很低，使用灰度
    if (saturation < 30) {
        if (brightness > 200) return "\033[97m"; // 很亮白
        else if (brightness > 150) return "\033[37m"; // 亮白
        else if (brightness > 100) return "\033[90m"; // 中灰
        else if (brightness > 50) return "\033[90m"; // 暗灰
        else return "\033[30m"; // 很暗
    }
    
    // 根据主色调确定颜色
    if (r > g && r > b) {
        // 红色系
        if (brightness > 180) return "\033[91m"; // 亮红
        else if (brightness > 120) return "\033[31m"; // 红
        else return "\033[31m"; // 暗红
    } else if (g > r && g > b) {
        // 绿色系
        if (brightness > 180) return "\033[92m"; // 亮绿
        else if (brightness > 120) return "\033[32m"; // 绿
        else return "\033[32m"; // 暗绿
    } else if (b > r && b > g) {
        // 蓝色系
        if (brightness > 180) return "\033[94m"; // 亮蓝
        else if (brightness > 120) return "\033[34m"; // 蓝
        else return "\033[34m"; // 暗蓝
    } else if (r > 150 && g > 150 && b < 100) {
        // 黄色系
        return "\033[93m"; // 亮黄
    } else if (r > 150 && g < 100 && b > 150) {
        // 洋红色系
        return "\033[95m"; // 亮洋红
    } else if (r < 100 && g > 150 && b > 150) {
        // 青色系
        return "\033[96m"; // 亮青
    } else if (r > 120 && g > 120 && b > 120) {
        // 白色系
        if (brightness > 200) return "\033[97m"; // 很亮白
        else return "\033[37m"; // 白
    }
    
    // 默认返回基于亮度的颜色
    if (brightness > 200) return "\033[97m"; // 很亮
    else if (brightness > 150) return "\033[37m"; // 亮
    else if (brightness > 100) return "\033[90m"; // 中
    else if (brightness > 50) return "\033[90m"; // 暗
    else return "\033[30m"; // 很暗
}

// 显示图片
int display_image(const char *filename, int width, int use_color) {
    int x, y, n;
    unsigned char *data = stbi_load(filename, &x, &y, &n, 0);
    
    if (!data) {
        fprintf(stderr, "❌ 错误: 无法加载图片 '%s'\n", filename);
        fprintf(stderr, "   请检查文件是否存在且格式正确 (支持JPG, PNG)\n");
        return 1;
    }
    
    printf("🖼️  图片信息: %dx%d, %d通道\n", x, y, n);
    printf("📏 显示宽度: %d 字符\n", width);
    printf("🎨 颜色模式: %s\n\n", use_color ? "启用" : "禁用");
    
    // 计算缩放比例 - 提高分辨率
    float scale = (float)width / x;
    int new_height = (int)(y * scale * 0.6); // 字符高度约为宽度的0.6倍，提高分辨率
    
    if (new_height <= 0) new_height = 1;
    
    printf("📐 缩放后尺寸: %dx%d\n\n", width, new_height);
    
    // 生成ASCII艺术 - 使用改进的采样算法
    for (int i = 0; i < new_height; i++) {
        for (int j = 0; j < width; j++) {
            // 计算原始图片中的对应位置
            int orig_x = (int)(j / scale);
            int orig_y = (int)(i / scale / 0.6);
            
            if (orig_x >= x) orig_x = x - 1;
            if (orig_y >= y) orig_y = y - 1;
            
            // 使用区域采样提高质量
            int sample_size = 2; // 采样区域大小
            int r_sum = 0, g_sum = 0, b_sum = 0, count = 0;
            
            for (int dy = -sample_size/2; dy <= sample_size/2; dy++) {
                for (int dx = -sample_size/2; dx <= sample_size/2; dx++) {
                    int sample_x = orig_x + dx;
                    int sample_y = orig_y + dy;
                    
                    if (sample_x >= 0 && sample_x < x && sample_y >= 0 && sample_y < y) {
                        int pixel_index = (sample_y * x + sample_x) * n;
                        r_sum += data[pixel_index];
                        g_sum += data[pixel_index + 1];
                        b_sum += data[pixel_index + 2];
                        count++;
                    }
                }
            }
            
            if (count > 0) {
                unsigned char r = r_sum / count;
                unsigned char g = g_sum / count;
                unsigned char b = b_sum / count;
                
                // 转换为灰度
                unsigned char gray = rgb_to_gray(r, g, b);
                
                // 获取Unicode字符
                char* unicode_char = get_unicode_char(gray);
                
                // 输出字符
                if (use_color) {
                    printf("%s%s%s", get_color_code(r, g, b), unicode_char, RESET);
                } else {
                    printf("%s", unicode_char);
                }
            } else {
                printf(" ");
            }
        }
        printf("\n");
    }
    
    printf("\n✨ 图片显示完成!\n");
    
    // 释放内存
    stbi_image_free(data);
    return 0;
}

int main(int argc, char *argv[]) {
    int width = DEFAULT_WIDTH;
    int use_color = DEFAULT_COLOR; // 默认启用颜色
    char *filename = NULL;
    
    // 解析命令行参数
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            print_help(argv[0]);
            return 0;
        } else if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--version") == 0) {
            print_version();
            return 0;
        } else if (strcmp(argv[i], "-c") == 0 || strcmp(argv[i], "--color") == 0) {
            use_color = 1;
        } else if (strcmp(argv[i], "-n") == 0 || strcmp(argv[i], "--no-color") == 0) {
            use_color = 0;
        } else if (strcmp(argv[i], "-w") == 0 || strcmp(argv[i], "--width") == 0) {
            if (i + 1 < argc) {
                width = atoi(argv[++i]);
                if (width <= 0) {
                    fprintf(stderr, "❌ 错误: 宽度必须大于0\n");
                    return 1;
                }
            } else {
                fprintf(stderr, "❌ 错误: --width 需要指定数值\n");
                return 1;
            }
        } else if (argv[i][0] != '-') {
            if (!filename) {
                filename = argv[i];
            } else if (width == DEFAULT_WIDTH) {
                // 如果已经设置了文件名，且宽度还是默认值，则第二个参数是宽度
                width = atoi(argv[i]);
                if (width <= 0) {
                    fprintf(stderr, "❌ 错误: 宽度必须大于0\n");
                    return 1;
                }
            }
        } else {
            fprintf(stderr, "❌ 错误: 未知选项 '%s'\n", argv[i]);
            fprintf(stderr, "使用 '%s --help' 查看帮助信息\n", argv[0]);
            return 1;
        }
    }
    
    // 检查是否指定了图片文件
    if (!filename) {
        fprintf(stderr, "❌ 错误: 请指定图片文件\n");
        fprintf(stderr, "使用 '%s --help' 查看帮助信息\n", argv[0]);
        return 1;
    }
    
    // 检查文件是否存在
    FILE *file = fopen(filename, "r");
    if (!file) {
        fprintf(stderr, "❌ 错误: 文件 '%s' 不存在或无法访问\n", filename);
        return 1;
    }
    fclose(file);
    
    // 显示图片
    return display_image(filename, width, use_color);
}
