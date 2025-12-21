#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
#include <getopt.h>
#include "../include/common.h"
#include "grapher_advanced.h"

#define MAX_POINTS 1000
#define MAX_WIDTH 80
#define MAX_HEIGHT 30
#define DEFAULT_WIDTH 60
#define DEFAULT_HEIGHT 20

// 数据点结构
typedef struct {
    double x;
    double y;
} DataPoint;

// 图表类型
typedef enum {
    CHART_BAR,         // 柱状图
    CHART_SCATTER,     // 散点图
    CHART_LINE,        // 曲线图
    CHART_PIE,         // 饼图
    CHART_AREA,        // 面积图
    CHART_CANDLESTICK, // K线图（金融）
    CHART_GANTT,       // 甘特图（项目管理）
    CHART_FUNNEL,      // 漏斗图（销售/营销）
    CHART_BOXPLOT,     // 箱线图（统计分析）
    CHART_RADAR        // 雷达图（多维度分析）
} ChartType;

// 图表选项
typedef struct {
    ChartType type;
    int width;
    int height;
    int show_grid;
    int show_labels;
    int show_legend;
    int color;
    char title[256];
    char xlabel[128];
    char ylabel[128];
} ChartOptions;

// 全局数据（允许高级图表文件访问）
DataPoint points[MAX_POINTS];
int point_count = 0;

// 初始化选项
void init_options(ChartOptions *opts) {
    memset(opts, 0, sizeof(ChartOptions));
    opts->type = CHART_LINE;
    opts->width = DEFAULT_WIDTH;
    opts->height = DEFAULT_HEIGHT;
    opts->show_grid = 1;
    opts->show_labels = 1;
    opts->show_legend = 1;
    opts->color = 1;
    strcpy(opts->title, "图表");
    strcpy(opts->xlabel, "X轴");
    strcpy(opts->ylabel, "Y轴");
}

// 显示帮助信息
void print_help(const char *program_name) {
    printf("用法: %s [选项] [数据点...]\n", program_name);
    printf("终端图表绘制工具，支持柱状图、散点图和曲线图\n\n");
    printf("选项:\n");
    printf("  -t, --type=TYPE        图表类型: bar(柱状图), scatter(散点图), line(曲线图) (默认: line)\n");
    printf("  -w, --width=N          图表宽度 (默认: 60)\n");
    printf("  -h, --height=N         图表高度 (默认: 20)\n");
    printf("  -T, --title=TITLE      图表标题\n");
    printf("  -x, --xlabel=LABEL     X轴标签\n");
    printf("  -y, --ylabel=LABEL     Y轴标签\n");
    printf("  -g, --no-grid          不显示网格\n");
    printf("  -l, --no-labels        不显示标签\n");
    printf("  -L, --no-legend        不显示图例\n");
    printf("  -c, --no-color         不使用颜色\n");
    printf("  -i, --interactive      交互模式\n");
    printf("  -f, --file=FILE        从文件读取数据 (格式: x,y 每行一个点)\n");
    printf("  --help                 显示此帮助信息\n");
    printf("  --version              显示版本信息\n\n");
    printf("数据输入格式:\n");
    printf("  命令行: x1,y1 x2,y2 x3,y3 ...\n");
    printf("  文件:   每行一个点，格式为 x,y\n");
    printf("  交互:   输入 'x,y' 格式的数据点，输入 'done' 结束\n\n");
    printf("示例:\n");
    printf("  %s -t bar 1,5 2,8 3,6 4,9 5,7\n", program_name);
    printf("  %s -t scatter -T \"销售数据\" 1,10 2,15 3,12 4,18\n", program_name);
    printf("  %s -t line -f data.txt\n", program_name);
    printf("  %s -i\n", program_name);
}

// 显示版本信息
void print_version() {
    printf("grapher - 终端图表绘制工具 v1.0\n");
    printf("支持多种图表类型：柱状图、散点图、曲线图、饼图、面积图、K线图、甘特图、漏斗图、箱线图、雷达图\n");
}

// 解析数据点
int parse_point(const char *str, DataPoint *point) {
    char *copy = strdup(str);
    char *token = strtok(copy, ",");
    
    if (token == NULL) {
        free(copy);
        return 0;
    }
    
    point->x = atof(token);
    token = strtok(NULL, ",");
    
    if (token == NULL) {
        free(copy);
        return 0;
    }
    
    point->y = atof(token);
    free(copy);
    return 1;
}

// 从文件读取数据
int load_from_file(const char *filename) {
    FILE *fp = fopen(filename, "r");
    if (fp == NULL) {
        print_error("无法打开文件");
        return 0;
    }
    
    char line[256];
    point_count = 0;
    
    while (fgets(line, sizeof(line), fp) && point_count < MAX_POINTS) {
        // 移除换行符
        line[strcspn(line, "\n")] = 0;
        
        // 跳过空行和注释
        if (line[0] == '\0' || line[0] == '#') {
            continue;
        }
        
        if (parse_point(line, &points[point_count])) {
            point_count++;
        }
    }
    
    fclose(fp);
    return point_count > 0;
}

// 交互式配置图表选项
void interactive_config(ChartOptions *opts) {
    char input[256];
    
    printf("%s═══════════════════════════════════════════════════════%s\n", COLOR_CYAN, COLOR_RESET);
    printf("%s           交互式图表配置模式%s\n", COLOR_CYAN, COLOR_RESET);
    printf("%s═══════════════════════════════════════════════════════%s\n\n", COLOR_CYAN, COLOR_RESET);
    
    // 设置图表类型
    printf("%s1. 图表类型设置%s\n", COLOR_YELLOW, COLOR_RESET);
    const char* type_names[] = {
        "柱状图 (bar)", "散点图 (scatter)", "曲线图 (line)", 
        "饼图 (pie)", "面积图 (area)", "K线图 (candlestick)",
        "甘特图 (gantt)", "漏斗图 (funnel)", "箱线图 (boxplot)",
        "雷达图 (radar)"
    };
    printf("  当前类型: %s%s%s\n", COLOR_GREEN, 
           opts->type < 10 ? type_names[opts->type] : "未知", COLOR_RESET);
    printf("  请输入图表类型 [bar/scatter/line/pie/area/candlestick/gantt/funnel/boxplot/radar] (直接回车保持当前): ");
    
    if (fgets(input, sizeof(input), stdin) != NULL) {
        input[strcspn(input, "\n")] = 0;
        if (strlen(input) > 0) {
            if (strcmp(input, "bar") == 0) {
                opts->type = CHART_BAR;
                printf("  %s✓%s 已设置为柱状图\n", COLOR_GREEN, COLOR_RESET);
            } else if (strcmp(input, "scatter") == 0) {
                opts->type = CHART_SCATTER;
                printf("  %s✓%s 已设置为散点图\n", COLOR_GREEN, COLOR_RESET);
            } else if (strcmp(input, "line") == 0) {
                opts->type = CHART_LINE;
                printf("  %s✓%s 已设置为曲线图\n", COLOR_GREEN, COLOR_RESET);
            } else if (strcmp(input, "pie") == 0) {
                opts->type = CHART_PIE;
                printf("  %s✓%s 已设置为饼图\n", COLOR_GREEN, COLOR_RESET);
            } else if (strcmp(input, "area") == 0) {
                opts->type = CHART_AREA;
                printf("  %s✓%s 已设置为面积图\n", COLOR_GREEN, COLOR_RESET);
            } else if (strcmp(input, "candlestick") == 0) {
                opts->type = CHART_CANDLESTICK;
                printf("  %s✓%s 已设置为K线图\n", COLOR_GREEN, COLOR_RESET);
            } else if (strcmp(input, "gantt") == 0) {
                opts->type = CHART_GANTT;
                printf("  %s✓%s 已设置为甘特图\n", COLOR_GREEN, COLOR_RESET);
            } else if (strcmp(input, "funnel") == 0) {
                opts->type = CHART_FUNNEL;
                printf("  %s✓%s 已设置为漏斗图\n", COLOR_GREEN, COLOR_RESET);
            } else if (strcmp(input, "boxplot") == 0) {
                opts->type = CHART_BOXPLOT;
                printf("  %s✓%s 已设置为箱线图\n", COLOR_GREEN, COLOR_RESET);
            } else if (strcmp(input, "radar") == 0) {
                opts->type = CHART_RADAR;
                printf("  %s✓%s 已设置为雷达图\n", COLOR_GREEN, COLOR_RESET);
            } else {
                printf("  %s✗%s 无效类型，保持当前设置\n", COLOR_RED, COLOR_RESET);
            }
        }
    }
    printf("\n");
    
    // 设置图表标题
    printf("%s2. 图表标题设置%s\n", COLOR_YELLOW, COLOR_RESET);
    printf("  当前标题: %s%s%s\n", COLOR_GREEN, 
           strlen(opts->title) > 0 ? opts->title : "(无)", COLOR_RESET);
    printf("  请输入图表标题 (直接回车保持当前): ");
    
    if (fgets(input, sizeof(input), stdin) != NULL) {
        input[strcspn(input, "\n")] = 0;
        if (strlen(input) > 0) {
            strncpy(opts->title, input, sizeof(opts->title) - 1);
            opts->title[sizeof(opts->title) - 1] = '\0';
            printf("  %s✓%s 标题已设置为: %s\n", COLOR_GREEN, COLOR_RESET, opts->title);
        }
    }
    printf("\n");
    
    // 设置X轴标签
    printf("%s3. X轴标签设置%s\n", COLOR_YELLOW, COLOR_RESET);
    printf("  当前标签: %s%s%s\n", COLOR_GREEN, 
           strlen(opts->xlabel) > 0 ? opts->xlabel : "(无)", COLOR_RESET);
    printf("  请输入X轴标签 (直接回车保持当前): ");
    
    if (fgets(input, sizeof(input), stdin) != NULL) {
        input[strcspn(input, "\n")] = 0;
        if (strlen(input) > 0) {
            strncpy(opts->xlabel, input, sizeof(opts->xlabel) - 1);
            opts->xlabel[sizeof(opts->xlabel) - 1] = '\0';
            printf("  %s✓%s X轴标签已设置为: %s\n", COLOR_GREEN, COLOR_RESET, opts->xlabel);
        }
    }
    printf("\n");
    
    // 设置Y轴标签
    printf("%s4. Y轴标签设置%s\n", COLOR_YELLOW, COLOR_RESET);
    printf("  当前标签: %s%s%s\n", COLOR_GREEN, 
           strlen(opts->ylabel) > 0 ? opts->ylabel : "(无)", COLOR_RESET);
    printf("  请输入Y轴标签 (直接回车保持当前): ");
    
    if (fgets(input, sizeof(input), stdin) != NULL) {
        input[strcspn(input, "\n")] = 0;
        if (strlen(input) > 0) {
            strncpy(opts->ylabel, input, sizeof(opts->ylabel) - 1);
            opts->ylabel[sizeof(opts->ylabel) - 1] = '\0';
            printf("  %s✓%s Y轴标签已设置为: %s\n", COLOR_GREEN, COLOR_RESET, opts->ylabel);
        }
    }
    printf("\n");
    
    // 设置图表尺寸
    printf("%s5. 图表尺寸设置%s\n", COLOR_YELLOW, COLOR_RESET);
    printf("  当前尺寸: 宽度=%d, 高度=%d\n", opts->width, opts->height);
    printf("  请输入宽度 (10-%d, 直接回车保持当前): ", MAX_WIDTH);
    
    if (fgets(input, sizeof(input), stdin) != NULL) {
        input[strcspn(input, "\n")] = 0;
        if (strlen(input) > 0) {
            int w = atoi(input);
            if (w >= 10 && w <= MAX_WIDTH) {
                opts->width = w;
                printf("  %s✓%s 宽度已设置为: %d\n", COLOR_GREEN, COLOR_RESET, opts->width);
            } else {
                printf("  %s✗%s 无效宽度，保持当前设置\n", COLOR_RED, COLOR_RESET);
            }
        }
    }
    
    printf("  请输入高度 (5-%d, 直接回车保持当前): ", MAX_HEIGHT);
    if (fgets(input, sizeof(input), stdin) != NULL) {
        input[strcspn(input, "\n")] = 0;
        if (strlen(input) > 0) {
            int h = atoi(input);
            if (h >= 5 && h <= MAX_HEIGHT) {
                opts->height = h;
                printf("  %s✓%s 高度已设置为: %d\n", COLOR_GREEN, COLOR_RESET, opts->height);
            } else {
                printf("  %s✗%s 无效高度，保持当前设置\n", COLOR_RED, COLOR_RESET);
            }
        }
    }
    printf("\n");
    
    printf("%s配置完成！开始输入数据点...%s\n\n", COLOR_GREEN, COLOR_RESET);
}

// 交互式输入数据
int interactive_input(ChartOptions *opts) {
    char input[256];
    point_count = 0;
    
    // 首先进行配置
    interactive_config(opts);
    
    printf("%s═══════════════════════════════════════════════════════%s\n", COLOR_CYAN, COLOR_RESET);
    printf("%s           数据点输入模式%s\n", COLOR_CYAN, COLOR_RESET);
    printf("%s═══════════════════════════════════════════════════════%s\n\n", COLOR_CYAN, COLOR_RESET);
    
    printf("%s输入数据点 (格式: x,y)，输入 'done' 结束，输入 'help' 查看帮助%s\n", 
           COLOR_YELLOW, COLOR_RESET);
    printf("%s支持的命令: help, clear, config, done, quit%s\n\n", DIM, COLOR_RESET);
    
    while (point_count < MAX_POINTS) {
        printf("%s[%d] > %s", COLOR_GREEN, point_count + 1, COLOR_RESET);
        
        if (fgets(input, sizeof(input), stdin) == NULL) {
            break;
        }
        
        // 移除换行符
        input[strcspn(input, "\n")] = 0;
        
        // 跳过空行
        if (strlen(input) == 0) {
            continue;
        }
        
        if (strcmp(input, "done") == 0 || strcmp(input, "q") == 0 || 
            strcmp(input, "quit") == 0) {
            break;
        }
        
        if (strcmp(input, "help") == 0 || strcmp(input, "h") == 0) {
            printf("\n%s帮助信息:%s\n", COLOR_CYAN, COLOR_RESET);
            printf("  数据格式: x,y (例如: 1,5 或 2.5,10.3)\n");
            printf("  命令:\n");
            printf("    help/h      - 显示此帮助信息\n");
            printf("    clear/c     - 清空所有数据点\n");
            printf("    config      - 重新配置图表选项\n");
            printf("    done/q/quit - 完成输入并显示图表\n");
            printf("  示例:\n");
            printf("    [1] > 1,5\n");
            printf("    [2] > 2,8\n");
            printf("    [3] > 3,6\n");
            printf("    [4] > done\n\n");
            continue;
        }
        
        if (strcmp(input, "clear") == 0 || strcmp(input, "c") == 0) {
            point_count = 0;
            printf("  %s✓%s 已清空所有数据点\n", COLOR_YELLOW, COLOR_RESET);
            continue;
        }
        
        if (strcmp(input, "config") == 0) {
            printf("\n");
            interactive_config(opts);
            printf("%s继续输入数据点...%s\n\n", COLOR_YELLOW, COLOR_RESET);
            continue;
        }
        
        if (parse_point(input, &points[point_count])) {
            printf("  %s✓%s 点 (%g, %g) 已添加\n", 
                   COLOR_GREEN, COLOR_RESET, 
                   points[point_count].x, points[point_count].y);
            point_count++;
        } else {
            printf("  %s✗%s 无效格式，请使用 x,y 格式 (例如: 1,5)\n", COLOR_RED, COLOR_RESET);
            printf("  %s提示:%s 输入 'help' 查看帮助\n", DIM, COLOR_RESET);
        }
    }
    
    return point_count;
}

// 计算数据范围
void calculate_range(double *min_x, double *max_x, double *min_y, double *max_y) {
    if (point_count == 0) {
        *min_x = *max_x = *min_y = *max_y = 0;
        return;
    }
    
    *min_x = *max_x = points[0].x;
    *min_y = *max_y = points[0].y;
    
    for (int i = 1; i < point_count; i++) {
        if (points[i].x < *min_x) *min_x = points[i].x;
        if (points[i].x > *max_x) *max_x = points[i].x;
        if (points[i].y < *min_y) *min_y = points[i].y;
        if (points[i].y > *max_y) *max_y = points[i].y;
    }
    
    // 添加一些边距
    double x_range = *max_x - *min_x;
    double y_range = *max_y - *min_y;
    
    if (x_range == 0) x_range = 1;
    if (y_range == 0) y_range = 1;
    
    *min_x -= x_range * 0.05;
    *max_x += x_range * 0.05;
    *min_y -= y_range * 0.05;
    *max_y += y_range * 0.05;
}

// 将坐标转换为屏幕坐标
void world_to_screen(double x, double y, double min_x, double max_x, 
                     double min_y, double max_y, int width, int height,
                     int *sx, int *sy) {
    double x_range = max_x - min_x;
    double y_range = max_y - min_y;
    
    if (x_range == 0) x_range = 1;
    if (y_range == 0) y_range = 1;
    
    *sx = (int)((x - min_x) / x_range * (width - 1));
    *sy = (int)((max_y - y) / y_range * (height - 1));
    
    if (*sx < 0) *sx = 0;
    if (*sx >= width) *sx = width - 1;
    if (*sy < 0) *sy = 0;
    if (*sy >= height) *sy = height - 1;
}

// 绘制柱状图
void draw_bar_chart(ChartOptions *opts, double min_x, double max_x, 
                    double min_y, double max_y) {
    int width = opts->width;
    int height = opts->height;
    char canvas[height][width + 1];
    
    // 初始化画布
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            canvas[i][j] = ' ';
        }
        canvas[i][width] = '\0';
    }
    
    // 绘制网格和坐标轴（参考YouPlot风格，使用标记字符）
    if (opts->show_grid) {
        // 绘制水平网格线（使用标记字符，打印时替换为Unicode）
        int grid_lines = 5;
        for (int g = 0; g <= grid_lines; g++) {
            int y_pos = (height - 1) * g / grid_lines;
            for (int j = 1; j < width - 1; j++) {
                if (canvas[y_pos][j] == ' ') {
                    canvas[y_pos][j] = 'h';  // 标记：水平线
                }
            }
        }
        
        // Y轴（左侧，使用标记字符）
        for (int i = 0; i < height - 1; i++) {
            if (canvas[i][0] == 'h') {
                canvas[i][0] = 'c';  // 标记：交叉点
            } else if (canvas[i][0] == ' ') {
                canvas[i][0] = 'v';  // 标记：垂直线
            }
        }
        // Y轴箭头（顶部）
        canvas[0][0] = 'u';  // 标记：向上箭头
        
        // X轴（底部）
        for (int j = 0; j < width - 1; j++) {
            if (canvas[height - 1][j] == ' ') {
                canvas[height - 1][j] = 'h';  // 标记：水平线
            }
        }
        // X轴箭头（右侧）
        canvas[height - 1][width - 1] = 'r';  // 标记：向右箭头
        
        // 原点
        canvas[height - 1][0] = 'o';  // 标记：原点
        
        // 绘制垂直网格线
        for (int i = 0; i < height - 1; i++) {
            for (int g = 0; g <= grid_lines; g++) {
                int x_pos = (width - 1) * g / grid_lines;
                if (canvas[i][x_pos] == ' ') {
                    canvas[i][x_pos] = 'v';  // 标记：垂直线
                } else if (canvas[i][x_pos] == 'h') {
                    canvas[i][x_pos] = 'x';  // 标记：网格交叉点
                }
            }
        }
    }
    
    // 绘制柱状（使用Unicode块状字符）
    double x_range = max_x - min_x;
    double y_range = max_y - min_y;
    if (x_range == 0) x_range = 1;
    if (y_range == 0) y_range = 1;
    
    // 为坐标轴预留空间
    int plot_width = width - 2;  // 减去Y轴和X轴箭头
    int plot_height = height - 1;  // 减去X轴
    
    // 计算柱状宽度和间距
    int bar_width = plot_width / (point_count * 2);  // 柱状宽度约为间距的一半
    if (bar_width < 2) bar_width = 2;
    if (bar_width > 6) bar_width = 6;
    
    // 统一使用▓字符标记柱状（在打印时替换为Unicode字符）
    char bar_marker = '#';  // 使用#作为标记，打印时替换为▓
    
    for (int i = 0; i < point_count; i++) {
        // 计算柱状高度（从min_y到points[i].y）
        int bar_height = (int)((points[i].y - min_y) / y_range * plot_height);
        if (bar_height < 1) bar_height = 1;
        
        // 计算柱状X位置（基于数据点的X坐标）
        int x_pos = 1 + (int)((points[i].x - min_x) / x_range * plot_width);
        // 将柱状居中在X坐标位置
        x_pos = x_pos - bar_width / 2;
        if (x_pos < 1) x_pos = 1;
        if (x_pos + bar_width >= width - 1) x_pos = width - 1 - bar_width;
        
        // 从X轴底部向上绘制柱状
        for (int h = 0; h < bar_height && h < plot_height; h++) {
            for (int w = 0; w < bar_width && x_pos + w < width - 1; w++) {
                int y_pos = height - 1 - h - 1;  // 从X轴上方开始（height-1是X轴）
                if (y_pos >= 0 && y_pos < height - 1 && x_pos + w > 0) {
                    // 不覆盖网格线和坐标轴
                    if (canvas[y_pos][x_pos + w] == ' ' || 
                        canvas[y_pos][x_pos + w] == 'h') {
                        canvas[y_pos][x_pos + w] = bar_marker;
                    }
                }
            }
        }
    }
    
    // 打印画布和Y轴刻度值
    char y_labels[height][16];
    for (int i = 0; i < height; i++) {
        y_labels[i][0] = '\0';
    }
    
    // 计算Y轴刻度值
    if (opts->show_labels) {
        int grid_lines = 5;
        for (int g = 0; g <= grid_lines; g++) {
            int y_pos = (height - 1) * g / grid_lines;
            double y_value = max_y - (max_y - min_y) * g / grid_lines;
            snprintf(y_labels[y_pos], sizeof(y_labels[y_pos]), "%.1f", y_value);
        }
    }
    
    // 打印画布（左侧显示Y轴刻度值）
    int max_label_len = 0;
    for (int i = 0; i < height; i++) {
        int len = strlen(y_labels[i]);
        if (len > max_label_len) max_label_len = len;
    }
    
    for (int i = 0; i < height; i++) {
        // 打印Y轴刻度值
        if (opts->show_labels && strlen(y_labels[i]) > 0) {
            printf("%s%*s%s ", COLOR_YELLOW, max_label_len, y_labels[i], COLOR_RESET);
        } else {
            printf("%*s ", max_label_len, "");
        }
        // 打印画布行，将标记字符替换为Unicode块状字符，并使用不同颜色
        for (int j = 0; j < width; j++) {
            char ch = canvas[i][j];
            // 将标记字符替换为Unicode字符，并使用不同颜色
            if (ch == '#') {
                // 柱状图使用绿色，与坐标轴颜色区分
                printf("%s█%s", opts->color ? COLOR_GREEN : "", opts->color ? COLOR_RESET : "");
            } else if (ch == 'h') {
                // 水平线
                printf("%s─%s", opts->color ? COLOR_CYAN : "", opts->color ? COLOR_RESET : "");
            } else if (ch == 'v') {
                // 垂直线
                printf("%s│%s", opts->color ? COLOR_CYAN : "", opts->color ? COLOR_RESET : "");
            } else if (ch == 'x') {
                // 网格交叉点
                printf("%s┼%s", opts->color ? COLOR_CYAN : "", opts->color ? COLOR_RESET : "");
            } else if (ch == 'c') {
                // 左连接
                printf("%s├%s", opts->color ? COLOR_CYAN : "", opts->color ? COLOR_RESET : "");
            } else if (ch == 'o') {
                // 原点
                printf("%s└%s", opts->color ? COLOR_CYAN : "", opts->color ? COLOR_RESET : "");
            } else if (ch == 'u') {
                // 向上箭头
                printf("%s↑%s", opts->color ? COLOR_CYAN : "", opts->color ? COLOR_RESET : "");
            } else if (ch == 'r') {
                // 向右箭头
                printf("%s→%s", opts->color ? COLOR_CYAN : "", opts->color ? COLOR_RESET : "");
            } else {
                printf("%c", ch);
            }
        }
        printf("\n");
    }
    
    // 打印X轴刻度值（显示数据点的X坐标）
    if (opts->show_labels) {
        printf("%*s", max_label_len + 1, "");
        int last_pos = 0;
        for (int i = 0; i < point_count; i++) {
            // 计算数据点的X位置
            int x_pos = 1 + (int)((points[i].x - min_x) / x_range * plot_width);
            double x_value = points[i].x;
            char x_label[16];
            snprintf(x_label, sizeof(x_label), "%.1f", x_value);
            int label_len = strlen(x_label);
            
            // 计算位置，使刻度值对齐到柱状图中心
            if (i == 0) {
                // 第一个标签，对齐到柱状图位置
                int spaces = x_pos - label_len / 2;
                if (spaces > 0) {
                    printf("%*s", spaces, "");
                }
                printf("%s", x_label);
                last_pos = x_pos + label_len / 2;
            } else {
                int spaces = x_pos - last_pos - label_len / 2;
                if (spaces > 0) {
                    printf("%*s", spaces, "");
                }
                printf("%s", x_label);
                last_pos = x_pos + label_len / 2;
            }
        }
        printf("\n");
    }
}

// 绘制散点图
void draw_scatter_chart(ChartOptions *opts, double min_x, double max_x,
                        double min_y, double max_y) {
    int width = opts->width;
    int height = opts->height;
    char canvas[height][width + 1];
    
    // 初始化画布
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            canvas[i][j] = ' ';
        }
        canvas[i][width] = '\0';
    }
    
    // 绘制网格和坐标轴（参考YouPlot风格，使用标记字符）
    if (opts->show_grid) {
        // 绘制水平网格线
        int grid_lines = 5;
        for (int g = 0; g <= grid_lines; g++) {
            int y_pos = (height - 1) * g / grid_lines;
            for (int j = 1; j < width - 1; j++) {
                if (canvas[y_pos][j] == ' ') {
                    canvas[y_pos][j] = 'h';  // 标记：水平线
                }
            }
        }
        
        // Y轴（左侧）
        for (int i = 0; i < height - 1; i++) {
            if (canvas[i][0] == 'h') {
                canvas[i][0] = 'c';  // 标记：交叉点
            } else if (canvas[i][0] == ' ') {
                canvas[i][0] = 'v';  // 标记：垂直线
            }
        }
        // Y轴箭头（顶部）
        canvas[0][0] = 'u';  // 标记：向上箭头
        
        // X轴（底部）
        for (int j = 0; j < width - 1; j++) {
            if (canvas[height - 1][j] == ' ') {
                canvas[height - 1][j] = 'h';  // 标记：水平线
            }
        }
        // X轴箭头（右侧）
        canvas[height - 1][width - 1] = 'r';  // 标记：向右箭头
        
        // 原点
        canvas[height - 1][0] = 'o';  // 标记：原点
        
        // 绘制垂直网格线
        for (int i = 0; i < height - 1; i++) {
            for (int g = 0; g <= grid_lines; g++) {
                int x_pos = (width - 1) * g / grid_lines;
                if (canvas[i][x_pos] == ' ') {
                    canvas[i][x_pos] = 'v';  // 标记：垂直线
                } else if (canvas[i][x_pos] == 'h') {
                    canvas[i][x_pos] = 'x';  // 标记：网格交叉点
                }
            }
        }
    }
    
    // 绘制点（使用Unicode字符使散点更明显）
    // 使用统一的符号，但可以通过颜色区分
    for (int i = 0; i < point_count; i++) {
        int sx, sy;
        // 调整坐标范围，为坐标轴预留空间
        world_to_screen(points[i].x, points[i].y, min_x, max_x, 
                       min_y, max_y, width - 1, height - 1, &sx, &sy);
        
        if (sx > 0 && sy >= 0 && sx < width - 1 && sy < height - 1) {
            // 使用标记字符，打印时替换为Unicode字符
            char point_marker = '*';  // 使用*作为标记，打印时替换为●
            // 不覆盖网格线交叉点
            if (canvas[sy][sx] != 'x' && canvas[sy][sx] != 'v' && 
                canvas[sy][sx] != 'h' && canvas[sy][sx] != 'c' && 
                canvas[sy][sx] != 'o' && canvas[sy][sx] != 'u' && 
                canvas[sy][sx] != 'r') {
                canvas[sy][sx] = point_marker;
            } else {
                // 如果点在网格线上，使用更明显的符号
                canvas[sy][sx] = point_marker;
            }
        }
    }
    
    // 打印画布和Y轴刻度值
    char y_labels[height][16];
    for (int i = 0; i < height; i++) {
        y_labels[i][0] = '\0';
    }
    
    // 计算Y轴刻度值
    if (opts->show_labels) {
        int grid_lines = 5;
        for (int g = 0; g <= grid_lines; g++) {
            int y_pos = (height - 1) * g / grid_lines;
            double y_value = max_y - (max_y - min_y) * g / grid_lines;
            snprintf(y_labels[y_pos], sizeof(y_labels[y_pos]), "%.1f", y_value);
        }
    }
    
    // 打印画布（左侧显示Y轴刻度值）
    int max_label_len = 0;
    for (int i = 0; i < height; i++) {
        int len = strlen(y_labels[i]);
        if (len > max_label_len) max_label_len = len;
    }
    
    for (int i = 0; i < height; i++) {
        // 打印Y轴刻度值
        if (opts->show_labels && strlen(y_labels[i]) > 0) {
            printf("%s%*s%s ", COLOR_YELLOW, max_label_len, y_labels[i], COLOR_RESET);
        } else {
            printf("%*s ", max_label_len, "");
        }
        // 打印画布行，将标记字符替换为Unicode字符，并使用不同颜色
        for (int j = 0; j < width; j++) {
            char ch = canvas[i][j];
            // 将标记字符替换为Unicode字符，并使用不同颜色
            if (ch == '*') {
                // 散点使用黄色，与坐标轴颜色区分
                printf("%s●%s", opts->color ? COLOR_YELLOW : "", opts->color ? COLOR_RESET : "");
            } else if (ch == 'h') {
                // 水平线
                printf("%s─%s", opts->color ? COLOR_GREEN : "", opts->color ? COLOR_RESET : "");
            } else if (ch == 'v') {
                // 垂直线
                printf("%s│%s", opts->color ? COLOR_GREEN : "", opts->color ? COLOR_RESET : "");
            } else if (ch == 'x') {
                // 网格交叉点
                printf("%s┼%s", opts->color ? COLOR_GREEN : "", opts->color ? COLOR_RESET : "");
            } else if (ch == 'c') {
                // 左连接
                printf("%s├%s", opts->color ? COLOR_GREEN : "", opts->color ? COLOR_RESET : "");
            } else if (ch == 'o') {
                // 原点
                printf("%s└%s", opts->color ? COLOR_GREEN : "", opts->color ? COLOR_RESET : "");
            } else if (ch == 'u') {
                // 向上箭头
                printf("%s↑%s", opts->color ? COLOR_GREEN : "", opts->color ? COLOR_RESET : "");
            } else if (ch == 'r') {
                // 向右箭头
                printf("%s→%s", opts->color ? COLOR_GREEN : "", opts->color ? COLOR_RESET : "");
            } else {
                printf("%c", ch);
            }
        }
        printf("\n");
    }
    
    // 打印X轴刻度值（显示网格线的X坐标）
    if (opts->show_labels) {
        printf("%*s", max_label_len + 1, "");
        int grid_lines = 5;
        int last_pos = 0;
        for (int g = 0; g <= grid_lines; g++) {
            int x_pos = (width - 1) * g / grid_lines;
            double x_value = min_x + (max_x - min_x) * g / grid_lines;
            char x_label[16];
            snprintf(x_label, sizeof(x_label), "%.1f", x_value);
            int label_len = strlen(x_label);
            
            // 计算位置，使刻度值对齐到网格线
            if (g == 0) {
                printf("%s", x_label);
                last_pos = label_len;
            } else {
                int spaces = x_pos - last_pos;
                if (spaces > label_len) {
                    spaces = spaces - label_len / 2;
                    if (spaces > 0) {
                        printf("%*s", spaces, "");
                    }
                    printf("%s", x_label);
                    last_pos = x_pos + label_len / 2;
                } else {
                    printf(" %s", x_label);
                    last_pos = x_pos + 1 + label_len;
                }
            }
        }
        printf("\n");
    }
}

// 绘制曲线图
void draw_line_chart(ChartOptions *opts, double min_x, double max_x,
                     double min_y, double max_y) {
    int width = opts->width;
    int height = opts->height;
    char canvas[height][width + 1];
    
    // 初始化画布
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            canvas[i][j] = ' ';
        }
        canvas[i][width] = '\0';
    }
    
    // 绘制网格和坐标轴（参考YouPlot风格，使用标记字符）
    if (opts->show_grid) {
        // 绘制水平网格线
        int grid_lines = 5;
        for (int g = 0; g <= grid_lines; g++) {
            int y_pos = (height - 1) * g / grid_lines;
            for (int j = 1; j < width - 1; j++) {
                if (canvas[y_pos][j] == ' ') {
                    canvas[y_pos][j] = 'h';  // 标记：水平线
                }
            }
        }
        
        // Y轴（左侧）
        for (int i = 0; i < height - 1; i++) {
            if (canvas[i][0] == 'h') {
                canvas[i][0] = 'c';  // 标记：交叉点
            } else if (canvas[i][0] == ' ') {
                canvas[i][0] = 'v';  // 标记：垂直线
            }
        }
        // Y轴箭头（顶部）
        canvas[0][0] = 'u';  // 标记：向上箭头
        
        // X轴（底部）
        for (int j = 0; j < width - 1; j++) {
            if (canvas[height - 1][j] == ' ') {
                canvas[height - 1][j] = 'h';  // 标记：水平线
            }
        }
        // X轴箭头（右侧）
        canvas[height - 1][width - 1] = 'r';  // 标记：向右箭头
        
        // 原点
        canvas[height - 1][0] = 'o';  // 标记：原点
        
        // 绘制垂直网格线
        for (int i = 0; i < height - 1; i++) {
            for (int g = 0; g <= grid_lines; g++) {
                int x_pos = (width - 1) * g / grid_lines;
                if (canvas[i][x_pos] == ' ') {
                    canvas[i][x_pos] = 'v';  // 标记：垂直线
                } else if (canvas[i][x_pos] == 'h') {
                    canvas[i][x_pos] = 'x';  // 标记：网格交叉点
                }
            }
        }
    }
    
    // 绘制曲线（避免覆盖坐标轴）
    if (point_count > 1) {
        for (int i = 0; i < point_count - 1; i++) {
            int sx1, sy1, sx2, sy2;
            // 调整坐标范围，为坐标轴预留空间
            world_to_screen(points[i].x, points[i].y, min_x, max_x,
                           min_y, max_y, width - 1, height - 1, &sx1, &sy1);
            world_to_screen(points[i + 1].x, points[i + 1].y, min_x, max_x,
                           min_y, max_y, width - 1, height - 1, &sx2, &sy2);
            
            // 使用Bresenham算法绘制直线
            int dx = abs(sx2 - sx1);
            int dy = abs(sy2 - sy1);
            int sx = sx1 < sx2 ? 1 : -1;
            int sy = sy1 < sy2 ? 1 : -1;
            int err = dx - dy;
            
            int x = sx1, y = sy1;
            while (1) {
                if (x > 0 && y >= 0 && x < width - 1 && y < height - 1) {
                    // 使用标记字符绘制曲线，打印时替换为Unicode字符
                    char ch = 'l';  // 使用l作为标记，打印时替换为●
                    // 不覆盖网格线，但可以覆盖空白和网格线字符
                    if (canvas[y][x] == ' ' || canvas[y][x] == 'h' || 
                        canvas[y][x] == 'v') {
                        canvas[y][x] = ch;
                    } else if (canvas[y][x] == 'x' || canvas[y][x] == 'c' || 
                               canvas[y][x] == 'o') {
                        // 在网格交叉点使用特殊字符
                        canvas[y][x] = ch;
                    }
                }
                
                if (x == sx2 && y == sy2) break;
                
                int e2 = 2 * err;
                if (e2 > -dy) {
                    err -= dy;
                    x += sx;
                }
                if (e2 < dx) {
                    err += dx;
                    y += sy;
                }
            }
        }
        
        // 标记数据点（使用更明显的符号）
        for (int i = 0; i < point_count; i++) {
            int sx, sy;
            world_to_screen(points[i].x, points[i].y, min_x, max_x,
                           min_y, max_y, width - 1, height - 1, &sx, &sy);
            if (sx > 0 && sy >= 0 && sx < width - 1 && sy < height - 1) {
                // 数据点使用标记字符，打印时替换为Unicode字符
                char point_marker = '@';  // 使用@作为标记，打印时替换为◆
                if (canvas[sy][sx] == ' ' || canvas[sy][sx] == 'h' || 
                    canvas[sy][sx] == 'v') {
                    canvas[sy][sx] = point_marker;
                } else if (canvas[sy][sx] == 'x' || canvas[sy][sx] == 'c' || 
                           canvas[sy][sx] == 'o') {
                    // 如果点在网格线上，使用特殊标记
                    canvas[sy][sx] = point_marker;
                } else {
                    canvas[sy][sx] = point_marker;
                }
            }
        }
    } else if (point_count == 1) {
        int sx, sy;
        world_to_screen(points[0].x, points[0].y, min_x, max_x,
                       min_y, max_y, width - 1, height - 1, &sx, &sy);
        if (sx > 0 && sy >= 0 && sx < width - 1 && sy < height - 1) {
            canvas[sy][sx] = '@';  // 使用@作为标记
        }
    }
    
    // 打印画布和Y轴刻度值
    char y_labels[height][16];
    for (int i = 0; i < height; i++) {
        y_labels[i][0] = '\0';
    }
    
    // 计算Y轴刻度值
    if (opts->show_labels) {
        int grid_lines = 5;
        for (int g = 0; g <= grid_lines; g++) {
            int y_pos = (height - 1) * g / grid_lines;
            double y_value = max_y - (max_y - min_y) * g / grid_lines;
            snprintf(y_labels[y_pos], sizeof(y_labels[y_pos]), "%.1f", y_value);
        }
    }
    
    // 打印画布（左侧显示Y轴刻度值）
    int max_label_len = 0;
    for (int i = 0; i < height; i++) {
        int len = strlen(y_labels[i]);
        if (len > max_label_len) max_label_len = len;
    }
    
    for (int i = 0; i < height; i++) {
        // 打印Y轴刻度值
        if (opts->show_labels && strlen(y_labels[i]) > 0) {
            printf("%s%*s%s ", COLOR_YELLOW, max_label_len, y_labels[i], COLOR_RESET);
        } else {
            printf("%*s ", max_label_len, "");
        }
        // 打印画布行，将标记字符替换为Unicode字符，并使用不同颜色
        for (int j = 0; j < width; j++) {
            char ch = canvas[i][j];
            // 将标记字符替换为Unicode字符，并使用不同颜色
            if (ch == 'l') {
                // 曲线使用红色，与坐标轴颜色区分
                printf("%s●%s", opts->color ? COLOR_RED : "", opts->color ? COLOR_RESET : "");
            } else if (ch == '@') {
                // 数据点使用黄色，更明显
                printf("%s◆%s", opts->color ? COLOR_YELLOW : "", opts->color ? COLOR_RESET : "");
            } else if (ch == 'h') {
                // 水平线
                printf("%s─%s", opts->color ? COLOR_MAGENTA : "", opts->color ? COLOR_RESET : "");
            } else if (ch == 'v') {
                // 垂直线
                printf("%s│%s", opts->color ? COLOR_MAGENTA : "", opts->color ? COLOR_RESET : "");
            } else if (ch == 'x') {
                // 网格交叉点
                printf("%s┼%s", opts->color ? COLOR_MAGENTA : "", opts->color ? COLOR_RESET : "");
            } else if (ch == 'c') {
                // 左连接
                printf("%s├%s", opts->color ? COLOR_MAGENTA : "", opts->color ? COLOR_RESET : "");
            } else if (ch == 'o') {
                // 原点
                printf("%s└%s", opts->color ? COLOR_MAGENTA : "", opts->color ? COLOR_RESET : "");
            } else if (ch == 'u') {
                // 向上箭头
                printf("%s↑%s", opts->color ? COLOR_MAGENTA : "", opts->color ? COLOR_RESET : "");
            } else if (ch == 'r') {
                // 向右箭头
                printf("%s→%s", opts->color ? COLOR_MAGENTA : "", opts->color ? COLOR_RESET : "");
            } else {
                printf("%c", ch);
            }
        }
        printf("\n");
    }
    
    // 打印X轴刻度值（显示网格线的X坐标）
    if (opts->show_labels) {
        printf("%*s", max_label_len + 1, "");
        int grid_lines = 5;
        int last_pos = 0;
        for (int g = 0; g <= grid_lines; g++) {
            int x_pos = (width - 1) * g / grid_lines;
            double x_value = min_x + (max_x - min_x) * g / grid_lines;
            char x_label[16];
            snprintf(x_label, sizeof(x_label), "%.1f", x_value);
            int label_len = strlen(x_label);
            
            // 计算位置，使刻度值对齐到网格线
            if (g == 0) {
                printf("%s", x_label);
                last_pos = label_len;
            } else {
                int spaces = x_pos - last_pos;
                if (spaces > label_len) {
                    spaces = spaces - label_len / 2;
                    if (spaces > 0) {
                        printf("%*s", spaces, "");
                    }
                    printf("%s", x_label);
                    last_pos = x_pos + label_len / 2;
                } else {
                    printf(" %s", x_label);
                    last_pos = x_pos + 1 + label_len;
                }
            }
        }
        printf("\n");
    }
}

// 显示图表
void display_chart(ChartOptions *opts) {
    if (point_count == 0) {
        print_error("没有数据点可显示");
        return;
    }
    
    double min_x, max_x, min_y, max_y;
    calculate_range(&min_x, &max_x, &min_y, &max_y);
    
    // 显示标题（参考YouPlot风格，更简洁）
    if (opts->show_labels && strlen(opts->title) > 0) {
        // 计算标题居中位置
        int title_len = strlen(opts->title);
        int padding = (opts->width > title_len) ? (opts->width - title_len) / 2 : 0;
        printf("\n");
        if (padding > 0) {
            printf("%*s", padding, "");
        }
        printf("%s%s%s\n\n", BOLD, opts->title, COLOR_RESET);
    }
    
    // 显示Y轴标签
    if (opts->show_labels && strlen(opts->ylabel) > 0) {
        printf("%s%s%s ", COLOR_CYAN, opts->ylabel, COLOR_RESET);
    }
    
    printf("\n");
    
    // 绘制图表
    switch (opts->type) {
        case CHART_BAR:
            draw_bar_chart(opts, min_x, max_x, min_y, max_y);
            break;
        case CHART_SCATTER:
            draw_scatter_chart(opts, min_x, max_x, min_y, max_y);
            break;
        case CHART_LINE:
            draw_line_chart(opts, min_x, max_x, min_y, max_y);
            break;
        case CHART_PIE:
            draw_pie_chart(opts, min_x, max_x, min_y, max_y);
            break;
        case CHART_AREA:
            draw_area_chart(opts, min_x, max_x, min_y, max_y);
            break;
        case CHART_CANDLESTICK:
            draw_candlestick_chart(opts, min_x, max_x, min_y, max_y);
            break;
        case CHART_GANTT:
            draw_gantt_chart(opts, min_x, max_x, min_y, max_y);
            break;
        case CHART_FUNNEL:
            draw_funnel_chart(opts, min_x, max_x, min_y, max_y);
            break;
        case CHART_BOXPLOT:
            draw_boxplot_chart(opts, min_x, max_x, min_y, max_y);
            break;
        case CHART_RADAR:
            draw_radar_chart(opts, min_x, max_x, min_y, max_y);
            break;
    }
    
    // 显示X轴标签
    if (opts->show_labels && strlen(opts->xlabel) > 0) {
        printf("\n%s%s%s\n", COLOR_CYAN, opts->xlabel, COLOR_RESET);
    }
    
    // 显示图例
    if (opts->show_legend) {
        printf("\n%s图例:%s\n", COLOR_YELLOW, COLOR_RESET);
        printf("  数据点数量: %d\n", point_count);
        printf("  X范围: [%.2f, %.2f]\n", min_x, max_x);
        printf("  Y范围: [%.2f, %.2f]\n", min_y, max_y);
    }
    
    printf("\n");
}

// 解析命令行参数
int parse_arguments(int argc, char *argv[], ChartOptions *opts) {
    static struct option long_options[] = {
        {"type", required_argument, 0, 't'},
        {"width", required_argument, 0, 'w'},
        {"height", required_argument, 0, 'h'},
        {"title", required_argument, 0, 'T'},
        {"xlabel", required_argument, 0, 'x'},
        {"ylabel", required_argument, 0, 'y'},
        {"no-grid", no_argument, 0, 'g'},
        {"no-labels", no_argument, 0, 'l'},
        {"no-legend", no_argument, 0, 'L'},
        {"no-color", no_argument, 0, 'c'},
        {"interactive", no_argument, 0, 'i'},
        {"file", required_argument, 0, 'f'},
        {"help", no_argument, 0, 0},
        {"version", no_argument, 0, 0},
        {0, 0, 0, 0}
    };
    
    int opt;
    int option_index = 0;
    
    while ((opt = getopt_long(argc, argv, "t:w:h:T:x:y:glLcif:", 
                              long_options, &option_index)) != -1) {
        switch (opt) {
            case 't':
                if (strcmp(optarg, "bar") == 0) {
                    opts->type = CHART_BAR;
                } else if (strcmp(optarg, "scatter") == 0) {
                    opts->type = CHART_SCATTER;
                } else if (strcmp(optarg, "line") == 0) {
                    opts->type = CHART_LINE;
                } else if (strcmp(optarg, "pie") == 0) {
                    opts->type = CHART_PIE;
                } else if (strcmp(optarg, "area") == 0) {
                    opts->type = CHART_AREA;
                } else if (strcmp(optarg, "candlestick") == 0) {
                    opts->type = CHART_CANDLESTICK;
                } else if (strcmp(optarg, "gantt") == 0) {
                    opts->type = CHART_GANTT;
                } else if (strcmp(optarg, "funnel") == 0) {
                    opts->type = CHART_FUNNEL;
                } else if (strcmp(optarg, "boxplot") == 0) {
                    opts->type = CHART_BOXPLOT;
                } else if (strcmp(optarg, "radar") == 0) {
                    opts->type = CHART_RADAR;
                } else {
                    print_error("无效的图表类型，支持: bar, scatter, line, pie, area, candlestick, gantt, funnel, boxplot, radar");
                    return 0;
                }
                break;
            case 'w':
                opts->width = atoi(optarg);
                if (opts->width < 10 || opts->width > MAX_WIDTH) {
                    opts->width = DEFAULT_WIDTH;
                }
                break;
            case 'h':
                opts->height = atoi(optarg);
                if (opts->height < 5 || opts->height > MAX_HEIGHT) {
                    opts->height = DEFAULT_HEIGHT;
                }
                break;
            case 'T':
                strncpy(opts->title, optarg, sizeof(opts->title) - 1);
                break;
            case 'x':
                strncpy(opts->xlabel, optarg, sizeof(opts->xlabel) - 1);
                break;
            case 'y':
                strncpy(opts->ylabel, optarg, sizeof(opts->ylabel) - 1);
                break;
            case 'g':
                opts->show_grid = 0;
                break;
            case 'l':
                opts->show_labels = 0;
                break;
            case 'L':
                opts->show_legend = 0;
                break;
            case 'c':
                opts->color = 0;
                break;
            case 'i':
                return 2; // 交互模式
            case 'f':
                if (!load_from_file(optarg)) {
                    return 0;
                }
                return 1; // 从文件加载成功
            case 0:
                if (strcmp(long_options[option_index].name, "help") == 0) {
                    print_help(argv[0]);
                    return 3; // 帮助信息，正常退出
                } else if (strcmp(long_options[option_index].name, "version") == 0) {
                    print_version();
                    return 3; // 版本信息，正常退出
                }
                break;
            default:
                return 0;
        }
    }
    
    // 解析剩余的参数作为数据点
    point_count = 0;
    for (int i = optind; i < argc && point_count < MAX_POINTS; i++) {
        if (parse_point(argv[i], &points[point_count])) {
            point_count++;
        }
    }
    
    return 1;
}

int main(int argc, char *argv[]) {
    ChartOptions opts;
    init_options(&opts);
    
    if (argc < 2) {
        print_help(argv[0]);
        return 1;
    }
    
    int result = parse_arguments(argc, argv, &opts);
    
    // 处理帮助和版本信息（正常退出）
    if (result == 3) {
        return 0; // 帮助或版本信息，正常退出
    }
    
    if (result == 0) {
        return 1; // 解析错误
    }
    
    if (result == 2) {
        // 交互模式
        if (!interactive_input(&opts)) {
            print_error("没有输入数据点");
            return 1;
        }
    } else if (point_count == 0) {
        print_error("没有数据点可显示");
        return 1;
    }
    
    display_chart(&opts);
    
    return 0;
}

