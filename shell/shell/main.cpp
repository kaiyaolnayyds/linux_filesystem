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
    // ���ӵ������ڴ�
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

    // ���ź���
    sem_t* semCommand = sem_open(SEM_COMMAND, 0);
    sem_t* semResponse = sem_open(SEM_RESPONSE, 0);

    if (semCommand == SEM_FAILED || semResponse == SEM_FAILED) {
        std::cerr << "Failed to open semaphores." << std::endl;
        exit(EXIT_FAILURE);
    }

    // ��������ѭ��
    std::string command;
    while (true) {
        // ��ʾ��ʾ��
        std::cout << "shell> ";
        std::getline(std::cin, command);

        if (command == "exit") {
            break;
        }


        // ������͸� simdisk
        sem_wait(semCommand); // ��ȡ�ź���

        // ��չ����ڴ��е��������
        memset(sharedMemory->command, 0, sizeof(sharedMemory->command));

        strncpy(sharedMemory->command, command.c_str(), sizeof(sharedMemory->command) - 1);
        sharedMemory->commandReady = true;

        // ֪ͨ simdisk ������׼����
        sem_post(semResponse);

        // �ȴ���Ӧ
        sem_wait(semCommand);
        if (sharedMemory->responseReady) {
            std::string response(sharedMemory->response);
            sharedMemory->responseReady = false;

            // ��ʾ��Ӧ
            std::cout << response << std::endl;
        }
        else {
            std::cout << "No response from simdisk." << std::endl;
        }

        // �ͷ��ź���
        sem_post(semCommand);
    }

    // ������Դ
    shmdt(sharedMemory);
    sem_close(semCommand);
    sem_close(semResponse);

    return 0;
}