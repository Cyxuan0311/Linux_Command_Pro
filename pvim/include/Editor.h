#pragma once

#include <string>
#include <vector>
#include <memory>
#include <map>
#include <functional>
#include <ncurses.h>

namespace pvim {

// 前向声明
class Buffer;
class Window;
class CommandMode;
class InsertMode;
class VisualMode;
class FileManager;
class SyntaxHighlighter;
class KeyHandler;
class StatusBar;
class Config;

// 编辑器模式枚举
enum class EditorMode {
    COMMAND,
    INSERT,
    VISUAL,
    EX_COMMAND,
    HELP
};

// 光标位置结构
struct Cursor {
    int row = 0;
    int col = 0;
    
    Cursor(int r = 0, int c = 0) : row(r), col(c) {}
    bool operator==(const Cursor& other) const {
        return row == other.row && col == other.col;
    }
    bool operator!=(const Cursor& other) const {
        return !(*this == other);
    }
};

// 文本位置结构
struct TextPosition {
    int line = 0;
    int column = 0;
    
    TextPosition(int l = 0, int c = 0) : line(l), column(c) {}
    bool operator==(const TextPosition& other) const {
        return line == other.line && column == other.column;
    }
    bool operator!=(const TextPosition& other) const {
        return !(*this == other);
    }
};

// 编辑器主类
class Editor {
private:
    // 核心组件
    std::unique_ptr<Buffer> buffer_;
    std::unique_ptr<Window> window_;
    std::unique_ptr<CommandMode> command_mode_;
    std::unique_ptr<InsertMode> insert_mode_;
    std::unique_ptr<VisualMode> visual_mode_;
    std::unique_ptr<FileManager> file_manager_;
    std::unique_ptr<SyntaxHighlighter> syntax_highlighter_;
    std::unique_ptr<KeyHandler> key_handler_;
    std::unique_ptr<StatusBar> status_bar_;
    std::unique_ptr<Config> config_;
    
    // 编辑器状态
    EditorMode current_mode_;
    bool running_;
    bool modified_;
    std::string filename_;
    std::string status_message_;
    
    // 窗口信息
    int screen_rows_;
    int screen_cols_;
    
public:
    Editor();
    ~Editor();
    
    // 主要接口
    bool initialize();
    void run();
    void shutdown();
    
    // 模式切换
    void setMode(EditorMode mode);
    EditorMode getMode() const { return current_mode_; }
    
    // 文件操作
    bool openFile(const std::string& filename);
    bool saveFile();
    bool saveAsFile(const std::string& filename);
    
    // 状态管理
    bool isModified() const { return modified_; }
    void setModified(bool modified) { modified_ = modified; }
    const std::string& getFilename() const { return filename_; }
    void setFilename(const std::string& filename) { filename_ = filename; }
    
    // 消息显示
    void setStatusMessage(const std::string& message);
    const std::string& getStatusMessage() const { return status_message_; }
    
    // 获取组件
    Buffer* getBuffer() const { return buffer_.get(); }
    Window* getWindow() const { return window_.get(); }
    CommandMode* getCommandMode() const { return command_mode_.get(); }
    InsertMode* getInsertMode() const { return insert_mode_.get(); }
    VisualMode* getVisualMode() const { return visual_mode_.get(); }
    FileManager* getFileManager() const { return file_manager_.get(); }
    SyntaxHighlighter* getSyntaxHighlighter() const { return syntax_highlighter_.get(); }
    KeyHandler* getKeyHandler() const { return key_handler_.get(); }
    StatusBar* getStatusBar() const { return status_bar_.get(); }
    Config* getConfig() const { return config_.get(); }
    
    // 窗口信息
    int getScreenRows() const { return screen_rows_; }
    int getScreenCols() const { return screen_cols_; }
    
private:
    void updateScreenSize();
    void processInput();
    void refreshDisplay();
    void handleResize();
};

} // namespace pvim
