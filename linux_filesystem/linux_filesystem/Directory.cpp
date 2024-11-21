// src/Directory.cpp
#include "Directory.h"
#include <stdexcept>
#include "DiskManager.h"


void Directory::addEntry(const std::string& name, uint32_t inodeIndex, DiskManager& diskManager) {
    if (entries.find(name) != entries.end()) {
        throw std::runtime_error("Entry already exists: " + name);
    }
    entries[name] = inodeIndex;

    // 序列化 Directory 对象
    std::vector<char> buffer;
    serialize(buffer, diskManager.blockSize);

    // 获取该目录的 inode
    INode dirInode = diskManager.readINode(this->inodeIndex);

    // 将序列化数据写入该目录的块
    diskManager.writeBlock(dirInode.blockIndex, buffer.data());
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
    size_t requiredSize = sizeof(uint32_t) + sizeof(uint32_t); // parentInodeIndex + entryCount

    for (const auto& entry : entries) {
        uint32_t nameLength = static_cast<uint32_t>(entry.first.length());
        requiredSize += sizeof(uint32_t); // nameLength
        requiredSize += nameLength;       // name
        requiredSize += sizeof(uint32_t); // inodeIndex
    }

    if (requiredSize > blockSize) {
        throw std::runtime_error("Directory entries exceed block size");
    }

    buffer.clear();
    buffer.resize(blockSize, 0); // 确保缓冲区大小为 blockSize

    size_t offset = 0;

    // 序列化 parentInodeIndex
    std::memcpy(buffer.data() + offset, &parentInodeIndex, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    // 序列化 entries 的数量
    uint32_t entryCount = static_cast<uint32_t>(entries.size());
    std::memcpy(buffer.data() + offset, &entryCount, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    // 序列化每个目录项
    for (const auto& entry : entries) {
        const std::string& name = entry.first;
        uint32_t inodeIndex = entry.second;

        uint32_t nameLength = static_cast<uint32_t>(name.length());

        // 检查是否会超出块大小
        if (offset + sizeof(uint32_t) + nameLength + sizeof(uint32_t) > blockSize) {
            throw std::runtime_error("Directory serialization exceeds block size");
        }

        // 序列化名称长度
        std::memcpy(buffer.data() + offset, &nameLength, sizeof(uint32_t));
        offset += sizeof(uint32_t);

        // 序列化名称
        std::memcpy(buffer.data() + offset, name.data(), nameLength);
        offset += nameLength;

        // 序列化 inodeIndex
        std::memcpy(buffer.data() + offset, &inodeIndex, sizeof(uint32_t));
        offset += sizeof(uint32_t);
    }

    // 调试输出
    std::cout << "[DEBUG] Directory::serialize: inodeIndex=" << inodeIndex
        << ", parentInodeIndex=" << parentInodeIndex
        << ", entryCount=" << entries.size() << std::endl;
}



void Directory::deserialize(const char* data, size_t size) {
    size_t offset = 0;
    entries.clear();

    // 检查数据大小
    if (size < sizeof(uint32_t) * 2) {
        throw std::runtime_error("Insufficient data for deserialization");
    }

    // 反序列化 parentInodeIndex
    std::memcpy(&parentInodeIndex, data + offset, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    // 反序列化 entries 的数量
    uint32_t entryCount = 0;
    std::memcpy(&entryCount, data + offset, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    // 调试输出
    std::cout << "[DEBUG] Directory::deserialize: parentInodeIndex=" << parentInodeIndex
        << ", entryCount=" << entryCount << std::endl;

    // 反序列化每个目录项
    for (uint32_t i = 0; i < entryCount; ++i) {
        // 检查剩余数据是否足够读取名称长度
        if (offset + sizeof(uint32_t) > size) {
            throw std::runtime_error("Corrupted directory data (name length)");
        }

        // 反序列化名称长度
        uint32_t nameLength = 0;
        std::memcpy(&nameLength, data + offset, sizeof(uint32_t));
        offset += sizeof(uint32_t);

        // 检查剩余数据是否足够读取名称和 inodeIndex
        if (offset + nameLength + sizeof(uint32_t) > size) {
            throw std::runtime_error("Corrupted directory data (name and inodeIndex)");
        }

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

size_t Directory::getMaxEntries(size_t blockSize) const
{
    const size_t AVERAGE_NAME_LENGTH = 16; // 根据您的需求设置平均名称长度
    return (blockSize - sizeof(uint32_t) * 2) / (sizeof(uint32_t) * 2 + AVERAGE_NAME_LENGTH);

}
