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
    // ��ʼ�����ݿ�λͼ��С������ռ�
    bitmapSize = (totalBlocks + 7) / 8;
    bitmap.resize(bitmapSize, 0);

    // ��ʼ�� inode λͼ��С������ռ�
    inodeBitmapSize = (MAX_INODES + 7) / 8;
    inodeBitmap.resize(inodeBitmapSize, 0);

    // �������ļ��Ƿ����
    std::ifstream fileCheck(diskFile, std::ios::binary);
    if (fileCheck.good()) {
        // ���س������λͼ
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
    // �򿪴����ļ�����д�룬ʹ�� trunc ģʽ����ļ�
    std::ofstream file(diskFile, std::ios::binary | std::ios::out | std::ios::trunc);
    if (!file.is_open()) {
        std::cerr << "Error: Unable to create disk file." << std::endl;
        return;
    }

    // **Ԥ��������ļ���С**
    size_t diskFileSize = totalBlocks * blockSize;
    file.seekp(diskFileSize - 1);
    file.write("", 1); // д��һ���ֽڣ���ȷ���ļ���С
    file.close();

    // ��ʼ��������
    superBlock = SuperBlock(static_cast<uint32_t>(totalBlocks), static_cast<uint32_t>(totalBlocks), 0, 0);

    // ���㳬�����λͼ�Ĵ�С
    size_t superBlockSize = SUPERBLOCK_SIZE; // ʹ�ù̶���С

    // ��ʼ�� inode λͼ
    inodeBitmapSize = (MAX_INODES + 7) / 8;
    inodeBitmap.resize(inodeBitmapSize, 0);

    // ��ʼ�����ݿ�λͼ
    bitmapSize = (totalBlocks + 7) / 8;
    bitmap.resize(bitmapSize, 0);

    // inodeStartAddress �ǳ������С���� inode λͼ�����ݿ�λͼ��С
    superBlock.inodeStartAddress = static_cast<uint32_t>(superBlockSize + inodeBitmapSize + bitmapSize);
    std::cout << "[DEBUG] inodeStartAddress: " << superBlock.inodeStartAddress << std::endl;

    // �������ݿ����ʼ��ַ�����ݿ����Inode����
    superBlock.dataBlockStartAddress = superBlock.inodeStartAddress + MAX_INODES * INODE_SIZE;

    // ��������д�����
    updateSuperBlock(superBlock);

    // ����λͼ
    updateBitmaps();

    // Ϊ��Ŀ¼�����ݷ���һ����
    size_t rootBlockIndex = allocateBlock(); // ����λͼ
    if (rootBlockIndex == static_cast<size_t>(-1)) {
        std::cerr << "Error: Unable to allocate block for root directory." << std::endl;
        return;
    }

    // ����һ���µ� inode
    uint32_t rootInodeIndex = allocateINode();
    if (rootInodeIndex == static_cast<uint32_t>(-1)) {
        std::cerr << "Error: Unable to allocate inode for root directory." << std::endl;
        return;
    }

    // ������Ŀ¼�� inode
    INode rootInode;
    rootInode.size = 0;
    rootInode.mode = 0755; // Ŀ¼��Ĭ��Ȩ��
    rootInode.type = 1;    // Ŀ¼
    rootInode.blockIndex = static_cast<uint32_t>(rootBlockIndex);
    rootInode.inodeIndex = rootInodeIndex;

    // ���³�����
    superBlock.freeBlocks--;
    superBlock.rootInode = rootInodeIndex;
    updateSuperBlock(superBlock);

    // ����Ŀ¼�� inode д�����
    writeINode(rootInodeIndex, rootInode);

    // �����յĸ�Ŀ¼��д�����
    Directory rootDirectory;
    rootDirectory.entries["."] = rootInodeIndex;  // ��Ŀ¼��ǰĿ¼
    rootDirectory.entries[".."] = rootInodeIndex; // ��Ŀ¼�ĸ�Ŀ¼���Լ�
    std::vector<char> buffer;
    rootDirectory.serialize(buffer, blockSize);
    writeBlock(rootBlockIndex, buffer.data());

}

void DiskManager::writeBlock(size_t blockIndex, const char* data) {
    if (blockIndex >= totalBlocks) return;

    std::fstream file(diskFile, std::ios::binary | std::ios::in | std::ios::out);
    if (!file) return;

    // ���ݿ�ƫ������ʹ�ù̶������ݿ���ʼ��ַ�������ݿ���ʼλ�����������Ѿ�ʹ�õ������ݿ�λ��
    std::streampos offset = superBlock.dataBlockStartAddress + blockIndex * blockSize;
    file.seekp(offset);
    file.write(data, blockSize);
    file.close();
}

void DiskManager::readBlock(size_t blockIndex, char* buffer) {
    if (blockIndex >= totalBlocks) return;

    std::ifstream file(diskFile, std::ios::binary);
    if (!file) return;

    // ���ݿ�ƫ������ʹ�ù̶������ݿ���ʼ��ַ�������ݿ���ʼλ�����������Ѿ�ʹ�õ������ݿ�λ��
    std::streampos offset = superBlock.dataBlockStartAddress + blockIndex * blockSize;
    file.seekg(offset);
    file.read(buffer, blockSize);
    file.close();
}

size_t DiskManager::allocateBlock() {
    for (size_t i = 0; i < totalBlocks; ++i) {
        size_t byteIndex = i / 8;
        size_t bitIndex = i % 8;
        if ((bitmap[byteIndex] & (1 << bitIndex)) == 0) { // λΪ0����ʾ����
            bitmap[byteIndex] |= (1 << bitIndex); // ���Ϊ�ѷ���
            updateBitmaps(); // ���´����е�λͼ

            // ���³������еĿ��п�����
            superBlock.freeBlocks--;
            updateSuperBlock(superBlock);

            return i;
        }
    }
    return static_cast<size_t>(-1); // �޿��ÿ�
}

void DiskManager::freeBlock(size_t blockIndex) {
    if (blockIndex < totalBlocks) {
        size_t byteIndex = blockIndex / 8;
        size_t bitIndex = blockIndex % 8;
        bitmap[byteIndex] &= ~(1 << bitIndex); // ���Ϊ����
        updateBitmaps(); // ���´����е�λͼ

        // ���³������еĿ��п�����
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
        if ((inodeBitmap[byteIndex] & (1 << bitIndex)) == 0) { // λΪ0����ʾ����
            inodeBitmap[byteIndex] |= (1 << bitIndex); // ���Ϊ�ѷ���
            updateBitmaps(); // ���´����е�λͼ

            // ���³������е� inode ����
            superBlock.inodeCount++;
            updateSuperBlock(superBlock);

            return i;
        }
    }
    return static_cast<uint32_t>(-1); // �޿��� inode
}

void DiskManager::freeINode(uint32_t inodeIndex) {
    if (inodeIndex >= MAX_INODES) return;

    size_t byteIndex = inodeIndex / 8;
    size_t bitIndex = inodeIndex % 8;
    inodeBitmap[byteIndex] &= ~(1 << bitIndex); // ���Ϊ����
    updateBitmaps(); // ���´����е�λͼ

    // ���³������е� inode ����
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

    superBlock = sb; // ���³�Ա����
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
    superBlock = sb; // ���³�Ա����
    return sb;
}

void DiskManager::loadBitmaps() {
    std::ifstream file(diskFile, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Error: Unable to open disk file for reading bitmaps." << std::endl;
        return;
    }

    // ����������
    file.seekg(SUPERBLOCK_SIZE, std::ios::beg);

    // ��ȡ inode λͼ
    inodeBitmap.resize(inodeBitmapSize);
    file.read(reinterpret_cast<char*>(inodeBitmap.data()), inodeBitmapSize);

    // ��ȡ���ݿ�λͼ
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

    // λͼ�ڳ�����֮��
    file.seekp(SUPERBLOCK_SIZE, std::ios::beg);

    // д�� inode λͼ
    file.write(reinterpret_cast<const char*>(inodeBitmap.data()), inodeBitmapSize);

    // д�����ݿ�λͼ
    file.write(reinterpret_cast<const char*>(bitmap.data()), bitmapSize);

    file.close();
}

void DiskManager::writeINode(uint32_t inodeIndex, const INode& inode) {
    char buffer[INODE_SIZE];
    inode.serialize(buffer);

    // ���� inode �ڴ����ļ��е�ƫ����
    size_t offset = superBlock.inodeStartAddress + inodeIndex * INODE_SIZE;
    std::cout << "[DEBUG] writeINode: inodeIndex=" << inodeIndex << ", offset=" << offset << std::endl;

    // ���ļ���д�� inode
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

    // ���� inode �ڴ����ļ��е�ƫ����
    size_t offset = superBlock.inodeStartAddress + inodeIndex * INODE_SIZE;
    std::cout << "[DEBUG] readINode: inodeIndex=" << inodeIndex << ", offset=" << offset << std::endl;

    // ���ļ�����ȡ inode
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
    inodeBitmap[byteIndex] |= (1 << bitIndex); // ��λͼ�н���Ӧλ���Ϊ�ѷ���
}

void DiskManager::allocateBlockAtIndex(size_t blockIndex) {
    if (blockIndex >= totalBlocks) return;
    size_t byteIndex = blockIndex / 8;
    size_t bitIndex = blockIndex % 8;
    bitmap[byteIndex] |= (1 << bitIndex); // ��λͼ�н���Ӧλ���Ϊ�ѷ���
}

size_t DiskManager::calculateFreeBlocks() const {
    size_t freeBlocks = 0;
    // ����λͼ��ͳ��δ����Ŀ�
    for (size_t i = 0; i < totalBlocks; ++i) {
        if (!isBlockAllocated(i)) {
            ++freeBlocks;
        }
    }
    return freeBlocks;
}

uint32_t DiskManager::calculateAllocatedInodes() const {
    uint32_t allocatedInodes = 0;
    // ���� inode λͼ��ͳ���ѷ���� inode ����
    for (uint32_t i = 0; i < MAX_INODES; ++i) {
        if (isINodeAllocated(i)) {
            ++allocatedInodes;
        }
    }
    return allocatedInodes;
}

