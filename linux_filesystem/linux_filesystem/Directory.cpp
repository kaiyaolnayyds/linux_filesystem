// src/Directory.cpp
#include "Directory.h"
#include <stdexcept>
#include "DiskManager.h"


void Directory::addEntry(const std::string& name, uint32_t inodeIndex, DiskManager& diskManager) {
    if (entries.find(name) != entries.end()) {
        throw std::runtime_error("Entry already exists: " + name);
    }
    entries[name] = inodeIndex;

    // ���л� Directory ����
    std::vector<char> buffer;
    serialize(buffer, diskManager.blockSize);

    // ��ȡ��Ŀ¼�� inode
    INode dirInode = diskManager.readINode(this->inodeIndex);

    // �����л�����д���Ŀ¼�Ŀ�
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
    buffer.resize(blockSize, 0); // ȷ����������СΪ blockSize

    size_t offset = 0;

    // ���л� parentInodeIndex
    std::memcpy(buffer.data() + offset, &parentInodeIndex, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    // ���л� entries ������
    uint32_t entryCount = static_cast<uint32_t>(entries.size());
    std::memcpy(buffer.data() + offset, &entryCount, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    // ���л�ÿ��Ŀ¼��
    for (const auto& entry : entries) {
        const std::string& name = entry.first;
        uint32_t inodeIndex = entry.second;

        uint32_t nameLength = static_cast<uint32_t>(name.length());

        // ����Ƿ�ᳬ�����С
        if (offset + sizeof(uint32_t) + nameLength + sizeof(uint32_t) > blockSize) {
            throw std::runtime_error("Directory serialization exceeds block size");
        }

        // ���л����Ƴ���
        std::memcpy(buffer.data() + offset, &nameLength, sizeof(uint32_t));
        offset += sizeof(uint32_t);

        // ���л�����
        std::memcpy(buffer.data() + offset, name.data(), nameLength);
        offset += nameLength;

        // ���л� inodeIndex
        std::memcpy(buffer.data() + offset, &inodeIndex, sizeof(uint32_t));
        offset += sizeof(uint32_t);
    }

    // �������
    std::cout << "[DEBUG] Directory::serialize: inodeIndex=" << inodeIndex
        << ", parentInodeIndex=" << parentInodeIndex
        << ", entryCount=" << entries.size() << std::endl;
}



void Directory::deserialize(const char* data, size_t size) {
    size_t offset = 0;
    entries.clear();

    // ������ݴ�С
    if (size < sizeof(uint32_t) * 2) {
        throw std::runtime_error("Insufficient data for deserialization");
    }

    // �����л� parentInodeIndex
    std::memcpy(&parentInodeIndex, data + offset, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    // �����л� entries ������
    uint32_t entryCount = 0;
    std::memcpy(&entryCount, data + offset, sizeof(uint32_t));
    offset += sizeof(uint32_t);

    // �������
    std::cout << "[DEBUG] Directory::deserialize: parentInodeIndex=" << parentInodeIndex
        << ", entryCount=" << entryCount << std::endl;

    // �����л�ÿ��Ŀ¼��
    for (uint32_t i = 0; i < entryCount; ++i) {
        // ���ʣ�������Ƿ��㹻��ȡ���Ƴ���
        if (offset + sizeof(uint32_t) > size) {
            throw std::runtime_error("Corrupted directory data (name length)");
        }

        // �����л����Ƴ���
        uint32_t nameLength = 0;
        std::memcpy(&nameLength, data + offset, sizeof(uint32_t));
        offset += sizeof(uint32_t);

        // ���ʣ�������Ƿ��㹻��ȡ���ƺ� inodeIndex
        if (offset + nameLength + sizeof(uint32_t) > size) {
            throw std::runtime_error("Corrupted directory data (name and inodeIndex)");
        }

        // �����л�����
        std::string name(data + offset, nameLength);
        offset += nameLength;

        // �����л� inodeIndex
        uint32_t inodeIndex = 0;
        std::memcpy(&inodeIndex, data + offset, sizeof(uint32_t));
        offset += sizeof(uint32_t);

        entries[name] = inodeIndex;
    }
}

size_t Directory::getMaxEntries(size_t blockSize) const
{
    const size_t AVERAGE_NAME_LENGTH = 16; // ����������������ƽ�����Ƴ���
    return (blockSize - sizeof(uint32_t) * 2) / (sizeof(uint32_t) * 2 + AVERAGE_NAME_LENGTH);

}
