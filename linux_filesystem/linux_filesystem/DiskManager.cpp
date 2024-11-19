// src/DiskManager.cpp
#include "DiskManager.h"
#include "SuperBlock.h"
#include "INode.h"
#include <fstream>
#include <iostream>
#include <cstring>
#include  "SuperBlock.h"

DiskManager::DiskManager(const std::string& diskFile, size_t blockSize, size_t totalBlocks)
    : diskFile(diskFile), blockSize(blockSize), totalBlocks(totalBlocks) {
    bitmapSize = (totalBlocks + 7) / 8;
    bitmap.resize(bitmapSize, 0);

    // 检查磁盘文件是否存在
    std::ifstream fileCheck(diskFile, std::ios::binary);
    if (fileCheck.good()) {
        // 加载超级块和位图
        superBlock = loadSuperBlock();
        loadBitmap();
        std::cout << "[DEBUG] Disk file exists. Loaded SuperBlock and bitmap." << std::endl;
    }
    else {
        std::cout << "[DEBUG] Disk file does not exist. Need to initialize file system." << std::endl;
    }
    fileCheck.close();
}


void DiskManager::initialize() {
    // 仅在磁盘文件不存在时创建并初始化
    std::ifstream diskFileCheck(diskFile, std::ios::binary);
    if (diskFileCheck.good()) {
        // 磁盘文件已存在，不需要重新初始化
        diskFileCheck.close();
        return;
    }
    diskFileCheck.close();

    // 打开磁盘文件用于写入
    std::ofstream file(diskFile, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Error: Unable to create disk file." << std::endl;
        return;
    }

    // 初始化超级块
    superBlock = SuperBlock(static_cast<uint32_t>(totalBlocks), static_cast<uint32_t>(totalBlocks), 0, 0);

    // 计算超级块和位图的大小
    size_t superBlockSize = sizeof(SuperBlock);
    bitmapSize = (totalBlocks + 7) / 8;

    // 在超级块中设置 inodeStartAddress
    superBlock.inodeStartAddress = static_cast<uint32_t>(superBlockSize + bitmapSize);

    // 初始化位图（所有块空闲）
    bitmap.assign(bitmapSize, 0);

    // 为根目录的数据分配一个块
    size_t rootBlockIndex = allocateBlock(); // 这将更新位图
    if (rootBlockIndex == static_cast<size_t>(-1)) {
        std::cerr << "Error: Unable to allocate block for root directory." << std::endl;
        return;
    }

    // 创建根目录的 inode
    uint32_t rootInodeIndex = superBlock.inodeCount++;
    INode rootInode;
    rootInode.size = 0;
    rootInode.mode = 0755; // 目录的默认权限
    rootInode.type = 1;    // 目录
    rootInode.blockIndex = static_cast<uint32_t>(rootBlockIndex);
    rootInode.inodeIndex = rootInodeIndex;

    // 更新超级块
    superBlock.freeBlocks = static_cast<uint32_t>(totalBlocks - 1);
    superBlock.rootInode = rootInodeIndex;

    // 将超级块写入磁盘
    updateSuperBlock(superBlock);

    // 将位图写入磁盘
    updateBitmap();

    // 将根目录的 inode 写入磁盘
    writeINode(rootInodeIndex, rootInode);

    // 创建空的根目录并写入磁盘
    Directory rootDirectory;
    std::vector<char> buffer;
    rootDirectory.serialize(buffer, blockSize);
    writeBlock(rootBlockIndex, buffer.data());

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

void DiskManager::updateSuperBlock(const SuperBlock& sb) {
    std::ofstream file(diskFile, std::ios::binary | std::ios::in | std::ios::out);
    if (!file.is_open()) {
        std::cerr << "Error: Unable to open disk file for writing superblock." << std::endl;
        return;
    }
    char buffer[sizeof(SuperBlock)];
    sb.serialize(buffer);
    file.seekp(0);
    file.write(buffer, sizeof(SuperBlock));
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
    char buffer[sizeof(SuperBlock)];
    file.read(buffer, sizeof(SuperBlock));
    file.close();

    sb.deserialize(buffer);
    superBlock = sb; // 更新成员变量
    return sb;
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

// 定义 INode 的序列化大小
constexpr size_t INODE_SIZE = sizeof(uint32_t) + sizeof(uint16_t) + sizeof(uint8_t) + sizeof(uint32_t) + sizeof(uint32_t); // 总计15字节

void DiskManager::writeINode(uint32_t inodeIndex, const INode& inode) {
    char buffer[INODE_SIZE];
    inode.serialize(buffer);

    // 计算 inode 在磁盘文件中的偏移量
    size_t offset = superBlock.inodeStartAddress + inodeIndex * INODE_SIZE;

    // 打开文件并写入 inode
    std::fstream file(diskFile, std::ios::binary | std::ios::in | std::ios::out);
    if (!file.is_open()) {
        std::cerr << "Error: Unable to open disk file for writing inode." << std::endl;
        return;
    }
    file.seekp(offset);
    file.write(buffer, INODE_SIZE);
    file.close();
}

INode DiskManager::readINode(uint32_t inodeIndex) {
    INode inode;
    char buffer[INODE_SIZE];

    // 计算 inode 在磁盘文件中的偏移量
    size_t offset = superBlock.inodeStartAddress + inodeIndex * INODE_SIZE;

    // 打开文件并读取 inode
    std::ifstream file(diskFile, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Error: Unable to open disk file for reading inode." << std::endl;
        return inode;
    }
    file.seekg(offset);
    file.read(buffer, INODE_SIZE);
    file.close();

    inode.deserialize(buffer);
    return inode;
}