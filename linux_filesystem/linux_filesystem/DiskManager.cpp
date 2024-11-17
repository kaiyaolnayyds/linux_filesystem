// src/DiskManager.cpp
#include "DiskManager.h"
#include "SuperBlock.h"
#include "INode.h"
#include <fstream>
#include <iostream>

DiskManager::DiskManager(const std::string& diskFile, size_t blockSize, size_t totalBlocks)
    : diskFile(diskFile), blockSize(blockSize), totalBlocks(totalBlocks) {
    bitmap.resize(totalBlocks, true); // �����ʼʱ���п鶼�ǿ��е�
}

void DiskManager::initialize() {
    std::ofstream file(diskFile, std::ios::binary | std::ios::out | std::ios::in);
    if (!file) return;

    file.seekp(totalBlocks * blockSize - 1);
    file.write("", 1); // ��չ�ļ���ָ����С

    SuperBlock superBlock(totalBlocks, totalBlocks, 0, 0); // ʾ��������
    file.seekp(0);
    file.write(reinterpret_cast<char*>(&superBlock), sizeof(SuperBlock));

    file.close();
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
    for (size_t i = 0; i < totalBlocks; ++i) {
        if (bitmap[i]) {
            bitmap[i] = false;
            return i;
        }
    }
    return -1;
}

void DiskManager::freeBlock(size_t blockIndex) {
    if (blockIndex < totalBlocks) {
        bitmap[blockIndex] = true;
    }
}
