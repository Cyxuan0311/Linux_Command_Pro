#pragma once
#include "Editor.h"

namespace pvim {
class CommandMode {
private:
    Editor* editor_;
    std::string command_buffer_;
    bool in_ex_mode_;
    
public:
    CommandMode(Editor* editor);
    void handleKey(int key);
    void handleExKey(int key);
    void executeCommand(const std::string& command);
};
}
