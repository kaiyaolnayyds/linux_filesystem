// include/DiskManager.h
#ifndef DISKMANAGER_H
#define DISKMANAGER_H

#include <string>
#include <vector>
#include <cstdint>
#include "SuperBlock.h"
#include "Directory.h"

class DiskManager {
public:
    std::string diskFile;
    std::vector<uint8_t> bitmap;    // 位图，使用uint8_t存储
    size_t bitmapSize;        // 位图大小（字节数）
    size_t blockSize;    //块大小
    size_t totalBlocks;  //块总数

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

    //加载位图
    void loadBitmap();

    //更新位图
    void updateBitmap();

    //更新超级块
    void updateSuperBlock(const SuperBlock& superBlock);

    //加载超级块
    SuperBlock loadSuperBlock();
};

#endif // DISKMANAGER_H