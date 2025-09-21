#pragma once

#include "Record.h"
#include "Variables.h"
#include <string>
#include <vector>
#include <memory>

namespace pawk {

class Action {
public:
    virtual ~Action() = default;
    virtual void execute(const Record& record, Variables& variables) = 0;
    virtual std::string getDescription() const = 0;
};

class PrintAction : public Action {
private:
    std::vector<int> field_indices_;
    std::string format_;
    bool color_output_;
    
public:
    PrintAction(const std::vector<int>& indices = {}, bool color = false);
    PrintAction(const std::string& format, bool color = false);
    void execute(const Record& record, Variables& variables) override;
    std::string getDescription() const override;
};

class PrintfAction : public Action {
private:
    std::string format_;
    bool color_output_;
    
public:
    PrintfAction(const std::string& format, bool color = false);
    void execute(const Record& record, Variables& variables) override;
    std::string getDescription() const override;
};

class AssignmentAction : public Action {
private:
    std::string variable_name_;
    std::string expression_;
    
public:
    AssignmentAction(const std::string& var_name, const std::string& expr);
    void execute(const Record& record, Variables& variables) override;
    std::string getDescription() const override;
};

class IfAction : public Action {
private:
    std::string condition_;
    std::unique_ptr<Action> then_action_;
    std::unique_ptr<Action> else_action_;
    
public:
    IfAction(const std::string& condition, std::unique_ptr<Action> then_action, std::unique_ptr<Action> else_action = nullptr);
    void execute(const Record& record, Variables& variables) override;
    std::string getDescription() const override;
};

class ForAction : public Action {
private:
    std::string variable_name_;
    std::string start_expr_;
    std::string end_expr_;
    std::string step_expr_;
    std::unique_ptr<Action> body_action_;
    
public:
    ForAction(const std::string& var_name, const std::string& start, const std::string& end, 
              const std::string& step, std::unique_ptr<Action> body);
    void execute(const Record& record, Variables& variables) override;
    std::string getDescription() const override;
};

class WhileAction : public Action {
private:
    std::string condition_;
    std::unique_ptr<Action> body_action_;
    
public:
    WhileAction(const std::string& condition, std::unique_ptr<Action> body);
    void execute(const Record& record, Variables& variables) override;
    std::string getDescription() const override;
};

class FunctionCallAction : public Action {
private:
    std::string function_name_;
    std::vector<std::string> arguments_;
    
public:
    FunctionCallAction(const std::string& func_name, const std::vector<std::string>& args);
    void execute(const Record& record, Variables& variables) override;
    std::string getDescription() const override;
};

class BlockAction : public Action {
private:
    std::vector<std::unique_ptr<Action>> actions_;
    
public:
    BlockAction();
    void addAction(std::unique_ptr<Action> action);
    void execute(const Record& record, Variables& variables) override;
    std::string getDescription() const override;
};

class ActionFactory {
public:
    static std::unique_ptr<Action> createAction(const std::string& action_str);
    static std::unique_ptr<Action> createPrintAction(const std::string& args);
    static std::unique_ptr<Action> createPrintfAction(const std::string& format);
    static std::unique_ptr<Action> createAssignmentAction(const std::string& assignment);
    static std::unique_ptr<Action> createIfAction(const std::string& condition, const std::string& then_action, const std::string& else_action = "");
    static std::unique_ptr<Action> createForAction(const std::string& var, const std::string& start, const std::string& end, const std::string& step, const std::string& body);
    static std::unique_ptr<Action> createWhileAction(const std::string& condition, const std::string& body);
    static std::unique_ptr<Action> createFunctionCallAction(const std::string& func_name, const std::vector<std::string>& args);
    static std::unique_ptr<Action> createBlockAction(const std::vector<std::string>& action_strings);
};

} // namespace pawk
