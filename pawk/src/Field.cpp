#include "Field.h"
#include "Utils.h"
#include <iostream>
#include <regex>
#include <stdexcept>

namespace pawk {

Field::Field(const std::string& value, int index) 
    : value_(value), index_(index) {
}

const std::string& Field::value() const {
    return value_;
}

int Field::index() const {
    return index_;
}

void Field::setValue(const std::string& value) {
    value_ = value;
}

void Field::setIndex(int index) {
    index_ = index;
}

Field::operator std::string() const {
    return value_;
}

Field::operator int() const {
    try {
        return std::stoi(value_);
    } catch (const std::invalid_argument& e) {
        return 0;
    } catch (const std::out_of_range& e) {
        return 0;
    }
}

Field::operator double() const {
    try {
        return std::stod(value_);
    } catch (const std::invalid_argument& e) {
        return 0.0;
    } catch (const std::out_of_range& e) {
        return 0.0;
    }
}

size_t Field::length() const {
    return value_.length();
}

bool Field::empty() const {
    return value_.empty();
}

bool Field::operator==(const std::string& other) const {
    return value_ == other;
}

bool Field::operator!=(const std::string& other) const {
    return value_ != other;
}

bool Field::operator<(const std::string& other) const {
    return value_ < other;
}

bool Field::operator>(const std::string& other) const {
    return value_ > other;
}

bool Field::match(const std::string& pattern) const {
    try {
        std::regex regex_pattern(pattern);
        return std::regex_search(value_, regex_pattern);
    } catch (const std::regex_error& e) {
        Utils::printError("正则表达式错误: " + std::string(e.what()));
        return false;
    }
}

bool Field::match(const std::regex& regex_pattern) const {
    return std::regex_search(value_, regex_pattern);
}

std::string Field::replace(const std::string& pattern, const std::string& replacement) const {
    return Utils::regexReplace(value_, pattern, replacement);
}

std::string Field::substr(size_t pos, size_t len) const {
    return value_.substr(pos, len);
}

std::vector<std::string> Field::split(const std::string& delimiter) const {
    return Utils::split(value_, delimiter);
}

std::ostream& operator<<(std::ostream& os, const Field& field) {
    os << field.value_;
    return os;
}

} // namespace pawk
