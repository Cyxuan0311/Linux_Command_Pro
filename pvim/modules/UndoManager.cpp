#include "UndoManager.h"
#include "Buffer.h"

namespace pvim {

UndoManager::UndoManager(Editor* editor) : editor_(editor) {
}

void UndoManager::pushState(const std::string& state) {
    undo_stack_.push_back(state);
    if (undo_stack_.size() > static_cast<size_t>(max_levels_)) {
        undo_stack_.erase(undo_stack_.begin());
    }
    redo_stack_.clear();
}

bool UndoManager::undo() {
    if (undo_stack_.empty()) {
        return false;
    }
    
    std::string current_state = getCurrentState();
    redo_stack_.push_back(current_state);
    
    std::string previous_state = undo_stack_.back();
    undo_stack_.pop_back();
    
    restoreState(previous_state);
    return true;
}

bool UndoManager::redo() {
    if (redo_stack_.empty()) {
        return false;
    }
    
    std::string current_state = getCurrentState();
    undo_stack_.push_back(current_state);
    
    std::string next_state = redo_stack_.back();
    redo_stack_.pop_back();
    
    restoreState(next_state);
    return true;
}

bool UndoManager::canUndo() const {
    return !undo_stack_.empty();
}

bool UndoManager::canRedo() const {
    return !redo_stack_.empty();
}

void UndoManager::clear() {
    undo_stack_.clear();
    redo_stack_.clear();
}

std::string UndoManager::getCurrentState() const {
    if (editor_ && editor_->getBuffer()) {
        return editor_->getBuffer()->getContent();
    }
    return "";
}

void UndoManager::restoreState(const std::string& state) {
    (void)state;
    if (editor_ && editor_->getBuffer()) {
        // 简化实现
        editor_->getBuffer()->loadFromFile("temp");
    }
}

} // namespace pvim
