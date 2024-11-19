#include "DiskManager.h"
#include "CommandHandler.h"
#include <iostream>
#include <fstream>

int main() {
    const std::string diskFile = "simdisk.bin";
    const size_t blockSize = 1024;
    const size_t totalBlocks = 100; // ����100MB�Ĵ��̿ռ�

    // ��ʼ�����̹�����
    DiskManager diskManager(diskFile, blockSize, totalBlocks);

    // �������ļ��Ƿ����
    std::ifstream diskFileCheck(diskFile);
    if (!diskFileCheck.good()) {
        // ����ļ������ڣ���ʼ���ļ�ϵͳ
        diskManager.initialize();
        std::cout << "File system initialized." << std::endl;
    }
    else {
        std::cout << "File system loaded from existing disk file." << std::endl;
    }
    diskFileCheck.close();

    // �����������
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
