// include/DiskManager.h
#ifndef DISKMANAGER_H
#define DISKMANAGER_H

#include <string>
#include <vector>

class DiskManager {
public:
    std::string diskFile;
    std::vector<bool> bitmap;  // ŒªÕº£¨π‹¿Ìø’œ–øÈ
    size_t blockSize;
    size_t totalBlocks;

    DiskManager(const std::string& diskFile, size_t blockSize, size_t totalBlocks);

    void initialize();
    void readBlock(size_t blockIndex, char* buffer);
    void writeBlock(size_t blockIndex, const char* data);
    size_t allocateBlock();
    void freeBlock(size_t blockIndex);
};

#endif // DISKMANAGER_H