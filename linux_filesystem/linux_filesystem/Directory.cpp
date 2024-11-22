// src/Directory.cpp
#include "Directory.h"
#include <stdexcept>
#include "DiskManager.h"


//void Directory::addEntry(const std::string& name, uint32_t inodeIndex, DiskManager& diskManager) {
//    if (entries.find(name) != entries.end()) {
//        throw std::runtime_error("Entry already exists: " + name);
//    }
//    entries[name] = inodeIndex;
//
//    // 序列化 Directory 对象
//    std::vector<char> buffer;
//    serialize(buffer, diskManager.blockSize);
//
//    // 将序列化数据写入磁盘
//    diskManager.writeBlock(inodeIndex, buffer.data());
//}

void Directory::addEntry(const std::string& name, uint32_t inodeIndex) {
    if (entries.find(name) != entries.end()) {
        throw std::runtime_error("Entry already exists: " + name);
    }
    entries[name] = inodeIndex;
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

void Directory::serialize(std::vector<char>& buffer, size_t blockSize) const {
    buffer.clear();
    buffer.reserve(blockSize);

    size_t offset = 0;

    // 序列化 entries 的数量
    uint32_t entryCount = static_cast<uint32_t>(entries.size());
    buffer.insert(buffer.end(), reinterpret_cast<const char*>(&entryCount), reinterpret_cast<const char*>(&entryCount) + sizeof(uint32_t));

    // 序列化每个目录项
    for (const auto& entry : entries) {
        const std::string& name = entry.first;
        uint32_t inodeIndex = entry.second;

        // 序列化名称长度和名称
        uint32_t nameLength = static_cast<uint32_t>(name.length());
        buffer.insert(buffer.end(), reinterpret_cast<const char*>(&nameLength), reinterpret_cast<const char*>(&nameLength) + sizeof(uint32_t));
        buffer.insert(buffer.end(), name.begin(), name.end());

        // 序列化 inodeIndex
        buffer.insert(buffer.end(), reinterpret_cast<const char*>(&inodeIndex), reinterpret_cast<const char*>(&inodeIndex) + sizeof(uint32_t));
    }

    // 填充到块大小
    if (buffer.size() < blockSize) {
        buffer.resize(blockSize, 0);
    }
}




void Directory::deserialize(const char* data, size_t size) {
    size_t offset = 0;
    entries.clear();

    // 检查数据大小
    if (size < sizeof(uint32_t)) {
        throw std::runtime_error("Insufficient data for deserialization");
    }

    // 反序列化 entries 的数量
    uint32_t entryCount = 0;
    std::memcpy(&entryCount, data + offset, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    // 反序列化每个目录项
    for (uint32_t i = 0; i < entryCount; ++i) {
        if (offset + sizeof(uint32_t) > size) throw std::runtime_error("Corrupted directory data");

        // 反序列化名称长度
        uint32_t nameLength = 0;
        std::memcpy(&nameLength, data + offset, sizeof(uint32_t));
        offset += sizeof(uint32_t);

        if (offset + nameLength + sizeof(uint32_t) > size) throw std::runtime_error("Corrupted directory data");

        // 反序列化名称
        std::string name(data + offset, nameLength);
        offset += nameLength;

        // 反序列化 inodeIndex
        uint32_t inodeIndex = 0;
        std::memcpy(&inodeIndex, data + offset, sizeof(uint32_t));
        offset += sizeof(uint32_t);

        entries[name] = inodeIndex;
    }
}