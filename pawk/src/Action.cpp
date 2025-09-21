#include "Action.h"
#include "BuiltinFunctions.h"
#include "Utils.h"
#include "Constants.h"
#include <iostream>
#include <sstream>
#include <regex>
#include <algorithm>

namespace pawk {

// PrintAction 实现
PrintAction::PrintAction(const std::vector<int>& indices, bool color) 
    : field_indices_(indices), color_output_(color) {
}

PrintAction::PrintAction(const std::string& format, bool color) 
    : format_(format), color_output_(color) {
}

void PrintAction::execute(const Record& record, Variables& variables) {
    if (!format_.empty()) {
        record.printf(format_);
    } else if (!field_indices_.empty()) {
        record.print(field_indices_);
    } else {
        record.print();
    }
}

std::string PrintAction::getDescription() const {
    if (!format_.empty()) {
        return "打印格式: " + format_;
    } else if (!field_indices_.empty()) {
        std::ostringstream oss;
        oss << "打印字段: ";
        for (size_t i = 0; i < field_indices_.size(); ++i) {
            if (i > 0) oss << ", ";
            oss << "$" << field_indices_[i];
        }
        return oss.str();
    } else {
        return "打印所有字段";
    }
}

// PrintfAction 实现
PrintfAction::PrintfAction(const std::string& format, bool color) 
    : format_(format), color_output_(color) {
}

void PrintfAction::execute(const Record& record, Variables& variables) {
    record.printf(format_);
}

std::string PrintfAction::getDescription() const {
    return "格式化打印: " + format_;
}

// AssignmentAction 实现
AssignmentAction::AssignmentAction(const std::string& var_name, const std::string& expr) 
    : variable_name_(var_name), expression_(expr) {
}

void AssignmentAction::execute(const Record& record, Variables& variables) {
    // 简化的表达式求值
    // 这里可以实现更复杂的表达式解析和求值
    if (expression_.find("$") != std::string::npos) {
        // 包含字段引用
        std::string result = expression_;
        std::regex field_regex("\\$([0-9]+)");
        std::smatch match;
        
        while (std::regex_search(result, match, field_regex)) {
            int field_num = std::stoi(match[1].str());
            std::string replacement = (field_num >= 1 && field_num <= record.NF()) 
                ? record[field_num].value() : "";
            result = std::regex_replace(result, field_regex, replacement, std::regex_constants::format_first_only);
        }
        
        variables.setString(variable_name_, result);
    } else {
        // 简单赋值
        variables.setString(variable_name_, expression_);
    }
}

std::string AssignmentAction::getDescription() const {
    return "赋值: " + variable_name_ + " = " + expression_;
}

// IfAction 实现
IfAction::IfAction(const std::string& condition, std::unique_ptr<Action> then_action, std::unique_ptr<Action> else_action)
    : condition_(condition), then_action_(std::move(then_action)), else_action_(std::move(else_action)) {
}

void IfAction::execute(const Record& record, Variables& variables) {
    // 简化的条件判断
    // 这里可以实现更复杂的条件表达式解析
    bool condition_result = true; // 占位符实现
    
    if (condition_result) {
        if (then_action_) {
            then_action_->execute(record, variables);
        }
    } else {
        if (else_action_) {
            else_action_->execute(record, variables);
        }
    }
}

std::string IfAction::getDescription() const {
    std::string desc = "如果 " + condition_ + " 则 ";
    if (then_action_) {
        desc += then_action_->getDescription();
    }
    if (else_action_) {
        desc += " 否则 " + else_action_->getDescription();
    }
    return desc;
}

// ForAction 实现
ForAction::ForAction(const std::string& var_name, const std::string& start, const std::string& end, 
                     const std::string& step, std::unique_ptr<Action> body)
    : variable_name_(var_name), start_expr_(start), end_expr_(end), step_expr_(step), body_action_(std::move(body)) {
}

void ForAction::execute(const Record& record, Variables& variables) {
    // 简化的for循环实现
    // 这里可以实现更复杂的循环逻辑
    int start_val = 1; // 占位符实现
    int end_val = 10;   // 占位符实现
    int step_val = 1;   // 占位符实现
    
    for (int i = start_val; i <= end_val; i += step_val) {
        variables.setNumeric(variable_name_, i);
        if (body_action_) {
            body_action_->execute(record, variables);
        }
    }
}

std::string ForAction::getDescription() const {
    return "for循环: " + variable_name_ + " = " + start_expr_ + " to " + end_expr_ + " step " + step_expr_;
}

// WhileAction 实现
WhileAction::WhileAction(const std::string& condition, std::unique_ptr<Action> body)
    : condition_(condition), body_action_(std::move(body)) {
}

void WhileAction::execute(const Record& record, Variables& variables) {
    // 简化的while循环实现
    // 这里可以实现更复杂的循环逻辑
    bool condition_result = true; // 占位符实现
    int max_iterations = 1000; // 防止无限循环
    int iterations = 0;
    
    while (condition_result && iterations < max_iterations) {
        if (body_action_) {
            body_action_->execute(record, variables);
        }
        iterations++;
        // 这里应该重新评估条件
        condition_result = false; // 占位符实现
    }
}

std::string WhileAction::getDescription() const {
    return "while循环: " + condition_;
}

// FunctionCallAction 实现
FunctionCallAction::FunctionCallAction(const std::string& func_name, const std::vector<std::string>& args)
    : function_name_(func_name), arguments_(args) {
}

void FunctionCallAction::execute(const Record& record, Variables& variables) {
    // 简化的函数调用实现
    // 这里可以实现更复杂的函数调用逻辑
    if (function_name_ == "print") {
        record.print();
    } else if (function_name_ == "printf") {
        if (!arguments_.empty()) {
            record.printf(arguments_[0]);
        }
    }
    // 可以添加更多内置函数的调用
}

std::string FunctionCallAction::getDescription() const {
    std::ostringstream oss;
    oss << "函数调用: " << function_name_ << "(";
    for (size_t i = 0; i < arguments_.size(); ++i) {
        if (i > 0) oss << ", ";
        oss << arguments_[i];
    }
    oss << ")";
    return oss.str();
}

// BlockAction 实现
BlockAction::BlockAction() {
}

void BlockAction::addAction(std::unique_ptr<Action> action) {
    actions_.push_back(std::move(action));
}

void BlockAction::execute(const Record& record, Variables& variables) {
    for (const auto& action : actions_) {
        if (action) {
            action->execute(record, variables);
        }
    }
}

std::string BlockAction::getDescription() const {
    std::ostringstream oss;
    oss << "代码块(" << actions_.size() << "个动作)";
    return oss.str();
}

// ActionFactory 实现
std::unique_ptr<Action> ActionFactory::createAction(const std::string& action_str) {
    if (action_str.empty()) {
        return nullptr;
    }
    
    // 检查是否是print语句
    if (action_str.find("print") != std::string::npos) {
        return createPrintAction(action_str);
    }
    
    // 检查是否是printf语句
    if (action_str.find("printf") != std::string::npos) {
        return createPrintfAction(action_str);
    }
    
    // 检查是否是赋值语句
    if (action_str.find("=") != std::string::npos) {
        return createAssignmentAction(action_str);
    }
    
    // 检查是否是if语句
    if (action_str.find("if") != std::string::npos) {
        return createIfAction(action_str, "", "");
    }
    
    // 检查是否是for语句
    if (action_str.find("for") != std::string::npos) {
        return createForAction("", "", "", "", action_str);
    }
    
    // 检查是否是while语句
    if (action_str.find("while") != std::string::npos) {
        return createWhileAction(action_str, "");
    }
    
    // 默认为print语句
    return createPrintAction(action_str);
}

std::unique_ptr<Action> ActionFactory::createPrintAction(const std::string& args) {
    // 解析print参数
    std::vector<int> field_indices;
    std::string format;
    
    if (args.find("$") != std::string::npos) {
        // 包含字段引用
        std::regex field_regex("\\$([0-9]+)");
        std::smatch match;
        std::string search_str = args;
        
        while (std::regex_search(search_str, match, field_regex)) {
            int field_num = std::stoi(match[1].str());
            field_indices.push_back(field_num);
            search_str = match.suffix();
        }
    }
    
    if (field_indices.empty()) {
        return std::make_unique<PrintAction>();
    } else {
        return std::make_unique<PrintAction>(field_indices);
    }
}

std::unique_ptr<Action> ActionFactory::createPrintfAction(const std::string& format) {
    return std::make_unique<PrintfAction>(format);
}

std::unique_ptr<Action> ActionFactory::createAssignmentAction(const std::string& assignment) {
    size_t pos = assignment.find("=");
    if (pos != std::string::npos) {
        std::string var_name = assignment.substr(0, pos);
        std::string expr = assignment.substr(pos + 1);
        return std::make_unique<AssignmentAction>(var_name, expr);
    }
    return nullptr;
}

std::unique_ptr<Action> ActionFactory::createIfAction(const std::string& condition, const std::string& then_action, const std::string& else_action) {
    auto then_act = createAction(then_action);
    auto else_act = else_action.empty() ? nullptr : createAction(else_action);
    return std::make_unique<IfAction>(condition, std::move(then_act), std::move(else_act));
}

std::unique_ptr<Action> ActionFactory::createForAction(const std::string& var, const std::string& start, const std::string& end, const std::string& step, const std::string& body) {
    auto body_act = createAction(body);
    return std::make_unique<ForAction>(var, start, end, step, std::move(body_act));
}

std::unique_ptr<Action> ActionFactory::createWhileAction(const std::string& condition, const std::string& body) {
    auto body_act = createAction(body);
    return std::make_unique<WhileAction>(condition, std::move(body_act));
}

std::unique_ptr<Action> ActionFactory::createFunctionCallAction(const std::string& func_name, const std::vector<std::string>& args) {
    return std::make_unique<FunctionCallAction>(func_name, args);
}

std::unique_ptr<Action> ActionFactory::createBlockAction(const std::vector<std::string>& action_strings) {
    auto block = std::make_unique<BlockAction>();
    for (const auto& action_str : action_strings) {
        auto action = createAction(action_str);
        if (action) {
            block->addAction(std::move(action));
        }
    }
    return block;
}

} // namespace pawk
