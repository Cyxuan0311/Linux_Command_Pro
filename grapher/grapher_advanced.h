#ifndef GRAPHER_ADVANCED_H
#define GRAPHER_ADVANCED_H

#include <stdio.h>

// 高级图表绘制函数
// 注意：ChartOptions 和 DataPoint 类型在主文件 grapher.c 中定义
// 这里只声明函数，实现文件会通过包含主文件来获取类型定义
void draw_pie_chart(void *opts, double min_x, double max_x, 
                    double min_y, double max_y);
void draw_area_chart(void *opts, double min_x, double max_x,
                     double min_y, double max_y);
void draw_candlestick_chart(void *opts, double min_x, double max_x,
                            double min_y, double max_y);
void draw_gantt_chart(void *opts, double min_x, double max_x,
                      double min_y, double max_y);
void draw_funnel_chart(void *opts, double min_x, double max_x,
                       double min_y, double max_y);
void draw_boxplot_chart(void *opts, double min_x, double max_x,
                        double min_y, double max_y);
void draw_radar_chart(void *opts, double min_x, double max_x,
                      double min_y, double max_y);

#endif // GRAPHER_ADVANCED_H

