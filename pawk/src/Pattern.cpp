#include "Pattern.h"
#include "Utils.h"
#include <iostream>
#include <regex>
#include <sstream>

namespace pawk {

// RegexPattern 实现
RegexPattern::RegexPattern(const std::string& pattern) : pattern_(pattern) {
    try {
        regex_ = std::regex(pattern);
    } catch (const std::regex_error& e) {
        Utils::printError("正则表达式错误: " + std::string(e.what()));
    }
}

bool RegexPattern::matches(const Record& record) {
    try {
        return std::regex_search(record.$0(), regex_);
    } catch (const std::regex_error& e) {
        Utils::printError("正则表达式匹配错误: " + std::string(e.what()));
        return false;
    }
}

std::string RegexPattern::getDescription() const {
    return "正则表达式: " + pattern_;
}

// ExpressionPattern 实现
ExpressionPattern::ExpressionPattern(const std::string& expr) : expression_(expr) {
}

bool ExpressionPattern::matches(const Record& record) {
    // 简化的表达式匹配实现
    // 这里可以实现更复杂的表达式解析和求值
    // 目前只是占位符实现
    return true;
}

std::string ExpressionPattern::getDescription() const {
    return "表达式: " + expression_;
}

// RangePattern 实现
RangePattern::RangePattern(std::unique_ptr<Pattern> start, std::unique_ptr<Pattern> end)
    : start_pattern_(std::move(start)), end_pattern_(std::move(end)), in_range_(false) {
}

bool RangePattern::matches(const Record& record) {
    if (start_pattern_ && start_pattern_->matches(record)) {
        in_range_ = true;
        return true;
    }
    
    if (in_range_ && end_pattern_ && end_pattern_->matches(record)) {
        in_range_ = false;
        return true;
    }
    
    return in_range_;
}

std::string RangePattern::getDescription() const {
    std::string start_desc = start_pattern_ ? start_pattern_->getDescription() : "无";
    std::string end_desc = end_pattern_ ? end_pattern_->getDescription() : "无";
    return "范围模式: " + start_desc + " 到 " + end_desc;
}

// FieldPattern 实现
FieldPattern::FieldPattern(int field_num, const std::string& pattern) 
    : field_number_(field_num), pattern_(pattern) {
    try {
        regex_ = std::regex(pattern);
    } catch (const std::regex_error& e) {
        Utils::printError("字段模式正则表达式错误: " + std::string(e.what()));
    }
}

bool FieldPattern::matches(const Record& record) {
    if (field_number_ < 1 || field_number_ > record.NF()) {
        return false;
    }
    
    try {
        return std::regex_search(record[field_number_].value(), regex_);
    } catch (const std::regex_error& e) {
        Utils::printError("字段模式匹配错误: " + std::string(e.what()));
        return false;
    }
}

std::string FieldPattern::getDescription() const {
    return "字段" + std::to_string(field_number_) + "匹配: " + pattern_;
}

// CompoundPattern 实现
CompoundPattern::CompoundPattern(const std::string& op) : operator_(op) {
}

void CompoundPattern::addPattern(std::unique_ptr<Pattern> pattern) {
    patterns_.push_back(std::move(pattern));
}

bool CompoundPattern::matches(const Record& record) {
    if (patterns_.empty()) return false;
    
    if (operator_ == "&&" || operator_ == "AND") {
        for (const auto& pattern : patterns_) {
            if (!pattern->matches(record)) {
                return false;
            }
        }
        return true;
    } else if (operator_ == "||" || operator_ == "OR") {
        for (const auto& pattern : patterns_) {
            if (pattern->matches(record)) {
                return true;
            }
        }
        return false;
    } else if (operator_ == "!") {
        return !patterns_[0]->matches(record);
    }
    
    return false;
}

std::string CompoundPattern::getDescription() const {
    std::ostringstream oss;
    oss << "复合模式(" << operator_ << "): ";
    for (size_t i = 0; i < patterns_.size(); ++i) {
        if (i > 0) oss << " " << operator_ << " ";
        oss << patterns_[i]->getDescription();
    }
    return oss.str();
}

// PatternFactory 实现
std::unique_ptr<Pattern> PatternFactory::createPattern(const std::string& pattern_str) {
    if (pattern_str.empty()) {
        return nullptr;
    }
    
    // 检查是否是范围模式
    if (pattern_str.find(",") != std::string::npos) {
        size_t pos = pattern_str.find(",");
        std::string start = pattern_str.substr(0, pos);
        std::string end = pattern_str.substr(pos + 1);
        return createRangePattern(start, end);
    }
    
    // 检查是否是字段模式
    if (pattern_str.find("$") != std::string::npos) {
        size_t pos = pattern_str.find("$");
        if (pos + 1 < pattern_str.length()) {
            std::string field_num_str = pattern_str.substr(pos + 1);
            size_t space_pos = field_num_str.find(" ");
            if (space_pos != std::string::npos) {
                int field_num = std::stoi(field_num_str.substr(0, space_pos));
                std::string pattern = field_num_str.substr(space_pos + 1);
                return createFieldPattern(field_num, pattern);
            }
        }
    }
    
    // 检查是否是复合模式
    if (pattern_str.find("&&") != std::string::npos || 
        pattern_str.find("||") != std::string::npos ||
        pattern_str.find("!") != std::string::npos) {
        return createCompoundPattern(pattern_str);
    }
    
    // 默认为正则表达式模式
    return createRegexPattern(pattern_str);
}

std::unique_ptr<Pattern> PatternFactory::createRegexPattern(const std::string& pattern) {
    return std::make_unique<RegexPattern>(pattern);
}

std::unique_ptr<Pattern> PatternFactory::createExpressionPattern(const std::string& expression) {
    return std::make_unique<ExpressionPattern>(expression);
}

std::unique_ptr<Pattern> PatternFactory::createFieldPattern(int field_num, const std::string& pattern) {
    return std::make_unique<FieldPattern>(field_num, pattern);
}

std::unique_ptr<Pattern> PatternFactory::createRangePattern(const std::string& start, const std::string& end) {
    auto start_pattern = createPattern(start);
    auto end_pattern = createPattern(end);
    return std::make_unique<RangePattern>(std::move(start_pattern), std::move(end_pattern));
}

std::unique_ptr<Pattern> PatternFactory::createCompoundPattern(const std::string& op) {
    return std::make_unique<CompoundPattern>(op);
}

} // namespace pawk
