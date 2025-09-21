#include "CommandMode.h"
#include <iostream>

namespace pvim {

CommandMode::CommandMode(Editor* editor) 
    : editor_(editor), in_ex_mode_(false) {
}

void CommandMode::handleKey(int key) {
    switch (key) {
        case ':':
            in_ex_mode_ = true;
            command_buffer_.clear();
            editor_->setMode(EditorMode::EX_COMMAND);
            break;
        case 'i':
            editor_->setMode(EditorMode::INSERT);
            break;
        case 'v':
            editor_->setMode(EditorMode::VISUAL);
            break;
        case 'h':
            // 左移光标
            break;
        case 'j':
            // 下移光标
            break;
        case 'k':
            // 上移光标
            break;
        case 'l':
            // 右移光标
            break;
        case 'q':
            if (in_ex_mode_) {
                executeCommand("quit");
            }
            break;
        case 'w':
            if (in_ex_mode_) {
                executeCommand("write");
            }
            break;
    }
}

void CommandMode::handleExKey(int key) {
    switch (key) {
        case '\n':
        case '\r':
            executeCommand(command_buffer_);
            in_ex_mode_ = false;
            editor_->setMode(EditorMode::COMMAND);
            break;
        case 27: // ESC
            in_ex_mode_ = false;
            editor_->setMode(EditorMode::COMMAND);
            break;
        case 127: // Backspace
            if (!command_buffer_.empty()) {
                command_buffer_.pop_back();
            }
            break;
        default:
            if (key >= 32 && key <= 126) {
                command_buffer_ += static_cast<char>(key);
            }
            break;
    }
}

void CommandMode::executeCommand(const std::string& command) {
    if (command == "quit" || command == "q") {
        editor_->shutdown();
    } else if (command == "write" || command == "w") {
        editor_->saveFile();
    } else if (command == "help" || command == "h") {
        editor_->setMode(EditorMode::HELP);
    } else {
        editor_->setStatusMessage("未知命令: " + command);
    }
}

} // namespace pvim
