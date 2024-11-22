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
//    // ���л� Directory ����
//    std::vector<char> buffer;
//    serialize(buffer, diskManager.blockSize);
//
//    // �����л�����д�����
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
    buffer.resize(blockSize, 0); // ֱ�ӷ�����С�Ļ�����
    char* ptr = buffer.data();

    // ���л� entries ������
    uint32_t entryCount = static_cast<uint32_t>(entries.size());
    std::memcpy(ptr, &entryCount, sizeof(entryCount));
    ptr += sizeof(entryCount);

    // ���л�ÿ��Ŀ¼��
    for (const auto& entry : entries) {
        const std::string& name = entry.first;
        uint32_t inodeIndex = entry.second;

        // ���л����Ƴ���
        uint32_t nameLength = static_cast<uint32_t>(name.length());
        std::memcpy(ptr, &nameLength, sizeof(nameLength));
        ptr += sizeof(nameLength);

        // ���л�����
        std::memcpy(ptr, name.data(), nameLength);
        ptr += nameLength;

        // ���л� inodeIndex
        std::memcpy(ptr, &inodeIndex, sizeof(inodeIndex));
        ptr += sizeof(inodeIndex);
    }
}





void Directory::deserialize(const char* data, size_t size) {
    size_t offset = 0;
    entries.clear();

    // ������ݴ�С
    if (size < sizeof(uint32_t)) {
        throw std::runtime_error("Insufficient data for deserialization");
    }

    // �����л� entries ������
    uint32_t entryCount = 0;
    std::memcpy(&entryCount, data + offset, sizeof(entryCount));
    offset += sizeof(entryCount);

    // �����л�ÿ��Ŀ¼��
    for (uint32_t i = 0; i < entryCount; ++i) {
        if (offset + sizeof(uint32_t) > size) throw std::runtime_error("Corrupted directory data");

        // �����л����Ƴ���
        uint32_t nameLength = 0;
        std::memcpy(&nameLength, data + offset, sizeof(nameLength));
        offset += sizeof(nameLength);

        // ������Ƴ����Ƿ�Ϸ�
        if (nameLength > 255 || offset + nameLength + sizeof(uint32_t) > size) {
            throw std::runtime_error("Invalid or corrupted directory data");
        }

        // �����л�����
        std::string name(data + offset, nameLength);
        offset += nameLength;

        // �����л� inodeIndex
        uint32_t inodeIndex = 0;
        std::memcpy(&inodeIndex, data + offset, sizeof(inodeIndex));
        offset += sizeof(inodeIndex);

        entries[name] = inodeIndex;
    }
}
