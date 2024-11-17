// include/DiskManager.h
#ifndef DISKMANAGER_H
#define DISKMANAGER_H

#include <string>
#include <vector>
#include "SuperBlock.h"

class DiskManager {
public:
    std::string diskFile;
    std::vector<bool> bitmap;  // λͼ��������п�
    size_t blockSize;
    size_t totalBlocks;

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


    void updateSuperBlock(const SuperBlock& superBlock);

    SuperBlock loadSuperBlock();
};

#endif // DISKMANAGER_H