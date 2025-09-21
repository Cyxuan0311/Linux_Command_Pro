#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <getopt.h>
#include "../include/common.h"

#define MAX_MONTHS 12
#define MAX_DAYS 31

// 月份名称
const char *month_names[] = {
    "January", "February", "March", "April", "May", "June",
    "July", "August", "September", "October", "November", "December"
};

// 星期名称
const char *weekday_names[] = {
    "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"
};

// 日历选项
typedef struct {
    int year;
    int month;
    int show_year;
    int show_help;
    int show_version;
    int show_3months;
    int show_12months;
} CalOptions;

// 初始化选项
void init_options(CalOptions *opts) {
    memset(opts, 0, sizeof(CalOptions));
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    opts->year = tm_info->tm_year + 1900;
    opts->month = tm_info->tm_mon + 1;
}

// 显示帮助信息
void print_help(const char *program_name) {
    printf("用法: %s [选项] [[月] 年]\n", program_name);
    printf("增强版 cal 命令，提供美观的日历显示\n\n");
    printf("选项:\n");
    printf("  -3, --three-months    显示前月、当月、下月\n");
    printf("  -y, --year            显示整年日历\n");
    printf("  -h, --help            显示此帮助信息\n");
    printf("  -v, --version         显示版本信息\n\n");
    printf("参数:\n");
    printf("  月     月份 (1-12)\n");
    printf("  年     年份 (1-9999)\n\n");
    printf("示例:\n");
    printf("  %s                    # 显示当前月份\n", program_name);
    printf("  %s 2024               # 显示2024年\n", program_name);
    printf("  %s 3 2024             # 显示2024年3月\n", program_name);
    printf("  %s -3                 # 显示三个月\n", program_name);
    printf("  %s -y                 # 显示整年\n", program_name);
}

// 显示版本信息
void print_version() {
    printf("pcal - 增强版 cal 命令 v1.0\n");
    printf("提供美观的日历显示功能\n");
}

// 判断是否为闰年
int is_leap_year(int year) {
    return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

// 获取月份天数
int get_days_in_month(int year, int month) {
    int days[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    
    if (month == 2 && is_leap_year(year)) {
        return 29;
    }
    
    return days[month - 1];
}

// 获取某月1日是星期几 (0=Sunday, 1=Monday, ...)
int get_first_weekday(int year, int month) {
    struct tm tm_info = {0};
    tm_info.tm_year = year - 1900;
    tm_info.tm_mon = month - 1;
    tm_info.tm_mday = 1;
    
    time_t time_val = mktime(&tm_info);
    struct tm *result = localtime(&time_val);
    
    return result->tm_wday;
}

// 显示单月日历
void show_month_calendar(int year, int month) {
    int days_in_month = get_days_in_month(year, month);
    int first_weekday = get_first_weekday(year, month);
    
    // 显示月份和年份标题
    printf("%s%s %d%s\n", COLOR_CYAN, month_names[month - 1], year, COLOR_RESET);
    printf("%s%s%s\n", COLOR_YELLOW, "========================", COLOR_RESET);
    
    // 显示星期标题
    for (int i = 0; i < 7; i++) {
        printf("%s%3s%s ", COLOR_MAGENTA, weekday_names[i], COLOR_RESET);
    }
    printf("\n");
    
    // 显示日期
    int day = 1;
    int current_weekday = 0;
    
    // 填充第一周的空格
    for (int i = 0; i < first_weekday; i++) {
        printf("    ");
        current_weekday++;
    }
    
    // 显示日期
    while (day <= days_in_month) {
        if (current_weekday == 0) {
            printf("\n");
        }
        
        // 高亮今天
        time_t now = time(NULL);
        struct tm *today = localtime(&now);
        if (year == today->tm_year + 1900 && 
            month == today->tm_mon + 1 && 
            day == today->tm_mday) {
            printf("%s%3d%s ", COLOR_RED, day, COLOR_RESET);
        } else {
            printf("%3d ", day);
        }
        
        day++;
        current_weekday = (current_weekday + 1) % 7;
    }
    
    printf("\n\n");
}

// 显示三个月日历
void show_three_months(int year, int month) {
    printf("%s三个月日历: %d年%d月%s\n", COLOR_CYAN, year, month, COLOR_RESET);
    printf("%s%s%s\n\n", COLOR_YELLOW, "================================", COLOR_RESET);
    
    // 显示前一个月
    int prev_year = year;
    int prev_month = month - 1;
    if (prev_month < 1) {
        prev_month = 12;
        prev_year--;
    }
    show_month_calendar(prev_year, prev_month);
    
    // 显示当前月
    show_month_calendar(year, month);
    
    // 显示下一个月
    int next_year = year;
    int next_month = month + 1;
    if (next_month > 12) {
        next_month = 1;
        next_year++;
    }
    show_month_calendar(next_year, next_month);
}

// 显示整年日历
void show_year_calendar(int year) {
    printf("%s%d年日历%s\n", COLOR_CYAN, year, COLOR_RESET);
    printf("%s%s%s\n\n", COLOR_YELLOW, "================================", COLOR_RESET);
    
    for (int month = 1; month <= 12; month++) {
        show_month_calendar(year, month);
    }
}

// 解析命令行参数
int parse_arguments(int argc, char *argv[], CalOptions *opts) {
    int opt;
    static struct option long_options[] = {
        {"three-months", no_argument, 0, '3'},
        {"year", no_argument, 0, 'y'},
        {"help", no_argument, 0, 'h'},
        {"version", no_argument, 0, 'v'},
        {0, 0, 0, 0}
    };

    while ((opt = getopt_long(argc, argv, "3yhv", long_options, NULL)) != -1) {
        switch (opt) {
            case '3':
                opts->show_3months = 1;
                break;
            case 'y':
                opts->show_year = 1;
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

    // 处理位置参数
    if (optind < argc) {
        if (optind + 1 == argc) {
            // 只有一个参数，可能是年份
            opts->year = atoi(argv[optind]);
            if (opts->year < 1 || opts->year > 9999) {
                printf("错误: 年份必须在1-9999之间\n");
                return 1;
            }
        } else if (optind + 2 == argc) {
            // 两个参数，月份和年份
            opts->month = atoi(argv[optind]);
            opts->year = atoi(argv[optind + 1]);
            
            if (opts->month < 1 || opts->month > 12) {
                printf("错误: 月份必须在1-12之间\n");
                return 1;
            }
            if (opts->year < 1 || opts->year > 9999) {
                printf("错误: 年份必须在1-9999之间\n");
                return 1;
            }
        }
    }

    return 0;
}

int main(int argc, char *argv[]) {
    CalOptions opts;
    
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
    
    if (opts.show_year) {
        show_year_calendar(opts.year);
    } else if (opts.show_3months) {
        show_three_months(opts.year, opts.month);
    } else {
        show_month_calendar(opts.year, opts.month);
    }
    
    return 0;
}
