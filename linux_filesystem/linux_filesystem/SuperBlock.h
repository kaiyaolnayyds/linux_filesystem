// include/SuperBlock.h
#ifndef SUPERBLOCK_H
#define SUPERBLOCK_H

#include <cstdint>
#include <string>
#include <cstring>

class SuperBlock {
public:
    uint32_t totalBlocks;
    uint32_t freeBlocks;
    uint32_t inodeStartAddress;
    uint32_t rootInode;
    uint32_t inodeCount; // 一定要包含这个字段

    // 构造函数
    SuperBlock();
    SuperBlock(uint32_t totalBlocks, uint32_t freeBlocks, uint32_t inodeCount, uint32_t rootInode);

    // 将超级块数据保存到文件
    void saveToFile(const std::string& diskFile);

    // 从文件加载超级块数据
    void loadFromFile(const std::string& diskFile);

    void updateFreeBlocks(int change); // 增加或减少空闲块数
    void updateInodeCount(int change); // 增加或减少i节点数量

    // 序列化和反序列化方法
    void serialize(char* buffer) const;
    void deserialize(const char* buffer);

};

#endif // SUPERBLOCK_H
