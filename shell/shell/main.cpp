// shell.cpp

#include <iostream>
#include <string>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <semaphore.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>

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
    // 连接到共享内存
    int shmid = shmget(SHM_KEY, sizeof(SharedMemorySegment), 0666);
    if (shmid < 0) {
        std::cerr << "Failed to get shared memory. Is simdisk running?" << std::endl;
        exit(EXIT_FAILURE);
    }

    SharedMemorySegment* sharedMemory = (SharedMemorySegment*)shmat(shmid, nullptr, 0);
    if (sharedMemory == (void*)-1) {
        std::cerr << "Failed to attach shared memory." << std::endl;
        exit(EXIT_FAILURE);
    }

    // 打开信号量
    sem_t* semCommand = sem_open(SEM_COMMAND, 0);
    sem_t* semResponse = sem_open(SEM_RESPONSE, 0);

    if (semCommand == SEM_FAILED || semResponse == SEM_FAILED) {
        std::cerr << "Failed to open semaphores." << std::endl;
        exit(EXIT_FAILURE);
    }

    // 命令输入循环
    std::string command;
    while (true) {
        // 显示提示符
        std::cout << "shell> ";
        std::getline(std::cin, command);

        if (command == "exit") {
            break;
        }


        // 将命令发送给 simdisk
        sem_wait(semCommand); // 获取信号量

        // 清空共享内存中的命令缓冲区
        memset(sharedMemory->command, 0, sizeof(sharedMemory->command));

        strncpy(sharedMemory->command, command.c_str(), sizeof(sharedMemory->command) - 1);
        sharedMemory->commandReady = true;

        // 通知 simdisk 命令已准备好
        sem_post(semResponse);

        // 等待响应
        sem_wait(semCommand);
        if (sharedMemory->responseReady) {
            std::string response(sharedMemory->response);
            sharedMemory->responseReady = false;

            // 显示响应
            std::cout << response << std::endl;
        }
        else {
            std::cout << "No response from simdisk." << std::endl;
        }

        // 释放信号量
        sem_post(semCommand);
    }

    // 清理资源
    shmdt(sharedMemory);
    sem_close(semCommand);
    sem_close(semResponse);

    return 0;
}