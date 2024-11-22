#include "DiskManager.h"
#include "CommandHandler.h"
#include <iostream>
#include <fstream>

int main() {
    DiskManager diskManager("simdisk.bin", 512, 100);

    // �������ļ��Ƿ����
    std::ifstream diskFileCheck("simdisk.bin", std::ios::binary);
    if (!diskFileCheck.good()) {
        // �����ļ������ڣ���ʼ���ļ�ϵͳ
        diskManager.initialize();
        std::cout << "File system initialized." << std::endl;
    }
    else {
        std::cout << "[DEBUG] Disk file exists. Loading existing file system." << std::endl;

        // **�������ļ���С�Ƿ��㹻**
        diskFileCheck.seekg(0, std::ios::end);
        size_t fileSize = diskFileCheck.tellg();
        size_t expectedSize = diskManager.totalBlocks * diskManager.blockSize;
        if (fileSize < expectedSize) {
            diskFileCheck.close();

            // **Ԥ��������ļ���С**
            std::ofstream file(diskManager.diskFile, std::ios::binary | std::ios::out | std::ios::in);
            file.seekp(expectedSize - 1);
            file.write("", 1);
            file.close();
        }
        else {
            diskFileCheck.close();
        }

        // ���س������λͼ
        diskManager.loadSuperBlock();
        diskManager.loadBitmaps();
        std::cout << "File system loaded from existing disk file." << std::endl;
    }

    CommandHandler cmdHandler(diskManager);

    // ��ѭ����������
    std::string command;
    std::cout << "SimDisk System Initialized. Type EXIT to quit." << std::endl;
    while (true) {
        cmdHandler.updatePrompt(); // ��ʾ��ǰ·������ʾ��
        std::getline(std::cin, command);
        if (command == "EXIT") {
            break;
        }
        cmdHandler.handleCommand(command);
    }

    return 0;
}

