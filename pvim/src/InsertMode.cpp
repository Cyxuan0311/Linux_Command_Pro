#include "InsertMode.h"
#include "Buffer.h"

namespace pvim {

InsertMode::InsertMode(Editor* editor) : editor_(editor) {
}

void InsertMode::handleKey(int key) {
    switch (key) {
        case 27: // ESC
            editor_->setMode(EditorMode::COMMAND);
            break;
        case '\n':
        case '\r':
            newLine();
            break;
        case 127: // Backspace
            deleteChar();
            break;
        case 3: // Ctrl+C
            editor_->setMode(EditorMode::COMMAND);
            break;
        default:
            if (key >= 32 && key <= 126) {
                insertChar(static_cast<char>(key));
            }
            break;
    }
}

void InsertMode::insertChar(char ch) {
    if (editor_->getBuffer()) {
        TextPosition pos = editor_->getBuffer()->getCursor();
        editor_->getBuffer()->insertText(pos, std::string(1, ch));
        editor_->getBuffer()->moveCursor(0, 1);
        editor_->setModified(true);
    }
}

void InsertMode::insertString(const std::string& str) {
    if (editor_->getBuffer()) {
        TextPosition pos = editor_->getBuffer()->getCursor();
        editor_->getBuffer()->insertText(pos, str);
        editor_->getBuffer()->moveCursor(0, str.length());
        editor_->setModified(true);
    }
}

void InsertMode::deleteChar() {
    if (editor_->getBuffer()) {
        TextPosition pos = editor_->getBuffer()->getCursor();
        if (pos.column > 0) {
            editor_->getBuffer()->moveCursor(0, -1);
            editor_->getBuffer()->deleteText(pos, pos);
            editor_->setModified(true);
        }
    }
}

void InsertMode::newLine() {
    if (editor_->getBuffer()) {
        TextPosition pos = editor_->getBuffer()->getCursor();
        editor_->getBuffer()->insertText(pos, "\n");
        editor_->getBuffer()->moveCursor(1, 0);
        editor_->setModified(true);
    }
}

} // namespace pvim
