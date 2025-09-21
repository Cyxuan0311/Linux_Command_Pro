#pragma once

#include <string>
#include <vector>
#include <regex>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <cctype>

namespace pawk {

class Utils {
public:
    // 字符串工具
    static std::string toupper(const std::string& str);
    static std::string tolower(const std::string& str);
    static std::string trim(const std::string& str);
    static std::string ltrim(const std::string& str);
    static std::string rtrim(const std::string& str);
    
    // 分割字符串
    static std::vector<std::string> split(const std::string& str, const std::string& delimiter);
    static std::vector<std::string> splitRegex(const std::string& str, const std::string& pattern);
    
    // 类型检查
    static bool isNumeric(const std::string& str);
    static bool isInteger(const std::string& str);
    static bool isFloat(const std::string& str);
    static bool isUrl(const std::string& str);
    
    // 格式化
    static std::string formatNumber(double value, int precision = 2);
    static std::string padString(const std::string& str, size_t width, char pad = ' ');
    
    // 正则表达式工具
    static bool regexMatch(const std::string& str, const std::string& pattern);
    static std::string regexReplace(const std::string& str, const std::string& pattern, const std::string& replacement);
    static std::vector<std::string> regexFindAll(const std::string& str, const std::string& pattern);
    
    // 颜色工具
    static std::string colorize(const std::string& text, const std::string& color);
    static std::string highlightField(const std::string& field, int fieldNumber);
    
    // 错误处理
    static void printError(const std::string& message);
    static void printWarning(const std::string& message);
    static void printInfo(const std::string& message);
};

} // namespace pawk
