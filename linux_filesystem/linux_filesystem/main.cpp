#include "DiskManager.h"
#include "CommandHandler.h"
#include "UserManager.h" 
#include <iostream>
#include <fstream>

int main() {
    // �������̶���
    DiskManager diskManager("simdisk.bin", 1024, 100);

    // �������ļ��Ƿ����
    std::ifstream diskFileCheck("simdisk.bin", std::ios::binary);
    if (!diskFileCheck.good()) {
        // �����ļ������ڣ���ʼ���ļ�ϵͳ
        diskManager.initialize();
        std::cout << "File system initialized." << std::endl;
    }
    else {
        std::cout << "[DEBUG] Disk file exists. Loading existing file system." << std::endl;

        // �������ļ���С�Ƿ��㹻
        diskFileCheck.seekg(0, std::ios::end);
        size_t fileSize = diskFileCheck.tellg(); // .bin �����ļ���С
        size_t expectedSize = diskManager.totalBlocks * diskManager.blockSize; // Ԥ�ڴ�СΪ������ * ���С

        if (fileSize < expectedSize) {
            diskFileCheck.close();

            // Ԥ��������ļ���С
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

    UserManager userManager; // ���� UserManager ����
    CommandHandler cmdHandler(diskManager, userManager); // �� userManager ���ݸ� CommandHandler

    // ��ѭ��
    while (true) {
        // ��¼ѭ��
        while (!userManager.isLoggedIn()) {
            std::string username, password;

            std::cout << "SimDisk Login\nUsername: ";
            std::getline(std::cin, username);
            std::cout << "Password: ";
            std::getline(std::cin, password);

            if (userManager.login(username, password)) {
                std::cout << "Login successful. Welcome, " << username << "!" << std::endl;
            }
            else {
                std::cout << "Login failed. Invalid username or password." << std::endl;
            }
        }

        // �����û�����
        std::string command;
        while (userManager.isLoggedIn()) {
            cmdHandler.updatePrompt(); // ��ʾ��ǰ·������ʾ��
            std::getline(std::cin, command);
            if (command == "EXIT") {
                std::cout << "EXIT successful. Welcom to next use" << std::endl;
                return 0; // �˳�����
            }

            bool shouldLogout = cmdHandler.handleCommand(command);

            if (shouldLogout) {
                // �û���ע������������ѭ�������ص�¼ѭ��
                std::cout << "Logout successful." << std::endl;
                break;
            }
        }
    }

    return 0;
}