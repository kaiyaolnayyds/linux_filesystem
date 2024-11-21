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

    // 显示目录内容
    void displayDirectoryContents(const Directory& dir, uint32_t dirInodeIndex, bool recursive, const std::string& indent);

    // 新增：根据路径查找目录
    bool findDirectory(const std::vector<std::string>& pathComponents, uint32_t& resultInodeIndex, Directory& resultDirectory);

    // 新增：获取指定 inodeIndex 的目录
    bool getDirectory(uint32_t inodeIndex, Directory& directory);
};

#endif // COMMANDHANDLER_H
