// include/Directory.h
#ifndef DIRECTORY_H
#define DIRECTORY_H

#include <string>
#include <unordered_map>
#include <cstring>
#include<vector>

// ǰ������ DiskManager ��
class DiskManager;

struct DirectoryEntry {
    std::string name;
    uint32_t inodeIndex;

    DirectoryEntry(const std::string& name, uint32_t index) : name(name), inodeIndex(index) {}
};

class Directory {
public:
  
    std::unordered_map<std::string, uint32_t> entries; // ӳ�����Ƶ�i�ڵ�����

    void addEntry(const std::string& name, uint32_t inodeIndex, DiskManager& diskManager); // ���Ŀ¼��Ŀ
    void removeEntry(const std::string& name); // ɾ��Ŀ¼��Ŀ
    uint32_t findEntry(const std::string& name); // ����Ŀ¼��Ŀ

    // **�������л�����**
    void serialize(std::vector<char>& buffer, size_t blockSize) const;

    // **���������л�����**
    void deserialize(const char* data, size_t size);
    

};

#endif // DIRECTORY_H