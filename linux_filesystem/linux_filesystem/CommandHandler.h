// include/CommandHandler.h
#ifndef COMMANDHANDLER_H
#define COMMANDHANDLER_H

#include <string>
#include "DiskManager.h"
#include <fstream>
#include "Directory.h"
#include "SuperBlock.h"  
#include "INode.h"       // 包含 INode.h

class CommandHandler {
public:
    DiskManager& diskManager; // 磁盘管理器
    Directory currentDirectory; // 当前目录对象
    uint32_t currentInodeIndex; // 当前目录的 inodeIndex
    std::string currentPath;    // 当前路径

    // 构造函数
    CommandHandler(DiskManager& dm);

    // 处理命令的方法
    void handleCommand(const std::string& command);

    // 各个命令的具体处理方法
    void handleInfo();
    void handleCd(const std::string& path);
    void handleDir(const std::string& path = "", bool recursive = false);
    void handleMd(const std::string& dirName);
    void handleRd(const std::string& dirName);
    void handleNewFile(const std::string& fileName);
    void handleCat(const std::string& fileName);
    void handleCopy(const std::string& src, const std::string& dest);
    void handleDel(const std::string& fileName);
    void handleCheck();

    // 工具方法：解析路径
    std::vector<std::string> parsePath(const std::string& path);

    void updatePrompt();

    void displayDirectoryContents(const Directory& dir, uint32_t dirInodeIndex, bool recursive, const std::string& indent);

    Directory loadDirectoryFromINode(const INode& inode);
};

#endif // COMMANDHANDLER_H