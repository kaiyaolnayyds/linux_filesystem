// src/CommandHandler.cpp
#include "CommandHandler.h"
#include "Directory.h"
#include "INode.h"
#include "SuperBlock.h"
#include <iostream>
#include <sstream>
#include "SuperBlock.h"

void CommandHandler::handleCommand(const std::string& command) {
    std::istringstream iss(command);
    std::string cmd;
    iss >> cmd;

    if (cmd == "info") {
        handleInfo();
    }
    else if (cmd == "cd") {
        std::string path;
        iss >> path;
        handleCd(path);
    }
    else if (cmd == "dir") {
        std::string path;
        iss >> path;
        handleDir(path);
    }
    else if (cmd == "md") {
        std::string dirName;
        iss >> dirName;
        handleMd(dirName);
    }
    else if (cmd == "rd") {
        std::string dirName;
        iss >> dirName;
        handleRd(dirName);
    }
    else if (cmd == "newfile") {
        std::string fileName;
        iss >> fileName;
        handleNewFile(fileName);
    }
    else if (cmd == "cat") {
        std::string fileName;
        iss >> fileName;
        handleCat(fileName);
    }
    else if (cmd == "copy") {
        std::string src, dest;
        iss >> src >> dest;
        handleCopy(src, dest);
    }
    else if (cmd == "del") {
        std::string fileName;
        iss >> fileName;
        handleDel(fileName);
    }
    else if (cmd == "check") {
        handleCheck();
    }
    else {
        std::cout << "Unknown command: " << cmd << std::endl;
    }
}

void CommandHandler::handleInfo() {
    // ��ȡ��������Ϣ
    SuperBlock superBlock;
    std::ifstream file(diskManager.diskFile, std::ios::binary);
    if (file.is_open()) {
        file.seekg(0);
        file.read(reinterpret_cast<char*>(&superBlock), sizeof(SuperBlock));
        file.close();
    }
    else {
        std::cerr << "Error: Unable to open disk file." << std::endl;
        return;
    }

    // ��ӡϵͳ��Ϣ
    std::cout << "=== File System Information ===" << std::endl;
    std::cout << "Total Blocks: " << superBlock.totalBlocks << std::endl;
    std::cout << "Free Blocks: " << superBlock.freeBlocks << std::endl;
    std::cout << "Total iNodes: " << superBlock.inodeCount << std::endl;
    std::cout << "Root Directory iNode: " << superBlock.rootInode << std::endl;
    std::cout << "================================" << std::endl;
}

void CommandHandler::handleCd(const std::string& path) {
    // �߼����ı䵱ǰĿ¼������������߼���DiskManager�У�
    std::cout << "Changing directory to: " << path << std::endl;
}

void CommandHandler::handleDir(const std::string& path) {
    // �߼�����ʾĿ¼����
    std::cout << "Listing directory: " << path << std::endl;
}

void CommandHandler::handleMd(const std::string& dirName) {
    // �߼���������Ŀ¼
    std::cout << "Creating directory: " << dirName << std::endl;
}

void CommandHandler::handleRd(const std::string& dirName) {
    // �߼���ɾ��Ŀ¼��������
    std::cout << "Removing directory: " << dirName << std::endl;
}

void CommandHandler::handleNewFile(const std::string& fileName) {
    // �߼����������ļ�
    std::cout << "Creating new file: " << fileName << std::endl;
}

void CommandHandler::handleCat(const std::string& fileName) {
    // �߼�����ʾ�ļ�����
    std::cout << "Displaying file: " << fileName << std::endl;
}

void CommandHandler::handleCopy(const std::string& src, const std::string& dest) {
    // �߼��������ļ�
    std::cout << "Copying from " << src << " to " << dest << std::endl;
}

void CommandHandler::handleDel(const std::string& fileName) {
    // �߼���ɾ���ļ�
    std::cout << "Deleting file: " << fileName << std::endl;
}

void CommandHandler::handleCheck() {
    // �߼�����Ⲣ�ָ��ļ�ϵͳ
    std::cout << "Checking and restoring file system." << std::endl;
}

std::vector<std::string> CommandHandler::parsePath(const std::string& path) {
    std::vector<std::string> components;
    std::istringstream stream(path);
    std::string token;
    while (std::getline(stream, token, '/')) {
        if (!token.empty()) {
            components.push_back(token);
        }
    }
    return components;
}
