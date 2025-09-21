#pragma once

#include <string>
#include <map>
#include <vector>
#include <memory>

namespace pawk {

class Variable {
public:
    enum Type {
        STRING,
        NUMERIC,
        ARRAY
    };
    
private:
    Type type_;
    std::string string_value_;
    double numeric_value_;
    std::map<std::string, std::string> array_values_;
    
public:
    Variable();
    Variable(const std::string& value);
    Variable(double value);
    Variable(const std::map<std::string, std::string>& array);
    
    Type getType() const;
    
    // 字符串操作
    void setString(const std::string& value);
    std::string getString() const;
    
    // 数值操作
    void setNumeric(double value);
    double getNumeric() const;
    
    // 数组操作
    void setArray(const std::map<std::string, std::string>& array);
    void setArrayElement(const std::string& key, const std::string& value);
    std::string getArrayElement(const std::string& key) const;
    bool hasArrayElement(const std::string& key) const;
    void removeArrayElement(const std::string& key);
    void clearArray();
    std::vector<std::string> getArrayKeys() const;
    size_t getArraySize() const;
    
    // 类型转换
    operator std::string() const;
    operator double() const;
    operator int() const;
    
    // 比较操作
    bool operator==(const Variable& other) const;
    bool operator!=(const Variable& other) const;
    bool operator<(const Variable& other) const;
    bool operator>(const Variable& other) const;
};

class Variables {
private:
    std::map<std::string, std::unique_ptr<Variable>> variables_;
    
public:
    Variables();
    ~Variables();
    
    // 基本操作
    void setString(const std::string& name, const std::string& value);
    void setNumeric(const std::string& name, double value);
    void setArray(const std::string& name, const std::map<std::string, std::string>& array);
    
    std::string getString(const std::string& name) const;
    double getNumeric(const std::string& name) const;
    std::map<std::string, std::string> getArray(const std::string& name) const;
    
    // 数组元素操作
    void setArrayElement(const std::string& name, const std::string& key, const std::string& value);
    std::string getArrayElement(const std::string& name, const std::string& key) const;
    bool hasArrayElement(const std::string& name, const std::string& key) const;
    void removeArrayElement(const std::string& name, const std::string& key);
    
    // 变量管理
    bool exists(const std::string& name) const;
    void remove(const std::string& name);
    void clear();
    std::vector<std::string> getVariableNames() const;
    
    // 类型检查
    Variable::Type getType(const std::string& name) const;
    bool isString(const std::string& name) const;
    bool isNumeric(const std::string& name) const;
    bool isArray(const std::string& name) const;
    
    // 内置变量
    void setBuiltinVariables();
    void updateBuiltinVariables(const std::string& fs, const std::string& ofs, int nr, int nf);
};

} // namespace pawk
