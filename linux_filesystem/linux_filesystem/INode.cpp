// src/INode.cpp
#include "INode.h"

INode::INode(uint32_t size, uint32_t type, uint32_t permissions)
    : size(size), type(type), permissions(permissions), ownerUID(-1) {
    blocks.resize(12, 0); // 初始化数据块指针数组，假设最多12个块
}

void INode::addBlock(uint32_t blockIndex) {
    blocks.push_back(blockIndex);
}

uint32_t INode::getBlock(size_t index) const {
    if (index < blocks.size()) {
        return blocks[index];
    }
    return -1; // 返回-1表示块不存在
}

size_t INode::getBlockCount() const {
    return blocks.size();
}

void INode::serialize(char* buffer) const {
    size_t offset = 0;
    std::memcpy(buffer + offset, &size, sizeof(uint32_t));
    offset += sizeof(uint32_t);
    std::memcpy(buffer + offset, &blockIndex, sizeof(uint32_t));
    offset += sizeof(uint32_t);
    std::memcpy(buffer + offset, &inodeIndex, sizeof(uint32_t));
    offset += sizeof(uint32_t);
    std::memcpy(buffer + offset, &mode, sizeof(uint16_t));
    offset += sizeof(uint16_t);
    std::memcpy(buffer + offset, &type, sizeof(uint32_t)); 
    offset += sizeof(uint32_t);
    std::memcpy(buffer + offset, &permissions, sizeof(uint32_t)); 
    offset += sizeof(uint32_t);
    std::memcpy(buffer + offset, &ownerUID, sizeof(int)); 
    offset += sizeof(int);
    // 如果需要，序列化其他成员
}

void INode::deserialize(const char* buffer) {
    size_t offset = 0;
    std::memcpy(&size, buffer + offset, sizeof(uint32_t));
    offset += sizeof(uint32_t);
    std::memcpy(&blockIndex, buffer + offset, sizeof(uint32_t));
    offset += sizeof(uint32_t);
    std::memcpy(&inodeIndex, buffer + offset, sizeof(uint32_t));
    offset += sizeof(uint32_t);
    std::memcpy(&mode, buffer + offset, sizeof(uint16_t));
    offset += sizeof(uint16_t);
    std::memcpy(&type, buffer + offset, sizeof(uint32_t)); 
    offset += sizeof(uint32_t);
    std::memcpy(&permissions, buffer + offset, sizeof(uint32_t)); 
    offset += sizeof(uint32_t);
    std::memcpy(&ownerUID, buffer + offset, sizeof(int));
    offset += sizeof(int);
    // 如果需要，反序列化其他成员
}