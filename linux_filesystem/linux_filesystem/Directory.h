// include/Directory.h
#ifndef DIRECTORY_H
#define DIRECTORY_H

#include <string>
#include <unordered_map>
#include <cstring>
#include "DiskManager.h"


struct DirectoryEntry {
    std::string name;
    uint32_t inodeIndex;

    DirectoryEntry(const std::string& name, uint32_t index) : name(name), inodeIndex(index) {}
};

class Directory {
public:
  
    std::unordered_map<std::string, uint32_t> entries; // 映射名称到i节点索引

    void addEntry(const std::string& name, uint32_t inodeIndex); // 添加目录条目
    void removeEntry(const std::string& name); // 删除目录条目
    uint32_t findEntry(const std::string& name); // 查找目录条目
};

#endif // DIRECTORY_H