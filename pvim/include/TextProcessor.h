#pragma once
#include "Editor.h"
#include <functional>
#include <map>

namespace pvim {

class TextProcessor {
private:
    Editor* editor_;
    std::map<std::string, std::function<std::string(const std::string&)>> processors_;
    
public:
    TextProcessor(Editor* editor);
    std::string processText(const std::string& text);
    void setProcessor(const std::string& name, std::function<std::string(const std::string&)> processor);
    std::string getProcessor(const std::string& name) const;
};

} // namespace pvim
