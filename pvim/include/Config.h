#pragma once
#include "Editor.h"

namespace pvim {
class Config {
private:
    Editor* editor_;
    std::map<std::string, std::string> settings_;
    
public:
    Config(Editor* editor);
    void loadConfig();
    void saveConfig();
    std::string getSetting(const std::string& key, const std::string& default_value = "");
    void setSetting(const std::string& key, const std::string& value);
};
}
