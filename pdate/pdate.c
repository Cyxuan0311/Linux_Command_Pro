#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/time.h>
#include <getopt.h>
#include "../include/common.h"

#define MAX_FORMAT_LENGTH 256
#define MAX_TIMEZONE_LENGTH 64

// 时间格式选项
typedef struct {
    int show_date;
    int show_time;
    int show_iso;
    int show_rfc;
    int show_timestamp;
    int show_utc;
    int show_weekday;
    int show_year;
    int show_month;
    int show_day;
    int show_hour;
    int show_minute;
    int show_second;
    int show_microsecond;
    int show_timezone;
    int show_epoch;
    int show_help;
    int show_version;
    char custom_format[MAX_FORMAT_LENGTH];
    char timezone[MAX_TIMEZONE_LENGTH];
} DateOptions;

// 初始化选项
void init_options(DateOptions *opts) {
    memset(opts, 0, sizeof(DateOptions));
    opts->show_date = 1;
    opts->show_time = 1;
}

// 显示帮助信息
void print_help(const char *program_name) {
    printf("用法: %s [选项] [格式]\n", program_name);
    printf("增强版 date 命令，提供丰富的日期时间显示功能\n\n");
    printf("选项:\n");
    printf("  -d, --date=STRING     显示指定日期时间\n");
    printf("  -f, --file=FILE       从文件读取日期格式\n");
    printf("  -I, --iso-8601[=FMT]  ISO 8601格式 (默认: date)\n");
    printf("  -R, --rfc-email       RFC 5322格式\n");
    printf("  -r, --reference=FILE  显示文件最后修改时间\n");
    printf("  -s, --set=STRING      设置系统时间\n");
    printf("  -u, --utc             显示UTC时间\n");
    printf("  --date-format=FORMAT  自定义日期格式\n");
    printf("  --time-format=FORMAT  自定义时间格式\n");
    printf("  --timestamp           显示时间戳\n");
    printf("  --epoch               显示Unix时间戳\n");
    printf("  --weekday             显示星期几\n");
    printf("  --timezone=TZ         指定时区\n");
    printf("  -h, --help            显示此帮助信息\n");
    printf("  -v, --version         显示版本信息\n\n");
    printf("格式说明:\n");
    printf("  %%Y  年份 (4位数字)\n");
    printf("  %%y  年份 (2位数字)\n");
    printf("  %%m  月份 (01-12)\n");
    printf("  %%B  月份名称 (January-December)\n");
    printf("  %%b  月份简称 (Jan-Dec)\n");
    printf("  %%d  日期 (01-31)\n");
    printf("  %%j  一年中的第几天 (001-366)\n");
    printf("  %%w  星期几 (0-6, 0=Sunday)\n");
    printf("  %%A  星期几全名 (Sunday-Saturday)\n");
    printf("  %%a  星期几简称 (Sun-Sat)\n");
    printf("  %%H  小时 (00-23)\n");
    printf("  %%I  小时 (01-12)\n");
    printf("  %%M  分钟 (00-59)\n");
    printf("  %%S  秒 (00-59)\n");
    printf("  %%N  纳秒 (000000000-999999999)\n");
    printf("  %%Z  时区名称\n");
    printf("  %%z  时区偏移\n\n");
    printf("示例:\n");
    printf("  %s                    # 显示当前日期时间\n", program_name);
    printf("  %s +%%Y-%%m-%%d         # 显示日期 (YYYY-MM-DD)\n", program_name);
    printf("  %s +%%H:%%M:%%S         # 显示时间 (HH:MM:SS)\n", program_name);
    printf("  %s --iso-8601          # ISO 8601格式\n", program_name);
    printf("  %s --timestamp         # 显示时间戳\n", program_name);
    printf("  %s --epoch             # 显示Unix时间戳\n", program_name);
    printf("  %s --timezone=UTC      # 显示UTC时间\n", program_name);
}

// 显示版本信息
void print_version() {
    printf("pdate - 增强版 date 命令 v1.0\n");
    printf("支持多种日期时间格式和时区\n");
}

// 解析命令行参数
int parse_arguments(int argc, char *argv[], DateOptions *opts) {
    int opt;
    static struct option long_options[] = {
        {"date", required_argument, 0, 'd'},
        {"file", required_argument, 0, 'f'},
        {"iso-8601", optional_argument, 0, 'I'},
        {"rfc-email", no_argument, 0, 'R'},
        {"reference", required_argument, 0, 'r'},
        {"set", required_argument, 0, 's'},
        {"utc", no_argument, 0, 'u'},
        {"date-format", required_argument, 0, 1},
        {"time-format", required_argument, 0, 2},
        {"timestamp", no_argument, 0, 3},
        {"epoch", no_argument, 0, 4},
        {"weekday", no_argument, 0, 5},
        {"timezone", required_argument, 0, 6},
        {"help", no_argument, 0, 'h'},
        {"version", no_argument, 0, 'v'},
        {0, 0, 0, 0}
    };

    while ((opt = getopt_long(argc, argv, "d:f:I::Rr:s:uhv", long_options, NULL)) != -1) {
        switch (opt) {
            case 'd':
                // 解析指定日期
                printf("指定日期: %s\n", optarg);
                break;
            case 'f':
                // 从文件读取格式
                printf("从文件读取格式: %s\n", optarg);
                break;
            case 'I':
                opts->show_iso = 1;
                if (optarg) {
                    strncpy(opts->custom_format, optarg, MAX_FORMAT_LENGTH - 1);
                } else {
                    strcpy(opts->custom_format, "date");
                }
                break;
            case 'R':
                opts->show_rfc = 1;
                break;
            case 'r':
                // 显示文件修改时间
                printf("文件修改时间: %s\n", optarg);
                break;
            case 's':
                // 设置系统时间
                printf("设置系统时间: %s\n", optarg);
                break;
            case 'u':
                opts->show_utc = 1;
                break;
            case 1: // --date-format
                strncpy(opts->custom_format, optarg, MAX_FORMAT_LENGTH - 1);
                opts->show_date = 1;
                opts->show_time = 0;
                break;
            case 2: // --time-format
                strncpy(opts->custom_format, optarg, MAX_FORMAT_LENGTH - 1);
                opts->show_date = 0;
                opts->show_time = 1;
                break;
            case 3: // --timestamp
                opts->show_timestamp = 1;
                break;
            case 4: // --epoch
                opts->show_epoch = 1;
                break;
            case 5: // --weekday
                opts->show_weekday = 1;
                break;
            case 6: // --timezone
                strncpy(opts->timezone, optarg, MAX_TIMEZONE_LENGTH - 1);
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

    // 处理剩余参数（格式字符串）
    if (optind < argc) {
        if (argv[optind][0] == '+') {
            strncpy(opts->custom_format, argv[optind] + 1, MAX_FORMAT_LENGTH - 1);
        } else {
            strncpy(opts->custom_format, argv[optind], MAX_FORMAT_LENGTH - 1);
        }
    }

    return 0;
}

// 格式化时间
void format_time_string(const struct tm *tm, const char *format, char *output, size_t size) {
    strftime(output, size, format, tm);
}

// 显示当前时间
void show_current_time(const DateOptions *opts) {
    time_t now;
    struct tm *tm_info;
    char buffer[256];
    
    time(&now);
    
    if (opts->show_utc) {
        tm_info = gmtime(&now);
    } else {
        tm_info = localtime(&now);
    }
    
    if (opts->show_epoch) {
        printf("%s%ld%s\n", COLOR_CYAN, now, COLOR_RESET);
        return;
    }
    
    if (opts->show_timestamp) {
        printf("%s%ld%s\n", COLOR_CYAN, now, COLOR_RESET);
        return;
    }
    
    if (opts->show_iso) {
        if (strcmp(opts->custom_format, "date") == 0) {
            strftime(buffer, sizeof(buffer), "%Y-%m-%d", tm_info);
        } else if (strcmp(opts->custom_format, "hours") == 0) {
            strftime(buffer, sizeof(buffer), "%Y-%m-%dT%H", tm_info);
        } else if (strcmp(opts->custom_format, "minutes") == 0) {
            strftime(buffer, sizeof(buffer), "%Y-%m-%dT%H:%M", tm_info);
        } else if (strcmp(opts->custom_format, "seconds") == 0) {
            strftime(buffer, sizeof(buffer), "%Y-%m-%dT%H:%M:%S", tm_info);
        } else {
            strftime(buffer, sizeof(buffer), "%Y-%m-%dT%H:%M:%S", tm_info);
        }
        printf("%s%s%s\n", COLOR_GREEN, buffer, COLOR_RESET);
        return;
    }
    
    if (opts->show_rfc) {
        strftime(buffer, sizeof(buffer), "%a, %d %b %Y %H:%M:%S %z", tm_info);
        printf("%s%s%s\n", COLOR_GREEN, buffer, COLOR_RESET);
        return;
    }
    
    if (strlen(opts->custom_format) > 0) {
        format_time_string(tm_info, opts->custom_format, buffer, sizeof(buffer));
        printf("%s%s%s\n", COLOR_GREEN, buffer, COLOR_RESET);
        return;
    }
    
    // 默认格式
    if (opts->show_date && opts->show_time) {
        strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", tm_info);
    } else if (opts->show_date) {
        strftime(buffer, sizeof(buffer), "%Y-%m-%d", tm_info);
    } else if (opts->show_time) {
        strftime(buffer, sizeof(buffer), "%H:%M:%S", tm_info);
    } else {
        strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", tm_info);
    }
    
    printf("%s%s%s", COLOR_CYAN, "当前时间: ", COLOR_RESET);
    printf("%s%s%s\n", COLOR_GREEN, buffer, COLOR_RESET);
    
    if (opts->show_weekday) {
        strftime(buffer, sizeof(buffer), "%A", tm_info);
        printf("%s%s%s %s%s%s\n", COLOR_CYAN, "星期: ", COLOR_RESET, COLOR_YELLOW, buffer, COLOR_RESET);
    }
    
    if (opts->show_timezone) {
        strftime(buffer, sizeof(buffer), "%Z", tm_info);
        printf("%s%s%s %s%s%s\n", COLOR_CYAN, "时区: ", COLOR_RESET, COLOR_MAGENTA, buffer, COLOR_RESET);
    }
}

int main(int argc, char *argv[]) {
    DateOptions opts;
    
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
    
    show_current_time(&opts);
    
    return 0;
}
