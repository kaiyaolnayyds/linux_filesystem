// src/CommandHandler.cpp
#include "CommandHandler.h"
#include <iostream>
#include <sstream>


CommandHandler::CommandHandler(DiskManager& dm) : diskManager(dm) {
    // 加载根目录
    SuperBlock superBlock = diskManager.loadSuperBlock();
    uint32_t rootInode = superBlock.rootInode;

    // 读取根目录的 inode
    currentInodeIndex = rootInode;
    INode rootInodeObj = diskManager.readINode(rootInode);

    // 从磁盘读取根目录数据
    char buffer[diskManager.blockSize];
    diskManager.readBlock(rootInodeObj.blockIndex, buffer);

    // 反序列化根目录
    currentDirectory.deserialize(buffer, diskManager.blockSize);

    // 调试输出
    std::cout << "[DEBUG] Loaded current directory entries:" << std::endl;
    for (const auto& entry : currentDirectory.entries) {
        std::cout << "  " << entry.first << " -> inode " << entry.second << std::endl;
    }
}


void CommandHandler::handleCommand(const std::string& command) {
    std::istringstream iss(command);
    std::string cmd;
    iss >> cmd;

    if (cmd == "info") {
        handleInfo();
    }
    else if (cmd == "cd") {
        std::string path;
        iss >> path;
        handleCd(path);
    }
    else if (cmd == "dir") {
        std::string arg;
        bool recursive = false;
        std::string path;

        // 解析所有参数
        while (iss >> arg) {
            if (arg == "/s") {
                recursive = true;
            }
            else {
                path = arg;
            }
        }

        handleDir(path, recursive);
    }
    else if (cmd == "md") {
        std::string dirName;
        iss >> dirName;
        handleMd(dirName);
    }
    else if (cmd == "rd") {
        std::string dirName;
        iss >> dirName;
        handleRd(dirName);
    }
    else if (cmd == "newfile") {
        std::string fileName;
        iss >> fileName;
        handleNewFile(fileName);
    }
    else if (cmd == "cat") {
        std::string fileName;
        iss >> fileName;
        handleCat(fileName);
    }
    else if (cmd == "copy") {
        std::string src, dest;
        iss >> src >> dest;
        handleCopy(src, dest);
    }
    else if (cmd == "del") {
        std::string fileName;
        iss >> fileName;
        handleDel(fileName);
    }
    else if (cmd == "check") {
        handleCheck();
    }
    else {
        std::cout << "Unknown command: " << cmd << std::endl;
    }
}

void CommandHandler::handleInfo() {
    SuperBlock superBlock = diskManager.loadSuperBlock();

    // 打印超级块信息
    std::cout << "=== File System Information ===" << std::endl;
    std::cout << "Total Blocks: " << superBlock.totalBlocks << std::endl;
    std::cout << "Free Blocks: " << superBlock.freeBlocks << std::endl;
    std::cout << "Total iNodes: " << superBlock.inodeCount << std::endl;
    std::cout << "Root Directory iNode: " << superBlock.rootInode << std::endl;
    std::cout << "================================" << std::endl;
}


void CommandHandler::handleCd(const std::string& path) {
    try {
        std::vector<std::string> components = parsePath(path); // 解析路径
        uint32_t currentInode = 0; // 从根目录开始
        Directory tempDirectory = currentDirectory; // 临时目录对象

        for (const auto& component : components) {
            uint32_t nextInode = tempDirectory.findEntry(component); // 查找子目录
            char buffer[1024];
            diskManager.readBlock(nextInode, buffer); // 从磁盘读取子目录数据
            tempDirectory = *reinterpret_cast<Directory*>(buffer); // 更新当前目录
        }

        currentDirectory = tempDirectory; // 更新当前目录
        std::cout << "Changed to directory: " << path << "." << std::endl;
    }
    catch (const std::exception& e) {
        std::cerr << "Error changing directory: " << e.what() << std::endl;
    }
}



void CommandHandler::handleDir(const std::string& path, bool recursive) {
    // 确定要显示的目录
    Directory dirToDisplay = currentDirectory;
    uint32_t dirInodeIndex = currentInodeIndex;

    if (!path.empty()) {
        // 查找指定的路径
        // 这里需要实现路径解析（例如支持相对路径、绝对路径）
        // 为简化，假设只支持当前目录下的直接子目录
        auto it = currentDirectory.entries.find(path);
        if (it != currentDirectory.entries.end()) {
            uint32_t inodeIndex = it->second;
            INode inode = diskManager.readINode(inodeIndex);
            if (inode.type != 1) {
                std::cout << path << " is not a directory." << std::endl;
                return;
            }
            // 读取目录内容
            char buffer[diskManager.blockSize];
            diskManager.readBlock(inode.blockIndex, buffer);
            dirToDisplay.deserialize(buffer, diskManager.blockSize);
            dirInodeIndex = inodeIndex;
        }
        else {
            std::cout << "Directory not found: " << path << std::endl;
            return;
        }
    }

    // 显示目录内容
    displayDirectoryContents(dirToDisplay, dirInodeIndex, recursive, "");
}

void CommandHandler::displayDirectoryContents(const Directory& dir, uint32_t dirInodeIndex, bool recursive, const std::string& indent) {
    if (dir.entries.empty()) {
        std::cout << indent << "Directory is empty." << std::endl;
    }
    else {
        for (const auto& entry : dir.entries) {
            const std::string& name = entry.first;
            uint32_t inodeIndex = entry.second;
            INode inode = diskManager.readINode(inodeIndex);

            // 显示文件名、物理地址、保护码、文件长度、子目录等
            std::string typeStr = (inode.type == 1) ? "<DIR>" : "<FILE>";
            std::cout << indent << typeStr << " " << name
                << " [Addr: " << inode.blockIndex
                << ", Mode: " << std::oct << inode.mode << std::dec
                << ", Size: " << inode.size << "]" << std::endl;

            // 如果是目录且需要递归
            if (recursive && inode.type == 1) {
                // 读取子目录内容
                Directory subDir;
                char buffer[diskManager.blockSize];
                diskManager.readBlock(inode.blockIndex, buffer);
                subDir.deserialize(buffer, diskManager.blockSize);

                // 递归显示子目录内容
                displayDirectoryContents(subDir, inodeIndex, recursive, indent + "  ");
            }
        }
    }
}


void CommandHandler::handleMd(const std::string& dirName) {
    try {
        size_t blockIndex = diskManager.allocateBlock(); // 分配块
        if (blockIndex == static_cast<size_t>(-1)) {
            std::cerr << "Error: No free blocks available to create directory." << std::endl;
            return;
        }

        currentDirectory.addEntry(dirName, static_cast<uint32_t>(blockIndex), diskManager);
        std::cout << "Directory '" << dirName << "' created at block " << blockIndex << "." << std::endl;

        // 加载和更新超级块
        SuperBlock superBlock = diskManager.loadSuperBlock();
        superBlock.freeBlocks -= 1; // 减少空闲块
        superBlock.inodeCount += 1; // 增加 iNode 数量
        diskManager.updateSuperBlock(superBlock);

    }
    catch (const std::exception& e) {
        std::cerr << "Error creating directory: " << e.what() << std::endl;
    }
}



void CommandHandler::handleRd(const std::string& dirName) {
    // 逻辑：删除目录及其内容
    std::cout << "Removing directory: " << dirName << std::endl;
}

void CommandHandler::handleNewFile(const std::string& fileName) {
    // 逻辑：创建新文件
    std::cout << "Creating new file: " << fileName << std::endl;
}

void CommandHandler::handleCat(const std::string& fileName) {
    // 逻辑：显示文件内容
    std::cout << "Displaying file: " << fileName << std::endl;
}

void CommandHandler::handleCopy(const std::string& src, const std::string& dest) {
    // 逻辑：拷贝文件
    std::cout << "Copying from " << src << " to " << dest << std::endl;
}

void CommandHandler::handleDel(const std::string& fileName) {
    // 逻辑：删除文件
    std::cout << "Deleting file: " << fileName << std::endl;
}

void CommandHandler::handleCheck() {
    // 逻辑：检测并恢复文件系统
    std::cout << "Checking and restoring file system." << std::endl;
}

std::vector<std::string> CommandHandler::parsePath(const std::string& path) {
    std::vector<std::string> components;
    std::istringstream stream(path);
    std::string token;
    while (std::getline(stream, token, '/')) {
        if (!token.empty()) {
            components.push_back(token);
        }
    }
    return components;
}

