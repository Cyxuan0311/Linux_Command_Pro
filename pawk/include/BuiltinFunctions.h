#pragma once

#include <string>
#include <vector>
#include <ctime>
#include <cstdlib>

namespace pawk {

class BuiltinFunctions {
public:
    // 数学函数
    static double sqrt_func(double x);
    static double sin_func(double x);
    static double cos_func(double x);
    static double tan_func(double x);
    static double log_func(double x);
    static double log10_func(double x);
    static double exp_func(double x);
    static double pow_func(double x, double y);
    static int int_(double x);
    static double rand();
    static int rand_int(int max);
    
    // 字符串函数
    static int length(const std::string& str);
    static std::string toupper(const std::string& str);
    static std::string tolower(const std::string& str);
    static std::string substr(const std::string& str, int start, int length = -1);
    static int index(const std::string& str, const std::string& substr);
    static std::string gsub(const std::string& pattern, const std::string& replacement, std::string& str);
    static std::string sub(const std::string& pattern, const std::string& replacement, std::string& str);
    static std::string gensub(const std::string& pattern, const std::string& replacement, int how, const std::string& str);
    static std::string match(const std::string& str, const std::string& pattern);
    static std::string split_func(const std::string& str, const std::string& fs, std::vector<std::string>& array);
    
    // 时间函数
    static int systime();
    static std::string strftime_func(const std::string& format, int timestamp = -1);
    static int mktime_func(int year, int month, int day, int hour = 0, int min = 0, int sec = 0);
    
    // 类型转换函数
    static std::string sprintf_func(const std::string& format, const std::vector<std::string>& args);
    static std::string sprintf_func(const std::string& format, double value);
    static std::string sprintf_func(const std::string& format, int value);
    
    // 数组函数
    static int asort(std::vector<std::string>& array);
    static int asorti(std::vector<std::string>& array);
    static int split_array(const std::string& str, const std::string& fs, std::vector<std::string>& array);
    
    // 位操作函数
    static int and_func(int a, int b);
    static int or_func(int a, int b);
    static int xor_func(int a, int b);
    static int compl_func(int a);
    static int lshift(int a, int b);
    static int rshift(int a, int b);
    
    // 其他函数
    static std::string getline_func();
    static int close_func(const std::string& filename);
    static std::string system_func(const std::string& command);
};

} // namespace pawk
