// include/INode.h
#ifndef INODE_H
#define INODE_H

#include <vector>
#include <cstdint>
#include <cstddef> // ������ͷ�ļ��Զ���size_t

class INode {
public:
    uint32_t size;                  // �ļ���С
    uint32_t type;                  // �ļ����ͣ�0: �ļ�, 1: Ŀ¼��
    uint32_t permissions;           // Ȩ��
    std::vector<uint32_t> blocks;   // ���ݿ�ָ��

    // ���캯��
    INode(uint32_t size = 0, uint32_t type = 0, uint32_t permissions = 0);

    // ������ݿ�
    void addBlock(uint32_t blockIndex);

    // ��ȡָ���������Ŀ�
    uint32_t getBlock(size_t index) const;

    // ��ȡ��ǰ�����Ŀ�����
    size_t getBlockCount() const;
};

#endif // INODE_H
