#pragma once
#include "Editor.h"
#include <map>
#include <string>

namespace pvim {

class PluginManager {
private:
    Editor* editor_;
    std::map<std::string, bool> plugins_;
    
public:
    PluginManager(Editor* editor);
    bool loadPlugin(const std::string& name);
    bool unloadPlugin(const std::string& name);
    bool isPluginLoaded(const std::string& name) const;
    void listPlugins() const;
};

} // namespace pvim
