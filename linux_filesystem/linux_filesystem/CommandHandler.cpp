// src/CommandHandler.cpp
#include "CommandHandler.h"
#include <iostream>
#include <sstream>


CommandHandler::CommandHandler(DiskManager& dm) : diskManager(dm) {
    // ���ظ�Ŀ¼
    SuperBlock superBlock = diskManager.loadSuperBlock();
    uint32_t rootInode = superBlock.rootInode;

    // ��ȡ��Ŀ¼�� inode
    currentInodeIndex = rootInode;
    INode rootInodeObj = diskManager.readINode(rootInode);

    // �Ӵ��̶�ȡ��Ŀ¼����
    char buffer[diskManager.blockSize];
    diskManager.readBlock(rootInodeObj.blockIndex, buffer);

    // �����л���Ŀ¼
    currentDirectory.deserialize(buffer, diskManager.blockSize);

    // �������
    std::cout << "[DEBUG] Loaded current directory entries:" << std::endl;
    for (const auto& entry : currentDirectory.entries) {
        std::cout << "  " << entry.first << " -> inode " << entry.second << std::endl;
    }
}


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
        std::string arg;
        bool recursive = false;
        std::string path;

        // �������в���
        while (iss >> arg) {
            if (arg == "/s") {
                recursive = true;
            }
            else {
                path = arg;
            }
        }

        handleDir(path, recursive);
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
    SuperBlock superBlock = diskManager.loadSuperBlock();

    // ��ӡ��������Ϣ
    std::cout << "=== File System Information ===" << std::endl;
    std::cout << "Total Blocks: " << superBlock.totalBlocks << std::endl;
    std::cout << "Free Blocks: " << superBlock.freeBlocks << std::endl;
    std::cout << "Total iNodes: " << superBlock.inodeCount << std::endl;
    std::cout << "Root Directory iNode: " << superBlock.rootInode << std::endl;
    std::cout << "================================" << std::endl;
}


void CommandHandler::handleCd(const std::string& path) {
    try {
        std::vector<std::string> components = parsePath(path); // ����·��
        uint32_t currentInode = 0; // �Ӹ�Ŀ¼��ʼ
        Directory tempDirectory = currentDirectory; // ��ʱĿ¼����

        for (const auto& component : components) {
            uint32_t nextInode = tempDirectory.findEntry(component); // ������Ŀ¼
            char buffer[1024];
            diskManager.readBlock(nextInode, buffer); // �Ӵ��̶�ȡ��Ŀ¼����
            tempDirectory = *reinterpret_cast<Directory*>(buffer); // ���µ�ǰĿ¼
        }

        currentDirectory = tempDirectory; // ���µ�ǰĿ¼
        std::cout << "Changed to directory: " << path << "." << std::endl;
    }
    catch (const std::exception& e) {
        std::cerr << "Error changing directory: " << e.what() << std::endl;
    }
}



void CommandHandler::handleDir(const std::string& path, bool recursive) {
    // ȷ��Ҫ��ʾ��Ŀ¼
    Directory dirToDisplay = currentDirectory;
    uint32_t dirInodeIndex = currentInodeIndex;

    if (!path.empty()) {
        // ����ָ����·��
        // ������Ҫʵ��·������������֧�����·��������·����
        // Ϊ�򻯣�����ֻ֧�ֵ�ǰĿ¼�µ�ֱ����Ŀ¼
        auto it = currentDirectory.entries.find(path);
        if (it != currentDirectory.entries.end()) {
            uint32_t inodeIndex = it->second;
            INode inode = diskManager.readINode(inodeIndex);
            if (inode.type != 1) {
                std::cout << path << " is not a directory." << std::endl;
                return;
            }
            // ��ȡĿ¼����
            char buffer[diskManager.blockSize];
            diskManager.readBlock(inode.blockIndex, buffer);
            dirToDisplay.deserialize(buffer, diskManager.blockSize);
            dirInodeIndex = inodeIndex;
        }
        else {
            std::cout << "Directory not found: " << path << std::endl;
            return;
        }
    }

    // ��ʾĿ¼����
    displayDirectoryContents(dirToDisplay, dirInodeIndex, recursive, "");
}

void CommandHandler::displayDirectoryContents(const Directory& dir, uint32_t dirInodeIndex, bool recursive, const std::string& indent) {
    if (dir.entries.empty()) {
        std::cout << indent << "Directory is empty." << std::endl;
    }
    else {
        for (const auto& entry : dir.entries) {
            const std::string& name = entry.first;
            uint32_t inodeIndex = entry.second;
            INode inode = diskManager.readINode(inodeIndex);

            // ��ʾ�ļ����������ַ�������롢�ļ����ȡ���Ŀ¼��
            std::string typeStr = (inode.type == 1) ? "<DIR>" : "<FILE>";
            std::cout << indent << typeStr << " " << name
                << " [Addr: " << inode.blockIndex
                << ", Mode: " << std::oct << inode.mode << std::dec
                << ", Size: " << inode.size << "]" << std::endl;

            // �����Ŀ¼����Ҫ�ݹ�
            if (recursive && inode.type == 1) {
                // ��ȡ��Ŀ¼����
                Directory subDir;
                char buffer[diskManager.blockSize];
                diskManager.readBlock(inode.blockIndex, buffer);
                subDir.deserialize(buffer, diskManager.blockSize);

                // �ݹ���ʾ��Ŀ¼����
                displayDirectoryContents(subDir, inodeIndex, recursive, indent + "  ");
            }
        }
    }
}


void CommandHandler::handleMd(const std::string& dirName) {
    try {
        size_t blockIndex = diskManager.allocateBlock(); // �����
        if (blockIndex == static_cast<size_t>(-1)) {
            std::cerr << "Error: No free blocks available to create directory." << std::endl;
            return;
        }

        currentDirectory.addEntry(dirName, static_cast<uint32_t>(blockIndex), diskManager);
        std::cout << "Directory '" << dirName << "' created at block " << blockIndex << "." << std::endl;

        // ���غ͸��³�����
        SuperBlock superBlock = diskManager.loadSuperBlock();
        superBlock.freeBlocks -= 1; // ���ٿ��п�
        superBlock.inodeCount += 1; // ���� iNode ����
        diskManager.updateSuperBlock(superBlock);

    }
    catch (const std::exception& e) {
        std::cerr << "Error creating directory: " << e.what() << std::endl;
    }
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

