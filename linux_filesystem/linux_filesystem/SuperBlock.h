// include/SuperBlock.h
#ifndef SUPERBLOCK_H
#define SUPERBLOCK_H

#include <cstdint>
#include <string>

class SuperBlock {
public:
    uint32_t totalBlocks;
    uint32_t freeBlocks;
    uint32_t inodeCount;
    uint32_t rootInode;

    // ���캯��
    SuperBlock(uint32_t total = 0, uint32_t free = 0, uint32_t inodes = 0, uint32_t root = 0);

    // �����������ݱ��浽�ļ�
    void saveToFile(const std::string& diskFile);

    // ���ļ����س���������
    void loadFromFile(const std::string& diskFile);

    void updateFreeBlocks(int change); // ���ӻ���ٿ��п���
    void updateInodeCount(int change); // ���ӻ����i�ڵ�����

};

#endif // SUPERBLOCK_H
