// src/CommandHandler.cpp
#include "CommandHandler.h"
#include "Directory.h"
#include "INode.h"
#include <iostream>
#include <sstream>

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
    diskManager.initialize(); // 假设初始化会展示系统信息
    std::cout << "System initialized." << std::endl;
}

void CommandHandler::handleCd(const std::string& path) {
    // 逻辑：改变当前目录（假设有相关逻辑在DiskManager中）
    std::cout << "Changing directory to: " << path << std::endl;
}

void CommandHandler::handleDir(const std::string& path) {
    // 逻辑：显示目录内容
    std::cout << "Listing directory: " << path << std::endl;
}

void CommandHandler::handleMd(const std::string& dirName) {
    // 逻辑：创建新目录
    std::cout << "Creating directory: " << dirName << std::endl;
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
