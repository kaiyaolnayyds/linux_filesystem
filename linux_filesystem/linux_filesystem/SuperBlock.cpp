// src/SuperBlock.cpp
#include "SuperBlock.h"
#include <fstream>

SuperBlock::SuperBlock() : totalBlocks(0), freeBlocks(0), inodeCount(0), rootInode(0), rootDataBlock(0),inodeStartAddress(0)
{
}

SuperBlock::SuperBlock(uint32_t totalBlocks, uint32_t freeBlocks, uint32_t inodeCount, uint32_t rootInode, uint32_t rootDataBlock)
    : totalBlocks(totalBlocks), freeBlocks(freeBlocks), inodeCount(inodeCount), rootInode(rootInode), rootDataBlock(rootDataBlock),inodeStartAddress(0) {}


// 保存超级块到文件
void SuperBlock::saveToFile(const std::string& diskFile) {
    std::ofstream file(diskFile, std::ios::binary | std::ios::in | std::ios::out);
    if (file.is_open()) {
        file.seekp(0); // 超级块通常在文件的开头
        char buffer[SUPERBLOCK_SIZE];
        serialize(buffer);
        file.write(buffer, SUPERBLOCK_SIZE);
        file.close();
    }
}


// 从文件加载超级块
void SuperBlock::loadFromFile(const std::string& diskFile) {
    std::ifstream file(diskFile, std::ios::binary);
    if (file.is_open()) {
        file.seekg(0); // 读取文件开头的超级块
        char buffer[SUPERBLOCK_SIZE];
        file.read(buffer, SUPERBLOCK_SIZE);
        deserialize(buffer);
        file.close();
    }
}


void SuperBlock::updateFreeBlocks(int change) {
    freeBlocks += change;
}

void SuperBlock::updateInodeCount(int change) {
    inodeCount += change;
}


void SuperBlock::serialize(char* buffer) const
{
    size_t offset = 0;
    std::memcpy(buffer + offset, &totalBlocks, sizeof(uint32_t));
    offset += sizeof(uint32_t);
    std::memcpy(buffer + offset, &freeBlocks, sizeof(uint32_t));
    offset += sizeof(uint32_t);
    std::memcpy(buffer + offset, &inodeCount, sizeof(uint32_t));
    offset += sizeof(uint32_t);
    std::memcpy(buffer + offset, &rootInode, sizeof(uint32_t));
    offset += sizeof(uint32_t);
    std::memcpy(buffer + offset, &rootDataBlock, sizeof(uint32_t));
    offset += sizeof(uint32_t);
    std::memcpy(buffer + offset, &inodeStartAddress, sizeof(uint32_t));
    offset += sizeof(uint32_t);
    std::memcpy(buffer + offset, &dataBlockStartAddress, sizeof(uint32_t));
    offset += sizeof(uint32_t);
}

void SuperBlock::deserialize(const char* buffer)
{
    size_t offset = 0;
    std::memcpy(&totalBlocks, buffer + offset, sizeof(uint32_t));
    offset += sizeof(uint32_t);
    std::memcpy(&freeBlocks, buffer + offset, sizeof(uint32_t));
    offset += sizeof(uint32_t);
    std::memcpy(&inodeCount, buffer + offset, sizeof(uint32_t));
    offset += sizeof(uint32_t);
    std::memcpy(&rootInode, buffer + offset, sizeof(uint32_t));
    offset += sizeof(uint32_t);
    std::memcpy(&rootDataBlock, buffer + offset, sizeof(uint32_t));
    offset += sizeof(uint32_t);
    std::memcpy(&inodeStartAddress, buffer + offset, sizeof(uint32_t));
    offset += sizeof(uint32_t);
    std::memcpy(&dataBlockStartAddress, buffer + offset, sizeof(uint32_t));
    offset += sizeof(uint32_t);
}
