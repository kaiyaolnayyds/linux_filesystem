// src/main.cpp
#include "DiskManager.h"
#include "CommandHandler.h"
#include <iostream>

int main() {
    const std::string diskFile = "simdisk.bin";
    const size_t blockSize = 1024;
    const size_t totalBlocks = 100 * 1024 / blockSize; // 假设100MB的磁盘空间

    // 初始化磁盘管理器
    DiskManager diskManager(diskFile, blockSize, totalBlocks);

    // 调用 initialize() 方法以初始化文件系统
    diskManager.initialize();

    // 创建命令处理器
    CommandHandler commandHandler(diskManager);


    std::string command;
    std::cout << "SimDisk System Initialized. Type EXIT to quit." << std::endl;

    while (true) {
        std::cout << "simdisk> ";
        std::getline(std::cin, command);

        if (command == "EXIT" || command == "exit") {
            break;
        }

        commandHandler.handleCommand(command);
    }

    std::cout << "Goodbye!" << std::endl;
    return 0;
}
