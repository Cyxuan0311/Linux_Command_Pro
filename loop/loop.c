#include "loop.h"
#include "../include/common.h"
#include <getopt.h>
#include <sys/select.h>

// 显示帮助信息
void print_help(const char *program_name) {
    printf("用法: %s [选项] [文本]\n", program_name);
    printf("循环显示文字动画效果\n\n");
    printf("选项:\n");
    printf("  -t, --text TEXT         要显示的文字 (默认: \"Hello World\")\n");
    printf("  -a, --animation TYPE    动画类型 (typewriter/expand/fade/wave/blink/scroll/bounce)\n");
    printf("  -c, --color COLOR       颜色 (red/green/yellow/blue/magenta/cyan/white)\n");
    printf("  -d, --delay MS          延迟时间，毫秒 (默认: 100)\n");
    printf("  -i, --interactive       交互模式（运行时可按键切换）\n");
    printf("  -h, --help              显示此帮助信息\n");
    printf("  -v, --version           显示版本信息\n\n");
    printf("动画类型:\n");
    printf("  typewriter  打字效果（逐字符显示）\n");
    printf("  expand      展开效果（从中心展开）\n");
    printf("  fade        淡入效果（逐字符显示并带颜色变化）\n");
    printf("  wave        波浪效果（字符依次显示）\n");
    printf("  blink       闪烁效果（整个文字闪烁）\n");
    printf("  scroll      滚动效果（从右到左滚动）\n");
    printf("  bounce      弹跳效果（字符依次弹跳显示）\n\n");
    printf("交互模式快捷键:\n");
    printf("  c           切换颜色\n");
    printf("  a           切换动画类型\n");
    printf("  +/-         增加/减少延迟\n");
    printf("  q           退出\n\n");
    printf("示例:\n");
    printf("  %s -t \"Hello\" -a typewriter -c red\n", program_name);
    printf("  %s -t \"World\" -a wave -c cyan -d 50\n", program_name);
    printf("  %s -i                    # 交互模式\n", program_name);
}

// 显示版本信息
void print_version(void) {
    printf("loop - 文字循环动画工具 v1.0\n");
}

// 执行动画
void run_animation(LoopOptions *opts) {
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    
    if (opts->interactive) {
        set_raw_mode();
    }
    
    while (running) {
        switch (opts->anim_type) {
            case ANIM_TYPEWRITER:
                animate_typewriter(opts->text, opts->color, opts->delay);
                break;
            case ANIM_EXPAND:
                animate_expand(opts->text, opts->color, opts->delay);
                break;
            case ANIM_FADE:
                animate_fade(opts->text, opts->color, opts->delay);
                break;
            case ANIM_WAVE:
                animate_wave(opts->text, opts->color, opts->delay);
                break;
            case ANIM_BLINK:
                animate_blink(opts->text, opts->color, opts->delay);
                break;
            case ANIM_SCROLL:
                animate_scroll(opts->text, opts->color, opts->delay);
                break;
            case ANIM_BOUNCE:
                animate_bounce(opts->text, opts->color, opts->delay);
                break;
        }
        
        if (!opts->interactive) {
            usleep(opts->delay * 10);  // 动画之间的延迟
        } else {
            // 检查键盘输入
            fd_set readfds;
            struct timeval tv;
            FD_ZERO(&readfds);
            FD_SET(STDIN_FILENO, &readfds);
            tv.tv_sec = 0;
            tv.tv_usec = 10000;  // 10ms
            
            if (select(STDIN_FILENO + 1, &readfds, NULL, NULL, &tv) > 0) {
                char ch = getchar();
                switch (ch) {
                    case 'c':
                    case 'C':
                        opts->color = (opts->color + 1) % (COLOR_WHITE_ENUM + 1);
                        printf("\n%s颜色切换为: %s%s\n", COLOR_CYAN, get_color_name(opts->color), COLOR_RESET);
                        break;
                    case 'a':
                    case 'A':
                        opts->anim_type = (opts->anim_type + 1) % (ANIM_BOUNCE + 1);
                        printf("\n%s动画切换为: %s%s\n", COLOR_CYAN, get_animation_name(opts->anim_type), COLOR_RESET);
                        break;
                    case '+':
                    case '=':
                        opts->delay = opts->delay > 10000 ? opts->delay - 10000 : 10000;
                        printf("\n%s延迟: %d ms%s\n", COLOR_CYAN, opts->delay / 1000, COLOR_RESET);
                        break;
                    case '-':
                    case '_':
                        opts->delay += 10000;
                        if (opts->delay > 500000) opts->delay = 500000;
                        printf("\n%s延迟: %d ms%s\n", COLOR_CYAN, opts->delay / 1000, COLOR_RESET);
                        break;
                    case 'q':
                    case 'Q':
                        running = 0;
                        break;
                }
            }
        }
    }
    
    clear_screen();
    printf("%s动画已停止%s\n", COLOR_YELLOW, COLOR_RESET);
}

// 解析命令行参数
int parse_arguments(int argc, char *argv[], LoopOptions *opts) {
    int opt;
    static struct option long_options[] = {
        {"text", required_argument, 0, 't'},
        {"animation", required_argument, 0, 'a'},
        {"color", required_argument, 0, 'c'},
        {"delay", required_argument, 0, 'd'},
        {"interactive", no_argument, 0, 'i'},
        {"help", no_argument, 0, 'h'},
        {"version", no_argument, 0, 'v'},
        {0, 0, 0, 0}
    };
    
    while ((opt = getopt_long(argc, argv, "t:a:c:d:ihv", long_options, NULL)) != -1) {
        switch (opt) {
            case 't':
                strncpy(opts->text, optarg, MAX_TEXT_LENGTH - 1);
                opts->text[MAX_TEXT_LENGTH - 1] = '\0';
                break;
            case 'a':
                opts->anim_type = parse_animation_type(optarg);
                break;
            case 'c':
                opts->color = parse_color(optarg);
                break;
            case 'd':
                opts->delay = atoi(optarg) * 1000;  // 转换为微秒
                if (opts->delay < 10000) opts->delay = 10000;
                if (opts->delay > 500000) opts->delay = 500000;
                break;
            case 'i':
                opts->interactive = 1;
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
    
    // 处理位置参数（文本）
    if (optind < argc) {
        strncpy(opts->text, argv[optind], MAX_TEXT_LENGTH - 1);
        opts->text[MAX_TEXT_LENGTH - 1] = '\0';
    }
    
    return 0;
}

int main(int argc, char *argv[]) {
    LoopOptions opts;
    
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
    
    run_animation(&opts);
    
    return 0;
}
