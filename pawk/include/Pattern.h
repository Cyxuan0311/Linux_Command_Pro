#pragma once

#include "Record.h"
#include <string>
#include <regex>
#include <memory>

namespace pawk {

class Pattern {
public:
    virtual ~Pattern() = default;
    virtual bool matches(const Record& record) = 0;
    virtual std::string getDescription() const = 0;
};

class RegexPattern : public Pattern {
private:
    std::regex regex_;
    std::string pattern_;
    
public:
    RegexPattern(const std::string& pattern);
    bool matches(const Record& record) override;
    std::string getDescription() const override;
};

class ExpressionPattern : public Pattern {
private:
    std::string expression_;
    
public:
    ExpressionPattern(const std::string& expr);
    bool matches(const Record& record) override;
    std::string getDescription() const override;
};

class RangePattern : public Pattern {
private:
    std::unique_ptr<Pattern> start_pattern_;
    std::unique_ptr<Pattern> end_pattern_;
    bool in_range_;
    
public:
    RangePattern(std::unique_ptr<Pattern> start, std::unique_ptr<Pattern> end);
    bool matches(const Record& record) override;
    std::string getDescription() const override;
};

class FieldPattern : public Pattern {
private:
    int field_number_;
    std::string pattern_;
    std::regex regex_;
    
public:
    FieldPattern(int field_num, const std::string& pattern);
    bool matches(const Record& record) override;
    std::string getDescription() const override;
};

class CompoundPattern : public Pattern {
private:
    std::vector<std::unique_ptr<Pattern>> patterns_;
    std::string operator_;
    
public:
    CompoundPattern(const std::string& op);
    void addPattern(std::unique_ptr<Pattern> pattern);
    bool matches(const Record& record) override;
    std::string getDescription() const override;
};

class PatternFactory {
public:
    static std::unique_ptr<Pattern> createPattern(const std::string& pattern_str);
    static std::unique_ptr<Pattern> createRegexPattern(const std::string& pattern);
    static std::unique_ptr<Pattern> createExpressionPattern(const std::string& expression);
    static std::unique_ptr<Pattern> createFieldPattern(int field_num, const std::string& pattern);
    static std::unique_ptr<Pattern> createRangePattern(const std::string& start, const std::string& end);
    static std::unique_ptr<Pattern> createCompoundPattern(const std::string& op);
};

} // namespace pawk
