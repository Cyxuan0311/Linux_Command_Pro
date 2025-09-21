#include "CommandLineParser.h"
#include "Constants.h"
#include "Utils.h"
#include <iostream>
#include <fstream>
#include <sstream>

namespace pawk {

CommandLineParser::CommandLineParser(int argc, char* argv[]) {
    for (int i = 1; i < argc; ++i) {
        args_.push_back(argv[i]);
    }
    
    // 初始化默认选项
    options_.field_separator = Defaults::FIELD_SEPARATOR;
    options_.output_separator = Defaults::OUTPUT_SEPARATOR;
    options_.record_separator = Defaults::RECORD_SEPARATOR;
    options_.output_record_separator = Defaults::OUTPUT_RECORD_SEPARATOR;
    options_.color_output = Defaults::COLOR_OUTPUT;
    options_.show_line_numbers = Defaults::SHOW_LINE_NUMBERS;
    options_.show_field_numbers = Defaults::SHOW_FIELD_NUMBERS;
    options_.show_help = false;
    options_.show_version = false;
    options_.verbose = false;
    options_.debug = false;
    
    parse();
}

void CommandLineParser::parse() {
    for (size_t i = 0; i < args_.size(); ++i) {
        parseOption(args_[i], i);
    }
    
    validateOptions();
}

void CommandLineParser::parseOption(const std::string& arg, size_t& index) {
    if (arg == "-F" && index + 1 < args_.size()) {
        options_.field_separator = args_[++index];
    } else if (arg == "-v" && index + 1 < args_.size()) {
        parseVariableAssignment(args_[++index]);
    } else if (arg == "-f" && index + 1 < args_.size()) {
        options_.program_file = args_[++index];
    } else if (arg == "--color") {
        options_.color_output = true;
    } else if (arg == "--no-color") {
        options_.color_output = false;
    } else if (arg == "--line-numbers") {
        options_.show_line_numbers = true;
    } else if (arg == "--field-numbers") {
        options_.show_field_numbers = true;
    } else if (arg == "--verbose" || arg == "-v") {
        options_.verbose = true;
    } else if (arg == "--debug" || arg == "-d") {
        options_.debug = true;
    } else if (arg == "--help" || arg == "-h") {
        options_.show_help = true;
    } else if (arg == "--version" || arg == "-V") {
        options_.show_version = true;
    } else if (arg[0] != '-') {
        if (options_.program.empty() && options_.program_file.empty()) {
            options_.program = arg;
        } else {
            options_.input_files.push_back(arg);
        }
    }
}

void CommandLineParser::parseVariableAssignment(const std::string& assignment) {
    size_t pos = assignment.find('=');
    if (pos != std::string::npos) {
        std::string name = assignment.substr(0, pos);
        std::string value = assignment.substr(pos + 1);
        options_.variables[name] = value;
    }
}

void CommandLineParser::validateOptions() {
    if (options_.program.empty() && options_.program_file.empty() && !options_.show_help && !options_.show_version) {
        Utils::printError("没有指定程序或程序文件");
        options_.show_help = true;
    }
}

const CommandLineOptions& CommandLineParser::getOptions() const {
    return options_;
}

void CommandLineParser::printHelp() const {
    std::cout << Utils::colorize("pawk - 优化版 awk 命令", Colors::CYAN) << std::endl;
    std::cout << Utils::colorize("================================================", Colors::YELLOW) << std::endl;
    std::cout << "用法: pawk [选项] '程序' [文件...]" << std::endl;
    std::cout << "     pawk [选项] -f 程序文件 [文件...]" << std::endl;
    std::cout << std::endl;
    std::cout << "选项:" << std::endl;
    std::cout << "  -F fs           设置字段分隔符" << std::endl;
    std::cout << "  -v var=value    设置变量" << std::endl;
    std::cout << "  -f progfile     从文件读取程序" << std::endl;
    std::cout << "  --color         启用彩色输出" << std::endl;
    std::cout << "  --no-color      禁用彩色输出" << std::endl;
    std::cout << "  --line-numbers  显示行号" << std::endl;
    std::cout << "  --field-numbers 显示字段号" << std::endl;
    std::cout << "  --verbose       详细输出" << std::endl;
    std::cout << "  --debug         调试模式" << std::endl;
    std::cout << "  -h, --help      显示此帮助信息" << std::endl;
    std::cout << "  -V, --version   显示版本信息" << std::endl;
    std::cout << std::endl;
    std::cout << "示例:" << std::endl;
    std::cout << "  pawk '{print $1}' file.txt" << std::endl;
    std::cout << "  pawk -F: '{print $1, $3}' /etc/passwd" << std::endl;
    std::cout << "  pawk '/pattern/ {print $0}' file.txt" << std::endl;
    std::cout << "  pawk '{sum += $1} END {print sum}' numbers.txt" << std::endl;
    std::cout << std::endl;
    std::cout << "内置函数:" << std::endl;
    std::cout << "  length(), substr(), toupper(), tolower()" << std::endl;
    std::cout << "  sqrt(), sin(), cos(), log(), exp()" << std::endl;
    std::cout << "  systime(), strftime()" << std::endl;
    std::cout << std::endl;
    std::cout << "模式:" << std::endl;
    std::cout << "  /pattern/        正则表达式模式" << std::endl;
    std::cout << "  $1 ~ /pattern/   字段模式" << std::endl;
    std::cout << "  NR > 10         记录号模式" << std::endl;
    std::cout << "  NF > 5          字段数模式" << std::endl;
}

void CommandLineParser::printVersion() const {
    std::cout << "pawk - " << Version::DESCRIPTION << " v" << Version::VERSION << std::endl;
    std::cout << "使用C++17和面向对象设计" << std::endl;
    std::cout << "作者: " << Version::AUTHOR << std::endl;
}

} // namespace pawk
