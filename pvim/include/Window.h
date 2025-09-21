#pragma once

#include <string>
#include <vector>
#include <memory>
#include <map>
#include <functional>
#include <ncurses.h>
#include "Editor.h"

namespace pvim {

// 窗口类型枚举
enum class WindowType {
    MAIN_EDITOR,
    STATUS_BAR,
    COMMAND_LINE,
    HELP_WINDOW,
    SPLIT_WINDOW
};

// 窗口边框样式
struct BorderStyle {
    char top_left = '+';
    char top_right = '+';
    char bottom_left = '+';
    char bottom_right = '+';
    char horizontal = '-';
    char vertical = '|';
    int color_pair = 0;
};

// 窗口类
class Window {
private:
    WINDOW* win_;
    WindowType type_;
    int x_, y_, width_, height_;
    bool visible_;
    bool border_;
    BorderStyle border_style_;
    std::string title_;
    
    // 滚动相关
    int scroll_offset_x_;
    int scroll_offset_y_;
    int max_scroll_x_;
    int max_scroll_y_;
    
    // 颜色和属性
    int color_pair_;
    int attributes_;
    
public:
    Window(WindowType type, int x, int y, int width, int height);
    ~Window();
    
    // 基本操作
    bool create();
    void destroy();
    void refresh();
    void clear();
    void erase();
    
    // 窗口属性
    WindowType getType() const { return type_; }
    int getX() const { return x_; }
    int getY() const { return y_; }
    int getWidth() const { return width_; }
    int getHeight() const { return height_; }
    
    // 可见性
    bool isVisible() const { return visible_; }
    void setVisible(bool visible) { visible_ = visible; }
    void show() { setVisible(true); }
    void hide() { setVisible(false); }
    
    // 边框
    bool hasBorder() const { return border_; }
    void setBorder(bool border) { border_ = border; }
    void setBorderStyle(const BorderStyle& style) { border_style_ = style; }
    const BorderStyle& getBorderStyle() const { return border_style_; }
    
    // 标题
    const std::string& getTitle() const { return title_; }
    void setTitle(const std::string& title) { title_ = title; }
    
    // 滚动
    int getScrollX() const { return scroll_offset_x_; }
    int getScrollY() const { return scroll_offset_y_; }
    void setScrollX(int offset);
    void setScrollY(int offset);
    void scrollX(int delta);
    void scrollY(int delta);
    void setMaxScroll(int max_x, int max_y);
    
    // 颜色和属性
    int getColorPair() const { return color_pair_; }
    void setColorPair(int pair) { color_pair_ = pair; }
    int getAttributes() const { return attributes_; }
    void setAttributes(int attrs) { attributes_ = attrs; }
    
    // 文本输出
    void print(int y, int x, const std::string& text);
    void print(int y, int x, const std::string& text, int color_pair);
    void print(int y, int x, const std::string& text, int color_pair, int attributes);
    void printChar(int y, int x, char ch);
    void printChar(int y, int x, char ch, int color_pair);
    void printChar(int y, int x, char ch, int color_pair, int attributes);
    
    // 格式化输出
    void printf(int y, int x, const char* format, ...);
    void printf(int y, int x, int color_pair, const char* format, ...);
    
    // 填充
    void fill(char ch);
    void fill(int y, int x, int height, int width, char ch);
    void fill(int y, int x, int height, int width, char ch, int color_pair);
    
    // 边框绘制
    void drawBorder();
    void drawTitle();
    
    // 光标操作
    void moveCursor(int y, int x);
    void getCursor(int& y, int& x) const;
    
    // 窗口操作
    void resize(int width, int height);
    void move(int x, int y);
    void center();
    
    // 输入
    int getChar();
    int getCharWithTimeout(int timeout_ms = -1);
    std::string getString(int max_length = -1);
    
    // 选择
    void select();
    void deselect();
    bool isSelected() const;
    
    // 焦点
    void setFocus(bool focus);
    bool hasFocus() const;
    
    // 事件处理
    void onResize(std::function<void(int, int)> handler);
    void onKeyPress(std::function<bool(int)> handler);
    void onMouseClick(std::function<void(int, int, int)> handler);
    
private:
    std::function<void(int, int)> resize_handler_;
    std::function<bool(int)> keypress_handler_;
    std::function<void(int, int, int)> mouse_handler_;
    
    void updateScrollLimits();
    void clampScroll();
};

// 窗口管理器
class WindowManager {
private:
    std::vector<std::unique_ptr<Window>> windows_;
    Window* active_window_;
    Window* main_window_;
    
public:
    WindowManager();
    ~WindowManager();
    
    // 窗口管理
    Window* createWindow(WindowType type, int x, int y, int width, int height);
    void destroyWindow(Window* window);
    void destroyAllWindows();
    
    // 活动窗口
    Window* getActiveWindow() const { return active_window_; }
    void setActiveWindow(Window* window);
    Window* getMainWindow() const { return main_window_; }
    void setMainWindow(Window* window);
    
    // 窗口查找
    Window* findWindow(WindowType type) const;
    std::vector<Window*> findWindows(WindowType type) const;
    
    // 布局管理
    void arrangeWindows();
    void tileWindows();
    void cascadeWindows();
    
    // 刷新
    void refreshAll();
    void clearAll();
    
    // 事件处理
    void handleResize();
    bool handleKeyPress(int key);
    void handleMouseClick(int y, int x, int button);
    
    // 统计
    size_t getWindowCount() const { return windows_.size(); }
    bool isEmpty() const { return windows_.empty(); }
};

} // namespace pvim
