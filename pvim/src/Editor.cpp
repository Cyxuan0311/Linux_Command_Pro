#include "Editor.h"
#include "Buffer.h"
#include "Window.h"
#include "CommandMode.h"
#include "InsertMode.h"
#include "VisualMode.h"
#include "FileManager.h"
#include "SyntaxHighlighter.h"
#include "KeyHandler.h"
#include "StatusBar.h"
#include "Config.h"
#include <ncurses.h>
#include <iostream>
#include <stdexcept>

namespace pvim {

Editor::Editor() 
    : current_mode_(EditorMode::COMMAND)
    , running_(false)
    , modified_(false)
    , screen_rows_(0)
    , screen_cols_(0) {
}

Editor::~Editor() {
    shutdown();
}

bool Editor::initialize() {
    // 初始化ncurses
    initscr();
    raw();
    keypad(stdscr, TRUE);
    noecho();
    curs_set(0);
    
    // 启用颜色支持
    if (has_colors()) {
        start_color();
        use_default_colors();
        
        // 定义颜色对
        init_pair(1, COLOR_BLACK, COLOR_WHITE);    // 正常文本
        init_pair(2, COLOR_WHITE, COLOR_BLUE);     // 状态栏
        init_pair(3, COLOR_YELLOW, COLOR_BLACK);   // 命令模式
        init_pair(4, COLOR_GREEN, COLOR_BLACK);    // 插入模式
        init_pair(5, COLOR_RED, COLOR_BLACK);      // 错误
        init_pair(6, COLOR_CYAN, COLOR_BLACK);     // 注释
        init_pair(7, COLOR_MAGENTA, COLOR_BLACK);  // 关键字
        init_pair(8, COLOR_YELLOW, COLOR_BLACK);   // 字符串
    }
    
    // 获取屏幕尺寸
    updateScreenSize();
    
    // 创建核心组件
    try {
        buffer_ = std::make_unique<Buffer>();
        window_ = std::make_unique<Window>(WindowType::MAIN_EDITOR, 0, 0, screen_cols_, screen_rows_ - 1);
        command_mode_ = std::make_unique<CommandMode>(this);
        insert_mode_ = std::make_unique<InsertMode>(this);
        visual_mode_ = std::make_unique<VisualMode>(this);
        file_manager_ = std::make_unique<FileManager>(this);
        syntax_highlighter_ = std::make_unique<SyntaxHighlighter>(this);
        key_handler_ = std::make_unique<KeyHandler>(this);
        status_bar_ = std::make_unique<StatusBar>(this);
        config_ = std::make_unique<Config>(this);
        
        // 初始化窗口
        if (!window_->create()) {
            return false;
        }
        
        // 设置状态消息
        setStatusMessage("pvim - 优化版vim编辑器 | 按 :q 退出");
        
        return true;
        
    } catch (const std::exception& e) {
        std::cerr << "初始化错误: " << e.what() << std::endl;
        return false;
    }
}

void Editor::run() {
    running_ = true;
    
    while (running_) {
        updateScreenSize();
        refreshDisplay();
        
        (void)getch();
        processInput();
    }
}

void Editor::shutdown() {
    running_ = false;
    
    // 清理组件
    config_.reset();
    status_bar_.reset();
    key_handler_.reset();
    syntax_highlighter_.reset();
    file_manager_.reset();
    visual_mode_.reset();
    insert_mode_.reset();
    command_mode_.reset();
    window_.reset();
    buffer_.reset();
    
    // 清理ncurses
    endwin();
}

void Editor::setMode(EditorMode mode) {
    current_mode_ = mode;
    
    // 根据模式更新状态栏
    switch (mode) {
        case EditorMode::COMMAND:
            setStatusMessage("-- 命令模式 --");
            break;
        case EditorMode::INSERT:
            setStatusMessage("-- 插入模式 --");
            break;
        case EditorMode::VISUAL:
            setStatusMessage("-- 可视模式 --");
            break;
        case EditorMode::EX_COMMAND:
            setStatusMessage("-- 命令模式 --");
            break;
        case EditorMode::HELP:
            setStatusMessage("-- 帮助模式 --");
            break;
    }
}

bool Editor::openFile(const std::string& filename) {
    if (!buffer_) {
        return false;
    }
    
    if (buffer_->loadFromFile(filename)) {
        filename_ = filename;
        setStatusMessage("已打开文件: " + filename);
        return true;
    } else {
        setStatusMessage("错误: 无法打开文件 " + filename);
        return false;
    }
}

bool Editor::saveFile() {
    if (!buffer_ || filename_.empty()) {
        return false;
    }
    
    if (buffer_->saveToFile(filename_)) {
        modified_ = false;
        setStatusMessage("已保存文件: " + filename_);
        return true;
    } else {
        setStatusMessage("错误: 无法保存文件 " + filename_);
        return false;
    }
}

bool Editor::saveAsFile(const std::string& filename) {
    if (!buffer_) {
        return false;
    }
    
    if (buffer_->saveToFile(filename)) {
        filename_ = filename;
        modified_ = false;
        setStatusMessage("已保存文件: " + filename);
        return true;
    } else {
        setStatusMessage("错误: 无法保存文件 " + filename);
        return false;
    }
}

void Editor::setStatusMessage(const std::string& message) {
    status_message_ = message;
}

void Editor::updateScreenSize() {
    int new_rows, new_cols;
    getmaxyx(stdscr, new_rows, new_cols);
    
    if (new_rows != screen_rows_ || new_cols != screen_cols_) {
        screen_rows_ = new_rows;
        screen_cols_ = new_cols;
        
        // 通知窗口管理器调整大小
        if (window_) {
            window_->resize(screen_cols_, screen_rows_ - 1);
        }
        
        // 通知状态栏调整大小
        if (status_bar_) {
            status_bar_->resize(screen_cols_, 1);
        }
    }
}

void Editor::processInput() {
    int key = getch();
    
    // 处理特殊键
    switch (key) {
        case KEY_RESIZE:
            handleResize();
            return;
        case 27: // ESC键
            if (current_mode_ == EditorMode::INSERT || current_mode_ == EditorMode::VISUAL) {
                setMode(EditorMode::COMMAND);
            }
            return;
        case 3: // Ctrl+C
            if (current_mode_ == EditorMode::INSERT) {
                setMode(EditorMode::COMMAND);
            }
            return;
    }
    
    // 根据当前模式处理输入
    switch (current_mode_) {
        case EditorMode::COMMAND:
            if (command_mode_) {
                command_mode_->handleKey(key);
            }
            break;
        case EditorMode::INSERT:
            if (insert_mode_) {
                insert_mode_->handleKey(key);
            }
            break;
        case EditorMode::VISUAL:
            if (visual_mode_) {
                visual_mode_->handleKey(key);
            }
            break;
        case EditorMode::EX_COMMAND:
            if (command_mode_) {
                command_mode_->handleExKey(key);
            }
            break;
        case EditorMode::HELP:
            // 处理帮助模式
            break;
    }
}

void Editor::refreshDisplay() {
    if (!window_ || !buffer_) {
        return;
    }
    
    // 清除屏幕
    clear();
    
    // 绘制主窗口
    window_->clear();
    
    // 绘制文本内容
    for (int i = 0; i < screen_rows_ - 1 && i < static_cast<int>(buffer_->getLineCount()); i++) {
        std::string line = buffer_->getLineContent(i);
        
        // 截断过长的行
        if (line.length() > static_cast<size_t>(screen_cols_)) {
            line = line.substr(0, screen_cols_);
        }
        
        // 绘制行
        window_->print(i, 0, line);
    }
    
    // 绘制状态栏
    if (status_bar_) {
        status_bar_->draw();
    }
    
    // 刷新显示
    window_->refresh();
    refresh();
}

void Editor::handleResize() {
    updateScreenSize();
    refreshDisplay();
}

} // namespace pvim
