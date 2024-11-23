// include/DiskManager.h

#ifndef DISKMANAGER_H
#define DISKMANAGER_H

#include <string>
#include <vector>
#include <cstdint>
#include "Directory.h"
#include "SuperBlock.h"
#include "INode.h"


constexpr size_t INODE_SIZE = sizeof(uint32_t) * 5 + sizeof(uint16_t)+sizeof(int); // Inode大小
constexpr uint32_t MAX_INODES = 1024; // 最大Inode数，根据需要设置

class DiskManager {
public:
    std::string diskFile;                // 磁盘文件名
    std::vector<uint8_t> bitmap;         // 数据块位图
    size_t bitmapSize;                   // 数据块位图大小（字节数）
    std::vector<uint8_t> inodeBitmap;    // inode 位图
    size_t inodeBitmapSize;              // inode 位图大小（字节数）
    size_t blockSize;                    // 块大小
    size_t totalBlocks;                  // 块总数
    SuperBlock superBlock;               // 超级块成员变量

    DiskManager(const std::string& diskFile, size_t blockSize, size_t totalBlocks);

    //磁盘对象初始化方法
    void initialize();

    //读取数据块
    void readBlock(size_t blockIndex, char* buffer);

    //写入数据块
    void writeBlock(size_t blockIndex, const char* data);

    // 分配空闲块，返回块索引
    size_t allocateBlock();

    // 释放指定块
    void freeBlock(size_t blockIndex);

    // 检查块是否已分配
    bool isBlockAllocated(size_t blockIndex) const;

    // 分配空闲 inode，返回 inode 索引
    uint32_t allocateINode();

    // 释放指定 inode
    void freeINode(uint32_t inodeIndex);

    // 检查 inode 是否已分配
    bool isINodeAllocated(uint32_t inodeIndex) const;

    // 用于调试，输出当前位图状态
    void printBitmap() const;

    // 加载位图
    void loadBitmaps();

    // 更新位图
    void updateBitmaps();

    // 更新超级块
    void updateSuperBlock(const SuperBlock& superBlock);

    //重新计算并更新超级块中的 freeBlocks 和 inodeCount
    void updateSuperBlockUsage();

    // 加载超级块
    SuperBlock loadSuperBlock();

    // 写入 inode
    void writeINode(uint32_t inodeIndex, const INode& inode);

    // 读取 inode
    INode readINode(uint32_t inodeIndex);

    //在指定索引处分配一个 inode，用于修复文件系统不一致。
    void allocateINodeAtIndex(uint32_t inodeIndex);

    //在指定索引处分配一个数据块，用于修复文件系统不一致。
    void allocateBlockAtIndex(size_t blockIndex);

    //计算当前文件系统中空闲的块数量。
    size_t calculateFreeBlocks() const;

    //计算当前文件系统中已分配的 inode 数量。
    uint32_t calculateAllocatedInodes() const;

};

#endif // DISKMANAGER_H
