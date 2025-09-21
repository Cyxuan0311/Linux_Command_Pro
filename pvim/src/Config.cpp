#include "Config.h"
#include <fstream>
#include <sstream>

namespace pvim {

Config::Config(Editor* editor) : editor_(editor) {
    loadConfig();
}

void Config::loadConfig() {
    // 默认设置
    settings_["tab_size"] = "4";
    settings_["show_line_numbers"] = "true";
    settings_["syntax_highlighting"] = "true";
    settings_["auto_indent"] = "true";
    settings_["backup"] = "true";
    settings_["backup_dir"] = "~/.pvim/backup";
    settings_["undo_levels"] = "100";
    settings_["color_scheme"] = "default";
    settings_["font_size"] = "12";
    settings_["wrap_lines"] = "false";
    settings_["show_whitespace"] = "false";
    settings_["auto_save"] = "false";
    settings_["auto_save_interval"] = "300";
    
    // 尝试从配置文件加载
    std::string config_file = "~/.pvimrc";
    std::ifstream file(config_file);
    if (file.is_open()) {
        std::string line;
        while (std::getline(file, line)) {
            // 解析配置行
            size_t pos = line.find('=');
            if (pos != std::string::npos) {
                std::string key = line.substr(0, pos);
                std::string value = line.substr(pos + 1);
                
                // 去除空格
                key.erase(0, key.find_first_not_of(" \t"));
                key.erase(key.find_last_not_of(" \t") + 1);
                value.erase(0, value.find_first_not_of(" \t"));
                value.erase(value.find_last_not_of(" \t") + 1);
                
                settings_[key] = value;
            }
        }
        file.close();
    }
}

void Config::saveConfig() {
    std::string config_file = "~/.pvimrc";
    std::ofstream file(config_file);
    if (file.is_open()) {
        for (const auto& setting : settings_) {
            file << setting.first << "=" << setting.second << std::endl;
        }
        file.close();
    }
}

std::string Config::getSetting(const std::string& key, const std::string& default_value) {
    auto it = settings_.find(key);
    if (it != settings_.end()) {
        return it->second;
    }
    return default_value;
}

void Config::setSetting(const std::string& key, const std::string& value) {
    settings_[key] = value;
}

} // namespace pvim
