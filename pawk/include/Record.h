#pragma once

#include "Field.h"
#include <string>
#include <vector>
#include <regex>
#include <iostream>

namespace pawk {

class Record {
private:
    std::vector<Field> fields_;
    std::string raw_line_;
    std::string field_separator_;
    int record_number_;
    
public:
    Record(const std::string& line = "", const std::string& fs = "\\s+", int record_num = 0);
    
    // 解析字段
    void parseFields();
    
    // 字段访问
    Field& operator[](int index);
    const Field& operator[](int index) const;
    
    // 记录信息
    int NF() const;  // 字段数量
    const std::string& $0() const;  // 原始行
    int NR() const;  // 记录号
    
    // 设置器
    void setFieldSeparator(const std::string& fs);
    void setRecordNumber(int record_num);
    
    // 输出操作
    void print() const;
    void print(const std::vector<int>& field_indices) const;
    void printf(const std::string& format) const;
    
    // 字段操作
    void addField(const std::string& value);
    void removeField(int index);
    void clearFields();
    
    // 查找操作
    int findField(const std::string& value) const;
    std::vector<int> findFields(const std::string& pattern) const;
    
    // 验证
    bool isValid() const;
    bool hasField(int index) const;
};

} // namespace pawk
