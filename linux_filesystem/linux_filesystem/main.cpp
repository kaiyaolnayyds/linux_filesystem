// main.cpp

#include "DiskManager.h"
#include "CommandHandler.h"
#include "UserManager.h"
#include <iostream>
#include <fstream>
#include <unistd.h>       // fork, setsid, chdir
#include <signal.h>       // signal
#include <sys/ipc.h>      // shmget, shmctl
#include <sys/shm.h>      // shmget, shmat, shmdt
#include <semaphore.h>    // sem_open, sem_wait, sem_post, sem_close, sem_unlink
#include <fcntl.h>        // O_CREAT
#include <sys/stat.h>     // S_IRUSR, S_IWUSR
#include <cstring>        // strcpy, memset
#include <cstdlib>        // exit

#define SHM_KEY 0x1234    // 共享内存的键值，可以根据需要修改
#define COMMAND_SIZE 256
#define RESPONSE_SIZE 2048

// 定义共享内存中的数据结构
struct SharedMemorySegment {
    char command[COMMAND_SIZE];     // 命令字符串
    char response[RESPONSE_SIZE];   // 响应信息
    int commandReady;               // 命令就绪标志，0：未就绪，1：就绪
    int responseReady;              // 响应就绪标志，0：未就绪，1：就绪
};

int main() {
   // // 1. 守护化进程
   // pid_t pid = fork();
   // if (pid < 0) {
   //     std::cerr << "Failed to fork." << std::endl;
   //     exit(EXIT_FAILURE);
   // }
   // if (pid > 0) {
   //     // 父进程退出
   //     std::cout << "SimDisk daemon started with PID: " << pid << std::endl;
   //     exit(EXIT_SUCCESS);
   // }

   // // 子进程继续，成为守护进程
   // if (setsid() < 0) {
   //     std::cerr << "Failed to setsid." << std::endl;
   //     exit(EXIT_FAILURE);
   // }

   // // 忽略信号
   // signal(SIGCHLD, SIG_IGN);
   // signal(SIGHUP, SIG_IGN);

   // // 更改工作目录
   //// chdir("/");

   // // 重设文件权限掩码
   // umask(0);

   // // 关闭文件描述符
   // // 不要关闭标准输入输出，因为需要日志输出（可选）
   // // for (int x = sysconf(_SC_OPEN_MAX); x>=0; x--) {
   // //     close(x);
   // // }

    // 2. 初始化共享内存和信号量
    int shm_id = shmget(SHM_KEY, sizeof(SharedMemorySegment), IPC_CREAT | 0666);
    if (shm_id == -1) {
        // 创建共享内存失败
        std::cout << "Size of SharedMemorySegment: " << sizeof(SharedMemorySegment) << std::endl;
        std::cerr << "Failed to create shared memory. Error: " << strerror(errno) << std::endl;
        exit(EXIT_FAILURE);
    }

    SharedMemorySegment* shm_ptr = (SharedMemorySegment*)shmat(shm_id, NULL, 0);
    if (shm_ptr == (void*)-1) {
        // 连接共享内存失败
        std::cout << "link to sharedmemory failed." << std::endl;
        exit(EXIT_FAILURE);
    }

    // 初始化共享内存内容
    memset(shm_ptr, 0, sizeof(SharedMemorySegment));

    // 创建信号量
    sem_t* command_sem = sem_open("/simdisk_command_sem", O_CREAT, 0666, 1);
    sem_t* response_sem = sem_open("/simdisk_response_sem", O_CREAT, 0666, 1);

    if (command_sem == SEM_FAILED || response_sem == SEM_FAILED) {
        // 信号量创建失败
        std::cout << "create signal failed." << std::endl;
        exit(EXIT_FAILURE);
    }

    // 3. 创建磁盘对象
    DiskManager diskManager("simdisk.bin", 1024, 100);

    // 检查磁盘文件是否存在
    std::ifstream diskFileCheck("simdisk.bin", std::ios::binary);
    if (!diskFileCheck.good()) {
        // 磁盘文件不存在，初始化文件系统
        diskManager.initialize();
        // 可以在此添加日志记录
    }
    else {
        // 加载超级块和位图
        diskManager.loadSuperBlock();
        diskManager.loadBitmaps();
        // 可以在此添加日志记录
    }

    UserManager userManager; // 创建 UserManager 对象
    CommandHandler cmdHandler(diskManager, userManager); // 将 userManager 传递给 CommandHandler

    // 后台进程的会话状态
    bool isLoggedIn = false;

    // 4. 主循环，等待命令
    while (true) {
        // 等待命令信号量
        sem_wait(command_sem);

        if (shm_ptr->commandReady == 1) {
            // 读取命令
            std::string command = shm_ptr->command;
            shm_ptr->commandReady = 0; // 重置命令就绪标志

            // 释放命令信号量
            sem_post(command_sem);

            // 处理命令
            std::string response;

            if (!isLoggedIn) {
                // 尚未登录，需要处理登录命令
                if (command.substr(0, 5) == "login") {
                    // 命令格式：login username password
                    size_t pos1 = command.find(' ');
                    size_t pos2 = command.find(' ', pos1 + 1);
                    if (pos1 != std::string::npos && pos2 != std::string::npos) {
                        std::string username = command.substr(pos1 + 1, pos2 - pos1 - 1);
                        std::string password = command.substr(pos2 + 1);
                        if (userManager.login(username, password)) {
                            isLoggedIn = true;
                            response = "Login successful. Welcome, " + username + "!";
                        }
                        else {
                            response = "Login failed. Invalid username or password.";
                        }
                    }
                    else {
                        response = "Invalid login command format. Use: login username password";
                    }
                }
                else {
                    response = "Please login first. Use: login username password";
                }
            }
            else {
                // 已登录，处理其他命令
                if (command == "logout") {
                    userManager.logout();
                    isLoggedIn = false;
                    response = "Logout successful.";
                }
                else if (command == "shutdown") {
                    // 清理资源并退出
                    response = "SimDisk daemon is shutting down.";
                    // 写入响应
                    sem_wait(response_sem);
                    strncpy(shm_ptr->response, response.c_str(), RESPONSE_SIZE - 1);
                    shm_ptr->response[RESPONSE_SIZE - 1] = '\0';
                    shm_ptr->responseReady = 1;
                    sem_post(response_sem);
                    break; // 退出主循环
                }
                else {
                    // 处理一般命令
                    cmdHandler.setLastOutput(""); // 清空上一次的输出
                    bool shouldLogout = cmdHandler.handleCommand(command);
                    response = cmdHandler.getLastOutput();

                    // 如果用户在命令处理中注销，需要更新会话状态
                    if (shouldLogout) {
                        isLoggedIn = false;
                    }
                }
            }

            // 写入响应
            sem_wait(response_sem);
            strncpy(shm_ptr->response, response.c_str(), RESPONSE_SIZE - 1);
            shm_ptr->response[RESPONSE_SIZE - 1] = '\0'; // 确保字符串以 null 结尾
            shm_ptr->responseReady = 1;
            sem_post(response_sem);

        }
        else {
            // 没有新命令，释放命令信号量
            sem_post(command_sem);
            // 睡眠，避免忙等待
            usleep(100000); // 100ms
        }
    }

    // 5. 清理资源
    shmdt(shm_ptr); // 分离共享内存
    shmctl(shm_id, IPC_RMID, NULL); // 删除共享内存

    sem_close(command_sem);
    sem_unlink("/simdisk_command_sem");

    sem_close(response_sem);
    sem_unlink("/simdisk_response_sem");

    exit(EXIT_SUCCESS);
}
