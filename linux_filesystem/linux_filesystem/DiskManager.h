// include/DiskManager.h
#ifndef DISKMANAGER_H
#define DISKMANAGER_H

#include <string>
#include <vector>
#include <cstdint>
#include "SuperBlock.h"
#include "Directory.h"

class DiskManager {
public:
    std::string diskFile;
    std::vector<uint8_t> bitmap;    // λͼ��ʹ��uint8_t�洢
    size_t bitmapSize;        // λͼ��С���ֽ�����
    size_t blockSize;    //���С
    size_t totalBlocks;  //������

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
};

#endif // DISKMANAGER_H