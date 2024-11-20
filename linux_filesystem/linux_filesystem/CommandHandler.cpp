// src/CommandHandler.cpp
#include "CommandHandler.h"
#include <iostream>
#include <sstream>


CommandHandler::CommandHandler(DiskManager& dm) : diskManager(dm) {
    // ���س�����
    SuperBlock superBlock = diskManager.loadSuperBlock();
    uint32_t rootInodeIndex = superBlock.rootInode;
    currentInodeIndex = rootInodeIndex;

    // ��ȡ��Ŀ¼�� inode
    INode rootInode = diskManager.readINode(rootInodeIndex);

    // �Ӵ��̶�ȡ��Ŀ¼����
    char buffer[diskManager.blockSize];
    diskManager.readBlock(rootInode.blockIndex, buffer);

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
    if (path.empty()) {
        std::cout << "Usage: cd <path>" << std::endl;
        return;
    }

    // ����·��
    std::vector<std::string> components = parsePath(path);

    // �ж��Ƿ�Ϊ����·��
    bool isAbsolutePath = !path.empty() && path[0] == '/';

    uint32_t startInodeIndex;
    Directory startDirectory;

    if (isAbsolutePath) {
        // �Ӹ�Ŀ¼��ʼ
        startInodeIndex = diskManager.superBlock.rootInode;
        if (!getDirectory(startInodeIndex, startDirectory)) {
            std::cout << "Error: Unable to load root directory." << std::endl;
            return;
        }
    }
    else {
        // �ӵ�ǰĿ¼��ʼ
        startInodeIndex = currentInodeIndex;
        startDirectory = currentDirectory;
    }

    uint32_t targetInodeIndex = startInodeIndex;
    Directory targetDirectory = startDirectory;

    for (const std::string& component : components) {
        if (component == ".") {
            // ��ǰĿ¼������
            continue;
        }
        else if (component == "..") {
            // �ϼ�Ŀ¼
            if (targetInodeIndex == diskManager.superBlock.rootInode) {
                // �Ѿ��Ǹ�Ŀ¼���޷������ϼ�
                continue;
            }
            targetInodeIndex = targetDirectory.parentInodeIndex;
            if (!getDirectory(targetInodeIndex, targetDirectory)) {
                std::cout << "Error: Unable to load parent directory." << std::endl;
                return;
            }
        }
        else {
            auto it = targetDirectory.entries.find(component);
            if (it == targetDirectory.entries.end()) {
                // δ�ҵ�Ŀ¼
                std::cout << "Directory not found: " << component << std::endl;
                return;
            }

            uint32_t inodeIndex = it->second;
            INode inode = diskManager.readINode(inodeIndex);

            if (inode.type != 1) {
                // ����Ŀ¼
                std::cout << component << " is not a directory." << std::endl;
                return;
            }

            if (!getDirectory(inodeIndex, targetDirectory)) {
                std::cout << "Error: Unable to load directory " << component << std::endl;
                return;
            }

            targetInodeIndex = inodeIndex;
        }
    }

    // ���µ�ǰĿ¼
    currentInodeIndex = targetInodeIndex;
    currentDirectory = targetDirectory;
    std::cout << "Changed directory to: " << path << std::endl;
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

            // ������� inode ��Ϣ
            std::cout << "[DEBUG] INode for '" << name << "': size=" << inode.size << ", mode=" << inode.mode
                << ", type=" << static_cast<int>(inode.type) << ", blockIndex=" << inode.blockIndex << ", inodeIndex=" << inode.inodeIndex << std::endl;

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

bool CommandHandler::findDirectory(const std::vector<std::string>& pathComponents, uint32_t& resultInodeIndex, Directory& resultDirectory)
{
    uint32_t inodeIndex = (pathComponents.empty() || pathComponents[0].empty()) ? diskManager.superBlock.rootInode : currentInodeIndex;
    Directory directory;

    if (!getDirectory(inodeIndex, directory)) {
        return false;
    }

    for (const std::string& component : pathComponents) {
        if (component == ".") {
            // ��ǰĿ¼������
            continue;
        }
        else if (component == "..") {
            // �ϼ�Ŀ¼����δʵ�֣��򵥴���Ϊ��Ŀ¼
            inodeIndex = diskManager.superBlock.rootInode;
            if (!getDirectory(inodeIndex, directory)) {
                return false;
            }
            continue;
        }
        else {
            auto it = directory.entries.find(component);
            if (it == directory.entries.end()) {
                // δ�ҵ�Ŀ¼
                return false;
            }

            inodeIndex = it->second;
            INode inode = diskManager.readINode(inodeIndex);

            if (inode.type != 1) {
                // ����Ŀ¼
                return false;
            }

            if (!getDirectory(inodeIndex, directory)) {
                return false;
            }
        }
    }

    resultInodeIndex = inodeIndex;
    resultDirectory = directory;
    return true;
}

bool CommandHandler::getDirectory(uint32_t inodeIndex, Directory& directory)
{
    INode inode = diskManager.readINode(inodeIndex);

    if (inode.type != 1) {
        return false;
    }

    char buffer[diskManager.blockSize];
    diskManager.readBlock(inode.blockIndex, buffer);

    try {
        directory.deserialize(buffer, diskManager.blockSize);
    }
    catch (const std::exception& e) {
        std::cerr << "Error deserializing directory: " << e.what() << std::endl;
        return false;
    }

    return true;
}



void CommandHandler::handleMd(const std::string& dirName) {
    // ���Ŀ¼�Ƿ��Ѵ���
    if (currentDirectory.entries.find(dirName) != currentDirectory.entries.end()) {
        std::cout << "Directory '" << dirName << "' already exists." << std::endl;
        return;
    }

    // ����һ���µ� inode
    uint32_t newInodeIndex = diskManager.superBlock.inodeCount++;
    INode newInode;
    newInode.size = 0;
    newInode.mode = 0755; // Ĭ��Ȩ��
    newInode.type = 1;    // Ŀ¼����
    newInode.inodeIndex = newInodeIndex;

    // ����һ���µ����ݿ����Ŀ¼
    size_t newBlockIndex = diskManager.allocateBlock();
    if (newBlockIndex == static_cast<size_t>(-1)) {
        std::cout << "Failed to allocate block for new directory." << std::endl;
        return;
    }
    newInode.blockIndex = static_cast<uint32_t>(newBlockIndex);

    // ���³�����
    diskManager.superBlock.freeBlocks--;
    diskManager.updateSuperBlock(diskManager.superBlock);

    // ����Ŀ¼�� inode д�����
    diskManager.writeINode(newInodeIndex, newInode);

    // ������Ŀ¼����� `.` �� `..` ��Ŀ
    Directory newDirectory;
    newDirectory.parentInodeIndex = currentInodeIndex; // ���ø�Ŀ¼�� inodeIndex
    newDirectory.entries["."] = newInodeIndex;         // ��ǰĿ¼
    newDirectory.entries[".."] = currentInodeIndex;    // ��Ŀ¼

    // ����Ŀ¼���л���д�����
    std::vector<char> buffer;
    newDirectory.serialize(buffer, diskManager.blockSize);
    diskManager.writeBlock(newInode.blockIndex, buffer.data());

    // ���µ�ǰĿ¼�� entries
    currentDirectory.entries[dirName] = newInodeIndex;

    // �����º�ĵ�ǰĿ¼д�ش���
    // ��ȡ��ǰĿ¼�� inode
    INode currentDirInode = diskManager.readINode(currentInodeIndex);

    // ���л���ǰĿ¼��д�����
    buffer.clear();
    currentDirectory.serialize(buffer, diskManager.blockSize);
    diskManager.writeBlock(currentDirInode.blockIndex, buffer.data());

    // ����ǰĿ¼�� inode д�ش��̣������Ҫ���� size ����Ϣ��
    diskManager.writeINode(currentInodeIndex, currentDirInode);

    // ���������Ŀ¼�� inode ��Ϣ
    std::cout << "[DEBUG] New directory INode: size=" << newInode.size << ", mode=" << newInode.mode
        << ", type=" << static_cast<int>(newInode.type) << ", blockIndex=" << newInode.blockIndex
        << ", inodeIndex=" << newInode.inodeIndex << std::endl;

    std::cout << "Directory '" << dirName << "' created at block " << newBlockIndex << "." << std::endl;
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
