#pragma once

#include "Record.h"
#include "Pattern.h"
#include "Action.h"
#include "Variables.h"
#include "CommandLineParser.h"
#include <string>
#include <vector>
#include <memory>
#include <fstream>

namespace pawk {

class AwkEngine {
private:
    std::vector<std::unique_ptr<Pattern>> patterns_;
    std::vector<std::unique_ptr<Action>> actions_;
    Variables variables_;
    CommandLineOptions options_;
    
    // 统计信息
    int total_records_;
    int processed_records_;
    int matched_records_;
    
public:
    AwkEngine();
    AwkEngine(const CommandLineOptions& options);
    ~AwkEngine();
    
    // 配置
    void setOptions(const CommandLineOptions& options);
    void setFieldSeparator(const std::string& fs);
    void setOutputSeparator(const std::string& ofs);
    void setColorOutput(bool enable);
    void setShowLineNumbers(bool enable);
    void setShowFieldNumbers(bool enable);
    
    // 模式-动作管理
    void addPatternAction(std::unique_ptr<Pattern> pattern, std::unique_ptr<Action> action);
    void addAction(std::unique_ptr<Action> action);
    void addBeginAction(std::unique_ptr<Action> action);
    void addEndAction(std::unique_ptr<Action> action);
    
    // 程序解析
    void parseProgram(const std::string& program);
    void parseProgramFile(const std::string& filename);
    
    // 文件处理
    void processFile(const std::string& filename);
    void processStdin();
    void processStream(std::istream& stream);
    
    // 记录处理
    void processRecord(const Record& record);
    
    // 输出控制
    void printColoredRecord(const Record& record);
    void printRecord(const Record& record);
    
    // 统计信息
    int getTotalRecords() const { return total_records_; }
    int getProcessedRecords() const { return processed_records_; }
    int getMatchedRecords() const { return matched_records_; }
    
    // 变量管理
    Variables& getVariables() { return variables_; }
    const Variables& getVariables() const { return variables_; }
    
    // 内置变量更新
    void updateBuiltinVariables(const Record& record);
    
    // 清理
    void clear();
    void reset();
    
private:
    void initialize();
    void executeBeginActions();
    void executeEndActions();
    void updateStatistics();
};

} // namespace pawk
