#include "Window.h"
#include <ncurses.h>
#include <algorithm>

namespace pvim {

Window::Window(WindowType type, int x, int y, int width, int height)
    : win_(nullptr), type_(type), x_(x), y_(y), width_(width), height_(height)
    , visible_(true), border_(false), scroll_offset_x_(0), scroll_offset_y_(0)
    , max_scroll_x_(0), max_scroll_y_(0), color_pair_(0), attributes_(0) {
}

Window::~Window() {
    destroy();
}

bool Window::create() {
    if (win_) {
        destroy();
    }
    
    win_ = newwin(height_, width_, y_, x_);
    if (!win_) {
        return false;
    }
    
    keypad(win_, TRUE);
    nodelay(win_, TRUE);
    
    return true;
}

void Window::destroy() {
    if (win_) {
        delwin(win_);
        win_ = nullptr;
    }
}

void Window::refresh() {
    if (win_ && visible_) {
        wrefresh(win_);
    }
}

void Window::clear() {
    if (win_) {
        wclear(win_);
    }
}

void Window::erase() {
    if (win_) {
        werase(win_);
    }
}

void Window::print(int y, int x, const std::string& text) {
    if (win_ && visible_) {
        mvwprintw(win_, y, x, "%s", text.c_str());
    }
}

void Window::print(int y, int x, const std::string& text, int color_pair) {
    if (win_ && visible_) {
        wattron(win_, COLOR_PAIR(color_pair));
        mvwprintw(win_, y, x, "%s", text.c_str());
        wattroff(win_, COLOR_PAIR(color_pair));
    }
}

void Window::print(int y, int x, const std::string& text, int color_pair, int attributes) {
    if (win_ && visible_) {
        wattron(win_, COLOR_PAIR(color_pair) | attributes);
        mvwprintw(win_, y, x, "%s", text.c_str());
        wattroff(win_, COLOR_PAIR(color_pair) | attributes);
    }
}

void Window::printChar(int y, int x, char ch) {
    if (win_ && visible_) {
        mvwaddch(win_, y, x, ch);
    }
}

void Window::printChar(int y, int x, char ch, int color_pair) {
    if (win_ && visible_) {
        wattron(win_, COLOR_PAIR(color_pair));
        mvwaddch(win_, y, x, ch);
        wattroff(win_, COLOR_PAIR(color_pair));
    }
}

void Window::printChar(int y, int x, char ch, int color_pair, int attributes) {
    if (win_ && visible_) {
        wattron(win_, COLOR_PAIR(color_pair) | attributes);
        mvwaddch(win_, y, x, ch);
        wattroff(win_, COLOR_PAIR(color_pair) | attributes);
    }
}

void Window::printf(int y, int x, const char* format, ...) {
    if (win_ && visible_) {
        va_list args;
        va_start(args, format);
        mvwprintw(win_, y, x, format, args);
        va_end(args);
    }
}

void Window::printf(int y, int x, int color_pair, const char* format, ...) {
    if (win_ && visible_) {
        wattron(win_, COLOR_PAIR(color_pair));
        va_list args;
        va_start(args, format);
        mvwprintw(win_, y, x, format, args);
        va_end(args);
        wattroff(win_, COLOR_PAIR(color_pair));
    }
}

void Window::fill(char ch) {
    if (win_ && visible_) {
        for (int y = 0; y < height_; y++) {
            for (int x = 0; x < width_; x++) {
                mvwaddch(win_, y, x, ch);
            }
        }
    }
}

void Window::fill(int y, int x, int height, int width, char ch) {
    if (win_ && visible_) {
        for (int dy = 0; dy < height; dy++) {
            for (int dx = 0; dx < width; dx++) {
                mvwaddch(win_, y + dy, x + dx, ch);
            }
        }
    }
}

void Window::fill(int y, int x, int height, int width, char ch, int color_pair) {
    if (win_ && visible_) {
        wattron(win_, COLOR_PAIR(color_pair));
        for (int dy = 0; dy < height; dy++) {
            for (int dx = 0; dx < width; dx++) {
                mvwaddch(win_, y + dy, x + dx, ch);
            }
        }
        wattroff(win_, COLOR_PAIR(color_pair));
    }
}

void Window::drawBorder() {
    if (win_ && visible_ && border_) {
        wattron(win_, COLOR_PAIR(border_style_.color_pair));
        
        // 绘制边框
        for (int x = 0; x < width_; x++) {
            mvwaddch(win_, 0, x, border_style_.horizontal);
            mvwaddch(win_, height_ - 1, x, border_style_.horizontal);
        }
        
        for (int y = 0; y < height_; y++) {
            mvwaddch(win_, y, 0, border_style_.vertical);
            mvwaddch(win_, y, width_ - 1, border_style_.vertical);
        }
        
        // 绘制角落
        mvwaddch(win_, 0, 0, border_style_.top_left);
        mvwaddch(win_, 0, width_ - 1, border_style_.top_right);
        mvwaddch(win_, height_ - 1, 0, border_style_.bottom_left);
        mvwaddch(win_, height_ - 1, width_ - 1, border_style_.bottom_right);
        
        wattroff(win_, COLOR_PAIR(border_style_.color_pair));
    }
}

void Window::drawTitle() {
    if (win_ && visible_ && !title_.empty()) {
        int title_x = (width_ - title_.length()) / 2;
        if (title_x < 0) title_x = 0;
        if (title_x + title_.length() >= width_) {
            title_x = width_ - title_.length() - 1;
        }
        
        wattron(win_, COLOR_PAIR(border_style_.color_pair));
        mvwprintw(win_, 0, title_x, " %s ", title_.c_str());
        wattroff(win_, COLOR_PAIR(border_style_.color_pair));
    }
}

void Window::moveCursor(int y, int x) {
    if (win_) {
        wmove(win_, y, x);
    }
}

void Window::getCursor(int& y, int& x) const {
    if (win_) {
        getyx(win_, y, x);
    } else {
        y = x = 0;
    }
}

void Window::resize(int width, int height) {
    width_ = width;
    height_ = height;
    
    if (win_) {
        wresize(win_, height, width);
    }
}

void Window::move(int x, int y) {
    x_ = x;
    y_ = y;
    
    if (win_) {
        mvwin(win_, y, x);
    }
}

void Window::center() {
    int screen_width, screen_height;
    getmaxyx(stdscr, screen_height, screen_width);
    
    int new_x = (screen_width - width_) / 2;
    int new_y = (screen_height - height_) / 2;
    
    move(new_x, new_y);
}

int Window::getChar() {
    if (win_) {
        return wgetch(win_);
    }
    return -1;
}

int Window::getCharWithTimeout(int timeout_ms) {
    if (win_) {
        wtimeout(win_, timeout_ms);
        int ch = wgetch(win_);
        wtimeout(win_, -1);
        return ch;
    }
    return -1;
}

std::string Window::getString(int max_length) {
    if (win_) {
        char* buffer = new char[max_length > 0 ? max_length + 1 : 1024];
        wgetnstr(win_, buffer, max_length > 0 ? max_length : 1023);
        std::string result(buffer);
        delete[] buffer;
        return result;
    }
    return "";
}

void Window::select() {
    if (win_) {
        wbkgd(win_, COLOR_PAIR(1));
    }
}

void Window::deselect() {
    if (win_) {
        wbkgd(win_, COLOR_PAIR(0));
    }
}

bool Window::isSelected() const {
    // 简化实现
    return false;
}

void Window::setFocus(bool focus) {
    if (win_) {
        if (focus) {
            wbkgd(win_, COLOR_PAIR(1));
        } else {
            wbkgd(win_, COLOR_PAIR(0));
        }
    }
}

bool Window::hasFocus() const {
    // 简化实现
    return false;
}

void Window::onResize(std::function<void(int, int)> handler) {
    resize_handler_ = handler;
}

void Window::onKeyPress(std::function<bool(int)> handler) {
    keypress_handler_ = handler;
}

void Window::onMouseClick(std::function<void(int, int, int)> handler) {
    mouse_handler_ = handler;
}

void Window::setScrollX(int offset) {
    scroll_offset_x_ = std::max(0, std::min(offset, max_scroll_x_));
}

void Window::setScrollY(int offset) {
    scroll_offset_y_ = std::max(0, std::min(offset, max_scroll_y_));
}

void Window::scrollX(int delta) {
    setScrollX(scroll_offset_x_ + delta);
}

void Window::scrollY(int delta) {
    setScrollY(scroll_offset_y_ + delta);
}

void Window::setMaxScroll(int max_x, int max_y) {
    max_scroll_x_ = max_x;
    max_scroll_y_ = max_y;
    clampScroll();
}

void Window::updateScrollLimits() {
    // 简化实现
    max_scroll_x_ = std::max(0, width_ - 1);
    max_scroll_y_ = std::max(0, height_ - 1);
}

void Window::clampScroll() {
    scroll_offset_x_ = std::max(0, std::min(scroll_offset_x_, max_scroll_x_));
    scroll_offset_y_ = std::max(0, std::min(scroll_offset_y_, max_scroll_y_));
}

// WindowManager 实现
WindowManager::WindowManager() : active_window_(nullptr), main_window_(nullptr) {
}

WindowManager::~WindowManager() {
    destroyAllWindows();
}

Window* WindowManager::createWindow(WindowType type, int x, int y, int width, int height) {
    auto window = std::make_unique<Window>(type, x, y, width, height);
    if (!window->create()) {
        return nullptr;
    }
    
    Window* ptr = window.get();
    windows_.push_back(std::move(window));
    
    if (type == WindowType::MAIN_EDITOR) {
        main_window_ = ptr;
    }
    
    if (!active_window_) {
        active_window_ = ptr;
    }
    
    return ptr;
}

void WindowManager::destroyWindow(Window* window) {
    auto it = std::find_if(windows_.begin(), windows_.end(),
        [window](const std::unique_ptr<Window>& w) { return w.get() == window; });
    
    if (it != windows_.end()) {
        if (active_window_ == window) {
            active_window_ = nullptr;
        }
        if (main_window_ == window) {
            main_window_ = nullptr;
        }
        windows_.erase(it);
    }
}

void WindowManager::destroyAllWindows() {
    windows_.clear();
    active_window_ = nullptr;
    main_window_ = nullptr;
}

void WindowManager::setActiveWindow(Window* window) {
    active_window_ = window;
}

void WindowManager::setMainWindow(Window* window) {
    main_window_ = window;
}

Window* WindowManager::findWindow(WindowType type) const {
    auto it = std::find_if(windows_.begin(), windows_.end(),
        [type](const std::unique_ptr<Window>& w) { return w->getType() == type; });
    
    return it != windows_.end() ? it->get() : nullptr;
}

std::vector<Window*> WindowManager::findWindows(WindowType type) const {
    std::vector<Window*> result;
    for (const auto& window : windows_) {
        if (window->getType() == type) {
            result.push_back(window.get());
        }
    }
    return result;
}

void WindowManager::arrangeWindows() {
    // 简化实现
}

void WindowManager::tileWindows() {
    // 简化实现
}

void WindowManager::cascadeWindows() {
    // 简化实现
}

void WindowManager::refreshAll() {
    for (const auto& window : windows_) {
        window->refresh();
    }
}

void WindowManager::clearAll() {
    for (const auto& window : windows_) {
        window->clear();
    }
}

void WindowManager::handleResize() {
    for (const auto& window : windows_) {
        if (window->getType() == WindowType::MAIN_EDITOR) {
            int screen_rows, screen_cols;
            getmaxyx(stdscr, screen_rows, screen_cols);
            window->resize(screen_cols, screen_rows - 1);
        }
    }
}

bool WindowManager::handleKeyPress(int key) {
    if (active_window_) {
        int ch = active_window_->getChar();
        return ch == key;
    }
    return false;
}

void WindowManager::handleMouseClick(int y, int x, int button) {
    // 简化实现
}

} // namespace pvim
