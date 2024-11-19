// src/DiskManager.cpp
#include "DiskManager.h"
#include "SuperBlock.h"
#include "INode.h"
#include <fstream>
#include <iostream>
#include <cstring>


DiskManager::DiskManager(const std::string& diskFile, size_t blockSize, size_t totalBlocks)
    : diskFile(diskFile), blockSize(blockSize), totalBlocks(totalBlocks) {
    // ����λͼ��С�����ֽ�Ϊ��λ����ÿ���ֽ�8λ
    bitmapSize = (totalBlocks + 7) / 8;
    bitmap.resize(bitmapSize, 0);

    // ��������ļ����ڣ�����λͼ�ͳ�����
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
    // ���ļ����������
    std::ofstream file(diskFile, std::ios::binary | std::ios::trunc);
    if (!file.is_open()) {
        std::cerr << "Error: Unable to create disk file." << std::endl;
        return;
    }

    // ��ʼ��������
    SuperBlock superBlock(static_cast<uint32_t>(totalBlocks), static_cast<uint32_t>(totalBlocks), 0, 0);

    // ��ʼ��λͼ�����п���У�
    bitmap.assign(bitmapSize, 0);

    // �����Ŀ¼��
    size_t rootBlockIndex = allocateBlock();
    if (rootBlockIndex == static_cast<size_t>(-1)) {
        std::cerr << "Error: Unable to allocate block for root directory." << std::endl;
        return;
    }

    // ���³�����
    superBlock.freeBlocks = static_cast<uint32_t>(totalBlocks - 1);
    superBlock.rootInode = static_cast<uint32_t>(rootBlockIndex);
    updateSuperBlock(superBlock);

    // ����λͼ
    updateBitmap();

    // ��ʼ�����̿飨��ѡ������ writeBlock �д���
    // ...

    // �����յĸ�Ŀ¼��д�����
    Directory rootDirectory;
    std::vector<char> buffer;
    rootDirectory.serialize(buffer, blockSize);
    writeBlock(rootBlockIndex, buffer.data());

    // ������Ϣ
    std::cout << "[DEBUG] Root directory created at block " << rootBlockIndex << "." << std::endl;

    // �ر��ļ�
    file.close();

    // ������Ϣ
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

    // ������ȷ���ļ�ƫ�ƣ�����SuperBlock��λͼ
    std::streampos offset = sizeof(SuperBlock) + bitmapSize + blockIndex * blockSize;
    file.seekg(offset);
    file.read(buffer, blockSize);
    file.close();
}


void DiskManager::writeBlock(size_t blockIndex, const char* data) {
    if (blockIndex >= totalBlocks) return;

    std::fstream file(diskFile, std::ios::binary | std::ios::in | std::ios::out);
    if (!file) return;

    // ������ȷ���ļ�ƫ�ƣ�����SuperBlock��λͼ
    std::streampos offset = sizeof(SuperBlock) + bitmapSize + blockIndex * blockSize;
    file.seekp(offset);
    file.write(data, blockSize);
    file.close();
}



size_t DiskManager::allocateBlock() {
    for (size_t i = 0; i < totalBlocks; ++i) {
        size_t byteIndex = i / 8;
        size_t bitIndex = i % 8;
        if ((bitmap[byteIndex] & (1 << bitIndex)) == 0) { // λΪ0����ʾ����
           bitmap[byteIndex] |= (1 << bitIndex); // ���Ϊ�ѷ���
            updateBitmap(); // ���´����е�λͼ

            // ���³������еĿ��п�����
            //SuperBlock superBlock = loadSuperBlock();
           // superBlock.freeBlocks -= 1;
           // updateSuperBlock(superBlock);

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
        updateBitmap(); // ���´����е�λͼ

        // ���³������еĿ��п�����
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
        std::cout << (bitmap[i] ? "0" : "1"); // 0��ʾ���У�1��ʾ�ѷ���
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

        // ������Ϣ
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
        // ������Ϣ
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

    // ����SuperBlock
    file.seekg(sizeof(SuperBlock), std::ios::beg);

    // ��ȡλͼ����
    file.read(reinterpret_cast<char*>(bitmap.data()), bitmapSize);

    file.close();
}



void DiskManager::updateBitmap() {
    std::fstream file(diskFile, std::ios::binary | std::ios::in | std::ios::out);
    if (!file.is_open()) {
        std::cerr << "Error: Unable to open disk file for updating bitmap." << std::endl;
        return;
    }

    // λͼ��SuperBlock֮��
    file.seekp(sizeof(SuperBlock), std::ios::beg);

    // д��λͼ����
    file.write(reinterpret_cast<const char*>(bitmap.data()), bitmapSize);

    file.close();
}

