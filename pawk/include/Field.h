#pragma once

#include <string>
#include <vector>
#include <regex>
#include <iostream>

namespace pawk {

class Field {
private:
    std::string value_;
    int index_;
    
public:
    Field(const std::string& value = "", int index = 0);
    
    // 基本访问器
    const std::string& value() const;
    int index() const;
    
    // 设置器
    void setValue(const std::string& value);
    void setIndex(int index);
    
    // 类型转换
    operator std::string() const;
    operator int() const;
    operator double() const;
    
    // 字符串操作
    size_t length() const;
    bool empty() const;
    
    // 比较操作
    bool operator==(const std::string& other) const;
    bool operator!=(const std::string& other) const;
    bool operator<(const std::string& other) const;
    bool operator>(const std::string& other) const;
    
    // 匹配操作
    bool match(const std::string& pattern) const;
    bool match(const std::regex& regex_pattern) const;
    
    // 替换操作
    std::string replace(const std::string& pattern, const std::string& replacement) const;
    
    // 子字符串
    std::string substr(size_t pos, size_t len = std::string::npos) const;
    
    // 分割
    std::vector<std::string> split(const std::string& delimiter) const;
    
    // 格式化输出
    friend std::ostream& operator<<(std::ostream& os, const Field& field);
};

} // namespace pawk
