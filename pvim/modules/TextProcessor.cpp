#include "TextProcessor.h"

namespace pvim {

TextProcessor::TextProcessor(Editor* editor) : editor_(editor) {
}

std::string TextProcessor::processText(const std::string& text) {
    // 简化的文本处理实现
    return text;
}

void TextProcessor::setProcessor(const std::string& name, std::function<std::string(const std::string&)> processor) {
    processors_[name] = processor;
}

std::string TextProcessor::getProcessor(const std::string& name) const {
    auto it = processors_.find(name);
    if (it != processors_.end()) {
        return it->second("test");
    }
    return "";
}

} // namespace pvim
