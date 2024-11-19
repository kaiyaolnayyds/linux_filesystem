#include "DiskManager.h"
#include "CommandHandler.h"
#include <iostream>
#include <fstream>

int main() {
    const std::string diskFile = "simdisk.bin";
    const size_t blockSize = 1024;
    const size_t totalBlocks = 100; // 假设100MB的磁盘空间

    // 初始化磁盘管理器
    DiskManager diskManager(diskFile, blockSize, totalBlocks);

    // 检查磁盘文件是否存在
    std::ifstream diskFileCheck(diskFile);
    if (!diskFileCheck.good()) {
        // 如果文件不存在，初始化文件系统
        diskManager.initialize();
        std::cout << "File system initialized." << std::endl;
    }
    else {
        std::cout << "File system loaded from existing disk file." << std::endl;
    }
    diskFileCheck.close();

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
