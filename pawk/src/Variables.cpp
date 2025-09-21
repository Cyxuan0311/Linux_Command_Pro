#include "Variables.h"
#include "Utils.h"
#include <algorithm>
#include <stdexcept>

namespace pawk {

// Variable 实现
Variable::Variable() : type_(STRING), string_value_(""), numeric_value_(0.0) {
}

Variable::Variable(const std::string& value) : type_(STRING), string_value_(value), numeric_value_(0.0) {
}

Variable::Variable(double value) : type_(NUMERIC), string_value_(""), numeric_value_(value) {
}

Variable::Variable(const std::map<std::string, std::string>& array) : type_(ARRAY), string_value_(""), numeric_value_(0.0) {
    array_values_ = array;
}

Variable::Type Variable::getType() const {
    return type_;
}

void Variable::setString(const std::string& value) {
    type_ = STRING;
    string_value_ = value;
}

std::string Variable::getString() const {
    if (type_ == STRING) {
        return string_value_;
    } else if (type_ == NUMERIC) {
        return Utils::formatNumber(numeric_value_);
    } else if (type_ == ARRAY) {
        return "[array]";
    }
    return "";
}

void Variable::setNumeric(double value) {
    type_ = NUMERIC;
    numeric_value_ = value;
}

double Variable::getNumeric() const {
    if (type_ == NUMERIC) {
        return numeric_value_;
    } else if (type_ == STRING) {
        try {
            return std::stod(string_value_);
        } catch (const std::invalid_argument& e) {
            return 0.0;
        } catch (const std::out_of_range& e) {
            return 0.0;
        }
    }
    return 0.0;
}

void Variable::setArray(const std::map<std::string, std::string>& array) {
    type_ = ARRAY;
    array_values_ = array;
}

void Variable::setArrayElement(const std::string& key, const std::string& value) {
    type_ = ARRAY;
    array_values_[key] = value;
}

std::string Variable::getArrayElement(const std::string& key) const {
    if (type_ == ARRAY) {
        auto it = array_values_.find(key);
        return it != array_values_.end() ? it->second : "";
    }
    return "";
}

bool Variable::hasArrayElement(const std::string& key) const {
    return type_ == ARRAY && array_values_.find(key) != array_values_.end();
}

void Variable::removeArrayElement(const std::string& key) {
    if (type_ == ARRAY) {
        array_values_.erase(key);
    }
}

void Variable::clearArray() {
    if (type_ == ARRAY) {
        array_values_.clear();
    }
}

std::vector<std::string> Variable::getArrayKeys() const {
    std::vector<std::string> keys;
    if (type_ == ARRAY) {
        for (const auto& pair : array_values_) {
            keys.push_back(pair.first);
        }
    }
    return keys;
}

size_t Variable::getArraySize() const {
    return type_ == ARRAY ? array_values_.size() : 0;
}

Variable::operator std::string() const {
    return getString();
}

Variable::operator double() const {
    return getNumeric();
}

Variable::operator int() const {
    return static_cast<int>(getNumeric());
}

bool Variable::operator==(const Variable& other) const {
    if (type_ != other.type_) return false;
    
    switch (type_) {
        case STRING:
            return string_value_ == other.string_value_;
        case NUMERIC:
            return numeric_value_ == other.numeric_value_;
        case ARRAY:
            return array_values_ == other.array_values_;
        default:
            return false;
    }
}

bool Variable::operator!=(const Variable& other) const {
    return !(*this == other);
}

bool Variable::operator<(const Variable& other) const {
    if (type_ != other.type_) return type_ < other.type_;
    
    switch (type_) {
        case STRING:
            return string_value_ < other.string_value_;
        case NUMERIC:
            return numeric_value_ < other.numeric_value_;
        case ARRAY:
            return array_values_.size() < other.array_values_.size();
        default:
            return false;
    }
}

bool Variable::operator>(const Variable& other) const {
    return !(*this < other) && *this != other;
}

// Variables 实现
Variables::Variables() {
    setBuiltinVariables();
}

Variables::~Variables() = default;

void Variables::setString(const std::string& name, const std::string& value) {
    variables_[name] = std::make_unique<Variable>(value);
}

void Variables::setNumeric(const std::string& name, double value) {
    variables_[name] = std::make_unique<Variable>(value);
}

void Variables::setArray(const std::string& name, const std::map<std::string, std::string>& array) {
    variables_[name] = std::make_unique<Variable>(array);
}

std::string Variables::getString(const std::string& name) const {
    auto it = variables_.find(name);
    return it != variables_.end() ? it->second->getString() : "";
}

double Variables::getNumeric(const std::string& name) const {
    auto it = variables_.find(name);
    return it != variables_.end() ? it->second->getNumeric() : 0.0;
}

std::map<std::string, std::string> Variables::getArray(const std::string& name) const {
    auto it = variables_.find(name);
    if (it != variables_.end() && it->second->getType() == Variable::ARRAY) {
        std::map<std::string, std::string> result;
        for (const auto& key : it->second->getArrayKeys()) {
            result[key] = it->second->getArrayElement(key);
        }
        return result;
    }
    return {};
}

void Variables::setArrayElement(const std::string& name, const std::string& key, const std::string& value) {
    if (!exists(name)) {
        variables_[name] = std::make_unique<Variable>();
    }
    variables_[name]->setArrayElement(key, value);
}

std::string Variables::getArrayElement(const std::string& name, const std::string& key) const {
    auto it = variables_.find(name);
    return it != variables_.end() ? it->second->getArrayElement(key) : "";
}

bool Variables::hasArrayElement(const std::string& name, const std::string& key) const {
    auto it = variables_.find(name);
    return it != variables_.end() && it->second->hasArrayElement(key);
}

void Variables::removeArrayElement(const std::string& name, const std::string& key) {
    auto it = variables_.find(name);
    if (it != variables_.end()) {
        it->second->removeArrayElement(key);
    }
}

bool Variables::exists(const std::string& name) const {
    return variables_.find(name) != variables_.end();
}

void Variables::remove(const std::string& name) {
    variables_.erase(name);
}

void Variables::clear() {
    variables_.clear();
    setBuiltinVariables();
}

std::vector<std::string> Variables::getVariableNames() const {
    std::vector<std::string> names;
    for (const auto& pair : variables_) {
        names.push_back(pair.first);
    }
    return names;
}

Variable::Type Variables::getType(const std::string& name) const {
    auto it = variables_.find(name);
    return it != variables_.end() ? it->second->getType() : Variable::STRING;
}

bool Variables::isString(const std::string& name) const {
    return getType(name) == Variable::STRING;
}

bool Variables::isNumeric(const std::string& name) const {
    return getType(name) == Variable::NUMERIC;
}

bool Variables::isArray(const std::string& name) const {
    return getType(name) == Variable::ARRAY;
}

void Variables::setBuiltinVariables() {
    setString("FS", "\\s+");
    setString("OFS", " ");
    setString("RS", "\n");
    setString("ORS", "\n");
    setString("FILENAME", "");
    setNumeric("NR", 0);
    setNumeric("NF", 0);
    setNumeric("FNR", 0);
}

void Variables::updateBuiltinVariables(const std::string& fs, const std::string& ofs, int nr, int nf) {
    setString("FS", fs);
    setString("OFS", ofs);
    setNumeric("NR", nr);
    setNumeric("NF", nf);
    setNumeric("FNR", nr);
}

} // namespace pawk
