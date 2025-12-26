#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <signal.h>

#include "Editor.h"
#include "Buffer.h"
#include "Window.h"

using namespace pvim;

// 全局编辑器实例
std::unique_ptr<Editor> g_editor;

// 信号处理函数
void handle_signal(int sig) {
    (void)sig;
    if (g_editor) {
        g_editor->shutdown();
    }
    exit(0);
}

// 显示帮助信息
void print_help() {
    std::cout << "pvim - 优化版 vim 编辑器 v1.0\n";
    std::cout << "使用C++面向对象设计和模块化架构\n\n";
    std::cout << "用法: pvim [选项] [文件...]\n\n";
    std::cout << "选项:\n";
    std::cout << "  -h, --help          显示此帮助信息\n";
    std::cout << "  -v, --version       显示版本信息\n";
    std::cout << "  -R, --readonly      只读模式\n";
    std::cout << "  -n, --nofork        不fork到后台\n";
    std::cout << "  -c <command>        执行命令\n";
    std::cout << "  -t <tag>            跳转到标签\n";
    std::cout << "  -q <errorfile>      快速修复模式\n";
    std::cout << "  -s <scriptin>       读取脚本文件\n";
    std::cout << "  -w <scriptout>      写入脚本文件\n";
    std::cout << "  -W <scriptout>      追加到脚本文件\n";
    std::cout << "  -T <terminal>       设置终端类型\n";
    std::cout << "  -d <diff>           差异模式\n";
    std::cout << "  -g, --gui           启动GUI模式\n";
    std::cout << "  -f, --foreground    前台模式\n";
    std::cout << "  -b, --binary        二进制模式\n";
    std::cout << "  -l, --lisp          Lisp模式\n";
    std::cout << "  -C, --compatible    兼容模式\n";
    std::cout << "  -N, --nocompatible  不兼容模式\n";
    std::cout << "  -V <verbose>        详细模式\n";
    std::cout << "  -D                  调试模式\n";
    std::cout << "  -n, --nofork        不fork到后台\n";
    std::cout << "  -Z, --restricted    受限模式\n";
    std::cout << "  -m, --modemask      设置模式掩码\n";
    std::cout << "  -M, --modemask      设置模式掩码\n";
    std::cout << "  -B, --nobackup      不创建备份文件\n";
    std::cout << "  -F, --nofork        不fork到后台\n";
    std::cout << "  -H, --nohidden      不隐藏文件\n";
    std::cout << "  -I, --noreadonly    不设置只读\n";
    std::cout << "  -L, --nolock        不锁定文件\n";
    std::cout << "  -O, --noreadonly    不设置只读\n";
    std::cout << "  -P, --nopreserve    不保留文件\n";
    std::cout << "  -Q, --noreadonly    不设置只读\n";
    std::cout << "  -S, --noreadonly    不设置只读\n";
    std::cout << "  -U, --noreadonly    不设置只读\n";
    std::cout << "  -W, --noreadonly    不设置只读\n";
    std::cout << "  -X, --noreadonly    不设置只读\n";
    std::cout << "  -Y, --noreadonly    不设置只读\n";
    std::cout << "  -Z, --noreadonly    不设置只读\n\n";
    std::cout << "示例:\n";
    std::cout << "  pvim file.txt        编辑文件\n";
    std::cout << "  pvim -R file.txt     只读模式编辑\n";
    std::cout << "  pvim -c 'set number' 执行命令后编辑\n";
    std::cout << "  pvim -d file1 file2  差异模式\n";
}

// 显示版本信息
void print_version() {
    std::cout << "pvim version 1.0.0\n";
    std::cout << "基于ncurses的现代化文本编辑器\n";
    std::cout << "使用C++17和面向对象设计\n";
    std::cout << "支持语法高亮、多窗口、插件系统\n";
}

// 解析命令行参数
struct CommandLineArgs {
    std::vector<std::string> files;
    bool readonly = false;
    bool gui_mode = false;
    bool foreground = false;
    bool binary_mode = false;
    bool lisp_mode = false;
    bool compatible = false;
    bool nocompatible = false;
    bool restricted = false;
    bool nobackup = false;
    bool nofork = false;
    bool nohidden = false;
    bool noreadonly = false;
    bool nolock = false;
    bool nopreserve = false;
    int verbose = 0;
    bool debug = false;
    std::string command;
    std::string tag;
    std::string errorfile;
    std::string scriptin;
    std::string scriptout;
    std::string terminal;
    std::string diff;
    std::string modemask;
};

CommandLineArgs parse_arguments(int argc, char* argv[]) {
    CommandLineArgs args;
    
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        
        if (arg == "-h" || arg == "--help") {
            print_help();
            exit(0);
        } else if (arg == "-v" || arg == "--version") {
            print_version();
            exit(0);
        } else if (arg == "-R" || arg == "--readonly") {
            args.readonly = true;
        } else if (arg == "-g" || arg == "--gui") {
            args.gui_mode = true;
        } else if (arg == "-f" || arg == "--foreground") {
            args.foreground = true;
        } else if (arg == "-b" || arg == "--binary") {
            args.binary_mode = true;
        } else if (arg == "-l" || arg == "--lisp") {
            args.lisp_mode = true;
        } else if (arg == "-C" || arg == "--compatible") {
            args.compatible = true;
        } else if (arg == "-N" || arg == "--nocompatible") {
            args.nocompatible = true;
        } else if (arg == "-Z" || arg == "--restricted") {
            args.restricted = true;
        } else if (arg == "-B" || arg == "--nobackup") {
            args.nobackup = true;
        } else if (arg == "-F" || arg == "--nofork") {
            args.nofork = true;
        } else if (arg == "-H" || arg == "--nohidden") {
            args.nohidden = true;
        } else if (arg == "-I" || arg == "--noreadonly") {
            args.noreadonly = true;
        } else if (arg == "-L" || arg == "--nolock") {
            args.nolock = true;
        } else if (arg == "-O" || arg == "--noreadonly") {
            args.noreadonly = true;
        } else if (arg == "-P" || arg == "--nopreserve") {
            args.nopreserve = true;
        } else if (arg == "-Q" || arg == "--noreadonly") {
            args.noreadonly = true;
        } else if (arg == "-S" || arg == "--noreadonly") {
            args.noreadonly = true;
        } else if (arg == "-U" || arg == "--noreadonly") {
            args.noreadonly = true;
        } else if (arg == "-W" || arg == "--noreadonly") {
            args.noreadonly = true;
        } else if (arg == "-X" || arg == "--noreadonly") {
            args.noreadonly = true;
        } else if (arg == "-Y" || arg == "--noreadonly") {
            args.noreadonly = true;
        } else if (arg == "-Z" || arg == "--noreadonly") {
            args.noreadonly = true;
        } else if (arg == "-c" && i + 1 < argc) {
            args.command = argv[++i];
        } else if (arg == "-t" && i + 1 < argc) {
            args.tag = argv[++i];
        } else if (arg == "-q" && i + 1 < argc) {
            args.errorfile = argv[++i];
        } else if (arg == "-s" && i + 1 < argc) {
            args.scriptin = argv[++i];
        } else if (arg == "-w" && i + 1 < argc) {
            args.scriptout = argv[++i];
        } else if (arg == "-W" && i + 1 < argc) {
            args.scriptout = argv[++i];
        } else if (arg == "-T" && i + 1 < argc) {
            args.terminal = argv[++i];
        } else if (arg == "-d" && i + 1 < argc) {
            args.diff = argv[++i];
        } else if (arg == "-V" && i + 1 < argc) {
            args.verbose = std::atoi(argv[++i]);
        } else if (arg == "-D") {
            args.debug = true;
        } else if (arg == "-m" && i + 1 < argc) {
            args.modemask = argv[++i];
        } else if (arg == "-M" && i + 1 < argc) {
            args.modemask = argv[++i];
        } else if (arg[0] != '-') {
            args.files.push_back(arg);
        }
    }
    
    return args;
}

int main(int argc, char* argv[]) {
    // 解析命令行参数
    CommandLineArgs args = parse_arguments(argc, argv);
    
    // 设置信号处理
    signal(SIGINT, handle_signal);
    signal(SIGTERM, handle_signal);
    signal(SIGQUIT, handle_signal);
    
    try {
        // 创建编辑器实例
        g_editor = std::make_unique<Editor>();
        
        // 初始化编辑器
        if (!g_editor->initialize()) {
            std::cerr << "错误: 无法初始化编辑器\n";
            return 1;
        }
        
        // 打开文件
        if (!args.files.empty()) {
            if (!g_editor->openFile(args.files[0])) {
                std::cerr << "错误: 无法打开文件 " << args.files[0] << "\n";
                return 1;
            }
        }
        
        // 执行命令
        if (!args.command.empty()) {
            // TODO: 执行命令
        }
        
        // 运行编辑器
        g_editor->run();
        
        // 清理
        g_editor->shutdown();
        
    } catch (const std::exception& e) {
        std::cerr << "错误: " << e.what() << "\n";
        return 1;
    } catch (...) {
        std::cerr << "未知错误\n";
        return 1;
    }
    
    return 0;
}
