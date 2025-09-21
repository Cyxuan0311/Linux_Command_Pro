#pragma once
#include "Editor.h"

namespace pvim {
class StatusBar {
private:
    Editor* editor_;
    int width_;
    int height_;
    
public:
    StatusBar(Editor* editor);
    void draw();
    void resize(int width, int height);
    void setMessage(const std::string& message);
};
}
