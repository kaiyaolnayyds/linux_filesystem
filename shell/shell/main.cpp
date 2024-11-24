// shell.cpp

#include <iostream>
#include <string>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <semaphore.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>

#define SHM_KEY 0x1234  // ������Ч�ļ�ֵ
#define COMMAND_SIZE 256
#define RESPONSE_SIZE 2048

struct SharedMemorySegment {
    char command[COMMAND_SIZE];
    char response[RESPONSE_SIZE];
    int commandReady;
    int responseReady;
};

int main() {
    // ���ӹ����ڴ�
    int shm_id = shmget(SHM_KEY, sizeof(SharedMemorySegment), 0666);
    if (shm_id == -1) {
        std::cerr << "Failed to get shared memory." << std::endl;
        exit(EXIT_FAILURE);
    }

    SharedMemorySegment* shm_ptr = (SharedMemorySegment*)shmat(shm_id, NULL, 0);
    if (shm_ptr == (void*)-1) {
        std::cerr << "Failed to attach shared memory." << std::endl;
        exit(EXIT_FAILURE);
    }

    // ���ź���
    sem_t* command_sem = sem_open("/simdisk_command_sem", 0);
    sem_t* response_sem = sem_open("/simdisk_response_sem", 0);

    if (command_sem == SEM_FAILED || response_sem == SEM_FAILED) {
        std::cerr << "Failed to open semaphores." << std::endl;
        exit(EXIT_FAILURE);
    }

    // �Ự״̬
    bool isLoggedIn = false;

    // ��¼ѭ��
    while (!isLoggedIn) {
        std::string username, password;

        std::cout << "SimDisk Login\nUsername: ";
        std::getline(std::cin, username);
        std::cout << "Password: ";
        std::getline(std::cin, password);

        // ���͵�¼�����̨����
        std::string loginCommand = "login " + username + " " + password;

        // ��������
        sem_wait(command_sem);
        strncpy(shm_ptr->command, loginCommand.c_str(), COMMAND_SIZE - 1);
        shm_ptr->command[COMMAND_SIZE - 1] = '\0';
        shm_ptr->commandReady = 1;
        sem_post(command_sem);

        // �ȴ���Ӧ
        while (true) {
            sem_wait(response_sem);
            if (shm_ptr->responseReady == 1) {
                // ��ȡ��Ӧ
                std::string response = shm_ptr->response;
                shm_ptr->responseReady = 0;
                sem_post(response_sem);
                std::cout << response << std::endl;

                if (response.find("Login successful") != std::string::npos) {
                    isLoggedIn = true;
                }
                break;
            }
            sem_post(response_sem);
            // ˯�ߣ�����æ�ȴ�
            usleep(100000); // 100ms
        }
    }

    // ��ѭ��
    while (true) {
        std::cout << "simdisk> ";
        std::string userInput;
        std::getline(std::cin, userInput);

        if (userInput == "exit") {
            // ����ѡ���� logout ����
            // ��������
            sem_wait(command_sem);
            strncpy(shm_ptr->command, "logout", COMMAND_SIZE - 1);
            shm_ptr->command[COMMAND_SIZE - 1] = '\0';
            shm_ptr->commandReady = 1;
            sem_post(command_sem);

            // �ȴ���Ӧ
            while (true) {
                sem_wait(response_sem);
                if (shm_ptr->responseReady == 1) {
                    // ��ȡ��Ӧ
                    std::string response = shm_ptr->response;
                    shm_ptr->responseReady = 0;
                    sem_post(response_sem);
                    std::cout << response << std::endl;
                    break;
                }
                sem_post(response_sem);
                usleep(100000); // 100ms
            }
            break;
        }

        // ���������̨����
        sem_wait(command_sem);
        strncpy(shm_ptr->command, userInput.c_str(), COMMAND_SIZE - 1);
        shm_ptr->command[COMMAND_SIZE - 1] = '\0';
        shm_ptr->commandReady = 1;
        sem_post(command_sem);

        // �ȴ���Ӧ
        while (true) {
            sem_wait(response_sem);
            if (shm_ptr->responseReady == 1) {
                // ��ȡ��Ӧ
                std::string response = shm_ptr->response;
                shm_ptr->responseReady = 0;
                sem_post(response_sem);
                std::cout << response << std::endl;
                break;
            }
            sem_post(response_sem);
            // ˯�ߣ�����æ�ȴ�
            usleep(100000); // 100ms
        }
    }

    // ������Դ
    shmdt(shm_ptr); // ���빲���ڴ�

    sem_close(command_sem);
    sem_close(response_sem);

    return 0;
}
