// src/main.cpp
#include "DiskManager.h"
#include "CommandHandler.h"
#include <iostream>

int main() {
    const std::string diskFile = "simdisk.bin";
    const size_t blockSize = 1024;
    const size_t totalBlocks = 100 * 1024 / blockSize; // ºŸ…Ë100MBµƒ¥≈≈Ãø’º‰

    DiskManager diskManager(diskFile, blockSize, totalBlocks);
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
