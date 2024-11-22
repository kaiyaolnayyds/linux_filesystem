// include/INode.h
/*
Inodeʵ��
��Ϊ�ṹ��洢�ļ���Ϣ��ͨ��Inode���ʵ����ݿ飬����Inodeʵ��Ȩ�޿��ƵȲ���
*/
#ifndef INODE_H
#define INODE_H

#include <vector>
#include <cstdint>
#include <cstddef> 
#include <vector>
#include <cstring>

class DiskManager; // ǰ������

#pragma pack(push, 1)
class INode {
public:
    uint32_t size;                  // �ļ���С
    uint32_t blockIndex;         // �����ַ�����ݿ�������
    uint32_t inodeIndex;         // INode ������������Ψһ��ʶ��
    uint16_t mode;                 // �����루Ȩ�ޣ�
    uint32_t type;                  // �ļ����ͣ�0: �ļ�, 1: Ŀ¼��
    uint32_t permissions;           // Ȩ��
    std::vector<uint32_t> blocks;   // ���ݿ�ָ��

    // ������Ҫ��Ԫ���ݣ�����ʱ�����

    // ���캯��
    INode(uint32_t size = 0, uint32_t type = 0, uint32_t permissions = 0);

    // ������ݿ�
    void addBlock(uint32_t blockIndex);

    // ��ȡָ���������Ŀ�
    uint32_t getBlock(size_t index) const;

    // ��ȡ��ǰ�����Ŀ�����
    size_t getBlockCount() const;

    // ���л��ͷ����л�����
    void serialize(char* buffer) const;
    void deserialize(const char* buffer);
};
#pragma pack(pop)

#endif // INODE_H