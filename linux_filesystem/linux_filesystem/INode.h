// include/INode.h
#ifndef INODE_H
#define INODE_H

#include <vector>
#include <cstdint>
#include <cstddef> // 添加这个头文件以定义size_t

class INode {
public:
    uint32_t size;                  // 文件大小
    uint32_t type;                  // 文件类型（0: 文件, 1: 目录）
    uint32_t permissions;           // 权限
    std::vector<uint32_t> blocks;   // 数据块指针

    // 构造函数
    INode(uint32_t size = 0, uint32_t type = 0, uint32_t permissions = 0);

    // 添加数据块
    void addBlock(uint32_t blockIndex);

    // 获取指定索引处的块
    uint32_t getBlock(size_t index) const;

    // 获取当前关联的块数量
    size_t getBlockCount() const;
};

#endif // INODE_H
