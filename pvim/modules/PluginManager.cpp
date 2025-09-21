#include "PluginManager.h"

namespace pvim {

PluginManager::PluginManager(Editor* editor) : editor_(editor) {
}

bool PluginManager::loadPlugin(const std::string& name) {
    // 简化实现
    plugins_[name] = true;
    return true;
}

bool PluginManager::unloadPlugin(const std::string& name) {
    auto it = plugins_.find(name);
    if (it != plugins_.end()) {
        plugins_.erase(it);
        return true;
    }
    return false;
}

bool PluginManager::isPluginLoaded(const std::string& name) const {
    auto it = plugins_.find(name);
    return it != plugins_.end() && it->second;
}

void PluginManager::listPlugins() const {
    for (const auto& plugin : plugins_) {
        if (plugin.second) {
            // 显示插件信息
        }
    }
}

} // namespace pvim
