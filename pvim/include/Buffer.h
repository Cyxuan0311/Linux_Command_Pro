#pragma once

#include <string>
#include <vector>
#include <memory>
#include <map>
#include <functional>
#include "Editor.h"

namespace pvim {

// 文本行结构
class TextLine {
private:
    std::string content_;
    std::string rendered_;  // 渲染后的内容（包含语法高亮）
    bool modified_;
    
public:
    TextLine(const std::string& content = "");
    
    // 基本操作
    const std::string& getContent() const { return content_; }
    void setContent(const std::string& content);
    const std::string& getRendered() const { return rendered_; }
    void setRendered(const std::string& rendered) { rendered_ = rendered; }
    
    // 修改状态
    bool isModified() const { return modified_; }
    void setModified(bool modified) { modified_ = modified; }
    
    // 文本操作
    void insert(int pos, const std::string& text);
    void erase(int pos, int length);
    void append(const std::string& text);
    void prepend(const std::string& text);
    
    // 字符操作
    char getChar(int pos) const;
    void setChar(int pos, char c);
    void insertChar(int pos, char c);
    void deleteChar(int pos);
    
    // 长度和位置
    size_t length() const { return content_.length(); }
    size_t renderedLength() const { return rendered_.length(); }
    bool empty() const { return content_.empty(); }
    
    // 查找
    size_t find(const std::string& text, size_t start = 0) const;
    size_t rfind(const std::string& text, size_t start = std::string::npos) const;
    
    // 子字符串
    std::string substr(size_t pos = 0, size_t len = std::string::npos) const;
};

// 文本缓冲区类
class Buffer {
private:
    std::vector<std::unique_ptr<TextLine>> lines_;
    std::string filename_;
    bool modified_;
    bool readonly_;
    std::string encoding_;
    std::string line_ending_;
    
    // 光标位置
    TextPosition cursor_;
    TextPosition saved_cursor_;  // 保存的光标位置
    
    // 选择区域
    TextPosition selection_start_;
    TextPosition selection_end_;
    bool has_selection_;
    
    // 历史记录
    std::vector<std::string> undo_stack_;
    std::vector<std::string> redo_stack_;
    int max_undo_levels_;
    
public:
    Buffer();
    ~Buffer();
    
    // 文件操作
    bool loadFromFile(const std::string& filename);
    bool saveToFile(const std::string& filename = "");
    bool isModified() const { return modified_; }
    void setModified(bool modified) { modified_ = modified; }
    
    // 文件名和属性
    const std::string& getFilename() const { return filename_; }
    void setFilename(const std::string& filename) { filename_ = filename; }
    bool isReadOnly() const { return readonly_; }
    void setReadOnly(bool readonly) { readonly_ = readonly; }
    const std::string& getEncoding() const { return encoding_; }
    void setEncoding(const std::string& encoding) { encoding_ = encoding; }
    
    // 行操作
    size_t getLineCount() const { return lines_.size(); }
    TextLine* getLine(size_t index) const;
    void insertLine(size_t index, const std::string& content = "");
    void deleteLine(size_t index);
    void appendLine(const std::string& content = "");
    void prependLine(const std::string& content = "");
    
    // 文本操作
    void insertText(const TextPosition& pos, const std::string& text);
    void deleteText(const TextPosition& start, const TextPosition& end);
    void replaceText(const TextPosition& start, const TextPosition& end, const std::string& text);
    
    // 光标操作
    const TextPosition& getCursor() const { return cursor_; }
    void setCursor(const TextPosition& pos);
    void moveCursor(int delta_line, int delta_column);
    void moveCursorTo(int line, int column);
    void saveCursor() { saved_cursor_ = cursor_; }
    void restoreCursor() { cursor_ = saved_cursor_; }
    
    // 选择操作
    bool hasSelection() const { return has_selection_; }
    void setSelection(const TextPosition& start, const TextPosition& end);
    void clearSelection();
    TextPosition getSelectionStart() const { return selection_start_; }
    TextPosition getSelectionEnd() const { return selection_end_; }
    std::string getSelectedText() const;
    
    // 查找和替换
    TextPosition find(const std::string& text, const TextPosition& start, bool case_sensitive = true) const;
    TextPosition findNext(const std::string& text, bool case_sensitive = true);
    TextPosition findPrevious(const std::string& text, bool case_sensitive = true);
    int replaceAll(const std::string& search, const std::string& replace, bool case_sensitive = true);
    
    // 撤销/重做
    void pushUndo();
    bool undo();
    bool redo();
    bool canUndo() const;
    bool canRedo() const;
    
    // 内容获取
    std::string getContent() const;
    std::string getLineContent(size_t index) const;
    std::string getRangeContent(const TextPosition& start, const TextPosition& end) const;
    
    // 统计信息
    size_t getTotalLines() const { return lines_.size(); }
    size_t getTotalChars() const;
    size_t getLineLength(size_t index) const;
    
    // 验证
    bool isValidPosition(const TextPosition& pos) const;
    bool isValidLine(size_t line) const;
    bool isValidColumn(size_t line, size_t column) const;
    
private:
    void normalizePosition(TextPosition& pos) const;
    void updateModified();
    void addToUndoStack(const std::string& state);
    void clearRedoStack();
};

} // namespace pvim
