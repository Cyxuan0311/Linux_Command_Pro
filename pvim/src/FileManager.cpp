#include "FileManager.h"
#include "Buffer.h"

namespace pvim {

FileManager::FileManager(Editor* editor) : editor_(editor) {
}

bool FileManager::openFile(const std::string& filename) {
    if (editor_->getBuffer()) {
        return editor_->getBuffer()->loadFromFile(filename);
    }
    return false;
}

bool FileManager::saveFile(const std::string& filename) {
    if (editor_->getBuffer()) {
        if (filename.empty()) {
            return editor_->getBuffer()->saveToFile();
        } else {
            return editor_->getBuffer()->saveToFile(filename);
        }
    }
    return false;
}

bool FileManager::closeFile() {
    if (editor_->getBuffer()) {
        editor_->getBuffer()->setModified(false);
        return true;
    }
    return false;
}

bool FileManager::isModified() const {
    if (editor_->getBuffer()) {
        return editor_->getBuffer()->isModified();
    }
    return false;
}

void FileManager::setModified(bool modified) {
    if (editor_->getBuffer()) {
        editor_->getBuffer()->setModified(modified);
    }
}

} // namespace pvim
