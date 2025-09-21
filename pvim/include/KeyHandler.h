#pragma once
#include "Editor.h"

namespace pvim {
class KeyHandler {
private:
    Editor* editor_;
    
public:
    KeyHandler(Editor* editor);
    bool handleKey(int key);
    void processKeySequence(const std::string& sequence);
};
}
