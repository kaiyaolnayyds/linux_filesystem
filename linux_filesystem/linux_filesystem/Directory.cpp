// src/Directory.cpp
#include "Directory.h"
#include <stdexcept>
#include "DiskManager.h"


void Directory::addEntry(const std::string& name, uint32_t inodeIndex, DiskManager& diskManager, uint32_t dirInodeIndex) {
    if (entries.find(name) != entries.end()) {
        throw std::runtime_error("Entry already exists: " + name);
    }
    entries[name] = inodeIndex;

    // ���л� Directory ����
    std::vector<char> buffer;
    serialize(buffer, diskManager.blockSize);

    // ��ȡ��Ŀ¼�� inode
    INode dirInode = diskManager.readINode(dirInodeIndex);

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

        // ���л����Ƴ���
        uint32_t nameLength = static_cast<uint32_t>(name.length());
        std::memcpy(buffer.data() + offset, &nameLength, sizeof(uint32_t));
        offset += sizeof(uint32_t);

        // ���л�����
        std::memcpy(buffer.data() + offset, name.data(), nameLength);
        offset += nameLength;

        // ���л� inodeIndex
        std::memcpy(buffer.data() + offset, &inodeIndex, sizeof(uint32_t));
        offset += sizeof(uint32_t);
    }

    // ʣ��Ļ��������� resize ʱ���Ϊ��
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

    // �����л�ÿ��Ŀ¼��
    for (uint32_t i = 0; i < entryCount; ++i) {
        if (offset + sizeof(uint32_t) > size) {
            throw std::runtime_error("Corrupted directory data (name length)");
        }

        // �����л����Ƴ���
        uint32_t nameLength = 0;
        std::memcpy(&nameLength, data + offset, sizeof(uint32_t));
        offset += sizeof(uint32_t);

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
