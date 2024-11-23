// include/DiskManager.h

#ifndef DISKMANAGER_H
#define DISKMANAGER_H

#include <string>
#include <vector>
#include <cstdint>
#include "Directory.h"
#include "SuperBlock.h"
#include "INode.h"


constexpr size_t INODE_SIZE = sizeof(uint32_t) * 5 + sizeof(uint16_t)+sizeof(int); // Inode��С
constexpr uint32_t MAX_INODES = 1024; // ���Inode����������Ҫ����

class DiskManager {
public:
    std::string diskFile;                // �����ļ���
    std::vector<uint8_t> bitmap;         // ���ݿ�λͼ
    size_t bitmapSize;                   // ���ݿ�λͼ��С���ֽ�����
    std::vector<uint8_t> inodeBitmap;    // inode λͼ
    size_t inodeBitmapSize;              // inode λͼ��С���ֽ�����
    size_t blockSize;                    // ���С
    size_t totalBlocks;                  // ������
    SuperBlock superBlock;               // �������Ա����

    DiskManager(const std::string& diskFile, size_t blockSize, size_t totalBlocks);

    //���̶����ʼ������
    void initialize();

    //��ȡ���ݿ�
    void readBlock(size_t blockIndex, char* buffer);

    //д�����ݿ�
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

    //���¼��㲢���³������е� freeBlocks �� inodeCount
    void updateSuperBlockUsage();

    // ���س�����
    SuperBlock loadSuperBlock();

    // д�� inode
    void writeINode(uint32_t inodeIndex, const INode& inode);

    // ��ȡ inode
    INode readINode(uint32_t inodeIndex);

    //��ָ������������һ�� inode�������޸��ļ�ϵͳ��һ�¡�
    void allocateINodeAtIndex(uint32_t inodeIndex);

    //��ָ������������һ�����ݿ飬�����޸��ļ�ϵͳ��һ�¡�
    void allocateBlockAtIndex(size_t blockIndex);

    //���㵱ǰ�ļ�ϵͳ�п��еĿ�������
    size_t calculateFreeBlocks() const;

    //���㵱ǰ�ļ�ϵͳ���ѷ���� inode ������
    uint32_t calculateAllocatedInodes() const;

};

#endif // DISKMANAGER_H
