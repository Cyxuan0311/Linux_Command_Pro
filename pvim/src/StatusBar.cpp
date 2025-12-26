#include "StatusBar.h"
#include "Buffer.h"
#include <ncurses.h>
#include <sstream>

namespace pvim {

StatusBar::StatusBar(Editor* editor) : editor_(editor), width_(0), height_(1) {
}

void StatusBar::draw() {
    if (!editor_) return;
    
    // 获取屏幕尺寸
    int screen_cols = editor_->getScreenCols();
    int screen_rows = editor_->getScreenRows();
    
    // 设置状态栏位置
    int y = screen_rows - 1;
    
    // 清除状态栏行
    move(y, 0);
    clrtoeol();
    
    // 设置颜色
    attron(COLOR_PAIR(2));
    
    // 显示文件名
    std::string filename = editor_->getFilename();
    if (filename.empty()) {
        filename = "[未命名]";
    }
    
    // 显示修改状态
    std::string modified = editor_->isModified() ? "[+]" : "";
    
    // 显示模式
    std::string mode;
    switch (editor_->getMode()) {
        case EditorMode::COMMAND:
            mode = "命令";
            break;
        case EditorMode::INSERT:
            mode = "插入";
            break;
        case EditorMode::VISUAL:
            mode = "可视";
            break;
        case EditorMode::EX_COMMAND:
            mode = "命令";
            break;
        case EditorMode::HELP:
            mode = "帮助";
            break;
    }
    
    // 显示光标位置
    std::string cursor_pos;
    if (editor_->getBuffer()) {
        TextPosition pos = editor_->getBuffer()->getCursor();
        std::ostringstream oss;
        oss << "行" << (pos.line + 1) << ",列" << (pos.column + 1);
        cursor_pos = oss.str();
    }
    
    // 构建状态栏内容
    std::string status = filename + modified + " [" + mode + "] " + cursor_pos;
    
    // 显示状态消息
    std::string message = editor_->getStatusMessage();
    if (!message.empty()) {
        status = message;
    }
    
    // 截断过长的状态栏
    if (status.length() > static_cast<size_t>(screen_cols)) {
        status = status.substr(0, screen_cols - 3) + "...";
    }
    
    // 显示状态栏
    mvprintw(y, 0, "%s", status.c_str());
    
    // 重置颜色
    attroff(COLOR_PAIR(2));
}

void StatusBar::resize(int width, int height) {
    width_ = width;
    height_ = height;
}

void StatusBar::setMessage(const std::string& message) {
    (void)message;
    // 消息通过编辑器设置
}

} // namespace pvim
