// include/SuperBlock.h
/*
superblockʵ��
��Ҫ��¼block��Inode��������δʹ��/��ʹ�õ�inode/block����

*/


#ifndef SUPERBLOCK_H
#define SUPERBLOCK_H
#include <cstdint>
#include <string>
#include <cstring>

constexpr size_t SUPERBLOCK_SIZE = sizeof(uint32_t) * 7; // SuperBlock �Ĺ̶���С,ӦΪ28�ֽ�

class SuperBlock {
public:
    uint32_t totalBlocks;       //������
    uint32_t freeBlocks;        //ʣ�����
    uint32_t inodeCount;        //Inode��
    uint32_t rootInode;          //��ʼInode����
    uint32_t rootDataBlock;      //��ʼ���ݿ�����
    uint32_t inodeStartAddress;  // inode �����ڴ����ϵ���ʼ��ַ
    uint32_t dataBlockStartAddress; // ���ݿ������ڴ����ϵ���ʼ��ַ

    // ���캯��
    SuperBlock();
    SuperBlock(uint32_t totalBlocks, uint32_t freeBlocks, uint32_t inodeCount, uint32_t rootInode, uint32_t rootDataBlock);

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