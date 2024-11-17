// src/DiskManager.cpp
#include "DiskManager.h"
#include "SuperBlock.h"
#include "INode.h"
#include <fstream>
#include <iostream>
#include <cstring>


DiskManager::DiskManager(const std::string& diskFile, size_t blockSize, size_t totalBlocks)
    : diskFile(diskFile), blockSize(blockSize), totalBlocks(totalBlocks) {
    bitmap.resize(totalBlocks, true); // 假设初始时所有块都是空闲的
}

void DiskManager::initialize() {
    std::ofstream file(diskFile, std::ios::binary | std::ios::trunc);
    if (!file.is_open()) {
        std::cerr << "Error: Unable to create disk file." << std::endl;
        return;
    }

    // 初始化超级块
    SuperBlock superBlock(static_cast<uint32_t>(totalBlocks), static_cast<uint32_t>(totalBlocks), 0, 0);
    file.write(reinterpret_cast<const char*>(&superBlock), sizeof(SuperBlock));

    // 初始化位图
    bitmap.resize(totalBlocks, true);

    char emptyBlock[blockSize];
    std::memset(emptyBlock, 0, blockSize);

    // 写入空块到模拟磁盘
    for (size_t i = 0; i < totalBlocks; ++i) {
        file.write(emptyBlock, blockSize);
    }

    file.close();

    // 调试信息
    std::cout << "[DEBUG] SuperBlock initialized with:" << std::endl;
    std::cout << "Total Blocks: " << superBlock.totalBlocks << std::endl;
    std::cout << "Free Blocks: " << superBlock.freeBlocks << std::endl;
    std::cout << "iNode Count: " << superBlock.inodeCount << std::endl;
}





void DiskManager::readBlock(size_t blockIndex, char* buffer) {
    if (blockIndex >= totalBlocks) return;

    std::ifstream file(diskFile, std::ios::binary);
    if (!file) return;

    file.seekg(blockIndex * blockSize);
    file.read(buffer, blockSize);
    file.close();
}

void DiskManager::writeBlock(size_t blockIndex, const char* data) {
    if (blockIndex >= totalBlocks) return;

    std::ofstream file(diskFile, std::ios::binary | std::ios::in | std::ios::out);
    if (!file) return;

    file.seekp(blockIndex * blockSize);
    file.write(data, blockSize);
    file.close();
}



size_t DiskManager::allocateBlock() {
    for (size_t i = 0; i < bitmap.size(); ++i) {
        if (bitmap[i]) { // 找到空闲块
            bitmap[i] = false; // 标记为已分配
            return i;
        }
    }
    return -1; // 无可用块
}

void DiskManager::freeBlock(size_t blockIndex) {
    if (blockIndex < bitmap.size()) {
        bitmap[blockIndex] = true; // 标记为释放
    }
}

bool DiskManager::isBlockAllocated(size_t blockIndex) const {
    return blockIndex < bitmap.size() && !bitmap[blockIndex];
}

void DiskManager::printBitmap() const {
    std::cout << "Bitmap: ";
    for (size_t i = 0; i < bitmap.size(); ++i) {
        std::cout << (bitmap[i] ? "0" : "1"); // 0表示空闲，1表示已分配
    }
    std::cout << std::endl;
}

void DiskManager::updateSuperBlock(const SuperBlock& superBlock) {
    std::fstream file(diskFile, std::ios::binary | std::ios::in | std::ios::out);
    if (file.is_open()) {
        file.seekp(0, std::ios::beg); // 确保写入位置在文件开头
        file.write(reinterpret_cast<const char*>(&superBlock), sizeof(SuperBlock));
        file.close();

        // 调试信息
        std::cout << "[DEBUG] SuperBlock updated on disk:" << std::endl;
        std::cout << "Total Blocks: " << superBlock.totalBlocks << std::endl;
        std::cout << "Free Blocks: " << superBlock.freeBlocks << std::endl;
        std::cout << "iNode Count: " << superBlock.inodeCount << std::endl;
    }
    else {
        std::cerr << "Error: Unable to update super block." << std::endl;
    }
}



SuperBlock DiskManager::loadSuperBlock() {
    std::ifstream file(diskFile, std::ios::binary);
    SuperBlock superBlock(0, 0, 0, 0);
    if (file.is_open()) {
        file.seekg(0, std::ios::beg); // 确保从文件开头读取
        file.read(reinterpret_cast<char*>(&superBlock), sizeof(SuperBlock));
        file.close();

        // 调试信息
        std::cout << "[DEBUG] SuperBlock loaded from disk:" << std::endl;
        std::cout << "Total Blocks: " << superBlock.totalBlocks << std::endl;
        std::cout << "Free Blocks: " << superBlock.freeBlocks << std::endl;
        std::cout << "iNode Count: " << superBlock.inodeCount << std::endl;
    }
    else {
        std::cerr << "Error: Unable to load super block." << std::endl;
    }
    return superBlock;
}

