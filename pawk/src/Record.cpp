#include "Record.h"
#include "Utils.h"
#include "Constants.h"
#include <iostream>
#include <regex>
#include <sstream>

namespace pawk {

Record::Record(const std::string& line, const std::string& fs, int record_num)
    : raw_line_(line), field_separator_(fs), record_number_(record_num) {
    parseFields();
}

void Record::parseFields() {
    fields_.clear();
    if (raw_line_.empty()) return;
    
    try {
        std::regex fs_regex(field_separator_);
        std::sregex_token_iterator iter(raw_line_.begin(), raw_line_.end(), fs_regex, -1);
        std::sregex_token_iterator end;
        
        int index = 1;
        for (; iter != end; ++iter, ++index) {
            fields_.emplace_back(*iter, index);
        }
    } catch (const std::regex_error& e) {
        Utils::printError("字段分隔符正则表达式错误: " + std::string(e.what()));
    }
}

Field& Record::operator[](int index) {
    if (index < 1 || index > static_cast<int>(fields_.size())) {
        static Field empty_field;
        return empty_field;
    }
    return fields_[index - 1];
}

const Field& Record::operator[](int index) const {
    if (index < 1 || index > static_cast<int>(fields_.size())) {
        static Field empty_field;
        return empty_field;
    }
    return fields_[index - 1];
}

int Record::NF() const {
    return fields_.size();
}

const std::string& Record::$0() const {
    return raw_line_;
}

int Record::NR() const {
    return record_number_;
}

void Record::setFieldSeparator(const std::string& fs) {
    field_separator_ = fs;
    parseFields();
}

void Record::setRecordNumber(int record_num) {
    record_number_ = record_num;
}

void Record::print() const {
    for (size_t i = 0; i < fields_.size(); ++i) {
        if (i > 0) std::cout << " ";
        std::cout << fields_[i];
    }
    std::cout << std::endl;
}

void Record::print(const std::vector<int>& field_indices) const {
    for (size_t i = 0; i < field_indices.size(); ++i) {
        if (i > 0) std::cout << " ";
        int idx = field_indices[i];
        if (idx >= 1 && idx <= static_cast<int>(fields_.size())) {
            std::cout << fields_[idx - 1];
        }
    }
    std::cout << std::endl;
}

void Record::printf(const std::string& format) const {
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

void Record::addField(const std::string& value) {
    fields_.emplace_back(value, fields_.size() + 1);
}

void Record::removeField(int index) {
    if (index >= 1 && index <= static_cast<int>(fields_.size())) {
        fields_.erase(fields_.begin() + index - 1);
        // 重新编号
        for (size_t i = index - 1; i < fields_.size(); ++i) {
            fields_[i].setIndex(i + 1);
        }
    }
}

void Record::clearFields() {
    fields_.clear();
}

int Record::findField(const std::string& value) const {
    for (size_t i = 0; i < fields_.size(); ++i) {
        if (fields_[i].value() == value) {
            return i + 1;
        }
    }
    return -1;
}

std::vector<int> Record::findFields(const std::string& pattern) const {
    std::vector<int> result;
    for (size_t i = 0; i < fields_.size(); ++i) {
        if (fields_[i].match(pattern)) {
            result.push_back(i + 1);
        }
    }
    return result;
}

bool Record::isValid() const {
    return !raw_line_.empty() && !fields_.empty();
}

bool Record::hasField(int index) const {
    return index >= 1 && index <= static_cast<int>(fields_.size());
}

} // namespace pawk
