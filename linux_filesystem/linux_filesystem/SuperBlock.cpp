// src/SuperBlock.cpp
#include "SuperBlock.h"
#include <fstream>

SuperBlock::SuperBlock(uint32_t total, uint32_t free, uint32_t inodes, uint32_t root)
    : totalBlocks(total), freeBlocks(free), inodeCount(inodes), rootInode(root) {}

// ���泬���鵽�ļ�
void SuperBlock::saveToFile(const std::string& diskFile) {
    std::ofstream file(diskFile, std::ios::binary | std::ios::in | std::ios::out);
    if (file.is_open()) {
        file.seekp(0); // ������ͨ�����ļ��Ŀ�ͷ
        file.write(reinterpret_cast<const char*>(this), sizeof(SuperBlock));
        file.close();
    }
}

// ���ļ����س�����
void SuperBlock::loadFromFile(const std::string& diskFile) {
    std::ifstream file(diskFile, std::ios::binary);
    if (file.is_open()) {
        file.seekg(0); // ��ȡ�ļ���ͷ�ĳ�����
        file.read(reinterpret_cast<char*>(this), sizeof(SuperBlock));
        file.close();
    }
}
