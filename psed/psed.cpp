#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <regex>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <memory>
#include <functional>
#include <iomanip>
#include <cstring>
#include <cstdlib>
#include <cmath>
#include <ctime>
#include <climits>
#include <unistd.h>

// 颜色定义
const std::string COLOR_RED = "\033[31m";
const std::string COLOR_GREEN = "\033[32m";
const std::string COLOR_YELLOW = "\033[33m";
const std::string COLOR_BLUE = "\033[34m";
const std::string COLOR_MAGENTA = "\033[35m";
const std::string COLOR_CYAN = "\033[36m";
const std::string COLOR_WHITE = "\033[37m";
const std::string COLOR_RESET = "\033[0m";
const std::string COLOR_BOLD = "\033[1m";

// 前向声明
class SedCommand;
class SedEngine;
class PatternMatcher;
class TextProcessor;

// 地址范围类
class AddressRange {
private:
    int start_line_;
    int end_line_;
    bool is_regex_start_;
    bool is_regex_end_;
    std::string start_pattern_;
    std::string end_pattern_;
    
public:
    AddressRange() : start_line_(1), end_line_(INT_MAX), is_regex_start_(false), is_regex_end_(false) {}
    
    // 设置行号范围
    void setLineRange(int start, int end) {
        start_line_ = start;
        end_line_ = end;
        is_regex_start_ = false;
        is_regex_end_ = false;
    }
    
    // 设置正则表达式范围
    void setRegexRange(const std::string& start_pattern, const std::string& end_pattern) {
        start_pattern_ = start_pattern;
        end_pattern_ = end_pattern;
        is_regex_start_ = true;
        is_regex_end_ = true;
    }
    
    // 设置单行地址
    void setSingleLine(int line) {
        start_line_ = line;
        end_line_ = line;
        is_regex_start_ = false;
        is_regex_end_ = false;
    }
    
    // 设置正则表达式单行
    void setRegexLine(const std::string& pattern) {
        start_pattern_ = pattern;
        is_regex_start_ = true;
        is_regex_end_ = false;
    }
    
    // 检查是否匹配当前行
    bool matches(int line_number, const std::string& line) const {
        if (is_regex_start_) {
            std::regex pattern_regex(start_pattern_);
            return std::regex_search(line, pattern_regex);
        }
        return line_number >= start_line_ && line_number <= end_line_;
    }
    
    // 检查是否在范围内
    bool inRange(int line_number, const std::string& line) const {
        if (is_regex_start_ && is_regex_end_) {
            // 正则表达式范围
            static bool in_range = false;
            static std::regex start_regex(start_pattern_);
            static std::regex end_regex(end_pattern_);
            
            if (std::regex_search(line, start_regex)) {
                in_range = true;
                return true;
            } else if (in_range && std::regex_search(line, end_regex)) {
                in_range = false;
                return true;
            }
            return in_range;
        } else if (is_regex_start_) {
            // 单行正则表达式
            std::regex pattern_regex(start_pattern_);
            return std::regex_search(line, pattern_regex);
        } else {
            // 行号范围
            return line_number >= start_line_ && line_number <= end_line_;
        }
    }
};

// 模式匹配器基类
class PatternMatcher {
public:
    virtual ~PatternMatcher() = default;
    virtual bool matches(const std::string& line) = 0;
    virtual std::string getPattern() const = 0;
};

// 正则表达式匹配器
class RegexMatcher : public PatternMatcher {
private:
    std::string pattern_;
    std::regex regex_;
    
public:
    RegexMatcher(const std::string& pattern) : pattern_(pattern) {
        try {
            regex_ = std::regex(pattern);
        } catch (const std::regex_error& e) {
            std::cerr << "正则表达式错误: " << e.what() << std::endl;
        }
    }
    
    RegexMatcher(const std::string& pattern, std::regex_constants::syntax_option_type flags) : pattern_(pattern) {
        try {
            regex_ = std::regex(pattern, flags);
        } catch (const std::regex_error& e) {
            std::cerr << "正则表达式错误: " << e.what() << std::endl;
        }
    }
    
    bool matches(const std::string& line) override {
        return std::regex_search(line, regex_);
    }
    
    std::string getPattern() const override {
        return pattern_;
    }
};

// 简单字符串匹配器
class StringMatcher : public PatternMatcher {
private:
    std::string pattern_;
    
public:
    StringMatcher(const std::string& pattern) : pattern_(pattern) {}
    
    bool matches(const std::string& line) override {
        return line.find(pattern_) != std::string::npos;
    }
    
    std::string getPattern() const override {
        return pattern_;
    }
};

// 命令基类
class SedCommand {
public:
    virtual ~SedCommand() = default;
    virtual bool execute(const std::string& line, int line_number, std::string& result) = 0;
    virtual bool shouldPrint() const = 0;
    virtual std::string getDescription() const = 0;
};

// 替换命令
class SubstituteCommand : public SedCommand {
private:
    std::unique_ptr<PatternMatcher> pattern_matcher_;
    std::string replacement_;
    std::string flags_;
    int substitution_count_;
    bool global_replace_;
    bool case_insensitive_;
    
public:
    SubstituteCommand(const std::string& pattern, const std::string& replacement, 
                     const std::string& flags = "")
        : replacement_(replacement), flags_(flags), substitution_count_(0), 
          global_replace_(false), case_insensitive_(false) {
        
        // 解析标志
        for (char flag : flags) {
            switch (flag) {
                case 'g':
                    global_replace_ = true;
                    break;
                case 'i':
                    case_insensitive_ = true;
                    break;
                case '1':
                case '2':
                case '3':
                case '4':
                case '5':
                case '6':
                case '7':
                case '8':
                case '9':
                    substitution_count_ = flag - '0';
                    break;
            }
        }
        
        // 创建匹配器
        if (case_insensitive_) {
            pattern_matcher_ = std::make_unique<RegexMatcher>(pattern, std::regex_constants::icase);
        } else {
            pattern_matcher_ = std::make_unique<RegexMatcher>(pattern);
        }
    }
    
    bool execute(const std::string& line, int line_number, std::string& result) override {
        result = line;
        
        std::regex pattern_regex(pattern_matcher_->getPattern(), case_insensitive_ ? std::regex_constants::icase : std::regex_constants::ECMAScript);
        
        if (global_replace_) {
            result = std::regex_replace(line, pattern_regex, replacement_);
        } else if (substitution_count_ > 0) {
            std::sregex_iterator iter(line.begin(), line.end(), pattern_regex);
            std::sregex_iterator end;
            
            int count = 0;
            std::string new_result = line;
            size_t offset = 0;
            
            for (auto it = iter; it != end && count < substitution_count_; ++it, ++count) {
                auto match = *it;
                size_t pos = match.position() + offset;
                size_t len = match.length();
                
                new_result.replace(pos, len, replacement_);
                offset += replacement_.length() - len;
            }
            result = new_result;
        } else {
            result = std::regex_replace(line, pattern_regex, replacement_, 
                                      std::regex_constants::format_first_only);
        }
        
        return result != line;
    }
    
    bool shouldPrint() const override {
        return true;
    }
    
    std::string getDescription() const override {
        return "s/" + pattern_matcher_->getPattern() + "/" + replacement_ + "/" + flags_;
    }
};

// 删除命令
class DeleteCommand : public SedCommand {
private:
    std::unique_ptr<PatternMatcher> pattern_matcher_;
    
public:
    DeleteCommand(const std::string& pattern) {
        pattern_matcher_ = std::make_unique<RegexMatcher>(pattern);
    }
    
    bool execute(const std::string& line, int line_number, std::string& result) override {
        if (pattern_matcher_->matches(line)) {
            result = "";
            return true;
        }
        result = line;
        return false;
    }
    
    bool shouldPrint() const override {
        return false;
    }
    
    std::string getDescription() const override {
        return "d/" + pattern_matcher_->getPattern() + "/";
    }
};

// 打印命令
class PrintCommand : public SedCommand {
private:
    std::unique_ptr<PatternMatcher> pattern_matcher_;
    
public:
    PrintCommand(const std::string& pattern) {
        pattern_matcher_ = std::make_unique<RegexMatcher>(pattern);
    }
    
    bool execute(const std::string& line, int line_number, std::string& result) override {
        result = line;
        return pattern_matcher_->matches(line);
    }
    
    bool shouldPrint() const override {
        return true;
    }
    
    std::string getDescription() const override {
        return "p/" + pattern_matcher_->getPattern() + "/";
    }
};

// 插入命令
class InsertCommand : public SedCommand {
private:
    std::string text_;
    
public:
    InsertCommand(const std::string& text) : text_(text) {}
    
    bool execute(const std::string& line, int line_number, std::string& result) override {
        result = text_ + "\n" + line;
        return true;
    }
    
    bool shouldPrint() const override {
        return true;
    }
    
    std::string getDescription() const override {
        return "i/" + text_ + "/";
    }
};

// 追加命令
class AppendCommand : public SedCommand {
private:
    std::string text_;
    
public:
    AppendCommand(const std::string& text) : text_(text) {}
    
    bool execute(const std::string& line, int line_number, std::string& result) override {
        result = line + "\n" + text_;
        return true;
    }
    
    bool shouldPrint() const override {
        return true;
    }
    
    std::string getDescription() const override {
        return "a/" + text_ + "/";
    }
};

// 转换命令
class TransformCommand : public SedCommand {
private:
    std::string from_chars_;
    std::string to_chars_;
    
public:
    TransformCommand(const std::string& from, const std::string& to) 
        : from_chars_(from), to_chars_(to) {}
    
    bool execute(const std::string& line, int line_number, std::string& result) override {
        result = line;
        
        for (size_t i = 0; i < from_chars_.length() && i < to_chars_.length(); ++i) {
            std::replace(result.begin(), result.end(), from_chars_[i], to_chars_[i]);
        }
        
        return result != line;
    }
    
    bool shouldPrint() const override {
        return true;
    }
    
    std::string getDescription() const override {
        return "y/" + from_chars_ + "/" + to_chars_ + "/";
    }
};

// 保持空间操作命令
class HoldSpaceCommand : public SedCommand {
private:
    char operation_;
    static std::string hold_space_;
    
public:
    HoldSpaceCommand(char op) : operation_(op) {}
    
    bool execute(const std::string& line, int line_number, std::string& result) override {
        switch (operation_) {
            case 'h': // 将模式空间复制到保持空间
                hold_space_ = line;
                break;
            case 'H': // 将模式空间追加到保持空间
                hold_space_ += "\n" + line;
                break;
            case 'g': // 将保持空间复制到模式空间
                result = hold_space_;
                break;
            case 'G': // 将保持空间追加到模式空间
                result = line + "\n" + hold_space_;
                break;
            case 'x': // 交换模式空间和保持空间
                result = hold_space_;
                hold_space_ = line;
                break;
        }
        return true;
    }
    
    bool shouldPrint() const override {
        return true;
    }
    
    std::string getDescription() const override {
        return std::string("HoldSpace: ") + operation_;
    }
};

// 静态成员定义
std::string HoldSpaceCommand::hold_space_ = "";

// 文本处理器类
class TextProcessor {
private:
    std::vector<std::pair<AddressRange, std::unique_ptr<SedCommand>>> commands_;
    bool quiet_mode_;
    bool print_line_numbers_;
    bool color_output_;
    
public:
    TextProcessor() : quiet_mode_(false), print_line_numbers_(false), color_output_(true) {}
    
    void setQuietMode(bool quiet) { quiet_mode_ = quiet; }
    void setPrintLineNumbers(bool print) { print_line_numbers_ = print; }
    void setColorOutput(bool color) { color_output_ = color; }
    
    void addCommand(const AddressRange& range, std::unique_ptr<SedCommand> command) {
        commands_.emplace_back(range, std::move(command));
    }
    
    void processLine(const std::string& line, int line_number) {
        std::string result = line;
        bool modified = false;
        
        for (auto& cmd_pair : commands_) {
            const AddressRange& range = cmd_pair.first;
            SedCommand& command = *cmd_pair.second;
            
            if (range.inRange(line_number, line)) {
                std::string temp_result;
                if (command.execute(result, line_number, temp_result)) {
                    result = temp_result;
                    modified = true;
                }
            }
        }
        
        if (!quiet_mode_ && (modified || commands_.empty())) {
            printLine(result, line_number);
        }
    }
    
private:
    void printLine(const std::string& line, int line_number) {
        if (print_line_numbers_) {
            std::cout << COLOR_CYAN << std::setw(4) << line_number << ": " << COLOR_RESET;
        }
        
        if (color_output_) {
            // 简单的语法高亮
            std::string colored_line = highlightText(line);
            std::cout << colored_line << std::endl;
        } else {
            std::cout << line << std::endl;
        }
    }
    
    std::string highlightText(const std::string& text) {
        std::string result = text;
        
        // 高亮数字
        std::regex number_regex("\\b\\d+\\b");
        result = std::regex_replace(result, number_regex, 
                                   COLOR_GREEN + "$&" + COLOR_RESET);
        
        // 高亮字符串
        std::regex string_regex("\"[^\"]*\"");
        result = std::regex_replace(result, string_regex, 
                                   COLOR_YELLOW + "$&" + COLOR_RESET);
        
        // 高亮特殊字符
        std::regex special_regex("[\\[\\]{}()]");
        result = std::regex_replace(result, special_regex, 
                                   COLOR_MAGENTA + "$&" + COLOR_RESET);
        
        return result;
    }
};

// 主引擎类
class SedEngine {
private:
    TextProcessor processor_;
    std::vector<std::string> input_files_;
    bool in_place_edit_;
    std::string backup_suffix_;
    
public:
    SedEngine() : in_place_edit_(false) {}
    
    void setInPlaceEdit(bool in_place, const std::string& suffix = "") {
        in_place_edit_ = in_place;
        backup_suffix_ = suffix;
    }
    
    void addInputFile(const std::string& filename) {
        input_files_.push_back(filename);
    }
    
    void addCommand(const std::string& command_str) {
        parseAndAddCommand(command_str);
    }
    
    void setQuietMode(bool quiet) { processor_.setQuietMode(quiet); }
    void setPrintLineNumbers(bool print) { processor_.setPrintLineNumbers(print); }
    void setColorOutput(bool color) { processor_.setColorOutput(color); }
    
    void process() {
        if (input_files_.empty()) {
            processStdin();
        } else {
            for (const auto& file : input_files_) {
                processFile(file);
            }
        }
    }
    
private:
    void parseAndAddCommand(const std::string& command_str) {
        if (command_str.empty()) return;
        
        char command_type = command_str[0];
        std::string args = command_str.substr(1);
        
        AddressRange range;
        
        switch (command_type) {
            case 's': { // 替换命令
                if (args.empty() || args[0] != '/') break;
                
                size_t first_delim = 1; // 跳过第一个'/'
                size_t second_delim = args.find('/', first_delim);
                if (second_delim == std::string::npos) break;
                
                std::string pattern = args.substr(first_delim, second_delim - first_delim);
                std::string replacement = args.substr(second_delim + 1);
                
                // 查找标志（最后一个'/'之后的内容）
                size_t last_delim = replacement.find_last_of('/');
                std::string flags = "";
                if (last_delim != std::string::npos) {
                    flags = replacement.substr(last_delim + 1);
                    replacement = replacement.substr(0, last_delim);
                }
                
                auto cmd = std::make_unique<SubstituteCommand>(pattern, replacement, flags);
                processor_.addCommand(range, std::move(cmd));
                break;
            }
            case 'd': { // 删除命令
                std::string pattern = args;
                // 移除前后的斜杠
                if (!pattern.empty() && pattern[0] == '/') {
                    pattern = pattern.substr(1);
                }
                if (!pattern.empty() && pattern.back() == '/') {
                    pattern.pop_back();
                }
                if (pattern.empty()) pattern = ".*";
                auto cmd = std::make_unique<DeleteCommand>(pattern);
                processor_.addCommand(range, std::move(cmd));
                break;
            }
            case 'p': { // 打印命令
                std::string pattern = args;
                // 移除前后的斜杠
                if (!pattern.empty() && pattern[0] == '/') {
                    pattern = pattern.substr(1);
                }
                if (!pattern.empty() && pattern.back() == '/') {
                    pattern.pop_back();
                }
                if (pattern.empty()) pattern = ".*";
                auto cmd = std::make_unique<PrintCommand>(pattern);
                processor_.addCommand(range, std::move(cmd));
                break;
            }
            case 'i': { // 插入命令
                auto cmd = std::make_unique<InsertCommand>(args);
                processor_.addCommand(range, std::move(cmd));
                break;
            }
            case 'a': { // 追加命令
                auto cmd = std::make_unique<AppendCommand>(args);
                processor_.addCommand(range, std::move(cmd));
                break;
            }
            case 'y': { // 转换命令
                size_t delim = args.find('/');
                if (delim == std::string::npos) break;
                
                std::string from = args.substr(0, delim);
                std::string to = args.substr(delim + 1);
                auto cmd = std::make_unique<TransformCommand>(from, to);
                processor_.addCommand(range, std::move(cmd));
                break;
            }
        }
    }
    
    void processStdin() {
        std::string line;
        int line_number = 1;
        
        while (std::getline(std::cin, line)) {
            processor_.processLine(line, line_number++);
        }
    }
    
    void processFile(const std::string& filename) {
        std::ifstream file(filename);
        if (!file.is_open()) {
            std::cerr << "无法打开文件: " << filename << std::endl;
            return;
        }
        
        std::string line;
        int line_number = 1;
        
        while (std::getline(file, line)) {
            processor_.processLine(line, line_number++);
        }
    }
};

// 命令行解析类
class CommandLineParser {
private:
    std::vector<std::string> args_;
    std::vector<std::string> commands_;
    std::vector<std::string> input_files_;
    bool quiet_mode_;
    bool print_line_numbers_;
    bool color_output_;
    bool in_place_edit_;
    std::string backup_suffix_;
    bool show_help_;
    bool show_version_;
    
public:
    CommandLineParser(int argc, char* argv[]) 
        : quiet_mode_(false), print_line_numbers_(false), color_output_(true),
          in_place_edit_(false), show_help_(false), show_version_(false) {
        for (int i = 1; i < argc; ++i) {
            args_.push_back(argv[i]);
        }
        parse();
    }
    
    void parse() {
        for (size_t i = 0; i < args_.size(); ++i) {
            const std::string& arg = args_[i];
            
            if (arg == "-e" && i + 1 < args_.size()) {
                commands_.push_back(args_[++i]);
            } else if (arg == "-f" && i + 1 < args_.size()) {
                loadCommandsFromFile(args_[++i]);
            } else if (arg == "-i") {
                in_place_edit_ = true;
                if (i + 1 < args_.size() && args_[i + 1][0] != '-') {
                    backup_suffix_ = args_[++i];
                }
            } else if (arg == "-n" || arg == "--quiet") {
                quiet_mode_ = true;
            } else if (arg == "--line-numbers") {
                print_line_numbers_ = true;
            } else if (arg == "--color") {
                color_output_ = true;
            } else if (arg == "--no-color") {
                color_output_ = false;
            } else if (arg == "--help" || arg == "-h") {
                show_help_ = true;
                break; // 立即返回，不处理其他参数
            } else if (arg == "--version" || arg == "-V") {
                show_version_ = true;
                break; // 立即返回，不处理其他参数
            } else if (arg[0] != '-') {
                // 检查是否是命令（包含/字符）
                if (arg.find('/') != std::string::npos) {
                    commands_.push_back(arg);
                } else {
                    input_files_.push_back(arg);
                }
            }
        }
    }
    
    void loadCommandsFromFile(const std::string& filename) {
        std::ifstream file(filename);
        if (!file.is_open()) {
            std::cerr << "无法打开命令文件: " << filename << std::endl;
            return;
        }
        
        std::string line;
        while (std::getline(file, line)) {
            if (!line.empty() && line[0] != '#') {
                commands_.push_back(line);
            }
        }
    }
    
    const std::vector<std::string>& getCommands() const { return commands_; }
    const std::vector<std::string>& getInputFiles() const { return input_files_; }
    bool getQuietMode() const { return quiet_mode_; }
    bool getPrintLineNumbers() const { return print_line_numbers_; }
    bool getColorOutput() const { return color_output_; }
    bool getInPlaceEdit() const { return in_place_edit_; }
    const std::string& getBackupSuffix() const { return backup_suffix_; }
    bool getShowHelp() const { return show_help_; }
    bool getShowVersion() const { return show_version_; }
};

// 帮助信息
void printHelp() {
    std::cout << COLOR_CYAN << "psed - 优化版 sed 命令" << COLOR_RESET << std::endl;
    std::cout << COLOR_YELLOW << "================================================" << COLOR_RESET << std::endl;
    std::cout << "用法: psed [选项] 命令 [文件...]" << std::endl;
    std::cout << "     psed [选项] -f 脚本文件 [文件...]" << std::endl;
    std::cout << std::endl;
    std::cout << "选项:" << std::endl;
    std::cout << "  -e 命令         添加要执行的命令" << std::endl;
    std::cout << "  -f 文件         从文件读取命令" << std::endl;
    std::cout << "  -i[后缀]        就地编辑文件" << std::endl;
    std::cout << "  -n, --quiet     静默模式" << std::endl;
    std::cout << "  --line-numbers  显示行号" << std::endl;
    std::cout << "  --color         启用彩色输出" << std::endl;
    std::cout << "  --no-color      禁用彩色输出" << std::endl;
    std::cout << "  -h, --help      显示此帮助信息" << std::endl;
    std::cout << "  -V, --version   显示版本信息" << std::endl;
    std::cout << std::endl;
    std::cout << "命令:" << std::endl;
    std::cout << "  s/模式/替换/标志  替换匹配的文本" << std::endl;
    std::cout << "  d/模式/          删除匹配的行" << std::endl;
    std::cout << "  p/模式/          打印匹配的行" << std::endl;
    std::cout << "  i/文本/          在行前插入文本" << std::endl;
    std::cout << "  a/文本/          在行后追加文本" << std::endl;
    std::cout << "  y/源/目标/       字符转换" << std::endl;
    std::cout << std::endl;
    std::cout << "标志:" << std::endl;
    std::cout << "  g               全局替换" << std::endl;
    std::cout << "  i               忽略大小写" << std::endl;
    std::cout << "  1-9             替换第N个匹配" << std::endl;
    std::cout << std::endl;
    std::cout << "示例:" << std::endl;
    std::cout << "  psed 's/old/new/g' file.txt" << std::endl;
    std::cout << "  psed -i 's/old/new/' file.txt" << std::endl;
    std::cout << "  psed -e 's/old/new/' -e 'd/^#/' file.txt" << std::endl;
    std::cout << "  psed 'p/pattern/' file.txt" << std::endl;
}

void printVersion() {
    std::cout << "psed - 优化版 sed 命令 v1.0" << std::endl;
    std::cout << "使用C++17和面向对象设计" << std::endl;
}

int main(int argc, char* argv[]) {
    CommandLineParser parser(argc, argv);
    
    if (parser.getShowHelp()) {
        printHelp();
        return 0;
    }
    
    if (parser.getShowVersion()) {
        printVersion();
        return 0;
    }
    
    if (argc < 2) {
        printHelp();
        return 1;
    }
    
    // 创建sed引擎
    SedEngine engine;
    
    // 设置选项
    engine.setInPlaceEdit(parser.getInPlaceEdit(), parser.getBackupSuffix());
    engine.setQuietMode(parser.getQuietMode());
    engine.setPrintLineNumbers(parser.getPrintLineNumbers());
    engine.setColorOutput(parser.getColorOutput());
    
    // 添加输入文件
    for (const auto& file : parser.getInputFiles()) {
        engine.addInputFile(file);
    }
    
    // 添加命令
    const auto& commands = parser.getCommands();
    if (commands.empty()) {
        std::cerr << "错误: 没有指定命令" << std::endl;
        return 1;
    }
    
    for (const auto& command : commands) {
        engine.addCommand(command);
    }
    
    // 处理输入
    engine.process();
    
    return 0;
}
