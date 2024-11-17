// src/SuperBlock.cpp
#include "SuperBlock.h"
#include <fstream>

SuperBlock::SuperBlock(uint32_t total, uint32_t free, uint32_t inodes, uint32_t root)
    : totalBlocks(total), freeBlocks(free), inodeCount(inodes), rootInode(root) {}

// 保存超级块到文件
void SuperBlock::saveToFile(const std::string& diskFile) {
    std::ofstream file(diskFile, std::ios::binary | std::ios::in | std::ios::out);
    if (file.is_open()) {
        file.seekp(0); // 超级块通常在文件的开头
        file.write(reinterpret_cast<const char*>(this), sizeof(SuperBlock));
        file.close();
    }
}

// 从文件加载超级块
void SuperBlock::loadFromFile(const std::string& diskFile) {
    std::ifstream file(diskFile, std::ios::binary);
    if (file.is_open()) {
        file.seekg(0); // 读取文件开头的超级块
        file.read(reinterpret_cast<char*>(this), sizeof(SuperBlock));
        file.close();
    }
}
