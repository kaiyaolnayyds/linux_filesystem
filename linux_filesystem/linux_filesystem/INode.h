// include/INode.h
/*
Inode实现
作为结构体存储文件信息，通过Inode访问到数据块，并由Inode实现权限控制等操作
*/
#ifndef INODE_H
#define INODE_H

#include <vector>
#include <cstdint>
#include <cstddef> 
#include <vector>
#include <cstring>

class DiskManager; // 前向声明

#pragma pack(push, 1)
class INode {
public:
    uint32_t size;                  // 文件大小
    uint32_t blockIndex;         // 物理地址（数据块索引）
    uint32_t inodeIndex;         // INode 的索引（用于唯一标识）
    uint16_t mode;                 // 保护码（权限）
    uint32_t type;                  // 文件类型（0: 文件, 1: 目录）
    uint32_t permissions;           // 权限
    std::vector<uint32_t> blocks;   // 数据块指针

    // 其他需要的元数据，例如时间戳等

    // 构造函数
    INode(uint32_t size = 0, uint32_t type = 0, uint32_t permissions = 0);

    // 添加数据块
    void addBlock(uint32_t blockIndex);

    // 获取指定索引处的块
    uint32_t getBlock(size_t index) const;

    // 获取当前关联的块数量
    size_t getBlockCount() const;

    // 序列化和反序列化方法
    void serialize(char* buffer) const;
    void deserialize(const char* buffer);
};
#pragma pack(pop)

#endif // INODE_H