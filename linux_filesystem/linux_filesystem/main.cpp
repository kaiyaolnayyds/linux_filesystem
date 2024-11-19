#include "DiskManager.h"
#include "CommandHandler.h"
#include <iostream>
#include <fstream>

int main() {
    DiskManager diskManager("simdisk.bin", 512, 100);

    // �������ļ��Ƿ����
    std::ifstream diskFile("simdisk.bin", std::ios::binary);
    if (!diskFile.good()) {
        // �����ļ������ڣ���ʼ���ļ�ϵͳ
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

    // ��ѭ����������
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

