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
    buffer.resize(blockSize, 0); // 直接分配块大小的缓冲区
    char* ptr = buffer.data();

    // 序列化 entries 的数量
    uint32_t entryCount = static_cast<uint32_t>(entries.size());
    std::memcpy(ptr, &entryCount, sizeof(entryCount));
    ptr += sizeof(entryCount);

    // 序列化每个目录项
    for (const auto& entry : entries) {
        const std::string& name = entry.first;
        uint32_t inodeIndex = entry.second;

        // 序列化名称长度
        uint32_t nameLength = static_cast<uint32_t>(name.length());
        std::memcpy(ptr, &nameLength, sizeof(nameLength));
        ptr += sizeof(nameLength);

        // 序列化名称
        std::memcpy(ptr, name.data(), nameLength);
        ptr += nameLength;

        // 序列化 inodeIndex
        std::memcpy(ptr, &inodeIndex, sizeof(inodeIndex));
        ptr += sizeof(inodeIndex);
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
    std::memcpy(&entryCount, data + offset, sizeof(entryCount));
    offset += sizeof(entryCount);

    // 反序列化每个目录项
    for (uint32_t i = 0; i < entryCount; ++i) {
        if (offset + sizeof(uint32_t) > size) throw std::runtime_error("Corrupted directory data");

        // 反序列化名称长度
        uint32_t nameLength = 0;
        std::memcpy(&nameLength, data + offset, sizeof(nameLength));
        offset += sizeof(nameLength);

        // 检查名称长度是否合法
        if (nameLength > 255 || offset + nameLength + sizeof(uint32_t) > size) {
            throw std::runtime_error("Invalid or corrupted directory data");
        }

        // 反序列化名称
        std::string name(data + offset, nameLength);
        offset += nameLength;

        // 反序列化 inodeIndex
        uint32_t inodeIndex = 0;
        std::memcpy(&inodeIndex, data + offset, sizeof(inodeIndex));
        offset += sizeof(inodeIndex);

        entries[name] = inodeIndex;
    }
}
