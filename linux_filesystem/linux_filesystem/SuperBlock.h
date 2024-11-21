// include/SuperBlock.h
#ifndef SUPERBLOCK_H
#define SUPERBLOCK_H

#include <cstdint>
#include <string>
#include <cstring>

class SuperBlock {
public:
    uint32_t totalBlocks;
    uint32_t freeBlocks;
    uint32_t inodeStartAddress;
    uint32_t rootInode;
    uint32_t inodeCount; // һ��Ҫ��������ֶ�

    // ���캯��
    SuperBlock();
    SuperBlock(uint32_t totalBlocks, uint32_t freeBlocks, uint32_t inodeCount, uint32_t rootInode);

    // �����������ݱ��浽�ļ�
    void saveToFile(const std::string& diskFile);

    // ���ļ����س���������
    void loadFromFile(const std::string& diskFile);

    void updateFreeBlocks(int change); // ���ӻ���ٿ��п���
    void updateInodeCount(int change); // ���ӻ����i�ڵ�����

    // ���л��ͷ����л�����
    void serialize(char* buffer) const;
    void deserialize(const char* buffer);

};

#endif // SUPERBLOCK_H
