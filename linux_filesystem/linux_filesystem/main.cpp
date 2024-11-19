#include "DiskManager.h"
#include "CommandHandler.h"
#include <iostream>
#include <fstream>

int main() {
    DiskManager diskManager("simdisk.bin", 512, 100);

    // 检查磁盘文件是否存在
    std::ifstream diskFile("simdisk.bin", std::ios::binary);
    if (!diskFile.good()) {
        // 磁盘文件不存在，初始化文件系统
        diskManager.initialize();
        std::cout << "File system initialized." << std::endl;
    }
    else {
        std::cout << "[DEBUG] Disk file exists. Loading existing file system." << std::endl;
        diskManager.loadSuperBlock();
        diskManager.loadBitmap();
        std::cout << "File system loaded from existing disk file." << std::endl;
    }
    diskFile.close();

    CommandHandler cmdHandler(diskManager);

    // 主循环处理命令
    std::string command;
    std::cout << "SimDisk System Initialized. Type EXIT to quit." << std::endl;
    while (true) {
        std::cout << "simdisk> ";
        std::getline(std::cin, command);
        if (command == "EXIT") {
            break;
        }
        cmdHandler.handleCommand(command);
    }

    return 0;
}

