// include/DiskManager.h
#ifndef DISKMANAGER_H
#define DISKMANAGER_H

#include <string>
#include <vector>
#include <cstdint>
#include "Directory.h"
#include "SuperBlock.h"
#include "INode.h"

constexpr size_t SUPERBLOCK_SIZE = sizeof(uint32_t) * 5; // SuperBlock �Ĺ̶���С
//constexpr size_t INODE_SIZE = sizeof(uint32_t) * 5 + sizeof(uint16_t); // ӦΪ22�ֽ�
#define MAX_INODES 1024 // ������Ҫ����


class DiskManager {
public:
    std::string diskFile;
    std::vector<uint8_t> bitmap;    // λͼ��ʹ��uint8_t�洢
    size_t bitmapSize;        // λͼ��С���ֽ�����
    size_t blockSize;    //���С
    size_t totalBlocks;  //������
    SuperBlock superBlock;  // �������Ա����
   
    // **��� `dataBlocksStartAddress` ��Ա����**
    size_t dataBlocksStartAddress;  // ���ݿ��ڴ����ļ��е���ʼ��ַ

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

    // ���ڵ��ԣ������ǰλͼ״̬
    void printBitmap() const;

    //����λͼ
    void loadBitmap();

    //����λͼ
    void updateBitmap();

    //���³�����
    void updateSuperBlock(const SuperBlock& superBlock);

    //���س�����
    SuperBlock loadSuperBlock();

    //д��INode
    void writeINode(uint32_t inodeIndex, const INode& inode);

    //��ȡINode
    INode readINode(uint32_t inodeIndex);
};

#endif // DISKMANAGER_H