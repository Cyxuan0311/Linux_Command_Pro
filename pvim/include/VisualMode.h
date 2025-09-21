#pragma once
#include "Editor.h"

namespace pvim {
class VisualMode {
private:
    Editor* editor_;
    TextPosition selection_start_;
    TextPosition selection_end_;
    
public:
    VisualMode(Editor* editor);
    void handleKey(int key);
    void startSelection();
    void updateSelection();
    void endSelection();
    void clearSelection();
};
}
