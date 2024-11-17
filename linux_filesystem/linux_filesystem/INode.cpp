// src/INode.cpp
#include "INode.h"

INode::INode(uint32_t size, uint32_t type, uint32_t permissions)
    : size(size), type(type), permissions(permissions) {
    blocks.resize(12, 0); // ��ʼ�����ݿ�ָ�����飬�������12����
}

void INode::addBlock(uint32_t blockIndex) {
    blocks.push_back(blockIndex);
}

uint32_t INode::getBlock(size_t index) const {
    if (index < blocks.size()) {
        return blocks[index];
    }
    return -1; // ����-1��ʾ�鲻����
}

size_t INode::getBlockCount() const {
    return blocks.size();
}
