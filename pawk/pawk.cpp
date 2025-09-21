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
class Field;
class Record;
class AwkEngine;

// 字段类 - 表示记录中的单个字段
class Field {
private:
    std::string value_;
    int index_;
    
public:
    Field(const std::string& value = "", int index = 0) 
        : value_(value), index_(index) {}
    
    // 获取值
    const std::string& value() const { return value_; }
    int index() const { return index_; }
    
    // 设置值
    void setValue(const std::string& value) { value_ = value; }
    void setIndex(int index) { index_ = index; }
    
    // 类型转换
    operator std::string() const { return value_; }
    operator int() const { return std::stoi(value_); }
    operator double() const { return std::stod(value_); }
    
    // 字符串操作
    size_t length() const { return value_.length(); }
    bool empty() const { return value_.empty(); }
    
    // 比较操作
    bool operator==(const std::string& other) const { return value_ == other; }
    bool operator!=(const std::string& other) const { return value_ != other; }
    bool operator<(const std::string& other) const { return value_ < other; }
    bool operator>(const std::string& other) const { return value_ > other; }
    
    // 匹配操作
    bool match(const std::string& pattern) const {
        std::regex regex_pattern(pattern);
        return std::regex_search(value_, regex_pattern);
    }
    
    bool match(const std::regex& regex_pattern) const {
        return std::regex_search(value_, regex_pattern);
    }
    
    // 替换操作
    std::string replace(const std::string& pattern, const std::string& replacement) const {
        std::regex regex_pattern(pattern);
        return std::regex_replace(value_, regex_pattern, replacement);
    }
    
    // 子字符串
    std::string substr(size_t pos, size_t len = std::string::npos) const {
        return value_.substr(pos, len);
    }
    
    // 分割
    std::vector<std::string> split(const std::string& delimiter) const {
        std::vector<std::string> result;
        std::string str = value_;
        size_t pos = 0;
        
        while ((pos = str.find(delimiter)) != std::string::npos) {
            result.push_back(str.substr(0, pos));
            str.erase(0, pos + delimiter.length());
        }
        result.push_back(str);
        return result;
    }
    
    // 格式化输出
    friend std::ostream& operator<<(std::ostream& os, const Field& field) {
        os << field.value_;
        return os;
    }
};

// 记录类 - 表示一行数据
class Record {
private:
    std::vector<Field> fields_;
    std::string raw_line_;
    std::string field_separator_;
    int record_number_;
    
public:
    Record(const std::string& line = "", const std::string& fs = "\\s+", int record_num = 0)
        : raw_line_(line), field_separator_(fs), record_number_(record_num) {
        parseFields();
    }
    
    // 解析字段
    void parseFields() {
        fields_.clear();
        if (raw_line_.empty()) return;
        
        std::regex fs_regex(field_separator_);
        std::sregex_token_iterator iter(raw_line_.begin(), raw_line_.end(), fs_regex, -1);
        std::sregex_token_iterator end;
        
        int index = 1;
        for (; iter != end; ++iter, ++index) {
            fields_.emplace_back(*iter, index);
        }
    }
    
    // 获取字段
    Field& operator[](int index) {
        if (index < 1 || index > static_cast<int>(fields_.size())) {
            static Field empty_field;
            return empty_field;
        }
        return fields_[index - 1];
    }
    
    const Field& operator[](int index) const {
        if (index < 1 || index > static_cast<int>(fields_.size())) {
            static Field empty_field;
            return empty_field;
        }
        return fields_[index - 1];
    }
    
    // 获取字段数量
    int NF() const { return fields_.size(); }
    
    // 获取原始行
    const std::string& $0() const { return raw_line_; }
    
    // 获取记录号
    int NR() const { return record_number_; }
    
    // 设置字段分隔符
    void setFieldSeparator(const std::string& fs) {
        field_separator_ = fs;
        parseFields();
    }
    
    // 打印记录
    void print() const {
        for (size_t i = 0; i < fields_.size(); ++i) {
            if (i > 0) std::cout << " ";
            std::cout << fields_[i];
        }
        std::cout << std::endl;
    }
    
    // 打印指定字段
    void print(const std::vector<int>& field_indices) const {
        for (size_t i = 0; i < field_indices.size(); ++i) {
            if (i > 0) std::cout << " ";
            int idx = field_indices[i];
            if (idx >= 1 && idx <= static_cast<int>(fields_.size())) {
                std::cout << fields_[idx - 1];
            }
        }
        std::cout << std::endl;
    }
    
    // 格式化打印
    void printf(const std::string& format) const {
        // 简单的printf实现
        std::string result = format;
        std::regex field_regex("\\$([0-9]+)");
        std::smatch match;
        
        while (std::regex_search(result, match, field_regex)) {
            int field_num = std::stoi(match[1].str());
            std::string replacement = (field_num >= 1 && field_num <= static_cast<int>(fields_.size())) 
                ? fields_[field_num - 1].value() : "";
            result = std::regex_replace(result, field_regex, replacement, std::regex_constants::format_first_only);
        }
        
        std::cout << result << std::endl;
    }
};

// 内置函数类
class BuiltinFunctions {
public:
    // 数学函数
    static double sqrt_func(double x) { return std::sqrt(x); }
    static double sin_func(double x) { return std::sin(x); }
    static double cos_func(double x) { return std::cos(x); }
    static double log_func(double x) { return std::log(x); }
    static double exp_func(double x) { return std::exp(x); }
    static int int_(double x) { return static_cast<int>(x); }
    static double rand() { return static_cast<double>(std::rand()) / RAND_MAX; }
    
    // 字符串函数
    static int length(const std::string& str) { return str.length(); }
    static std::string toupper(const std::string& str) {
        std::string result = str;
        std::transform(result.begin(), result.end(), result.begin(), ::toupper);
        return result;
    }
    static std::string tolower(const std::string& str) {
        std::string result = str;
        std::transform(result.begin(), result.end(), result.begin(), ::tolower);
        return result;
    }
    static std::string substr(const std::string& str, int start, int length = -1) {
        if (start < 1) start = 1;
        if (length == -1) length = str.length() - start + 1;
        return str.substr(start - 1, length);
    }
    static int index(const std::string& str, const std::string& substr) {
        size_t pos = str.find(substr);
        return pos == std::string::npos ? 0 : pos + 1;
    }
    static std::string gsub(const std::string& pattern, const std::string& replacement, std::string& str) {
        std::regex regex_pattern(pattern);
        str = std::regex_replace(str, regex_pattern, replacement);
        return str;
    }
    
    // 时间函数
    static int systime() { return time(nullptr); }
    static std::string strftime_func(const std::string& format, int timestamp = -1) {
        if (timestamp == -1) timestamp = time(nullptr);
        time_t t = timestamp;
        char buffer[256];
        std::strftime(buffer, sizeof(buffer), format.c_str(), localtime(&t));
        return std::string(buffer);
    }
};

// 变量存储类
class Variables {
private:
    std::map<std::string, std::string> string_vars_;
    std::map<std::string, double> numeric_vars_;
    
public:
    void setString(const std::string& name, const std::string& value) {
        string_vars_[name] = value;
    }
    
    void setNumeric(const std::string& name, double value) {
        numeric_vars_[name] = value;
    }
    
    std::string getString(const std::string& name) const {
        auto it = string_vars_.find(name);
        return it != string_vars_.end() ? it->second : "";
    }
    
    double getNumeric(const std::string& name) const {
        auto it = numeric_vars_.find(name);
        return it != numeric_vars_.end() ? it->second : 0.0;
    }
    
    bool exists(const std::string& name) const {
        return string_vars_.find(name) != string_vars_.end() || 
               numeric_vars_.find(name) != numeric_vars_.end();
    }
};

// 模式匹配类
class Pattern {
public:
    virtual ~Pattern() = default;
    virtual bool matches(const Record& record) = 0;
};

class RegexPattern : public Pattern {
private:
    std::regex regex_;
    std::string pattern_;
    
public:
    RegexPattern(const std::string& pattern) : pattern_(pattern) {
        try {
            regex_ = std::regex(pattern);
        } catch (const std::regex_error& e) {
            std::cerr << "正则表达式错误: " << e.what() << std::endl;
        }
    }
    
    bool matches(const Record& record) override {
        return std::regex_search(record.$0(), regex_);
    }
};

class ExpressionPattern : public Pattern {
private:
    std::string expression_;
    
public:
    ExpressionPattern(const std::string& expr) : expression_(expr) {}
    
    bool matches(const Record& record) override {
        // 简化的表达式匹配实现
        // 这里可以实现更复杂的表达式解析
        return true; // 占位符实现
    }
};

// 动作类
class Action {
public:
    virtual ~Action() = default;
    virtual void execute(const Record& record) = 0;
};

class PrintAction : public Action {
private:
    std::vector<int> field_indices_;
    std::string format_;
    
public:
    PrintAction(const std::vector<int>& indices = {}) : field_indices_(indices) {}
    PrintAction(const std::string& format) : format_(format) {}
    
    void execute(const Record& record) override {
        if (!format_.empty()) {
            record.printf(format_);
        } else if (!field_indices_.empty()) {
            record.print(field_indices_);
        } else {
            record.print();
        }
    }
};

class PrintfAction : public Action {
private:
    std::string format_;
    
public:
    PrintfAction(const std::string& format) : format_(format) {}
    
    void execute(const Record& record) override {
        record.printf(format_);
    }
};

// 主引擎类
class AwkEngine {
private:
    std::vector<std::unique_ptr<Pattern>> patterns_;
    std::vector<std::unique_ptr<Action>> actions_;
    Variables variables_;
    std::string field_separator_;
    std::string output_separator_;
    bool color_output_;
    bool show_line_numbers_;
    bool show_field_numbers_;
    
public:
    AwkEngine() : field_separator_("\\s+"), output_separator_(" "), 
                  color_output_(true), show_line_numbers_(false), show_field_numbers_(false) {}
    
    // 设置选项
    void setFieldSeparator(const std::string& fs) { field_separator_ = fs; }
    void setOutputSeparator(const std::string& ofs) { output_separator_ = ofs; }
    void setColorOutput(bool enable) { color_output_ = enable; }
    void setShowLineNumbers(bool enable) { show_line_numbers_ = enable; }
    void setShowFieldNumbers(bool enable) { show_field_numbers_ = enable; }
    
    // 添加模式-动作对
    void addPatternAction(std::unique_ptr<Pattern> pattern, std::unique_ptr<Action> action) {
        patterns_.push_back(std::move(pattern));
        actions_.push_back(std::move(action));
    }
    
    // 添加动作（无模式）
    void addAction(std::unique_ptr<Action> action) {
        patterns_.push_back(nullptr);
        actions_.push_back(std::move(action));
    }
    
    // 处理文件
    void processFile(const std::string& filename) {
        std::ifstream file(filename);
        if (!file.is_open()) {
            std::cerr << "无法打开文件: " << filename << std::endl;
            return;
        }
        
        processStream(file);
    }
    
    // 处理标准输入
    void processStdin() {
        processStream(std::cin);
    }
    
    // 处理流
    void processStream(std::istream& stream) {
        std::string line;
        int record_number = 0;
        
        while (std::getline(stream, line)) {
            record_number++;
            Record record(line, field_separator_, record_number);
            
            // 执行所有模式-动作对
            for (size_t i = 0; i < patterns_.size(); ++i) {
                if (patterns_[i] == nullptr || patterns_[i]->matches(record)) {
                    if (actions_[i]) {
                        if (color_output_) {
                            printColoredRecord(record);
                        } else {
                            actions_[i]->execute(record);
                        }
                    }
                }
            }
        }
    }
    
    // 彩色输出
    void printColoredRecord(const Record& record) {
        if (show_line_numbers_) {
            std::cout << COLOR_CYAN << std::setw(4) << record.NR() << ": " << COLOR_RESET;
        }
        
        for (int i = 1; i <= record.NF(); ++i) {
            if (i > 1) std::cout << " ";
            
            if (show_field_numbers_) {
                std::cout << COLOR_YELLOW << "$" << i << "=" << COLOR_RESET;
            }
            
            // 根据字段内容选择颜色
            std::string field_value = record[i].value();
            if (std::regex_match(field_value, std::regex("^[0-9]+$"))) {
                std::cout << COLOR_GREEN << field_value << COLOR_RESET;
            } else if (std::regex_match(field_value, std::regex("^[0-9]+\\.[0-9]+$"))) {
                std::cout << COLOR_BLUE << field_value << COLOR_RESET;
            } else if (field_value.find("http") != std::string::npos) {
                std::cout << COLOR_MAGENTA << field_value << COLOR_RESET;
            } else {
                std::cout << COLOR_WHITE << field_value << COLOR_RESET;
            }
        }
        std::cout << std::endl;
    }
    
    // 内置变量
    void setBuiltinVariables() {
        variables_.setString("FS", field_separator_);
        variables_.setString("OFS", output_separator_);
        variables_.setString("RS", "\n");
        variables_.setString("ORS", "\n");
    }
};

// 命令行解析类
class CommandLineParser {
private:
    std::vector<std::string> args_;
    std::string program_;
    std::string field_separator_;
    std::string output_separator_;
    bool color_output_;
    bool show_help_;
    bool show_version_;
    std::vector<std::string> input_files_;
    
public:
    CommandLineParser(int argc, char* argv[]) {
        for (int i = 1; i < argc; ++i) {
            args_.push_back(argv[i]);
        }
        parse();
    }
    
    void parse() {
        for (size_t i = 0; i < args_.size(); ++i) {
            const std::string& arg = args_[i];
            
            if (arg == "-F" && i + 1 < args_.size()) {
                field_separator_ = args_[++i];
            } else if (arg == "-v" && i + 1 < args_.size()) {
                // 变量赋值
                std::string var_assignment = args_[++i];
                size_t pos = var_assignment.find('=');
                if (pos != std::string::npos) {
                    std::string name = var_assignment.substr(0, pos);
                    std::string value = var_assignment.substr(pos + 1);
                    // 这里可以设置变量
                }
            } else if (arg == "--color") {
                color_output_ = true;
            } else if (arg == "--no-color") {
                color_output_ = false;
            } else if (arg == "--line-numbers") {
                // 显示行号
            } else if (arg == "--field-numbers") {
                // 显示字段号
            } else if (arg == "--help" || arg == "-h") {
                show_help_ = true;
            } else if (arg == "--version" || arg == "-V") {
                show_version_ = true;
            } else if (arg[0] != '-') {
                if (program_.empty()) {
                    program_ = arg;
                } else {
                    input_files_.push_back(arg);
                }
            }
        }
    }
    
    const std::string& getProgram() const { return program_; }
    const std::string& getFieldSeparator() const { return field_separator_; }
    const std::string& getOutputSeparator() const { return output_separator_; }
    bool getColorOutput() const { return color_output_; }
    bool getShowHelp() const { return show_help_; }
    bool getShowVersion() const { return show_version_; }
    const std::vector<std::string>& getInputFiles() const { return input_files_; }
};

// 帮助信息
void printHelp() {
    std::cout << COLOR_CYAN << "pawk - 优化版 awk 命令" << COLOR_RESET << std::endl;
    std::cout << COLOR_YELLOW << "================================================" << COLOR_RESET << std::endl;
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
}

void printVersion() {
    std::cout << "pawk - 优化版 awk 命令 v1.0" << std::endl;
    std::cout << "使用C++17和面向对象设计" << std::endl;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printHelp();
        return 1;
    }
    
    CommandLineParser parser(argc, argv);
    
    if (parser.getShowHelp()) {
        printHelp();
        return 0;
    }
    
    if (parser.getShowVersion()) {
        printVersion();
        return 0;
    }
    
    // 创建AWK引擎
    AwkEngine engine;
    
    // 设置选项
    if (!parser.getFieldSeparator().empty()) {
        engine.setFieldSeparator(parser.getFieldSeparator());
    }
    engine.setColorOutput(parser.getColorOutput());
    
    // 解析程序
    std::string program = parser.getProgram();
    if (program.empty()) {
        std::cerr << "错误: 没有指定程序" << std::endl;
        return 1;
    }
    
    // 简化的程序解析 - 这里可以实现更复杂的解析器
    if (program.find("print") != std::string::npos) {
        // 简单的print语句
        engine.addAction(std::make_unique<PrintAction>());
    } else if (program.find("printf") != std::string::npos) {
        // printf语句
        engine.addAction(std::make_unique<PrintfAction>(program));
    }
    
    // 处理输入文件
    const auto& input_files = parser.getInputFiles();
    if (input_files.empty()) {
        engine.processStdin();
    } else {
        for (const auto& file : input_files) {
            engine.processFile(file);
        }
    }
    
    return 0;
}
