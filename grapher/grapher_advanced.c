#include "grapher_advanced.h"
#include "../include/common.h"
#include <math.h>
#include <string.h>

// 需要从主文件获取类型定义，但为了避免循环依赖，我们在这里重新定义
// 这些定义必须与 grapher.c 中的定义完全一致
typedef struct {
    double x;
    double y;
} DataPoint;

typedef struct {
    int type;
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

// 外部变量和函数（在主文件中定义）
extern DataPoint points[];
extern int point_count;
extern void world_to_screen(double x, double y, double min_x, double max_x,
                            double min_y, double max_y, int width, int height,
                            int *sx, int *sy);

// 绘制饼图
void draw_pie_chart(void *opts_ptr, double min_x, double max_x,
                    double min_y, double max_y) {
    (void)min_x; (void)max_x; (void)min_y; (void)max_y; // 未使用的参数
    ChartOptions *opts = (ChartOptions *)opts_ptr;
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
    
    if (point_count == 0) return;
    
    // 计算总和
    double total = 0;
    for (int i = 0; i < point_count; i++) {
        total += points[i].y;
    }
    if (total == 0) total = 1;
    
    // 计算中心点和半径（考虑字符宽高比，终端字符通常高度是宽度的2倍）
    int center_x = width / 2;
    int center_y = height / 2;
    int radius_x = (width - 4) / 2;  // 留出边距
    int radius_y = (height - 2) / 2;
    int radius = radius_x < radius_y ? radius_x : radius_y;
    if (radius < 3) radius = 3;
    
    // 颜色数组（用于不同扇形）
    const char* colors[] = {
        COLOR_RED, COLOR_GREEN, COLOR_YELLOW, COLOR_BLUE,
        COLOR_MAGENTA, COLOR_CYAN, COLOR_WHITE
    };
    int color_count = 7;
    
    // 绘制饼图扇形（填充整个区域）
    double current_angle = -M_PI / 2;  // 从顶部开始（-90度）
    
    for (int i = 0; i < point_count; i++) {
        double slice_angle = (points[i].y / total) * 2 * M_PI;
        double end_angle = current_angle + slice_angle;
        
        // 使用标记字符 '0'-'9', 'a'-'z' 来标记不同的扇形
        char marker = (i < 10) ? ('0' + i) : ('a' + (i - 10));
        if (marker > 'z') marker = 'z';
        
        // 填充整个扇形区域
        for (int y = -radius_y; y <= radius_y; y++) {
            for (int x = -radius_x; x <= radius_x; x++) {
                // 计算椭圆距离（考虑字符宽高比）
                double dist_x = (double)x / radius_x;
                double dist_y = (double)y / radius_y;
                double dist = sqrt(dist_x * dist_x + dist_y * dist_y);
                
                if (dist <= 1.0) {  // 在圆内
                    // 计算角度（注意：atan2 返回 -π 到 π）
                    double angle = atan2(-y, x);
                    // 标准化到 0 到 2π
                    if (angle < 0) angle += 2 * M_PI;
                    
                    // 标准化当前角度和结束角度
                    double norm_start = current_angle;
                    double norm_end = end_angle;
                    
                    // 处理角度跨越 0 度的情况
                    int in_slice = 0;
                    if (norm_end <= 2 * M_PI && norm_start >= 0) {
                        // 正常情况：起始角度和结束角度都在 0-2π 范围内
                        if (norm_start <= norm_end) {
                            in_slice = (angle >= norm_start && angle <= norm_end);
                        } else {
                            // 跨越 0 度
                            in_slice = (angle >= norm_start || angle <= norm_end);
                        }
                    } else {
                        // 处理负角度或超过 2π 的情况
                        if (norm_start < 0) norm_start += 2 * M_PI;
                        if (norm_end > 2 * M_PI) norm_end -= 2 * M_PI;
                        if (norm_end < 0) norm_end += 2 * M_PI;
                        
                        if (norm_start <= norm_end) {
                            in_slice = (angle >= norm_start && angle <= norm_end);
                        } else {
                            in_slice = (angle >= norm_start || angle <= norm_end);
                        }
                    }
                    
                    if (in_slice) {
                        int px = center_x + x;
                        int py = center_y + y;
                        if (px >= 0 && px < width && py >= 0 && py < height) {
                            canvas[py][px] = marker;
                        }
                    }
                }
            }
        }
        
        current_angle = end_angle;
    }
    
    // 打印画布，使用不同颜色和 Unicode 字符
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            char ch = canvas[i][j];
            if (ch >= '0' && ch <= '9') {
                int idx = ch - '0';
                const char* color = opts->color ? colors[idx % color_count] : "";
                printf("%s█%s", color, opts->color ? COLOR_RESET : "");
            } else if (ch >= 'a' && ch <= 'z') {
                int idx = 10 + (ch - 'a');
                const char* color = opts->color ? colors[idx % color_count] : "";
                printf("%s█%s", color, opts->color ? COLOR_RESET : "");
            } else {
                printf("%c", ch);
            }
        }
        printf("\n");
    }
    
    // 显示图例和百分比
    if (opts->show_legend) {
        printf("\n%s图例:%s\n", COLOR_YELLOW, COLOR_RESET);
        current_angle = -M_PI / 2;
        for (int i = 0; i < point_count; i++) {
            double slice_angle = (points[i].y / total) * 2 * M_PI;
            double percentage = (points[i].y / total) * 100.0;
            (void)slice_angle;  // 保留用于将来可能的标签位置计算
            
            const char* color = opts->color ? colors[i % color_count] : "";
            printf("  %s█%s 项目 %d: %.1f%% (值: %.2f)\n", 
                   color, opts->color ? COLOR_RESET : "", 
                   i + 1, percentage, points[i].y);
            
            current_angle += slice_angle;
        }
    }
}

// 绘制面积图
void draw_area_chart(void *opts_ptr, double min_x, double max_x,
                     double min_y, double max_y) {
    ChartOptions *opts = (ChartOptions *)opts_ptr;
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
    
    // 绘制网格（使用标记字符）
    if (opts->show_grid) {
        int grid_lines = 5;
        for (int g = 0; g <= grid_lines; g++) {
            int y_pos = (height - 1) * g / grid_lines;
            for (int j = 1; j < width - 1; j++) {
                if (canvas[y_pos][j] == ' ') {
                    canvas[y_pos][j] = 'h';
                }
            }
        }
        
        for (int i = 0; i < height - 1; i++) {
            if (canvas[i][0] == 'h') {
                canvas[i][0] = 'c';
            } else if (canvas[i][0] == ' ') {
                canvas[i][0] = 'v';
            }
        }
        canvas[0][0] = 'u';
        
        for (int j = 0; j < width - 1; j++) {
            if (canvas[height - 1][j] == ' ') {
                canvas[height - 1][j] = 'h';
            }
        }
        canvas[height - 1][width - 1] = 'r';
        canvas[height - 1][0] = 'o';
    }
    
    // 绘制面积（填充曲线下方区域）
    if (point_count > 1) {
        for (int i = 0; i < point_count - 1; i++) {
            int sx1, sy1, sx2, sy2;
            world_to_screen(points[i].x, points[i].y, min_x, max_x,
                           min_y, max_y, width - 1, height - 1, &sx1, &sy1);
            world_to_screen(points[i + 1].x, points[i + 1].y, min_x, max_x,
                           min_y, max_y, width - 1, height - 1, &sx2, &sy2);
            
            // 填充两点之间的区域
            int min_x_pos = sx1 < sx2 ? sx1 : sx2;
            int max_x_pos = sx1 > sx2 ? sx1 : sx2;
            
            for (int x = min_x_pos; x <= max_x_pos && x < width - 1; x++) {
                // 线性插值计算y值
                double ratio = (double)(x - sx1) / (sx2 - sx1);
                if (sx2 == sx1) ratio = 0;
                double y_val = points[i].y + (points[i + 1].y - points[i].y) * ratio;
                int sy;
                world_to_screen((points[i].x + points[i + 1].x) / 2, y_val,
                               min_x, max_x, min_y, max_y, width - 1, height - 1, &sx1, &sy);
                
                // 从底部填充到曲线
                int base_y = height - 1;
                for (int y = sy; y < base_y && y >= 0; y++) {
                    if (canvas[y][x] == ' ' || canvas[y][x] == 'h') {
                        canvas[y][x] = 'a';  // 标记：面积填充
                    }
                }
            }
        }
    }
    
    // 绘制曲线
    if (point_count > 1) {
        for (int i = 0; i < point_count - 1; i++) {
            int sx1, sy1, sx2, sy2;
            world_to_screen(points[i].x, points[i].y, min_x, max_x,
                           min_y, max_y, width - 1, height - 1, &sx1, &sy1);
            world_to_screen(points[i + 1].x, points[i + 1].y, min_x, max_x,
                           min_y, max_y, width - 1, height - 1, &sx2, &sy2);
            
            int dx = abs(sx2 - sx1);
            int dy = abs(sy2 - sy1);
            int sx = sx1 < sx2 ? 1 : -1;
            int sy = sy1 < sy2 ? 1 : -1;
            int err = dx - dy;
            
            int x = sx1, y = sy1;
            while (1) {
                if (x > 0 && y >= 0 && x < width - 1 && y < height - 1) {
                    if (canvas[y][x] == ' ' || canvas[y][x] == 'h' || canvas[y][x] == 'a') {
                        canvas[y][x] = 'l';
                    }
                }
                if (x == sx2 && y == sy2) break;
                int e2 = 2 * err;
                if (e2 > -dy) { err -= dy; x += sx; }
                if (e2 < dx) { err += dx; y += sy; }
            }
        }
    }
    
    // 打印画布
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            char ch = canvas[i][j];
            if (ch == 'a') {
                printf("%s▓%s", opts->color ? COLOR_BLUE : "", opts->color ? COLOR_RESET : "");
            } else if (ch == 'l') {
                printf("%s●%s", opts->color ? COLOR_RED : "", opts->color ? COLOR_RESET : "");
            } else if (ch == 'h') {
                printf("%s─%s", opts->color ? COLOR_CYAN : "", opts->color ? COLOR_RESET : "");
            } else if (ch == 'v') {
                printf("%s│%s", opts->color ? COLOR_CYAN : "", opts->color ? COLOR_RESET : "");
            } else if (ch == 'x') {
                printf("%s┼%s", opts->color ? COLOR_CYAN : "", opts->color ? COLOR_RESET : "");
            } else if (ch == 'c') {
                printf("%s├%s", opts->color ? COLOR_CYAN : "", opts->color ? COLOR_RESET : "");
            } else if (ch == 'o') {
                printf("%s└%s", opts->color ? COLOR_CYAN : "", opts->color ? COLOR_RESET : "");
            } else if (ch == 'u') {
                printf("%s↑%s", opts->color ? COLOR_CYAN : "", opts->color ? COLOR_RESET : "");
            } else if (ch == 'r') {
                printf("%s→%s", opts->color ? COLOR_CYAN : "", opts->color ? COLOR_RESET : "");
            } else {
                printf("%c", ch);
            }
        }
        printf("\n");
    }
}

// 绘制K线图（金融图表）
// 数据格式：x=日期序号, y=价格（需要4个点表示开高低收，这里简化处理）
void draw_candlestick_chart(void *opts_ptr, double min_x, double max_x,
                            double min_y, double max_y) {
    ChartOptions *opts = (ChartOptions *)opts_ptr;
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
    
    // 绘制网格
    if (opts->show_grid) {
        int grid_lines = 5;
        for (int g = 0; g <= grid_lines; g++) {
            int y_pos = (height - 1) * g / grid_lines;
            for (int j = 1; j < width - 1; j++) {
                if (canvas[y_pos][j] == ' ') {
                    canvas[y_pos][j] = 'h';
                }
            }
        }
        
        for (int i = 0; i < height - 1; i++) {
            if (canvas[i][0] == 'h') {
                canvas[i][0] = 'c';
            } else if (canvas[i][0] == ' ') {
                canvas[i][0] = 'v';
            }
        }
        canvas[0][0] = 'u';
        
        for (int j = 0; j < width - 1; j++) {
            if (canvas[height - 1][j] == ' ') {
                canvas[height - 1][j] = 'h';
            }
        }
        canvas[height - 1][width - 1] = 'r';
        canvas[height - 1][0] = 'o';
    }
    
    // 绘制K线（简化版：使用柱状表示价格范围）
    double x_range = max_x - min_x;
    double y_range = max_y - min_y;
    if (x_range == 0) x_range = 1;
    if (y_range == 0) y_range = 1;
    
    int plot_width = width - 2;
    int plot_height = height - 1;
    
    for (int i = 0; i < point_count; i++) {
        int x_pos = 1 + (int)((points[i].x - min_x) / x_range * plot_width);
        int high_y = height - 1 - (int)((points[i].y - min_y) / y_range * plot_height) - 1;
        int low_y = height - 2;  // 简化：使用固定低点
        
        // 绘制K线实体和影线
        if (x_pos > 0 && x_pos < width - 1) {
            // 影线（最高到最低）
            for (int y = high_y; y <= low_y && y >= 0 && y < height - 1; y++) {
                if (canvas[y][x_pos] == ' ' || canvas[y][x_pos] == 'h') {
                    canvas[y][x_pos] = 'k';  // 标记：K线
                }
            }
            // 实体（开盘到收盘，简化处理）
            int body_top = high_y + 1;
            int body_bottom = low_y - 1;
            for (int y = body_top; y <= body_bottom && y >= 0 && y < height - 1; y++) {
                if (x_pos - 1 > 0 && x_pos + 1 < width - 1) {
                    for (int w = -1; w <= 1; w++) {
                        if (canvas[y][x_pos + w] == ' ' || canvas[y][x_pos + w] == 'h') {
                            canvas[y][x_pos + w] = 'k';
                        }
                    }
                }
            }
        }
    }
    
    // 打印画布
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            char ch = canvas[i][j];
            if (ch == 'k') {
                printf("%s█%s", opts->color ? COLOR_YELLOW : "", opts->color ? COLOR_RESET : "");
            } else if (ch == 'h') {
                printf("%s─%s", opts->color ? COLOR_CYAN : "", opts->color ? COLOR_RESET : "");
            } else if (ch == 'v') {
                printf("%s│%s", opts->color ? COLOR_CYAN : "", opts->color ? COLOR_RESET : "");
            } else if (ch == 'x') {
                printf("%s┼%s", opts->color ? COLOR_CYAN : "", opts->color ? COLOR_RESET : "");
            } else if (ch == 'c') {
                printf("%s├%s", opts->color ? COLOR_CYAN : "", opts->color ? COLOR_RESET : "");
            } else if (ch == 'o') {
                printf("%s└%s", opts->color ? COLOR_CYAN : "", opts->color ? COLOR_RESET : "");
            } else if (ch == 'u') {
                printf("%s↑%s", opts->color ? COLOR_CYAN : "", opts->color ? COLOR_RESET : "");
            } else if (ch == 'r') {
                printf("%s→%s", opts->color ? COLOR_CYAN : "", opts->color ? COLOR_RESET : "");
            } else {
                printf("%c", ch);
            }
        }
        printf("\n");
    }
}

// 绘制甘特图（项目管理）
// 数据格式：x=开始时间, y=持续时间
void draw_gantt_chart(void *opts_ptr, double min_x, double max_x,
                      double min_y, double max_y) {
    (void)min_y; (void)max_y; // 未使用的参数
    ChartOptions *opts = (ChartOptions *)opts_ptr;
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
    
    // 绘制时间轴
    if (opts->show_grid) {
        for (int j = 0; j < width - 1; j++) {
            if (canvas[height - 1][j] == ' ') {
                canvas[height - 1][j] = 'h';
            }
        }
        canvas[height - 1][width - 1] = 'r';
        canvas[height - 1][0] = 'o';
    }
    
    // 绘制任务条
    double x_range = max_x - min_x;
    if (x_range == 0) x_range = 1;
    
    int plot_width = width - 2;
    int row_height = (height - 1) / point_count;
    if (row_height < 1) row_height = 1;
    
    for (int i = 0; i < point_count && i < height - 1; i++) {
        int start_x = 1 + (int)((points[i].x - min_x) / x_range * plot_width);
        int duration = (int)(points[i].y / x_range * plot_width);
        if (duration < 2) duration = 2;
        int end_x = start_x + duration;
        if (end_x >= width - 1) end_x = width - 1;
        
        int row_y = i * row_height;
        if (row_y >= height - 1) row_y = height - 2;
        
        // 绘制任务条
        for (int x = start_x; x < end_x && x < width - 1; x++) {
            for (int h = 0; h < row_height - 1 && row_y + h < height - 1; h++) {
                if (canvas[row_y + h][x] == ' ') {
                    canvas[row_y + h][x] = 'g';  // 标记：甘特图
                }
            }
        }
    }
    
    // 打印画布
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            char ch = canvas[i][j];
            if (ch == 'g') {
                printf("%s█%s", opts->color ? COLOR_GREEN : "", opts->color ? COLOR_RESET : "");
            } else if (ch == 'h') {
                printf("%s─%s", opts->color ? COLOR_CYAN : "", opts->color ? COLOR_RESET : "");
            } else if (ch == 'r') {
                printf("%s→%s", opts->color ? COLOR_CYAN : "", opts->color ? COLOR_RESET : "");
            } else if (ch == 'o') {
                printf("%s└%s", opts->color ? COLOR_CYAN : "", opts->color ? COLOR_RESET : "");
            } else {
                printf("%c", ch);
            }
        }
        printf("\n");
    }
}

// 绘制漏斗图（销售/营销）
// 数据格式：x=阶段序号, y=数量/百分比
void draw_funnel_chart(void *opts_ptr, double min_x, double max_x,
                       double min_y, double max_y) {
    (void)min_x; (void)max_x; (void)min_y; (void)max_y; // 未使用的参数
    ChartOptions *opts = (ChartOptions *)opts_ptr;
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
    
    if (point_count == 0) return;
    
    // 计算最大宽度（顶部）
    int max_width = width - 4;
    int row_height = (height - 1) / point_count;
    if (row_height < 1) row_height = 1;
    
    double max_val = max_y;
    if (max_val == 0) max_val = 1;
    
    // 绘制漏斗
    for (int i = 0; i < point_count; i++) {
        int row_y = i * row_height;
        if (row_y >= height - 1) row_y = height - 2;
        
        // 计算当前行的宽度（按比例缩小）
        double ratio = points[i].y / max_val;
        int current_width = (int)(max_width * ratio);
        if (current_width < 2) current_width = 2;
        
        int start_x = (width - current_width) / 2;
        
        // 绘制漏斗层
        for (int x = start_x; x < start_x + current_width && x < width; x++) {
            for (int h = 0; h < row_height - 1 && row_y + h < height - 1; h++) {
                if (canvas[row_y + h][x] == ' ') {
                    canvas[row_y + h][x] = 'f';  // 标记：漏斗图
                }
            }
        }
    }
    
    // 打印画布
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            char ch = canvas[i][j];
            if (ch == 'f') {
                printf("%s█%s", opts->color ? COLOR_MAGENTA : "", opts->color ? COLOR_RESET : "");
            } else {
                printf("%c", ch);
            }
        }
        printf("\n");
    }
}

// 绘制箱线图（统计分析）
// 数据格式：需要多个数据点，这里简化处理
void draw_boxplot_chart(void *opts_ptr, double min_x, double max_x,
                        double min_y, double max_y) {
    (void)min_x; (void)max_x; // 未使用的参数
    ChartOptions *opts = (ChartOptions *)opts_ptr;
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
    
    // 绘制网格
    if (opts->show_grid) {
        int grid_lines = 5;
        for (int g = 0; g <= grid_lines; g++) {
            int y_pos = (height - 1) * g / grid_lines;
            for (int j = 1; j < width - 1; j++) {
                if (canvas[y_pos][j] == ' ') {
                    canvas[y_pos][j] = 'h';
                }
            }
        }
        
        for (int i = 0; i < height - 1; i++) {
            if (canvas[i][0] == 'h') {
                canvas[i][0] = 'c';
            } else if (canvas[i][0] == ' ') {
                canvas[i][0] = 'v';
            }
        }
        canvas[0][0] = 'u';
        
        for (int j = 0; j < width - 1; j++) {
            if (canvas[height - 1][j] == ' ') {
                canvas[height - 1][j] = 'h';
            }
        }
        canvas[height - 1][width - 1] = 'r';
        canvas[height - 1][0] = 'o';
    }
    
    // 计算统计值（简化：使用所有数据点）
    if (point_count > 0) {
        // 排序找中位数、四分位数等（简化处理）
        double median = points[point_count / 2].y;
        double q1 = points[point_count / 4].y;
        double q3 = points[point_count * 3 / 4].y;
        
        int center_x = width / 2;
        double y_range = max_y - min_y;
        if (y_range == 0) y_range = 1;
        
        // 绘制箱线图
        int q1_y = height - 1 - (int)((q1 - min_y) / y_range * (height - 1));
        int median_y = height - 1 - (int)((median - min_y) / y_range * (height - 1));
        int q3_y = height - 1 - (int)((q3 - min_y) / y_range * (height - 1));
        
        // 绘制箱体
        int box_width = 8;
        for (int x = center_x - box_width / 2; x <= center_x + box_width / 2 && x < width - 1; x++) {
            for (int y = q1_y; y <= q3_y && y >= 0 && y < height - 1; y++) {
                if (canvas[y][x] == ' ' || canvas[y][x] == 'h') {
                    canvas[y][x] = 'b';  // 标记：箱线图
                }
            }
        }
        
        // 绘制中位数线
        for (int x = center_x - box_width / 2; x <= center_x + box_width / 2 && x < width - 1; x++) {
            if (median_y >= 0 && median_y < height - 1) {
                canvas[median_y][x] = 'm';  // 标记：中位数
            }
        }
        
        // 绘制须线
        int whisker_len = 5;
        for (int x = center_x - whisker_len; x <= center_x + whisker_len && x < width - 1; x++) {
            if (q1_y >= 0 && q1_y < height - 1) {
                if (canvas[q1_y][x] == ' ' || canvas[q1_y][x] == 'h') {
                    canvas[q1_y][x] = 'w';  // 标记：须线
                }
            }
            if (q3_y >= 0 && q3_y < height - 1) {
                if (canvas[q3_y][x] == ' ' || canvas[q3_y][x] == 'h') {
                    canvas[q3_y][x] = 'w';
                }
            }
        }
    }
    
    // 打印画布
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            char ch = canvas[i][j];
            if (ch == 'b') {
                printf("%s█%s", opts->color ? COLOR_BLUE : "", opts->color ? COLOR_RESET : "");
            } else if (ch == 'm') {
                printf("%s─%s", opts->color ? COLOR_RED : "", opts->color ? COLOR_RESET : "");
            } else if (ch == 'w') {
                printf("%s│%s", opts->color ? COLOR_YELLOW : "", opts->color ? COLOR_RESET : "");
            } else if (ch == 'h') {
                printf("%s─%s", opts->color ? COLOR_CYAN : "", opts->color ? COLOR_RESET : "");
            } else if (ch == 'v') {
                printf("%s│%s", opts->color ? COLOR_CYAN : "", opts->color ? COLOR_RESET : "");
            } else if (ch == 'c') {
                printf("%s├%s", opts->color ? COLOR_CYAN : "", opts->color ? COLOR_RESET : "");
            } else if (ch == 'o') {
                printf("%s└%s", opts->color ? COLOR_CYAN : "", opts->color ? COLOR_RESET : "");
            } else if (ch == 'u') {
                printf("%s↑%s", opts->color ? COLOR_CYAN : "", opts->color ? COLOR_RESET : "");
            } else if (ch == 'r') {
                printf("%s→%s", opts->color ? COLOR_CYAN : "", opts->color ? COLOR_RESET : "");
            } else {
                printf("%c", ch);
            }
        }
        printf("\n");
    }
}

// 绘制雷达图（多维度分析）
// 数据格式：x=维度序号, y=数值
void draw_radar_chart(void *opts_ptr, double min_x, double max_x,
                      double min_y, double max_y) {
    (void)min_x; (void)max_x; (void)min_y; (void)max_y; // 未使用的参数
    ChartOptions *opts = (ChartOptions *)opts_ptr;
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
    
    if (point_count < 3) return;  // 雷达图至少需要3个维度
    
    // 计算中心点和最大半径
    int center_x = width / 2;
    int center_y = height / 2;
    int max_radius = (width < height ? width : height) / 2 - 2;
    if (max_radius < 5) max_radius = 5;
    
    double max_val = max_y;
    if (max_val == 0) max_val = 1;
    
    // 绘制雷达图网格（同心圆和放射线）
    if (opts->show_grid) {
        // 绘制同心圆
        for (int r = max_radius / 4; r <= max_radius; r += max_radius / 4) {
            for (int angle = 0; angle < 360; angle += 5) {
                double rad = angle * M_PI / 180.0;
                int x = center_x + (int)(r * cos(rad));
                int y = center_y - (int)(r * sin(rad));
                if (x >= 0 && x < width && y >= 0 && y < height) {
                    if (canvas[y][x] == ' ') {
                        canvas[y][x] = 'c';  // 标记：圆
                    }
                }
            }
        }
        
        // 绘制放射线
        for (int i = 0; i < point_count; i++) {
            double angle = (2 * M_PI * i) / point_count - M_PI / 2;
            for (int r = 0; r <= max_radius; r++) {
                int x = center_x + (int)(r * cos(angle));
                int y = center_y - (int)(r * sin(angle));
                if (x >= 0 && x < width && y >= 0 && y < height) {
                    if (canvas[y][x] == ' ' || canvas[y][x] == 'c') {
                        canvas[y][x] = 'r';  // 标记：放射线
                    }
                }
            }
        }
    }
    
    // 绘制数据多边形
    int prev_x = -1, prev_y = -1;
    int first_x = -1, first_y = -1;
    
    for (int i = 0; i <= point_count; i++) {
        int idx = i % point_count;
        double angle = (2 * M_PI * idx) / point_count - M_PI / 2;
        double ratio = points[idx].y / max_val;
        int radius = (int)(max_radius * ratio);
        
        int x = center_x + (int)(radius * cos(angle));
        int y = center_y - (int)(radius * sin(angle));
        
        if (x >= 0 && x < width && y >= 0 && y < height) {
            canvas[y][x] = 'p';  // 标记：数据点
            
            if (prev_x >= 0 && prev_y >= 0) {
                // 绘制连线
                int dx = abs(x - prev_x);
                int dy = abs(y - prev_y);
                int sx = prev_x < x ? 1 : -1;
                int sy = prev_y < y ? 1 : -1;
                int err = dx - dy;
                
                int px = prev_x, py = prev_y;
                while (1) {
                    if (px >= 0 && px < width && py >= 0 && py < height) {
                        if (canvas[py][px] == ' ' || canvas[py][px] == 'c' || canvas[py][px] == 'r') {
                            canvas[py][px] = 'l';  // 标记：连线
                        }
                    }
                    if (px == x && py == y) break;
                    int e2 = 2 * err;
                    if (e2 > -dy) { err -= dy; px += sx; }
                    if (e2 < dx) { err += dx; py += sy; }
                }
            } else {
                first_x = x;
                first_y = y;
            }
            
            prev_x = x;
            prev_y = y;
        }
    }
    
    // 连接最后一个点和第一个点
    if (prev_x >= 0 && prev_y >= 0 && first_x >= 0 && first_y >= 0) {
        int dx = abs(first_x - prev_x);
        int dy = abs(first_y - prev_y);
        int sx = prev_x < first_x ? 1 : -1;
        int sy = prev_y < first_y ? 1 : -1;
        int err = dx - dy;
        
        int px = prev_x, py = prev_y;
        while (1) {
            if (px >= 0 && px < width && py >= 0 && py < height) {
                if (canvas[py][px] == ' ' || canvas[py][px] == 'c' || canvas[py][px] == 'r') {
                    canvas[py][px] = 'l';
                }
            }
            if (px == first_x && py == first_y) break;
            int e2 = 2 * err;
            if (e2 > -dy) { err -= dy; px += sx; }
            if (e2 < dx) { err += dx; py += sy; }
        }
    }
    
    // 打印画布
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            char ch = canvas[i][j];
            if (ch == 'p') {
                printf("%s●%s", opts->color ? COLOR_RED : "", opts->color ? COLOR_RESET : "");
            } else if (ch == 'l') {
                printf("%s●%s", opts->color ? COLOR_GREEN : "", opts->color ? COLOR_RESET : "");
            } else if (ch == 'c' || ch == 'r') {
                printf("%s·%s", opts->color ? COLOR_CYAN : "", opts->color ? COLOR_RESET : "");
            } else {
                printf("%c", ch);
            }
        }
        printf("\n");
    }
}

