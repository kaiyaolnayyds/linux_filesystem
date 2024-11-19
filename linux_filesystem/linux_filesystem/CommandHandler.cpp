// src/CommandHandler.cpp
#include "CommandHandler.h"
#include "Directory.h"
#include "INode.h"
#include "SuperBlock.h"
#include <iostream>
#include <sstream>
#include "SuperBlock.h"



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
        std::string path;
        iss >> path;
        handleDir(path);
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



void CommandHandler::handleDir(const std::string& path) {
    // 逻辑：显示目录内容
    std::cout << "Listing directory: " << path << std::endl;
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
