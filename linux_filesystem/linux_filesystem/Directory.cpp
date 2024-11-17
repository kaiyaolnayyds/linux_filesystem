// src/Directory.cpp
#include "Directory.h"
#include <stdexcept>



void Directory::addEntry(const std::string& name, uint32_t inodeIndex) {
    if (entries.find(name) != entries.end()) {
        throw std::runtime_error("Entry already exists: " + name);
    }
    entries[name] = inodeIndex;

    // 模拟持久化操作
    char buffer[1024];
    std::memset(buffer, 0, sizeof(buffer));
    std::memcpy(buffer, this, sizeof(Directory));
    DiskManager diskManager("simdisk.bin", 1024, 100); // 假设DiskManager已经定义
    diskManager.writeBlock(inodeIndex, buffer);
}


void Directory::removeEntry(const std::string& name) {
    auto it = entries.find(name);
    if (it != entries.end()) {
        entries.erase(it);
    }
    else {
        throw std::runtime_error("Entry not found: " + name);
    }
}

uint32_t Directory::findEntry(const std::string& name) {
    auto it = entries.find(name);
    if (it != entries.end()) {
        return it->second;
    }
    throw std::runtime_error("Entry not found: " + name);
}