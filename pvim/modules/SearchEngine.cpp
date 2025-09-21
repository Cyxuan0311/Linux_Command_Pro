#include "SearchEngine.h"
#include "Buffer.h"

namespace pvim {

SearchEngine::SearchEngine(Editor* editor) : editor_(editor) {
}

TextPosition SearchEngine::find(const std::string& text, const TextPosition& start, bool case_sensitive) {
    if (editor_ && editor_->getBuffer()) {
        return editor_->getBuffer()->find(text, start, case_sensitive);
    }
    return TextPosition(-1, -1);
}

TextPosition SearchEngine::findNext(const std::string& text, bool case_sensitive) {
    if (editor_ && editor_->getBuffer()) {
        return editor_->getBuffer()->findNext(text, case_sensitive);
    }
    return TextPosition(-1, -1);
}

TextPosition SearchEngine::findPrevious(const std::string& text, bool case_sensitive) {
    if (editor_ && editor_->getBuffer()) {
        return editor_->getBuffer()->findPrevious(text, case_sensitive);
    }
    return TextPosition(-1, -1);
}

int SearchEngine::replaceAll(const std::string& search, const std::string& replace, bool case_sensitive) {
    if (editor_ && editor_->getBuffer()) {
        return editor_->getBuffer()->replaceAll(search, replace, case_sensitive);
    }
    return 0;
}

void SearchEngine::setSearchPattern(const std::string& pattern) {
    search_pattern_ = pattern;
}

std::string SearchEngine::getSearchPattern() const {
    return search_pattern_;
}

void SearchEngine::setReplacePattern(const std::string& pattern) {
    replace_pattern_ = pattern;
}

std::string SearchEngine::getReplacePattern() const {
    return replace_pattern_;
}

} // namespace pvim
