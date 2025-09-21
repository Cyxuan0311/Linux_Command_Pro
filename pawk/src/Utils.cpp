#include "Utils.h"
#include "Constants.h"
#include <iostream>
#include <regex>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <cctype>

namespace pawk {

std::string Utils::toupper(const std::string& str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(), ::toupper);
    return result;
}

std::string Utils::tolower(const std::string& str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(), ::tolower);
    return result;
}

std::string Utils::trim(const std::string& str) {
    return ltrim(rtrim(str));
}

std::string Utils::ltrim(const std::string& str) {
    size_t start = str.find_first_not_of(" \t\n\r\f\v");
    return (start == std::string::npos) ? "" : str.substr(start);
}

std::string Utils::rtrim(const std::string& str) {
    size_t end = str.find_last_not_of(" \t\n\r\f\v");
    return (end == std::string::npos) ? "" : str.substr(0, end + 1);
}

std::vector<std::string> Utils::split(const std::string& str, const std::string& delimiter) {
    std::vector<std::string> result;
    std::string s = str;
    size_t pos = 0;
    
    while ((pos = s.find(delimiter)) != std::string::npos) {
        result.push_back(s.substr(0, pos));
        s.erase(0, pos + delimiter.length());
    }
    result.push_back(s);
    return result;
}

std::vector<std::string> Utils::splitRegex(const std::string& str, const std::string& pattern) {
    std::vector<std::string> result;
    std::regex regex_pattern(pattern);
    std::sregex_token_iterator iter(str.begin(), str.end(), regex_pattern, -1);
    std::sregex_token_iterator end;
    
    for (; iter != end; ++iter) {
        result.push_back(*iter);
    }
    return result;
}

bool Utils::isNumeric(const std::string& str) {
    if (str.empty()) return false;
    
    size_t start = 0;
    if (str[0] == '-' || str[0] == '+') start = 1;
    if (start >= str.length()) return false;
    
    bool hasDot = false;
    for (size_t i = start; i < str.length(); ++i) {
        if (str[i] == '.') {
            if (hasDot) return false;
            hasDot = true;
        } else if (!std::isdigit(str[i])) {
            return false;
        }
    }
    return true;
}

bool Utils::isInteger(const std::string& str) {
    if (str.empty()) return false;
    
    size_t start = 0;
    if (str[0] == '-' || str[0] == '+') start = 1;
    if (start >= str.length()) return false;
    
    for (size_t i = start; i < str.length(); ++i) {
        if (!std::isdigit(str[i])) {
            return false;
        }
    }
    return true;
}

bool Utils::isFloat(const std::string& str) {
    if (str.empty()) return false;
    
    size_t start = 0;
    if (str[0] == '-' || str[0] == '+') start = 1;
    if (start >= str.length()) return false;
    
    bool hasDot = false;
    for (size_t i = start; i < str.length(); ++i) {
        if (str[i] == '.') {
            if (hasDot) return false;
            hasDot = true;
        } else if (!std::isdigit(str[i])) {
            return false;
        }
    }
    return hasDot;
}

bool Utils::isUrl(const std::string& str) {
    return str.find("http") != std::string::npos || 
           str.find("https") != std::string::npos ||
           str.find("ftp") != std::string::npos;
}

std::string Utils::formatNumber(double value, int precision) {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(precision) << value;
    return oss.str();
}

std::string Utils::padString(const std::string& str, size_t width, char pad) {
    if (str.length() >= width) return str;
    return str + std::string(width - str.length(), pad);
}

bool Utils::regexMatch(const std::string& str, const std::string& pattern) {
    try {
        std::regex regex_pattern(pattern);
        return std::regex_match(str, regex_pattern);
    } catch (const std::regex_error& e) {
        printError("正则表达式错误: " + std::string(e.what()));
        return false;
    }
}

std::string Utils::regexReplace(const std::string& str, const std::string& pattern, const std::string& replacement) {
    try {
        std::regex regex_pattern(pattern);
        return std::regex_replace(str, regex_pattern, replacement);
    } catch (const std::regex_error& e) {
        printError("正则表达式错误: " + std::string(e.what()));
        return str;
    }
}

std::vector<std::string> Utils::regexFindAll(const std::string& str, const std::string& pattern) {
    std::vector<std::string> result;
    try {
        std::regex regex_pattern(pattern);
        std::sregex_iterator iter(str.begin(), str.end(), regex_pattern);
        std::sregex_iterator end;
        
        for (; iter != end; ++iter) {
            result.push_back(iter->str());
        }
    } catch (const std::regex_error& e) {
        printError("正则表达式错误: " + std::string(e.what()));
    }
    return result;
}

std::string Utils::colorize(const std::string& text, const std::string& color) {
    return color + text + Colors::RESET;
}

std::string Utils::highlightField(const std::string& field, int fieldNumber) {
    std::string color;
    
    if (isInteger(field)) {
        color = Colors::GREEN;
    } else if (isFloat(field)) {
        color = Colors::BLUE;
    } else if (isUrl(field)) {
        color = Colors::MAGENTA;
    } else {
        color = Colors::WHITE;
    }
    
    return color + field + Colors::RESET;
}

void Utils::printError(const std::string& message) {
    std::cerr << Colors::RED << "错误: " << message << Colors::RESET << std::endl;
}

void Utils::printWarning(const std::string& message) {
    std::cerr << Colors::YELLOW << "警告: " << message << Colors::RESET << std::endl;
}

void Utils::printInfo(const std::string& message) {
    std::cout << Colors::CYAN << "信息: " << message << Colors::RESET << std::endl;
}

} // namespace pawk
