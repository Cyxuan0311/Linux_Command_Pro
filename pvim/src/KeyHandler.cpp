#include "KeyHandler.h"

namespace pvim {

KeyHandler::KeyHandler(Editor* editor) : editor_(editor) {
}

bool KeyHandler::handleKey(int key) {
    // 处理特殊键序列
    switch (key) {
        case 27: // ESC
            return true;
        case 3: // Ctrl+C
            return true;
        case 4: // Ctrl+D
            return true;
        case 9: // Tab
            return true;
        case 10: // Enter
            return true;
        case 13: // Carriage Return
            return true;
        case 127: // Backspace
            return true;
        default:
            return false;
    }
}

void KeyHandler::processKeySequence(const std::string& sequence) {
    // 处理键序列
    for (char c : sequence) {
        handleKey(static_cast<int>(c));
    }
}

} // namespace pvim
