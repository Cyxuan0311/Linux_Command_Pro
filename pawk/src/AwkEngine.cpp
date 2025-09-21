#include "AwkEngine.h"
#include "Utils.h"
#include "Constants.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <regex>

namespace pawk {

AwkEngine::AwkEngine() : total_records_(0), processed_records_(0), matched_records_(0) {
    initialize();
}

AwkEngine::AwkEngine(const CommandLineOptions& options) 
    : options_(options), total_records_(0), processed_records_(0), matched_records_(0) {
    initialize();
}

AwkEngine::~AwkEngine() = default;

void AwkEngine::initialize() {
    variables_.setBuiltinVariables();
    variables_.updateBuiltinVariables(options_.field_separator, options_.output_separator, 0, 0);
    
    // 设置用户定义的变量
    for (const auto& var : options_.variables) {
        variables_.setString(var.first, var.second);
    }
}

void AwkEngine::setOptions(const CommandLineOptions& options) {
    options_ = options;
    initialize();
}

void AwkEngine::setFieldSeparator(const std::string& fs) {
    options_.field_separator = fs;
    variables_.setString("FS", fs);
}

void AwkEngine::setOutputSeparator(const std::string& ofs) {
    options_.output_separator = ofs;
    variables_.setString("OFS", ofs);
}

void AwkEngine::setColorOutput(bool enable) {
    options_.color_output = enable;
}

void AwkEngine::setShowLineNumbers(bool enable) {
    options_.show_line_numbers = enable;
}

void AwkEngine::setShowFieldNumbers(bool enable) {
    options_.show_field_numbers = enable;
}

void AwkEngine::addPatternAction(std::unique_ptr<Pattern> pattern, std::unique_ptr<Action> action) {
    patterns_.push_back(std::move(pattern));
    actions_.push_back(std::move(action));
}

void AwkEngine::addAction(std::unique_ptr<Action> action) {
    patterns_.push_back(nullptr);
    actions_.push_back(std::move(action));
}

void AwkEngine::addBeginAction(std::unique_ptr<Action> action) {
    // BEGIN动作在开始处理前执行
    if (action) {
        action->execute(Record(), variables_);
    }
}

void AwkEngine::addEndAction(std::unique_ptr<Action> action) {
    // END动作在处理完所有记录后执行
    if (action) {
        action->execute(Record(), variables_);
    }
}

void AwkEngine::parseProgram(const std::string& program) {
    // 简化的程序解析
    // 这里可以实现更复杂的AWK程序解析器
    
    if (program.find("BEGIN") != std::string::npos) {
        // 解析BEGIN块
        std::regex begin_regex("BEGIN\\s*\\{([^}]+)\\}");
        std::smatch match;
        if (std::regex_search(program, match, begin_regex)) {
            std::string begin_code = match[1].str();
            auto action = ActionFactory::createAction(begin_code);
            if (action) {
                addBeginAction(std::move(action));
            }
        }
    }
    
    if (program.find("END") != std::string::npos) {
        // 解析END块
        std::regex end_regex("END\\s*\\{([^}]+)\\}");
        std::smatch match;
        if (std::regex_search(program, match, end_regex)) {
            std::string end_code = match[1].str();
            auto action = ActionFactory::createAction(end_code);
            if (action) {
                addEndAction(std::move(action));
            }
        }
    }
    
    // 解析主程序
    std::string main_program = program;
    
    // 移除BEGIN和END块
    main_program = std::regex_replace(main_program, std::regex("BEGIN\\s*\\{[^}]+\\}"), "");
    main_program = std::regex_replace(main_program, std::regex("END\\s*\\{[^}]+\\}"), "");
    
    // 解析模式-动作对
    std::regex pattern_action_regex("(.*?)\\s*\\{([^}]+)\\}");
    std::smatch match;
    
    if (std::regex_search(main_program, match, pattern_action_regex)) {
        std::string pattern_str = match[1].str();
        std::string action_str = match[2].str();
        
        // 创建模式
        std::unique_ptr<Pattern> pattern = nullptr;
        if (!pattern_str.empty() && pattern_str != " ") {
            pattern = PatternFactory::createPattern(pattern_str);
        }
        
        // 创建动作
        auto action = ActionFactory::createAction(action_str);
        
        if (action) {
            addPatternAction(std::move(pattern), std::move(action));
        }
    } else {
        // 没有模式，只有动作
        auto action = ActionFactory::createAction(main_program);
        if (action) {
            addAction(std::move(action));
        }
    }
}

void AwkEngine::parseProgramFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        Utils::printError("无法打开程序文件: " + filename);
        return;
    }
    
    std::string program;
    std::string line;
    while (std::getline(file, line)) {
        program += line + "\n";
    }
    
    parseProgram(program);
}

void AwkEngine::processFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        Utils::printError("无法打开文件: " + filename);
        return;
    }
    
    variables_.setString("FILENAME", filename);
    processStream(file);
}

void AwkEngine::processStdin() {
    processStream(std::cin);
}

void AwkEngine::processStream(std::istream& stream) {
    executeBeginActions();
    
    std::string line;
    int record_number = 0;
    
    while (std::getline(stream, line)) {
        record_number++;
        Record record(line, options_.field_separator, record_number);
        processRecord(record);
    }
    
    executeEndActions();
}

void AwkEngine::processRecord(const Record& record) {
    total_records_++;
    processed_records_++;
    
    updateBuiltinVariables(record);
    
    // 执行所有模式-动作对
    for (size_t i = 0; i < patterns_.size(); ++i) {
        if (patterns_[i] == nullptr || patterns_[i]->matches(record)) {
            matched_records_++;
            if (actions_[i]) {
                if (options_.color_output) {
                    printColoredRecord(record);
                } else {
                    actions_[i]->execute(record, variables_);
                }
            }
        }
    }
}

void AwkEngine::printColoredRecord(const Record& record) {
    if (options_.show_line_numbers) {
        std::cout << Utils::colorize(std::to_string(record.NR()) + ": ", Colors::CYAN);
    }
    
    for (int i = 1; i <= record.NF(); ++i) {
        if (i > 1) std::cout << " ";
        
        if (options_.show_field_numbers) {
            std::cout << Utils::colorize("$" + std::to_string(i) + "=", Colors::YELLOW);
        }
        
        std::string field_value = record[i].value();
        std::cout << Utils::highlightField(field_value, i);
    }
    std::cout << std::endl;
}

void AwkEngine::printRecord(const Record& record) {
    record.print();
}

void AwkEngine::updateBuiltinVariables(const Record& record) {
    variables_.updateBuiltinVariables(options_.field_separator, options_.output_separator, 
                                     record.NR(), record.NF());
}

void AwkEngine::executeBeginActions() {
    // BEGIN动作已经在addBeginAction中执行
}

void AwkEngine::executeEndActions() {
    // END动作已经在addEndAction中执行
}

void AwkEngine::clear() {
    patterns_.clear();
    actions_.clear();
    variables_.clear();
    total_records_ = 0;
    processed_records_ = 0;
    matched_records_ = 0;
}

void AwkEngine::reset() {
    clear();
    initialize();
}

} // namespace pawk
