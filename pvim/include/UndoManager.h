#pragma once
#include "Editor.h"
#include <vector>
#include <string>

namespace pvim {

class UndoManager {
private:
    Editor* editor_;
    std::vector<std::string> undo_stack_;
    std::vector<std::string> redo_stack_;
    int max_levels_;
    
public:
    UndoManager(Editor* editor);
    void pushState(const std::string& state);
    bool undo();
    bool redo();
    bool canUndo() const;
    bool canRedo() const;
    void clear();
    
private:
    std::string getCurrentState() const;
    void restoreState(const std::string& state);
};

} // namespace pvim
