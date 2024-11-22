// include/DiskManager.h

#ifndef DISKMANAGER_H
#define DISKMANAGER_H

#include <string>
#include <vector>
#include <cstdint>
#include "Directory.h"
#include "SuperBlock.h"
#include "INode.h"

constexpr size_t SUPERBLOCK_SIZE = sizeof(uint32_t) * 6; // SuperBlock �Ĺ̶���С
constexpr size_t INODE_SIZE = sizeof(uint32_t) * 5 + sizeof(uint16_t); // ӦΪ22�ֽ�
constexpr uint32_t MAX_INODES = 1024; // ������Ҫ����

class DiskManager {
public:
    std::string diskFile;
    std::vector<uint8_t> bitmap;         // ���ݿ�λͼ
    size_t bitmapSize;                   // ���ݿ�λͼ��С���ֽ�����
    std::vector<uint8_t> inodeBitmap;    // inode λͼ
    size_t inodeBitmapSize;              // inode λͼ��С���ֽ�����
    size_t blockSize;                    // ���С
    size_t totalBlocks;                  // ������
    SuperBlock superBlock;               // �������Ա����

    DiskManager(const std::string& diskFile, size_t blockSize, size_t totalBlocks);

    void initialize();

    void readBlock(size_t blockIndex, char* buffer);

    void writeBlock(size_t blockIndex, const char* data);

    // ������п飬���ؿ�����
    size_t allocateBlock();

    // �ͷ�ָ����
    void freeBlock(size_t blockIndex);

    // �����Ƿ��ѷ���
    bool isBlockAllocated(size_t blockIndex) const;

    // ������� inode������ inode ����
    uint32_t allocateINode();

    // �ͷ�ָ�� inode
    void freeINode(uint32_t inodeIndex);

    // ��� inode �Ƿ��ѷ���
    bool isINodeAllocated(uint32_t inodeIndex) const;

    // ���ڵ��ԣ������ǰλͼ״̬
    void printBitmap() const;

    // ����λͼ
    void loadBitmaps();

    // ����λͼ
    void updateBitmaps();

    // ���³�����
    void updateSuperBlock(const SuperBlock& superBlock);

    // ���س�����
    SuperBlock loadSuperBlock();

    // д�� inode
    void writeINode(uint32_t inodeIndex, const INode& inode);

    // ��ȡ inode
    INode readINode(uint32_t inodeIndex);
};

#endif // DISKMANAGER_H
