#include "VisualMode.h"
#include "Buffer.h"

namespace pvim {

VisualMode::VisualMode(Editor* editor) : editor_(editor) {
}

void VisualMode::handleKey(int key) {
    switch (key) {
        case 27: // ESC
            clearSelection();
            editor_->setMode(EditorMode::COMMAND);
            break;
        case 'h':
        case 'j':
        case 'k':
        case 'l':
            updateSelection();
            break;
        case 'd':
            // 删除选中内容
            break;
        case 'y':
            // 复制选中内容
            break;
    }
}

void VisualMode::startSelection() {
    if (editor_->getBuffer()) {
        selection_start_ = editor_->getBuffer()->getCursor();
        selection_end_ = selection_start_;
    }
}

void VisualMode::updateSelection() {
    if (editor_->getBuffer()) {
        selection_end_ = editor_->getBuffer()->getCursor();
        editor_->getBuffer()->setSelection(selection_start_, selection_end_);
    }
}

void VisualMode::endSelection() {
    updateSelection();
}

void VisualMode::clearSelection() {
    if (editor_->getBuffer()) {
        editor_->getBuffer()->clearSelection();
    }
}

} // namespace pvim
