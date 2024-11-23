#include "DiskManager.h"
#include "CommandHandler.h"
#include "UserManager.h" 
#include <iostream>
#include <fstream>

int main() {
    // 创建磁盘对象
    DiskManager diskManager("simdisk.bin", 1024, 100);

    // 检查磁盘文件是否存在
    std::ifstream diskFileCheck("simdisk.bin", std::ios::binary);
    if (!diskFileCheck.good()) {
        // 磁盘文件不存在，初始化文件系统
        diskManager.initialize();
        std::cout << "File system initialized." << std::endl;
    }
    else {
        std::cout << "[DEBUG] Disk file exists. Loading existing file system." << std::endl;

        // 检查磁盘文件大小是否足够
        diskFileCheck.seekg(0, std::ios::end);
        size_t fileSize = diskFileCheck.tellg(); // .bin 磁盘文件大小
        size_t expectedSize = diskManager.totalBlocks * diskManager.blockSize; // 预期大小为块总数 * 块大小

        if (fileSize < expectedSize) {
            diskFileCheck.close();

            // 预分配磁盘文件大小
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

    UserManager userManager; // 创建 UserManager 对象
    CommandHandler cmdHandler(diskManager, userManager); // 将 userManager 传递给 CommandHandler

    // 主循环
    while (true) {
        // 登录循环
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

        // 处理用户命令
        std::string command;
        while (userManager.isLoggedIn()) {
            cmdHandler.updatePrompt(); // 显示当前路径的提示符
            std::getline(std::cin, command);
            if (command == "EXIT") {
                std::cout << "EXIT successful. Welcom to next use" << std::endl;
                return 0; // 退出程序
            }

            bool shouldLogout = cmdHandler.handleCommand(command);

            if (shouldLogout) {
                // 用户已注销，跳出命令循环，返回登录循环
                std::cout << "Logout successful." << std::endl;
                break;
            }
        }
    }

    return 0;
}