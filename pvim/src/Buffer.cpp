#include "Buffer.h"
#include <fstream>
#include <sstream>
#include <algorithm>

namespace pvim {

// TextLine 实现
TextLine::TextLine(const std::string& content) 
    : content_(content), modified_(false) {
    rendered_ = content_;
}

void TextLine::setContent(const std::string& content) {
    content_ = content;
    rendered_ = content;
    modified_ = true;
}

void TextLine::insert(int pos, const std::string& text) {
    if (pos >= 0 && pos <= static_cast<int>(content_.length())) {
        content_.insert(pos, text);
        rendered_ = content_;
        modified_ = true;
    }
}

void TextLine::erase(int pos, int length) {
    if (pos >= 0 && pos < static_cast<int>(content_.length())) {
        content_.erase(pos, length);
        rendered_ = content_;
        modified_ = true;
    }
}

void TextLine::append(const std::string& text) {
    content_ += text;
    rendered_ = content_;
    modified_ = true;
}

void TextLine::prepend(const std::string& text) {
    content_ = text + content_;
    rendered_ = content_;
    modified_ = true;
}

char TextLine::getChar(int pos) const {
    if (pos >= 0 && pos < static_cast<int>(content_.length())) {
        return content_[pos];
    }
    return '\0';
}

void TextLine::setChar(int pos, char c) {
    if (pos >= 0 && pos < static_cast<int>(content_.length())) {
        content_[pos] = c;
        rendered_ = content_;
        modified_ = true;
    }
}

void TextLine::insertChar(int pos, char c) {
    insert(pos, std::string(1, c));
}

void TextLine::deleteChar(int pos) {
    if (pos >= 0 && pos < static_cast<int>(content_.length())) {
        erase(pos, 1);
    }
}

size_t TextLine::find(const std::string& text, size_t start) const {
    return content_.find(text, start);
}

size_t TextLine::rfind(const std::string& text, size_t start) const {
    return content_.rfind(text, start);
}

std::string TextLine::substr(size_t pos, size_t len) const {
    return content_.substr(pos, len);
}

// Buffer 实现
Buffer::Buffer() 
    : modified_(false), readonly_(false), encoding_("utf-8"), line_ending_("\n")
    , has_selection_(false), max_undo_levels_(100) {
    // 添加一个空行
    lines_.push_back(std::make_unique<TextLine>());
}

Buffer::~Buffer() = default;

bool Buffer::loadFromFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        return false;
    }
    
    lines_.clear();
    std::string line;
    while (std::getline(file, line)) {
        lines_.push_back(std::make_unique<TextLine>(line));
    }
    
    // 如果文件为空，添加一个空行
    if (lines_.empty()) {
        lines_.push_back(std::make_unique<TextLine>());
    }
    
    filename_ = filename;
    modified_ = false;
    cursor_ = TextPosition(0, 0);
    clearSelection();
    
    return true;
}

bool Buffer::saveToFile(const std::string& filename) {
    std::string target_file = filename.empty() ? filename_ : filename;
    if (target_file.empty()) {
        return false;
    }
    
    std::ofstream file(target_file);
    if (!file.is_open()) {
        return false;
    }
    
    for (size_t i = 0; i < lines_.size(); i++) {
        file << lines_[i]->getContent();
        if (i < lines_.size() - 1) {
            file << line_ending_;
        }
    }
    
    modified_ = false;
    return true;
}

TextLine* Buffer::getLine(size_t index) const {
    if (index < lines_.size()) {
        return lines_[index].get();
    }
    return nullptr;
}

void Buffer::insertLine(size_t index, const std::string& content) {
    if (index <= lines_.size()) {
        lines_.insert(lines_.begin() + index, std::make_unique<TextLine>(content));
        updateModified();
    }
}

void Buffer::deleteLine(size_t index) {
    if (index < lines_.size() && lines_.size() > 1) {
        lines_.erase(lines_.begin() + index);
        updateModified();
    }
}

void Buffer::appendLine(const std::string& content) {
    lines_.push_back(std::make_unique<TextLine>(content));
    updateModified();
}

void Buffer::prependLine(const std::string& content) {
    lines_.insert(lines_.begin(), std::make_unique<TextLine>(content));
    updateModified();
}

void Buffer::insertText(const TextPosition& pos, const std::string& text) {
    if (!isValidPosition(pos)) {
        return;
    }
    
    TextLine* line = getLine(pos.line);
    if (line) {
        line->insert(pos.column, text);
        updateModified();
    }
}

void Buffer::deleteText(const TextPosition& start, const TextPosition& end) {
    if (!isValidPosition(start) || !isValidPosition(end)) {
        return;
    }
    
    if (start.line == end.line) {
        // 同一行内删除
        TextLine* line = getLine(start.line);
        if (line) {
            line->erase(start.column, end.column - start.column);
            updateModified();
        }
    } else {
        // 跨行删除
        TextLine* start_line = getLine(start.line);
        TextLine* end_line = getLine(end.line);
        
        if (start_line && end_line) {
            // 合并首尾行
            std::string end_content = end_line->substr(end.column);
            start_line->append(end_content);
            
            // 删除中间行
            for (int i = start.line + 1; i <= end.line; i++) {
                deleteLine(start.line + 1);
            }
            
            updateModified();
        }
    }
}

void Buffer::replaceText(const TextPosition& start, const TextPosition& end, const std::string& text) {
    deleteText(start, end);
    insertText(start, text);
}

void Buffer::setCursor(const TextPosition& pos) {
    if (isValidPosition(pos)) {
        cursor_ = pos;
    }
}

void Buffer::moveCursor(int delta_line, int delta_column) {
    TextPosition new_pos = cursor_;
    new_pos.line += delta_line;
    new_pos.column += delta_column;
    setCursor(new_pos);
}

void Buffer::moveCursorTo(int line, int column) {
    setCursor(TextPosition(line, column));
}

void Buffer::setSelection(const TextPosition& start, const TextPosition& end) {
    if (isValidPosition(start) && isValidPosition(end)) {
        selection_start_ = start;
        selection_end_ = end;
        has_selection_ = true;
    }
}

void Buffer::clearSelection() {
    has_selection_ = false;
}

std::string Buffer::getSelectedText() const {
    if (!has_selection_) {
        return "";
    }
    
    return getRangeContent(selection_start_, selection_end_);
}

TextPosition Buffer::find(const std::string& text, const TextPosition& start, bool case_sensitive) const {
    if (!isValidPosition(start)) {
        return TextPosition(-1, -1);
    }
    
    std::string search_text = text;
    if (!case_sensitive) {
        std::transform(search_text.begin(), search_text.end(), search_text.begin(), ::tolower);
    }
    
    for (size_t line = start.line; line < lines_.size(); line++) {
        std::string line_content = lines_[line]->getContent();
        if (!case_sensitive) {
            std::transform(line_content.begin(), line_content.end(), line_content.begin(), ::tolower);
        }
        
        size_t pos = line_content.find(search_text, line == static_cast<size_t>(start.line) ? static_cast<size_t>(start.column) : 0);
        if (pos != std::string::npos) {
            return TextPosition(line, pos);
        }
    }
    
    return TextPosition(-1, -1);
}

TextPosition Buffer::findNext(const std::string& text, bool case_sensitive) {
    TextPosition start = cursor_;
    start.column++; // 从下一个位置开始搜索
    return find(text, start, case_sensitive);
}

TextPosition Buffer::findPrevious(const std::string& text, bool case_sensitive) {
    // 简化实现，从当前位置向前搜索
    for (int line = cursor_.line; line >= 0; line--) {
        std::string line_content = lines_[line]->getContent();
        if (!case_sensitive) {
            std::transform(line_content.begin(), line_content.end(), line_content.begin(), ::tolower);
        }
        
        std::string search_text = text;
        if (!case_sensitive) {
            std::transform(search_text.begin(), search_text.end(), search_text.begin(), ::tolower);
        }
        
        size_t pos = line_content.rfind(search_text, line == cursor_.line ? cursor_.column - 1 : std::string::npos);
        if (pos != std::string::npos) {
            return TextPosition(line, pos);
        }
    }
    
    return TextPosition(-1, -1);
}

int Buffer::replaceAll(const std::string& search, const std::string& replace, bool case_sensitive) {
    int count = 0;
    TextPosition pos = find(search, TextPosition(0, 0), case_sensitive);
    
    while (pos.line >= 0) {
        TextPosition end_pos(pos.line, pos.column + search.length());
        replaceText(pos, end_pos, replace);
        count++;
        
        pos = find(search, TextPosition(pos.line, pos.column + replace.length()), case_sensitive);
    }
    
    return count;
}

void Buffer::pushUndo() {
    std::string state = getContent();
    undo_stack_.push_back(state);
    
    if (undo_stack_.size() > static_cast<size_t>(max_undo_levels_)) {
        undo_stack_.erase(undo_stack_.begin());
    }
    
    clearRedoStack();
}

bool Buffer::undo() {
    if (undo_stack_.empty()) {
        return false;
    }
    
    std::string current_state = getContent();
    redo_stack_.push_back(current_state);
    
    std::string previous_state = undo_stack_.back();
    undo_stack_.pop_back();
    
    // 恢复状态
    lines_.clear();
    std::istringstream stream(previous_state);
    std::string line;
    while (std::getline(stream, line)) {
        lines_.push_back(std::make_unique<TextLine>(line));
    }
    
    if (lines_.empty()) {
        lines_.push_back(std::make_unique<TextLine>());
    }
    
    updateModified();
    return true;
}

bool Buffer::redo() {
    if (redo_stack_.empty()) {
        return false;
    }
    
    std::string current_state = getContent();
    undo_stack_.push_back(current_state);
    
    std::string next_state = redo_stack_.back();
    redo_stack_.pop_back();
    
    // 恢复状态
    lines_.clear();
    std::istringstream stream(next_state);
    std::string line;
    while (std::getline(stream, line)) {
        lines_.push_back(std::make_unique<TextLine>(line));
    }
    
    if (lines_.empty()) {
        lines_.push_back(std::make_unique<TextLine>());
    }
    
    updateModified();
    return true;
}

bool Buffer::canUndo() const {
    return !undo_stack_.empty();
}

bool Buffer::canRedo() const {
    return !redo_stack_.empty();
}

std::string Buffer::getContent() const {
    std::ostringstream stream;
    for (size_t i = 0; i < lines_.size(); i++) {
        stream << lines_[i]->getContent();
        if (i < lines_.size() - 1) {
            stream << line_ending_;
        }
    }
    return stream.str();
}

std::string Buffer::getLineContent(size_t index) const {
    if (index < lines_.size()) {
        return lines_[index]->getContent();
    }
    return "";
}

std::string Buffer::getRangeContent(const TextPosition& start, const TextPosition& end) const {
    if (!isValidPosition(start) || !isValidPosition(end)) {
        return "";
    }
    
    std::ostringstream stream;
    if (start.line == end.line) {
        stream << lines_[start.line]->substr(start.column, end.column - start.column);
    } else {
        // 跨行内容
        stream << lines_[start.line]->substr(start.column);
        for (int line = start.line + 1; line < end.line; line++) {
            stream << line_ending_ << lines_[line]->getContent();
        }
        if (end.line < static_cast<int>(lines_.size())) {
            stream << line_ending_ << lines_[end.line]->substr(0, end.column);
        }
    }
    
    return stream.str();
}

size_t Buffer::getTotalChars() const {
    size_t total = 0;
    for (const auto& line : lines_) {
        total += line->length();
    }
    return total;
}

size_t Buffer::getLineLength(size_t index) const {
    if (index < lines_.size()) {
        return lines_[index]->length();
    }
    return 0;
}

bool Buffer::isValidPosition(const TextPosition& pos) const {
    return pos.line >= 0 && pos.line < static_cast<int>(lines_.size()) &&
           pos.column >= 0 && pos.column <= static_cast<int>(getLineLength(pos.line));
}

bool Buffer::isValidLine(size_t line) const {
    return line < lines_.size();
}

bool Buffer::isValidColumn(size_t line, size_t column) const {
    return line < lines_.size() && column <= getLineLength(line);
}

void Buffer::normalizePosition(TextPosition& pos) const {
    if (pos.line < 0) {
        pos.line = 0;
    } else if (pos.line >= static_cast<int>(lines_.size())) {
        pos.line = lines_.size() - 1;
    }
    
    if (pos.column < 0) {
        pos.column = 0;
    } else if (pos.column > static_cast<int>(getLineLength(pos.line))) {
        pos.column = getLineLength(pos.line);
    }
}

void Buffer::updateModified() {
    modified_ = true;
}

void Buffer::addToUndoStack(const std::string& state) {
    undo_stack_.push_back(state);
    if (undo_stack_.size() > static_cast<size_t>(max_undo_levels_)) {
        undo_stack_.erase(undo_stack_.begin());
    }
}

void Buffer::clearRedoStack() {
    redo_stack_.clear();
}

} // namespace pvim
