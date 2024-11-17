// include/SuperBlock.h
#ifndef SUPERBLOCK_H
#define SUPERBLOCK_H

#include <cstdint>
#include <string>

class SuperBlock {
public:
    uint32_t totalBlocks;
    uint32_t freeBlocks;
    uint32_t inodeCount;
    uint32_t rootInode;

    // 构造函数
    SuperBlock(uint32_t total = 0, uint32_t free = 0, uint32_t inodes = 0, uint32_t root = 0);

    // 将超级块数据保存到文件
    void saveToFile(const std::string& diskFile);

    // 从文件加载超级块数据
    void loadFromFile(const std::string& diskFile);

    void updateFreeBlocks(int change); // 增加或减少空闲块数
    void updateInodeCount(int change); // 增加或减少i节点数量

};

#endif // SUPERBLOCK_H
