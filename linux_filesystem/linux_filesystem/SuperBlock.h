// include/SuperBlock.h
/*
superblock实现
主要记录block与Inode的总量，未使用/已使用的inode/block数量

*/


#ifndef SUPERBLOCK_H
#define SUPERBLOCK_H
#include <cstdint>
#include <string>
#include <cstring>

class SuperBlock {
public:
    uint32_t totalBlocks;       //块总数
    uint32_t freeBlocks;        //剩余块数
    uint32_t inodeCount;        //Inode数
    uint32_t rootInode;          //初始Inode数
    uint32_t inodeStartAddress;  // inode 区域在磁盘上的起始地址
    uint32_t dataBlockStartAddress; // 数据块区域在磁盘上的起始地址

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