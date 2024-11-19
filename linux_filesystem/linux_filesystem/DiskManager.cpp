// src/DiskManager.cpp
#include "DiskManager.h"
#include "SuperBlock.h"
#include "INode.h"
#include <fstream>
#include <iostream>
#include <cstring>


DiskManager::DiskManager(const std::string& diskFile, size_t blockSize, size_t totalBlocks)
    : diskFile(diskFile), blockSize(blockSize), totalBlocks(totalBlocks) {
    // 计算位图大小（以字节为单位），每个字节8位
    bitmapSize = (totalBlocks + 7) / 8;
    bitmap.resize(bitmapSize, 0);

    // 如果磁盘文件存在，加载位图和超级块
    std::ifstream fileCheck(diskFile);
    if (fileCheck.good()) {
        loadSuperBlock();
        loadBitmap();
        std::cout << "[DEBUG] Disk file exists. Loaded SuperBlock and bitmap." << std::endl;
    }
    else {
        std::cout << "[DEBUG] Disk file does not exist. Need to initialize file system." << std::endl;
    }
    fileCheck.close();
}

void DiskManager::initialize() {
    // 打开文件并清空内容
    std::ofstream file(diskFile, std::ios::binary | std::ios::trunc);
    if (!file.is_open()) {
        std::cerr << "Error: Unable to create disk file." << std::endl;
        return;
    }

    // 初始化超级块
    SuperBlock superBlock(static_cast<uint32_t>(totalBlocks), static_cast<uint32_t>(totalBlocks), 0, 0);

    // 初始化位图（所有块空闲）
    bitmap.assign(bitmapSize, 0);

    // 分配根目录块
    size_t rootBlockIndex = allocateBlock();
    if (rootBlockIndex == static_cast<size_t>(-1)) {
        std::cerr << "Error: Unable to allocate block for root directory." << std::endl;
        return;
    }

    // 更新超级块
    superBlock.freeBlocks = static_cast<uint32_t>(totalBlocks - 1);
    superBlock.rootInode = static_cast<uint32_t>(rootBlockIndex);
    updateSuperBlock(superBlock);

    // 更新位图
    updateBitmap();

    // 初始化磁盘块（可选，已在 writeBlock 中处理）
    // ...

    // 创建空的根目录并写入磁盘
    Directory rootDirectory;
    std::vector<char> buffer;
    rootDirectory.serialize(buffer, blockSize);
    writeBlock(rootBlockIndex, buffer.data());

    // 调试信息
    std::cout << "[DEBUG] Root directory created at block " << rootBlockIndex << "." << std::endl;

    // 关闭文件
    file.close();

    // 调试信息
    std::cout << "[DEBUG] SuperBlock initialized with:" << std::endl;
    std::cout << "Total Blocks: " << superBlock.totalBlocks << std::endl;
    std::cout << "Free Blocks: " << superBlock.freeBlocks << std::endl;
    std::cout << "iNode Count: " << superBlock.inodeCount << std::endl;
    std::cout << "[DEBUG] Bitmap size: " << bitmapSize << " bytes." << std::endl;
}



void DiskManager::readBlock(size_t blockIndex, char* buffer) {
    if (blockIndex >= totalBlocks) return;

    std::ifstream file(diskFile, std::ios::binary);
    if (!file) return;

    // 计算正确的文件偏移，跳过SuperBlock和位图
    std::streampos offset = sizeof(SuperBlock) + bitmapSize + blockIndex * blockSize;
    file.seekg(offset);
    file.read(buffer, blockSize);
    file.close();
}


void DiskManager::writeBlock(size_t blockIndex, const char* data) {
    if (blockIndex >= totalBlocks) return;

    std::fstream file(diskFile, std::ios::binary | std::ios::in | std::ios::out);
    if (!file) return;

    // 计算正确的文件偏移，跳过SuperBlock和位图
    std::streampos offset = sizeof(SuperBlock) + bitmapSize + blockIndex * blockSize;
    file.seekp(offset);
    file.write(data, blockSize);
    file.close();
}



size_t DiskManager::allocateBlock() {
    for (size_t i = 0; i < totalBlocks; ++i) {
        size_t byteIndex = i / 8;
        size_t bitIndex = i % 8;
        if ((bitmap[byteIndex] & (1 << bitIndex)) == 0) { // 位为0，表示空闲
           bitmap[byteIndex] |= (1 << bitIndex); // 标记为已分配
            updateBitmap(); // 更新磁盘中的位图

            // 更新超级块中的空闲块数量
            //SuperBlock superBlock = loadSuperBlock();
           // superBlock.freeBlocks -= 1;
           // updateSuperBlock(superBlock);

            return i;
        }
    }
    return static_cast<size_t>(-1); // 无可用块
}


void DiskManager::freeBlock(size_t blockIndex) {
    if (blockIndex < totalBlocks) {
        size_t byteIndex = blockIndex / 8;
        size_t bitIndex = blockIndex % 8;
        bitmap[byteIndex] &= ~(1 << bitIndex); // 标记为空闲
        updateBitmap(); // 更新磁盘中的位图

        // 更新超级块中的空闲块数量
        SuperBlock superBlock = loadSuperBlock();
        superBlock.freeBlocks += 1;
        updateSuperBlock(superBlock);
    }
}


bool DiskManager::isBlockAllocated(size_t blockIndex) const {
    if (blockIndex >= totalBlocks) return false;
    size_t byteIndex = blockIndex / 8;
    size_t bitIndex = blockIndex % 8;
    return (bitmap[byteIndex] & (1 << bitIndex)) != 0;
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
        file.seekp(0, std::ios::beg);
        file.write(reinterpret_cast<const char*>(&superBlock.totalBlocks), sizeof(uint32_t));
        file.write(reinterpret_cast<const char*>(&superBlock.freeBlocks), sizeof(uint32_t));
        file.write(reinterpret_cast<const char*>(&superBlock.inodeCount), sizeof(uint32_t));
        file.write(reinterpret_cast<const char*>(&superBlock.rootInode), sizeof(uint32_t));
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
    SuperBlock superBlock;
    if (file.is_open()) {
        file.seekg(0, std::ios::beg);
        file.read(reinterpret_cast<char*>(&superBlock.totalBlocks), sizeof(uint32_t));
        file.read(reinterpret_cast<char*>(&superBlock.freeBlocks), sizeof(uint32_t));
        file.read(reinterpret_cast<char*>(&superBlock.inodeCount), sizeof(uint32_t));
        file.read(reinterpret_cast<char*>(&superBlock.rootInode), sizeof(uint32_t));
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


void DiskManager::loadBitmap() {
    std::ifstream file(diskFile, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Error: Unable to open disk file for reading bitmap." << std::endl;
        return;
    }

    // 跳过SuperBlock
    file.seekg(sizeof(SuperBlock), std::ios::beg);

    // 读取位图数据
    file.read(reinterpret_cast<char*>(bitmap.data()), bitmapSize);

    file.close();
}



void DiskManager::updateBitmap() {
    std::fstream file(diskFile, std::ios::binary | std::ios::in | std::ios::out);
    if (!file.is_open()) {
        std::cerr << "Error: Unable to open disk file for updating bitmap." << std::endl;
        return;
    }

    // 位图在SuperBlock之后
    file.seekp(sizeof(SuperBlock), std::ios::beg);

    // 写入位图数据
    file.write(reinterpret_cast<const char*>(bitmap.data()), bitmapSize);

    file.close();
}

