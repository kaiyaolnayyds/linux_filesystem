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

    // �������ļ��Ƿ����
    std::ifstream fileCheck(diskFile, std::ios::binary);
    if (fileCheck.good()) {
        // ���س������λͼ
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
    bitmapSize = (totalBlocks + 7) / 8;

    // inodeStartAddress Ӧ���ǳ������С����λͼ��С
    superBlock.inodeStartAddress = static_cast<uint32_t>(superBlockSize + bitmapSize);
    std::cout << "[DEBUG] inodeStartAddress: " << superBlock.inodeStartAddress << std::endl;

    // ��ʼ��λͼ�����п���У�
    bitmap.assign(bitmapSize, 0);

    // Ϊ��Ŀ¼�����ݷ���һ����
    size_t rootBlockIndex = allocateBlock(); // �⽫����λͼ
    if (rootBlockIndex == static_cast<size_t>(-1)) {
        std::cerr << "Error: Unable to allocate block for root directory." << std::endl;
        return;
    }

    // ������Ŀ¼�� inode
    uint32_t rootInodeIndex = superBlock.inodeCount++;
    INode rootInode;
    rootInode.size = 0;
    rootInode.mode = 0755; // Ŀ¼��Ĭ��Ȩ��
    rootInode.type = 1;    // Ŀ¼
    rootInode.blockIndex = static_cast<uint32_t>(rootBlockIndex);
    rootInode.inodeIndex = rootInodeIndex;

    // ���³�����
    superBlock.freeBlocks = static_cast<uint32_t>(totalBlocks - 1);
    superBlock.rootInode = rootInodeIndex;

    // ��������д�����
    updateSuperBlock(superBlock);

    // ��λͼд�����
    updateBitmap();

    // ����Ŀ¼�� inode д�����
    writeINode(rootInodeIndex, rootInode);

    // �����յĸ�Ŀ¼��д�����
    Directory rootDirectory;
    std::vector<char> buffer;
    rootDirectory.serialize(buffer, blockSize);
    writeBlock(rootBlockIndex, buffer.data());

    // ����Ҫ�ر��ļ�����Ϊ�����ں�����ͷ�Ѿ��ر���
}




void DiskManager::readBlock(size_t blockIndex, char* buffer) {
    if (blockIndex >= totalBlocks) return;

    std::ifstream file(diskFile, std::ios::binary);
    if (!file) return;

    // ������ȷ���ļ�ƫ�ƣ�����SuperBlock��λͼ
    //std::streampos offset = sizeof(SuperBlock) + bitmapSize + blockIndex * blockSize;
    std::streampos offset = superBlock.inodeStartAddress + superBlock.inodeCount * INODE_SIZE + blockIndex * blockSize;
    file.seekg(offset);
    file.read(buffer, blockSize);
    file.close();
}


void DiskManager::writeBlock(size_t blockIndex, const char* data) {
    if (blockIndex >= totalBlocks) return;

    std::fstream file(diskFile, std::ios::binary | std::ios::in | std::ios::out);
    if (!file) return;

    // ������ȷ���ļ�ƫ�ƣ�����SuperBlock��λͼ
    //std::streampos offset = sizeof(SuperBlock) + bitmapSize + blockIndex * blockSize;
    std::streampos offset = superBlock.inodeStartAddress + superBlock.inodeCount * INODE_SIZE + blockIndex * blockSize;
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

    superBlock = sb; // ���³�Ա����
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
    superBlock = sb; // ���³�Ա����
    return sb;
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

// ���� INode �����л���С
//constexpr size_t INODE_SIZE = sizeof(uint32_t) * 4 + sizeof(uint16_t) + sizeof(uint8_t);


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

