#pragma once
#include "Editor.h"
#include <string>

namespace pvim {

class SearchEngine {
private:
    Editor* editor_;
    std::string search_pattern_;
    std::string replace_pattern_;
    
public:
    SearchEngine(Editor* editor);
    TextPosition find(const std::string& text, const TextPosition& start, bool case_sensitive = true);
    TextPosition findNext(const std::string& text, bool case_sensitive = true);
    TextPosition findPrevious(const std::string& text, bool case_sensitive = true);
    int replaceAll(const std::string& search, const std::string& replace, bool case_sensitive = true);
    
    void setSearchPattern(const std::string& pattern);
    std::string getSearchPattern() const;
    void setReplacePattern(const std::string& pattern);
    std::string getReplacePattern() const;
};

} // namespace pvim
