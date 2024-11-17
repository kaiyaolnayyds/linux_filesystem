// include/Directory.h
#ifndef DIRECTORY_H
#define DIRECTORY_H

#include <string>
#include <unordered_map>

struct DirectoryEntry {
    std::string name;
    uint32_t inodeIndex;

    DirectoryEntry(const std::string& name, uint32_t index) : name(name), inodeIndex(index) {}
};

class Directory {
public:
    std::unordered_map<std::string, uint32_t> entries;

    void addEntry(const std::string& name, uint32_t inodeIndex);
    void removeEntry(const std::string& name);
    uint32_t findEntry(const std::string& name);
};

#endif // DIRECTORY_H