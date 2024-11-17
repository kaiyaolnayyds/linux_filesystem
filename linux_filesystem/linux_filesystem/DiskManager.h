// include/DiskManager.h
#ifndef DISKMANAGER_H
#define DISKMANAGER_H

#include <string>
#include <vector>
#include "SuperBlock.h"

class DiskManager {
public:
    std::string diskFile;
    std::vector<bool> bitmap;  // 位图，管理空闲块
    size_t blockSize;
    size_t totalBlocks;

    DiskManager(const std::string& diskFile, size_t blockSize, size_t totalBlocks);

    void initialize();
    void readBlock(size_t blockIndex, char* buffer);
    void writeBlock(size_t blockIndex, const char* data);
    

    // 分配空闲块，返回块索引
    size_t allocateBlock();

    // 释放指定块
    void freeBlock(size_t blockIndex);

    // 检查块是否已分配
    bool isBlockAllocated(size_t blockIndex) const;

    // 用于调试，输出当前位图状态
    void printBitmap() const;


    void updateSuperBlock(const SuperBlock& superBlock);

    SuperBlock loadSuperBlock();
};

#endif // DISKMANAGER_H