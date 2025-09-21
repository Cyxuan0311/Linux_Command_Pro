#pragma once
#include "Editor.h"

namespace pvim {
class FileManager {
private:
    Editor* editor_;
    
public:
    FileManager(Editor* editor);
    bool openFile(const std::string& filename);
    bool saveFile(const std::string& filename = "");
    bool closeFile();
    bool isModified() const;
    void setModified(bool modified);
};
}
