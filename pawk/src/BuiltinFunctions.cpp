#include "BuiltinFunctions.h"
#include "Utils.h"
#include <cmath>
#include <ctime>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <regex>
#include <cstdio>

namespace pawk {

// 数学函数
double BuiltinFunctions::sqrt_func(double x) {
    return std::sqrt(x);
}

double BuiltinFunctions::sin_func(double x) {
    return std::sin(x);
}

double BuiltinFunctions::cos_func(double x) {
    return std::cos(x);
}

double BuiltinFunctions::tan_func(double x) {
    return std::tan(x);
}

double BuiltinFunctions::log_func(double x) {
    return std::log(x);
}

double BuiltinFunctions::log10_func(double x) {
    return std::log10(x);
}

double BuiltinFunctions::exp_func(double x) {
    return std::exp(x);
}

double BuiltinFunctions::pow_func(double x, double y) {
    return std::pow(x, y);
}

int BuiltinFunctions::int_(double x) {
    return static_cast<int>(x);
}

double BuiltinFunctions::rand() {
    return static_cast<double>(std::rand()) / RAND_MAX;
}

int BuiltinFunctions::rand_int(int max) {
    return std::rand() % max;
}

// 字符串函数
int BuiltinFunctions::length(const std::string& str) {
    return str.length();
}

std::string BuiltinFunctions::toupper(const std::string& str) {
    return Utils::toupper(str);
}

std::string BuiltinFunctions::tolower(const std::string& str) {
    return Utils::tolower(str);
}

std::string BuiltinFunctions::substr(const std::string& str, int start, int length) {
    if (start < 1) start = 1;
    if (length == -1) length = str.length() - start + 1;
    if (start > static_cast<int>(str.length())) return "";
    return str.substr(start - 1, length);
}

int BuiltinFunctions::index(const std::string& str, const std::string& substr) {
    size_t pos = str.find(substr);
    return pos == std::string::npos ? 0 : pos + 1;
}

std::string BuiltinFunctions::gsub(const std::string& pattern, const std::string& replacement, std::string& str) {
    std::regex regex_pattern(pattern);
    str = std::regex_replace(str, regex_pattern, replacement);
    return str;
}

std::string BuiltinFunctions::sub(const std::string& pattern, const std::string& replacement, std::string& str) {
    std::regex regex_pattern(pattern);
    str = std::regex_replace(str, regex_pattern, replacement, std::regex_constants::format_first_only);
    return str;
}

std::string BuiltinFunctions::gensub(const std::string& pattern, const std::string& replacement, int how, const std::string& str) {
    std::regex regex_pattern(pattern);
    if (how == 0) {
        return std::regex_replace(str, regex_pattern, replacement);
    } else {
        return std::regex_replace(str, regex_pattern, replacement, std::regex_constants::format_first_only);
    }
}

std::string BuiltinFunctions::match(const std::string& str, const std::string& pattern) {
    std::regex regex_pattern(pattern);
    std::smatch match_result;
    if (std::regex_search(str, match_result, regex_pattern)) {
        return match_result[0].str();
    }
    return "";
}

std::string BuiltinFunctions::split_func(const std::string& str, const std::string& fs, std::vector<std::string>& array) {
    array = Utils::splitRegex(str, fs);
    return array.empty() ? "" : array[0];
}

// 时间函数
int BuiltinFunctions::systime() {
    return time(nullptr);
}

std::string BuiltinFunctions::strftime_func(const std::string& format, int timestamp) {
    if (timestamp == -1) timestamp = time(nullptr);
    time_t t = timestamp;
    char buffer[256];
    std::strftime(buffer, sizeof(buffer), format.c_str(), localtime(&t));
    return std::string(buffer);
}

int BuiltinFunctions::mktime_func(int year, int month, int day, int hour, int min, int sec) {
    struct tm timeinfo = {};
    timeinfo.tm_year = year - 1900;
    timeinfo.tm_mon = month - 1;
    timeinfo.tm_mday = day;
    timeinfo.tm_hour = hour;
    timeinfo.tm_min = min;
    timeinfo.tm_sec = sec;
    return mktime(&timeinfo);
}

// 类型转换函数
std::string BuiltinFunctions::sprintf_func(const std::string& format, const std::vector<std::string>& args) {
    // 简化的sprintf实现
    std::string result = format;
    std::regex format_regex("%[sd]");
    std::smatch match;
    size_t arg_index = 0;
    
    while (std::regex_search(result, match, format_regex) && arg_index < args.size()) {
        result = std::regex_replace(result, format_regex, args[arg_index], std::regex_constants::format_first_only);
        arg_index++;
    }
    
    return result;
}

std::string BuiltinFunctions::sprintf_func(const std::string& format, double value) {
    char buffer[256];
    std::snprintf(buffer, sizeof(buffer), format.c_str(), value);
    return std::string(buffer);
}

std::string BuiltinFunctions::sprintf_func(const std::string& format, int value) {
    char buffer[256];
    std::snprintf(buffer, sizeof(buffer), format.c_str(), value);
    return std::string(buffer);
}

// 数组函数
int BuiltinFunctions::asort(std::vector<std::string>& array) {
    std::sort(array.begin(), array.end());
    return array.size();
}

int BuiltinFunctions::asorti(std::vector<std::string>& array) {
    std::sort(array.begin(), array.end());
    return array.size();
}

int BuiltinFunctions::split_array(const std::string& str, const std::string& fs, std::vector<std::string>& array) {
    array = Utils::splitRegex(str, fs);
    return array.size();
}

// 位操作函数
int BuiltinFunctions::and_func(int a, int b) {
    return a & b;
}

int BuiltinFunctions::or_func(int a, int b) {
    return a | b;
}

int BuiltinFunctions::xor_func(int a, int b) {
    return a ^ b;
}

int BuiltinFunctions::compl_func(int a) {
    return ~a;
}

int BuiltinFunctions::lshift(int a, int b) {
    return a << b;
}

int BuiltinFunctions::rshift(int a, int b) {
    return a >> b;
}

// 其他函数
std::string BuiltinFunctions::getline_func() {
    std::string line;
    std::getline(std::cin, line);
    return line;
}

int BuiltinFunctions::close_func(const std::string& filename) {
    // 简化实现，实际应该管理文件句柄
    return 0;
}

std::string BuiltinFunctions::system_func(const std::string& command) {
    char buffer[128];
    std::string result = "";
    FILE* pipe = popen(command.c_str(), "r");
    if (!pipe) return "";
    
    while (fgets(buffer, sizeof buffer, pipe) != nullptr) {
        result += buffer;
    }
    pclose(pipe);
    return result;
}

} // namespace pawk
