#include "DiskManager.h"
#include "CommandHandler.h"
#include "UserManager.h" 
#include <iostream>
#include <fstream>
#include <unistd.h>      // for fork()
#include <sys/types.h>   // for pid_t
#include <sys/ipc.h>     // for IPC
#include <sys/shm.h>     // for shared memory
#include <semaphore.h>   // for semaphores
#include <fcntl.h>       // for O_CREAT
#include <cstring>       // for strerror
#include <cerrno>        // for errno
#include <string>
#include <sstream>


// 定义共享内存和信号量的键和名称
#define SHM_KEY 0x1234
#define SEM_COMMAND "/sem_command"
#define SEM_RESPONSE "/sem_response"

struct SharedMemorySegment {
    char command[1024];
    char response[4096];
    bool commandReady;
    bool responseReady;
};

int main() {
    // 创建守护进程
    pid_t pid = fork();
    if (pid < 0) {
        std::cerr << "Fork failed: " << strerror(errno) << std::endl;
        exit(EXIT_FAILURE);
    }
    else if (pid > 0) {
        // 父进程退出，子进程继续运行
        std::cout << "simdisk is running as a background process with PID: " << pid << std::endl;
        exit(EXIT_SUCCESS);
    }

    // 子进程继续执行，成为守护进程
    setsid(); // 创建新的会话

    // 继续原有的初始化逻辑
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

    // 初始化共享内存和信号量
    // ...

        // 初始化共享内存
    int shmid = shmget(SHM_KEY, sizeof(SharedMemorySegment), IPC_CREAT | 0666);
    if (shmid < 0) {
        std::cerr << "Failed to create shared memory: " << strerror(errno) << std::endl;
        exit(EXIT_FAILURE);
    }

    SharedMemorySegment* sharedMemory = (SharedMemorySegment*)shmat(shmid, nullptr, 0);
    if (sharedMemory == (void*)-1) {
        std::cerr << "Failed to attach shared memory: " << strerror(errno) << std::endl;
        exit(EXIT_FAILURE);
    }

    // 初始化信号量
    sem_t* semCommand = sem_open(SEM_COMMAND, O_CREAT, 0666, 1);
    sem_t* semResponse = sem_open(SEM_RESPONSE, O_CREAT, 0666, 0);

    if (semCommand == SEM_FAILED || semResponse == SEM_FAILED) {
        std::cerr << "Failed to create semaphores: " << strerror(errno) << std::endl;
        exit(EXIT_FAILURE);
    }

    // 初始化共享内存中的标志
    sharedMemory->commandReady = false;
    sharedMemory->responseReady = false;

    // 主循环
    while (true) {
        // 登录循环
        while (!userManager.isLoggedIn()) {
            // 从共享内存读取用户名和密码
            std::string username, password;

            // 等待来自 Shell 的登录命令，例如格式为 "login username password"
            sem_wait(semResponse); // 等待命令准备好

            if (sharedMemory->commandReady) {
                std::string command(sharedMemory->command);
                sharedMemory->commandReady = false;

                // 解析登录命令
                std::istringstream iss(command);
                std::string cmd;
                iss >> cmd;
                if (cmd == "login") {
                    iss >> username >> password;

                    if (userManager.login(username, password)) {
                        std::string response = "Login successful. Welcome, " + username + "!";
                        strncpy(sharedMemory->response, response.c_str(), sizeof(sharedMemory->response));
                        sharedMemory->responseReady = true;
                        // 通知 Shell 响应已准备好
                        sem_post(semCommand);
                        break; // 退出登录循环，进入命令处理循环
                    }
                    else {
                        std::string response = "Login failed. Invalid username or password.";
                        strncpy(sharedMemory->response, response.c_str(), sizeof(sharedMemory->response));
                        sharedMemory->responseReady = true;
                        // 通知 Shell 响应已准备好
                        sem_post(semCommand);
                    }
                }
                else {
                    std::string response = "Please login first using 'login <username> <password>'.";
                    strncpy(sharedMemory->response, response.c_str(), sizeof(sharedMemory->response));
                    sharedMemory->responseReady = true;
                    // 通知 Shell 响应已准备好
                    sem_post(semCommand);
                }
            }
        }

        // 处理用户命令
        while (userManager.isLoggedIn()) {
            // 从共享内存读取命令
            sem_wait(semResponse); // 等待命令准备好

            if (sharedMemory->commandReady) {
                std::string command(sharedMemory->command);
                sharedMemory->commandReady = false;

                if (command == "EXIT") {
                    std::string response = "EXIT successful. Welcome to next use.";
                    strncpy(sharedMemory->response, response.c_str(), sizeof(sharedMemory->response));
                    sharedMemory->responseReady = true;
                    sem_post(semCommand);
                    // 清理资源并退出
                    shmdt(sharedMemory);
                    shmctl(shmid, IPC_RMID, nullptr);
                    sem_close(semCommand);
                    sem_close(semResponse);
                    sem_unlink(SEM_COMMAND);
                    sem_unlink(SEM_RESPONSE);
                    return 0;
                }

                bool shouldLogout = cmdHandler.handleCommand(command);

                // 获取命令处理的输出
                std::string response = cmdHandler.getLastOutput();

                // 清空共享内存中的响应缓冲区
                memset(sharedMemory->response, 0, sizeof(sharedMemory->response));

                strncpy(sharedMemory->response, response.c_str(), sizeof(sharedMemory->response) - 1);
                sharedMemory->responseReady = true;
                sem_post(semCommand);

                if (shouldLogout) {
                    std::string logoutMsg = "Logout successful.";
                    strncpy(sharedMemory->response, logoutMsg.c_str(), sizeof(sharedMemory->response));
                    sharedMemory->responseReady = true;
                    sem_post(semCommand);
                    userManager.logout();
                    break; // 返回登录循环
                }
            }
        }
    }

    // 清理资源并退出
    shmdt(sharedMemory);
    shmctl(shmid, IPC_RMID, nullptr);
    sem_close(semCommand);
    sem_close(semResponse);
    sem_unlink(SEM_COMMAND);
    sem_unlink(SEM_RESPONSE);


}
