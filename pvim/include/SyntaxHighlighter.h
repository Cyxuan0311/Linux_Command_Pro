#pragma once
#include "Editor.h"

namespace pvim {
class SyntaxHighlighter {
private:
    Editor* editor_;
    std::string file_type_;
    
public:
    SyntaxHighlighter(Editor* editor);
    void setFileType(const std::string& type);
    std::string highlightLine(const std::string& line);
    void highlightBuffer();
};
}
