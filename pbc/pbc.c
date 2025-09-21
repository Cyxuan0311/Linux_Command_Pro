#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
#include <getopt.h>
#include "../include/common.h"

#define MAX_EXPRESSION_LENGTH 1024
#define MAX_HISTORY 100

// 计算器选项
typedef struct {
    int interactive;
    int show_help;
    int show_version;
    int precision;
    int show_history;
    int quiet;
} CalcOptions;

// 计算历史
typedef struct {
    char expression[MAX_EXPRESSION_LENGTH];
    double result;
} CalcHistory;

static CalcHistory history[MAX_HISTORY];
static int history_count = 0;

// 初始化选项
void init_options(CalcOptions *opts) {
    memset(opts, 0, sizeof(CalcOptions));
    opts->precision = 6;
    opts->interactive = 1;
}

// 显示帮助信息
void print_help(const char *program_name) {
    printf("用法: %s [选项] [表达式]\n", program_name);
    printf("增强版基础计算器，支持基本数学运算\n\n");
    printf("选项:\n");
    printf("  -i, --interactive     交互模式 (默认)\n");
    printf("  -p, --precision=N     设置精度 (默认: 6)\n");
    printf("  -h, --history         显示计算历史\n");
    printf("  -q, --quiet           静默模式\n");
    printf("  --help                显示此帮助信息\n");
    printf("  --version             显示版本信息\n\n");
    printf("支持的运算符:\n");
    printf("  +      加法\n");
    printf("  -      减法\n");
    printf("  *      乘法\n");
    printf("  /      除法\n");
    printf("  %%      取模\n");
    printf("  ^      幂运算\n");
    printf("  sqrt() 平方根\n");
    printf("  sin()  正弦\n");
    printf("  cos()  余弦\n");
    printf("  tan()  正切\n");
    printf("  log()  自然对数\n");
    printf("  log10() 常用对数\n");
    printf("  exp()  指数\n");
    printf("  abs()  绝对值\n");
    printf("  pi      圆周率\n");
    printf("  e       自然常数\n\n");
    printf("示例:\n");
    printf("  %s                    # 启动交互模式\n", program_name);
    printf("  %s \"2 + 3 * 4\"        # 计算表达式\n", program_name);
    printf("  %s \"sqrt(16)\"         # 计算平方根\n", program_name);
    printf("  %s \"sin(pi/2)\"        # 计算三角函数\n", program_name);
}

// 显示版本信息
void print_version() {
    printf("pbc - 增强版基础计算器 v1.0\n");
    printf("支持基本数学运算和函数\n");
}

// 添加计算历史
void add_history(const char *expression, double result) {
    if (history_count < MAX_HISTORY) {
        strncpy(history[history_count].expression, expression, MAX_EXPRESSION_LENGTH - 1);
        history[history_count].result = result;
        history_count++;
    }
}

// 显示计算历史
void show_history() {
    if (history_count == 0) {
        printf("%s暂无计算历史%s\n", COLOR_YELLOW, COLOR_RESET);
        return;
    }
    
    printf("%s计算历史:%s\n", COLOR_CYAN, COLOR_RESET);
    printf("%s%s%s\n", COLOR_YELLOW, "========================", COLOR_RESET);
    
    for (int i = 0; i < history_count; i++) {
        printf("%s%2d.%s %s = %s%.*f%s\n", 
               COLOR_MAGENTA, i + 1, COLOR_RESET,
               history[i].expression,
               COLOR_GREEN, 6, history[i].result, COLOR_RESET);
    }
}

// 简单的表达式解析器
double parse_expression(const char *expr) {
    // 这里实现一个简单的表达式解析器
    // 为了简化，只支持基本的四则运算
    
    double result = 0;
    char *endptr;
    
    // 尝试直接解析为数字
    result = strtod(expr, &endptr);
    
    if (*endptr == '\0') {
        return result;
    }
    
    // 简单的表达式解析（仅支持两个操作数）
    char *op = NULL;
    char *left_str = NULL;
    char *right_str = NULL;
    
    // 查找运算符
    for (int i = 0; expr[i]; i++) {
        if (expr[i] == '+' || expr[i] == '-' || expr[i] == '*' || expr[i] == '/' || expr[i] == '%' || expr[i] == '^') {
            op = (char*)&expr[i];
            break;
        }
    }
    
    if (op) {
        // 分割左右操作数
        int op_pos = op - expr;
        left_str = malloc(op_pos + 1);
        strncpy(left_str, expr, op_pos);
        left_str[op_pos] = '\0';
        
        right_str = strdup(op + 1);
        
        double left = strtod(left_str, NULL);
        double right = strtod(right_str, NULL);
        
        switch (*op) {
            case '+':
                result = left + right;
                break;
            case '-':
                result = left - right;
                break;
            case '*':
                result = left * right;
                break;
            case '/':
                if (right != 0) {
                    result = left / right;
                } else {
                    printf("错误: 除零错误\n");
                    result = 0;
                }
                break;
            case '%':
                result = fmod(left, right);
                break;
            case '^':
                result = pow(left, right);
                break;
        }
        
        free(left_str);
        free(right_str);
    }
    
    return result;
}

// 计算表达式
double calculate(const char *expression) {
    char expr[MAX_EXPRESSION_LENGTH];
    strncpy(expr, expression, MAX_EXPRESSION_LENGTH - 1);
    expr[MAX_EXPRESSION_LENGTH - 1] = '\0';
    
    // 移除空格
    char *dst = expr;
    for (const char *src = expr; *src; src++) {
        if (*src != ' ') {
            *dst++ = *src;
        }
    }
    *dst = '\0';
    
    // 处理特殊函数和常数
    if (strcmp(expr, "pi") == 0) {
        return M_PI;
    }
    if (strcmp(expr, "e") == 0) {
        return M_E;
    }
    
    // 处理函数
    if (strncmp(expr, "sqrt(", 5) == 0) {
        char *arg = expr + 5;
        char *end = strchr(arg, ')');
        if (end) {
            *end = '\0';
            double value = strtod(arg, NULL);
            return sqrt(value);
        }
    }
    
    if (strncmp(expr, "sin(", 4) == 0) {
        char *arg = expr + 4;
        char *end = strchr(arg, ')');
        if (end) {
            *end = '\0';
            double value = strtod(arg, NULL);
            return sin(value);
        }
    }
    
    if (strncmp(expr, "cos(", 4) == 0) {
        char *arg = expr + 4;
        char *end = strchr(arg, ')');
        if (end) {
            *end = '\0';
            double value = strtod(arg, NULL);
            return cos(value);
        }
    }
    
    if (strncmp(expr, "tan(", 4) == 0) {
        char *arg = expr + 4;
        char *end = strchr(arg, ')');
        if (end) {
            *end = '\0';
            double value = strtod(arg, NULL);
            return tan(value);
        }
    }
    
    if (strncmp(expr, "log(", 4) == 0) {
        char *arg = expr + 4;
        char *end = strchr(arg, ')');
        if (end) {
            *end = '\0';
            double value = strtod(arg, NULL);
            return log(value);
        }
    }
    
    if (strncmp(expr, "log10(", 6) == 0) {
        char *arg = expr + 6;
        char *end = strchr(arg, ')');
        if (end) {
            *end = '\0';
            double value = strtod(arg, NULL);
            return log10(value);
        }
    }
    
    if (strncmp(expr, "exp(", 4) == 0) {
        char *arg = expr + 4;
        char *end = strchr(arg, ')');
        if (end) {
            *end = '\0';
            double value = strtod(arg, NULL);
            return exp(value);
        }
    }
    
    if (strncmp(expr, "abs(", 4) == 0) {
        char *arg = expr + 4;
        char *end = strchr(arg, ')');
        if (end) {
            *end = '\0';
            double value = strtod(arg, NULL);
            return fabs(value);
        }
    }
    
    return parse_expression(expr);
}

// 交互模式
void interactive_mode(const CalcOptions *opts) {
    char input[MAX_EXPRESSION_LENGTH];
    
    printf("%s增强版计算器 v1.0%s\n", COLOR_CYAN, COLOR_RESET);
    printf("%s输入 'help' 查看帮助, 'quit' 退出%s\n", COLOR_YELLOW, COLOR_RESET);
    printf("%s%s%s\n", COLOR_YELLOW, "========================", COLOR_RESET);
    
    while (1) {
        printf("%spbc>%s ", COLOR_GREEN, COLOR_RESET);
        
        if (fgets(input, sizeof(input), stdin) == NULL) {
            break;
        }
        
        // 移除换行符
        input[strcspn(input, "\n")] = '\0';
        
        // 检查退出命令
        if (strcmp(input, "quit") == 0 || strcmp(input, "exit") == 0 || strcmp(input, "q") == 0) {
            break;
        }
        
        // 检查帮助命令
        if (strcmp(input, "help") == 0 || strcmp(input, "h") == 0) {
            print_help("pbc");
            continue;
        }
        
        // 检查历史命令
        if (strcmp(input, "history") == 0) {
            show_history();
            continue;
        }
        
        // 检查清空历史命令
        if (strcmp(input, "clear") == 0) {
            history_count = 0;
            printf("%s历史已清空%s\n", COLOR_YELLOW, COLOR_RESET);
            continue;
        }
        
        // 跳过空输入
        if (strlen(input) == 0) {
            continue;
        }
        
        // 计算表达式
        double result = calculate(input);
        printf("%s%.*f%s\n", COLOR_GREEN, opts->precision, result, COLOR_RESET);
        
        // 添加到历史
        add_history(input, result);
    }
}

// 解析命令行参数
int parse_arguments(int argc, char *argv[], CalcOptions *opts) {
    int opt;
    static struct option long_options[] = {
        {"interactive", no_argument, 0, 'i'},
        {"precision", required_argument, 0, 'p'},
        {"history", no_argument, 0, 'h'},
        {"quiet", no_argument, 0, 'q'},
        {"help", no_argument, 0, 1},
        {"version", no_argument, 0, 2},
        {0, 0, 0, 0}
    };

    while ((opt = getopt_long(argc, argv, "ip:hqv", long_options, NULL)) != -1) {
        switch (opt) {
            case 'i':
                opts->interactive = 1;
                break;
            case 'p':
                opts->precision = atoi(optarg);
                if (opts->precision < 0 || opts->precision > 15) {
                    printf("错误: 精度必须在0-15之间\n");
                    return 1;
                }
                break;
            case 'h':
                opts->show_history = 1;
                break;
            case 'q':
                opts->quiet = 1;
                break;
            case 1: // --help
                opts->show_help = 1;
                break;
            case 2: // --version
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
    CalcOptions opts;
    
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
    
    if (opts.show_history) {
        show_history();
        return 0;
    }
    
    // 检查是否有表达式参数
    if (optind < argc) {
        // 有表达式参数，计算并显示结果
        char expression[MAX_EXPRESSION_LENGTH] = {0};
        
        // 合并所有参数
        for (int i = optind; i < argc; i++) {
            if (i > optind) {
                strcat(expression, " ");
            }
            strcat(expression, argv[i]);
        }
        
        double result = calculate(expression);
        printf("%s%.*f%s\n", COLOR_GREEN, opts.precision, result, COLOR_RESET);
        
        // 添加到历史
        add_history(expression, result);
    } else {
        // 没有参数，启动交互模式
        interactive_mode(&opts);
    }
    
    return 0;
}
