#pragma once
#include "Editor.h"

namespace pvim {
class InsertMode {
private:
    Editor* editor_;
    
public:
    InsertMode(Editor* editor);
    void handleKey(int key);
    void insertChar(char ch);
    void insertString(const std::string& str);
    void deleteChar();
    void newLine();
};
}
