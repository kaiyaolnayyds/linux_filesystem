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

#define SHM_KEY 0x1234    // �����ڴ�ļ�ֵ�����Ը�����Ҫ�޸�
#define COMMAND_SIZE 256
#define RESPONSE_SIZE 2048

// ���干���ڴ��е����ݽṹ
struct SharedMemorySegment {
    char command[COMMAND_SIZE];     // �����ַ���
    char response[RESPONSE_SIZE];   // ��Ӧ��Ϣ
    int commandReady;               // ���������־��0��δ������1������
    int responseReady;              // ��Ӧ������־��0��δ������1������
};

int main() {
   // // 1. �ػ�������
   // pid_t pid = fork();
   // if (pid < 0) {
   //     std::cerr << "Failed to fork." << std::endl;
   //     exit(EXIT_FAILURE);
   // }
   // if (pid > 0) {
   //     // �������˳�
   //     std::cout << "SimDisk daemon started with PID: " << pid << std::endl;
   //     exit(EXIT_SUCCESS);
   // }

   // // �ӽ��̼�������Ϊ�ػ�����
   // if (setsid() < 0) {
   //     std::cerr << "Failed to setsid." << std::endl;
   //     exit(EXIT_FAILURE);
   // }

   // // �����ź�
   // signal(SIGCHLD, SIG_IGN);
   // signal(SIGHUP, SIG_IGN);

   // // ���Ĺ���Ŀ¼
   //// chdir("/");

   // // �����ļ�Ȩ������
   // umask(0);

   // // �ر��ļ�������
   // // ��Ҫ�رձ�׼�����������Ϊ��Ҫ��־�������ѡ��
   // // for (int x = sysconf(_SC_OPEN_MAX); x>=0; x--) {
   // //     close(x);
   // // }

    // 2. ��ʼ�������ڴ���ź���
    int shm_id = shmget(SHM_KEY, sizeof(SharedMemorySegment), IPC_CREAT | 0666);
    if (shm_id == -1) {
        // ���������ڴ�ʧ��
        std::cout << "Size of SharedMemorySegment: " << sizeof(SharedMemorySegment) << std::endl;
        std::cerr << "Failed to create shared memory. Error: " << strerror(errno) << std::endl;
        exit(EXIT_FAILURE);
    }

    SharedMemorySegment* shm_ptr = (SharedMemorySegment*)shmat(shm_id, NULL, 0);
    if (shm_ptr == (void*)-1) {
        // ���ӹ����ڴ�ʧ��
        std::cout << "link to sharedmemory failed." << std::endl;
        exit(EXIT_FAILURE);
    }

    // ��ʼ�������ڴ�����
    memset(shm_ptr, 0, sizeof(SharedMemorySegment));

    // �����ź���
    sem_t* command_sem = sem_open("/simdisk_command_sem", O_CREAT, 0666, 1);
    sem_t* response_sem = sem_open("/simdisk_response_sem", O_CREAT, 0666, 1);

    if (command_sem == SEM_FAILED || response_sem == SEM_FAILED) {
        // �ź�������ʧ��
        std::cout << "create signal failed." << std::endl;
        exit(EXIT_FAILURE);
    }

    // 3. �������̶���
    DiskManager diskManager("simdisk.bin", 1024, 100);

    // �������ļ��Ƿ����
    std::ifstream diskFileCheck("simdisk.bin", std::ios::binary);
    if (!diskFileCheck.good()) {
        // �����ļ������ڣ���ʼ���ļ�ϵͳ
        diskManager.initialize();
        // �����ڴ������־��¼
    }
    else {
        // ���س������λͼ
        diskManager.loadSuperBlock();
        diskManager.loadBitmaps();
        // �����ڴ������־��¼
    }

    UserManager userManager; // ���� UserManager ����
    CommandHandler cmdHandler(diskManager, userManager); // �� userManager ���ݸ� CommandHandler

    // ��̨���̵ĻỰ״̬
    bool isLoggedIn = false;

    // 4. ��ѭ�����ȴ�����
    while (true) {
        // �ȴ������ź���
        sem_wait(command_sem);

        if (shm_ptr->commandReady == 1) {
            // ��ȡ����
            std::string command = shm_ptr->command;
            shm_ptr->commandReady = 0; // �������������־

            // �ͷ������ź���
            sem_post(command_sem);

            // ��������
            std::string response;

            if (!isLoggedIn) {
                // ��δ��¼����Ҫ�����¼����
                if (command.substr(0, 5) == "login") {
                    // �����ʽ��login username password
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
                // �ѵ�¼��������������
                if (command == "logout") {
                    userManager.logout();
                    isLoggedIn = false;
                    response = "Logout successful.";
                }
                else if (command == "shutdown") {
                    // ������Դ���˳�
                    response = "SimDisk daemon is shutting down.";
                    // д����Ӧ
                    sem_wait(response_sem);
                    strncpy(shm_ptr->response, response.c_str(), RESPONSE_SIZE - 1);
                    shm_ptr->response[RESPONSE_SIZE - 1] = '\0';
                    shm_ptr->responseReady = 1;
                    sem_post(response_sem);
                    break; // �˳���ѭ��
                }
                else {
                    // ����һ������
                    cmdHandler.setLastOutput(""); // �����һ�ε����
                    bool shouldLogout = cmdHandler.handleCommand(command);
                    response = cmdHandler.getLastOutput();

                    // ����û����������ע������Ҫ���»Ự״̬
                    if (shouldLogout) {
                        isLoggedIn = false;
                    }
                }
            }

            // д����Ӧ
            sem_wait(response_sem);
            strncpy(shm_ptr->response, response.c_str(), RESPONSE_SIZE - 1);
            shm_ptr->response[RESPONSE_SIZE - 1] = '\0'; // ȷ���ַ����� null ��β
            shm_ptr->responseReady = 1;
            sem_post(response_sem);

        }
        else {
            // û��������ͷ������ź���
            sem_post(command_sem);
            // ˯�ߣ�����æ�ȴ�
            usleep(100000); // 100ms
        }
    }

    // 5. ������Դ
    shmdt(shm_ptr); // ���빲���ڴ�
    shmctl(shm_id, IPC_RMID, NULL); // ɾ�������ڴ�

    sem_close(command_sem);
    sem_unlink("/simdisk_command_sem");

    sem_close(response_sem);
    sem_unlink("/simdisk_response_sem");

    exit(EXIT_SUCCESS);
}
