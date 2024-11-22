#include "DiskManager.h"
#include "CommandHandler.h"
#include <iostream>
#include <fstream>

int main() {
    DiskManager diskManager("simdisk.bin", 512, 100);

    // 检查磁盘文件是否存在
    std::ifstream diskFileCheck("simdisk.bin", std::ios::binary);
    if (!diskFileCheck.good()) {
        // 磁盘文件不存在，初始化文件系统
        diskManager.initialize();
        std::cout << "File system initialized." << std::endl;
    }
    else {
        std::cout << "[DEBUG] Disk file exists. Loading existing file system." << std::endl;

        // **检查磁盘文件大小是否足够**
        diskFileCheck.seekg(0, std::ios::end);
        size_t fileSize = diskFileCheck.tellg();
        size_t expectedSize = diskManager.totalBlocks * diskManager.blockSize;
        if (fileSize < expectedSize) {
            diskFileCheck.close();

            // **预分配磁盘文件大小**
            std::ofstream file(diskManager.diskFile, std::ios::binary | std::ios::out | std::ios::in);
            file.seekp(expectedSize - 1);
            file.write("", 1);
            file.close();
        }
        else {
            diskFileCheck.close();
        }

        // 加载超级块和位图
        diskManager.loadSuperBlock();
        diskManager.loadBitmaps();
        std::cout << "File system loaded from existing disk file." << std::endl;
    }

    CommandHandler cmdHandler(diskManager);

    // 主循环处理命令
    std::string command;
    std::cout << "SimDisk System Initialized. Type EXIT to quit." << std::endl;
    while (true) {
        cmdHandler.updatePrompt(); // 显示当前路径的提示符
        std::getline(std::cin, command);
        if (command == "EXIT") {
            break;
        }
        cmdHandler.handleCommand(command);
    }

    return 0;
}

