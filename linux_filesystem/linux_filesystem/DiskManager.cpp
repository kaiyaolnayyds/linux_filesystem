// src/DiskManager.cpp
#include "DiskManager.h"
#include "SuperBlock.h"
#include "INode.h"
#include <fstream>
#include <iostream>
#include <cstring>
#include "SuperBlock.h"

DiskManager::DiskManager(const std::string& diskFile, size_t blockSize, size_t totalBlocks)
    : diskFile(diskFile), blockSize(blockSize), totalBlocks(totalBlocks) {
    // 初始化数据块位图大小并分配空间
    bitmapSize = (totalBlocks + 7) / 8;
    bitmap.resize(bitmapSize, 0);

    // 初始化 inode 位图大小并分配空间
    inodeBitmapSize = (MAX_INODES + 7) / 8;
    inodeBitmap.resize(inodeBitmapSize, 0);

    // 检查磁盘文件是否存在
    std::ifstream fileCheck(diskFile, std::ios::binary);
    if (fileCheck.good()) {
        // 加载超级块和位图
        superBlock = loadSuperBlock();
        loadBitmaps();
        std::cout << "[DEBUG] Disk file exists. Loaded SuperBlock and bitmaps." << std::endl;
    }
    else {
        std::cout << "[DEBUG] Disk file does not exist. Need to initialize file system." << std::endl;
    }
    fileCheck.close();
}

void DiskManager::initialize() {
    // 打开磁盘文件用于写入，使用 trunc 模式清空文件
    std::ofstream file(diskFile, std::ios::binary | std::ios::out | std::ios::trunc);
    if (!file.is_open()) {
        std::cerr << "Error: Unable to create disk file." << std::endl;
        return;
    }

    // **预分配磁盘文件大小**
    size_t diskFileSize = totalBlocks * blockSize;
    file.seekp(diskFileSize - 1);
    file.write("", 1); // 写入一个字节，以确保文件大小
    file.close();

    // 初始化超级块
    superBlock = SuperBlock(static_cast<uint32_t>(totalBlocks), static_cast<uint32_t>(totalBlocks), 0, 0);

    // 计算超级块和位图的大小
    size_t superBlockSize = SUPERBLOCK_SIZE; // 使用固定大小

    // 初始化 inode 位图
    inodeBitmapSize = (MAX_INODES + 7) / 8;
    inodeBitmap.resize(inodeBitmapSize, 0);

    // 初始化数据块位图
    bitmapSize = (totalBlocks + 7) / 8;
    bitmap.resize(bitmapSize, 0);

    // inodeStartAddress 是超级块大小加上 inode 位图和数据块位图大小
    superBlock.inodeStartAddress = static_cast<uint32_t>(superBlockSize + inodeBitmapSize + bitmapSize);
    std::cout << "[DEBUG] inodeStartAddress: " << superBlock.inodeStartAddress << std::endl;

    // 计算数据块的起始地址，数据块存在Inode区后
    superBlock.dataBlockStartAddress = superBlock.inodeStartAddress + MAX_INODES * INODE_SIZE;

    // 将超级块写入磁盘
    updateSuperBlock(superBlock);

    // 更新位图
    updateBitmaps();

    // 为根目录的数据分配一个块
    size_t rootBlockIndex = allocateBlock(); // 更新位图
    if (rootBlockIndex == static_cast<size_t>(-1)) {
        std::cerr << "Error: Unable to allocate block for root directory." << std::endl;
        return;
    }

    // 分配一个新的 inode
    uint32_t rootInodeIndex = allocateINode();
    if (rootInodeIndex == static_cast<uint32_t>(-1)) {
        std::cerr << "Error: Unable to allocate inode for root directory." << std::endl;
        return;
    }

    // 创建根目录的 inode
    INode rootInode;
    rootInode.size = 0;
    rootInode.mode = 0755; // 目录的默认权限
    rootInode.type = 1;    // 目录
    rootInode.blockIndex = static_cast<uint32_t>(rootBlockIndex);
    rootInode.inodeIndex = rootInodeIndex;

    // 更新超级块
    superBlock.freeBlocks--;
    superBlock.rootInode = rootInodeIndex;
    updateSuperBlock(superBlock);

    // 将根目录的 inode 写入磁盘
    writeINode(rootInodeIndex, rootInode);

    // 创建空的根目录并写入磁盘
    Directory rootDirectory;
    rootDirectory.entries["."] = rootInodeIndex;  // 根目录当前目录
    rootDirectory.entries[".."] = rootInodeIndex; // 根目录的父目录是自己
    std::vector<char> buffer;
    rootDirectory.serialize(buffer, blockSize);
    writeBlock(rootBlockIndex, buffer.data());

}

void DiskManager::writeBlock(size_t blockIndex, const char* data) {
    if (blockIndex >= totalBlocks) return;

    std::fstream file(diskFile, std::ios::binary | std::ios::in | std::ios::out);
    if (!file) return;

    // 数据块偏移量，使用固定的数据块起始地址，从数据块起始位置往后数到已经使用掉的数据块位置
    std::streampos offset = superBlock.dataBlockStartAddress + blockIndex * blockSize;
    file.seekp(offset);
    file.write(data, blockSize);
    file.close();
}

void DiskManager::readBlock(size_t blockIndex, char* buffer) {
    if (blockIndex >= totalBlocks) return;

    std::ifstream file(diskFile, std::ios::binary);
    if (!file) return;

    // 数据块偏移量，使用固定的数据块起始地址，从数据块起始位置往后数到已经使用掉的数据块位置
    std::streampos offset = superBlock.dataBlockStartAddress + blockIndex * blockSize;
    file.seekg(offset);
    file.read(buffer, blockSize);
    file.close();
}

size_t DiskManager::allocateBlock() {
    for (size_t i = 0; i < totalBlocks; ++i) {
        size_t byteIndex = i / 8;
        size_t bitIndex = i % 8;
        if ((bitmap[byteIndex] & (1 << bitIndex)) == 0) { // 位为0，表示空闲
            bitmap[byteIndex] |= (1 << bitIndex); // 标记为已分配
            updateBitmaps(); // 更新磁盘中的位图

            // 更新超级块中的空闲块数量
            superBlock.freeBlocks--;
            updateSuperBlock(superBlock);

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
        updateBitmaps(); // 更新磁盘中的位图

        // 更新超级块中的空闲块数量
        superBlock.freeBlocks++;
        updateSuperBlock(superBlock);
    }
}

bool DiskManager::isBlockAllocated(size_t blockIndex) const {
    if (blockIndex >= totalBlocks) return false;
    size_t byteIndex = blockIndex / 8;
    size_t bitIndex = blockIndex % 8;
    return (bitmap[byteIndex] & (1 << bitIndex)) != 0;
}

uint32_t DiskManager::allocateINode() {
    for (uint32_t i = 0; i < MAX_INODES; ++i) {
        size_t byteIndex = i / 8;
        size_t bitIndex = i % 8;
        if ((inodeBitmap[byteIndex] & (1 << bitIndex)) == 0) { // 位为0，表示空闲
            inodeBitmap[byteIndex] |= (1 << bitIndex); // 标记为已分配
            updateBitmaps(); // 更新磁盘中的位图

            // 更新超级块中的 inode 计数
            superBlock.inodeCount++;
            updateSuperBlock(superBlock);

            return i;
        }
    }
    return static_cast<uint32_t>(-1); // 无可用 inode
}

void DiskManager::freeINode(uint32_t inodeIndex) {
    if (inodeIndex >= MAX_INODES) return;

    size_t byteIndex = inodeIndex / 8;
    size_t bitIndex = inodeIndex % 8;
    inodeBitmap[byteIndex] &= ~(1 << bitIndex); // 标记为空闲
    updateBitmaps(); // 更新磁盘中的位图

    // 更新超级块中的 inode 计数
    superBlock.inodeCount--;
    updateSuperBlock(superBlock);
}

bool DiskManager::isINodeAllocated(uint32_t inodeIndex) const {
    if (inodeIndex >= MAX_INODES) return false;
    size_t byteIndex = inodeIndex / 8;
    size_t bitIndex = inodeIndex % 8;
    return (inodeBitmap[byteIndex] & (1 << bitIndex)) != 0;
}

void DiskManager::printBitmap() const {
    std::cout << "Data Block Bitmap: ";
    for (size_t i = 0; i < bitmap.size(); ++i) {
        for (int bit = 7; bit >= 0; --bit) {
            std::cout << ((bitmap[i] & (1 << bit)) ? "1" : "0");
        }
    }
    std::cout << std::endl;
    std::cout << "INode Bitmap: ";
    for (size_t i = 0; i < inodeBitmap.size(); ++i) {
        for (int bit = 7; bit >= 0; --bit) {
            std::cout << ((inodeBitmap[i] & (1 << bit)) ? "1" : "0");
        }
    }
    std::cout << std::endl;
}

void DiskManager::updateSuperBlock(const SuperBlock& sb) {
    std::ofstream file(diskFile, std::ios::binary | std::ios::in | std::ios::out);
    if (!file.is_open()) {
        std::cerr << "Error: Unable to open disk file for writing superblock." << std::endl;
        return;
    }
    char buffer[SUPERBLOCK_SIZE];
    sb.serialize(buffer);
    file.seekp(0);
    file.write(buffer, SUPERBLOCK_SIZE);
    file.close();

    superBlock = sb; // 更新成员变量
}

SuperBlock DiskManager::loadSuperBlock() {
    SuperBlock sb;
    std::ifstream file(diskFile, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Error: Unable to open disk file for reading superblock." << std::endl;
        return sb;
    }
    char buffer[SUPERBLOCK_SIZE];
    file.read(buffer, SUPERBLOCK_SIZE);
    file.close();

    sb.deserialize(buffer);
    superBlock = sb; // 更新成员变量
    return sb;
}

void DiskManager::loadBitmaps() {
    std::ifstream file(diskFile, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Error: Unable to open disk file for reading bitmaps." << std::endl;
        return;
    }

    // 跳过超级块
    file.seekg(SUPERBLOCK_SIZE, std::ios::beg);

    // 读取 inode 位图
    inodeBitmap.resize(inodeBitmapSize);
    file.read(reinterpret_cast<char*>(inodeBitmap.data()), inodeBitmapSize);

    // 读取数据块位图
    bitmap.resize(bitmapSize);
    file.read(reinterpret_cast<char*>(bitmap.data()), bitmapSize);

    file.close();
}

void DiskManager::updateBitmaps() {
    std::fstream file(diskFile, std::ios::binary | std::ios::in | std::ios::out);
    if (!file.is_open()) {
        std::cerr << "Error: Unable to open disk file for updating bitmaps." << std::endl;
        return;
    }

    // 位图在超级块之后
    file.seekp(SUPERBLOCK_SIZE, std::ios::beg);

    // 写入 inode 位图
    file.write(reinterpret_cast<const char*>(inodeBitmap.data()), inodeBitmapSize);

    // 写入数据块位图
    file.write(reinterpret_cast<const char*>(bitmap.data()), bitmapSize);

    file.close();
}

void DiskManager::writeINode(uint32_t inodeIndex, const INode& inode) {
    char buffer[INODE_SIZE];
    inode.serialize(buffer);

    // 计算 inode 在磁盘文件中的偏移量
    size_t offset = superBlock.inodeStartAddress + inodeIndex * INODE_SIZE;
    std::cout << "[DEBUG] writeINode: inodeIndex=" << inodeIndex << ", offset=" << offset << std::endl;

    // 打开文件并写入 inode
    std::fstream file(diskFile, std::ios::binary | std::ios::in | std::ios::out);
    if (!file.is_open()) {
        std::cerr << "Error: Unable to open disk file for writing inode." << std::endl;
        return;
    }
    file.seekp(offset);
    if (file.fail()) {
        std::cerr << "Error: Failed to seek to position " << offset << " in disk file." << std::endl;
        file.close();
        return;
    }
    file.write(buffer, INODE_SIZE);
    if (file.fail()) {
        std::cerr << "Error: Failed to write inode to disk file." << std::endl;
    }
    file.close();
}

INode DiskManager::readINode(uint32_t inodeIndex) {
    INode inode;
    char buffer[INODE_SIZE];

    // 计算 inode 在磁盘文件中的偏移量
    size_t offset = superBlock.inodeStartAddress + inodeIndex * INODE_SIZE;
    std::cout << "[DEBUG] readINode: inodeIndex=" << inodeIndex << ", offset=" << offset << std::endl;

    // 打开文件并读取 inode
    std::ifstream file(diskFile, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Error: Unable to open disk file for reading inode." << std::endl;
        return inode;
    }
    file.seekg(offset);
    if (file.fail()) {
        std::cerr << "Error: Failed to seek to position " << offset << " in disk file." << std::endl;
        file.close();
        return inode;
    }
    file.read(buffer, INODE_SIZE);
    if (file.gcount() != INODE_SIZE) {
        std::cerr << "Error: Failed to read complete inode from disk file." << std::endl;
        file.close();
        return inode;
    }
    file.close();

    inode.deserialize(buffer);
    return inode;
}

void DiskManager::allocateINodeAtIndex(uint32_t inodeIndex) {
    if (inodeIndex >= MAX_INODES) return;
    size_t byteIndex = inodeIndex / 8;
    size_t bitIndex = inodeIndex % 8;
    inodeBitmap[byteIndex] |= (1 << bitIndex); // 在位图中将对应位标记为已分配
}

void DiskManager::allocateBlockAtIndex(size_t blockIndex) {
    if (blockIndex >= totalBlocks) return;
    size_t byteIndex = blockIndex / 8;
    size_t bitIndex = blockIndex % 8;
    bitmap[byteIndex] |= (1 << bitIndex); // 在位图中将对应位标记为已分配
}

size_t DiskManager::calculateFreeBlocks() const {
    size_t freeBlocks = 0;
    // 遍历位图，统计未分配的块
    for (size_t i = 0; i < totalBlocks; ++i) {
        if (!isBlockAllocated(i)) {
            ++freeBlocks;
        }
    }
    return freeBlocks;
}

uint32_t DiskManager::calculateAllocatedInodes() const {
    uint32_t allocatedInodes = 0;
    // 遍历 inode 位图，统计已分配的 inode 数量
    for (uint32_t i = 0; i < MAX_INODES; ++i) {
        if (isINodeAllocated(i)) {
            ++allocatedInodes;
        }
    }
    return allocatedInodes;
}

