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


// ���干���ڴ���ź����ļ�������
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
    // �����ػ�����
    pid_t pid = fork();
    if (pid < 0) {
        std::cerr << "Fork failed: " << strerror(errno) << std::endl;
        exit(EXIT_FAILURE);
    }
    else if (pid > 0) {
        // �������˳����ӽ��̼�������
        std::cout << "simdisk is running as a background process with PID: " << pid << std::endl;
        exit(EXIT_SUCCESS);
    }

    // �ӽ��̼���ִ�У���Ϊ�ػ�����
    setsid(); // �����µĻỰ

    // ����ԭ�еĳ�ʼ���߼�
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

    // ��ʼ�������ڴ���ź���
    // ...

        // ��ʼ�������ڴ�
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

    // ��ʼ���ź���
    sem_t* semCommand = sem_open(SEM_COMMAND, O_CREAT, 0666, 1);
    sem_t* semResponse = sem_open(SEM_RESPONSE, O_CREAT, 0666, 0);

    if (semCommand == SEM_FAILED || semResponse == SEM_FAILED) {
        std::cerr << "Failed to create semaphores: " << strerror(errno) << std::endl;
        exit(EXIT_FAILURE);
    }

    // ��ʼ�������ڴ��еı�־
    sharedMemory->commandReady = false;
    sharedMemory->responseReady = false;

    // ��ѭ��
    while (true) {
        // ��¼ѭ��
        while (!userManager.isLoggedIn()) {
            // �ӹ����ڴ��ȡ�û���������
            std::string username, password;

            // �ȴ����� Shell �ĵ�¼��������ʽΪ "login username password"
            sem_wait(semResponse); // �ȴ�����׼����

            if (sharedMemory->commandReady) {
                std::string command(sharedMemory->command);
                sharedMemory->commandReady = false;

                // ������¼����
                std::istringstream iss(command);
                std::string cmd;
                iss >> cmd;
                if (cmd == "login") {
                    iss >> username >> password;

                    if (userManager.login(username, password)) {
                        std::string response = "Login successful. Welcome, " + username + "!";
                        strncpy(sharedMemory->response, response.c_str(), sizeof(sharedMemory->response));
                        sharedMemory->responseReady = true;
                        // ֪ͨ Shell ��Ӧ��׼����
                        sem_post(semCommand);
                        break; // �˳���¼ѭ�������������ѭ��
                    }
                    else {
                        std::string response = "Login failed. Invalid username or password.";
                        strncpy(sharedMemory->response, response.c_str(), sizeof(sharedMemory->response));
                        sharedMemory->responseReady = true;
                        // ֪ͨ Shell ��Ӧ��׼����
                        sem_post(semCommand);
                    }
                }
                else {
                    std::string response = "Please login first using 'login <username> <password>'.";
                    strncpy(sharedMemory->response, response.c_str(), sizeof(sharedMemory->response));
                    sharedMemory->responseReady = true;
                    // ֪ͨ Shell ��Ӧ��׼����
                    sem_post(semCommand);
                }
            }
        }

        // �����û�����
        while (userManager.isLoggedIn()) {
            // �ӹ����ڴ��ȡ����
            sem_wait(semResponse); // �ȴ�����׼����

            if (sharedMemory->commandReady) {
                std::string command(sharedMemory->command);
                sharedMemory->commandReady = false;

                if (command == "EXIT") {
                    std::string response = "EXIT successful. Welcome to next use.";
                    strncpy(sharedMemory->response, response.c_str(), sizeof(sharedMemory->response));
                    sharedMemory->responseReady = true;
                    sem_post(semCommand);
                    // ������Դ���˳�
                    shmdt(sharedMemory);
                    shmctl(shmid, IPC_RMID, nullptr);
                    sem_close(semCommand);
                    sem_close(semResponse);
                    sem_unlink(SEM_COMMAND);
                    sem_unlink(SEM_RESPONSE);
                    return 0;
                }

                bool shouldLogout = cmdHandler.handleCommand(command);

                // ��ȡ���������
                std::string response = cmdHandler.getLastOutput();

                // ��չ����ڴ��е���Ӧ������
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
                    break; // ���ص�¼ѭ��
                }
            }
        }
    }

    // ������Դ���˳�
    shmdt(sharedMemory);
    shmctl(shmid, IPC_RMID, nullptr);
    sem_close(semCommand);
    sem_close(semResponse);
    sem_unlink(SEM_COMMAND);
    sem_unlink(SEM_RESPONSE);


}
