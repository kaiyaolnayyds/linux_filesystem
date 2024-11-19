// include/CommandHandler.h
#ifndef COMMANDHANDLER_H
#define COMMANDHANDLER_H

#include <string>
#include "DiskManager.h"
#include <fstream>
#include "Directory.h"

class CommandHandler {
private:
    DiskManager& diskManager; // 磁盘管理器
    Directory currentDirectory; // 当前目录对象

public:
    // 构造函数
   CommandHandler(DiskManager& dm) : diskManager(dm) {
    // 加载根目录
    SuperBlock superBlock = diskManager.loadSuperBlock();
    uint32_t rootInode = superBlock.rootInode;

    // 从磁盘读取根目录数据
    char buffer[diskManager.blockSize];
    diskManager.readBlock(rootInode, buffer);

    // 反序列化根目录
    currentDirectory.deserialize(buffer, diskManager.blockSize);
}


    // 处理命令的方法
    void handleCommand(const std::string& command);

    // 各个命令的具体处理方法
    void handleInfo();
    void handleCd(const std::string& path);
    void handleDir(const std::string& path);
    void handleMd(const std::string& dirName);
    void handleRd(const std::string& dirName);
    void handleNewFile(const std::string& fileName);
    void handleCat(const std::string& fileName);
    void handleCopy(const std::string& src, const std::string& dest);
    void handleDel(const std::string& fileName);
    void handleCheck();

    // 工具方法：解析路径
    std::vector<std::string> parsePath(const std::string& path);
};

#endif // COMMANDHANDLER_H
