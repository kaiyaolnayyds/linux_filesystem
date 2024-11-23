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

    // ��ʼ����ǰ·��Ϊ��Ŀ¼
    currentPath = "/";
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
        path;
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
    else if (cmd == "writefile") {
        std::string filePath;
        std::string mode;
        iss >> filePath >> mode;
        bool append = (mode == "append");
        handleWriteFile(filePath, append);
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
    std::cout << "File System Information:" << std::endl;

    // �ڻ�ȡ��������Ϣ֮ǰ������ʹ�����
    diskManager.updateSuperBlockUsage();
    // ��ȡ��������Ϣ
    const SuperBlock& sb = diskManager.superBlock;

    // ��ȡ�ܿ����Ϳ��п���
    uint32_t totalBlocks = sb.totalBlocks;
    uint32_t freeBlocks = sb.freeBlocks;
    uint32_t usedBlocks = totalBlocks - freeBlocks;

    // ��ȡ���С
    size_t blockSize = diskManager.blockSize;

    // ��������������ʹ�ÿռ�Ϳ��ÿռ䣨���ֽ�Ϊ��λ��
    uint64_t totalSize = static_cast<uint64_t>(totalBlocks) * blockSize;
    uint64_t usedSize = static_cast<uint64_t>(usedBlocks) * blockSize;
    uint64_t freeSize = static_cast<uint64_t>(freeBlocks) * blockSize;

    // ��ȡ�ѷ���� inode ����
    uint32_t allocatedInodes = sb.inodeCount;
    uint32_t freeInodes = MAX_INODES - allocatedInodes;

    // ��ʾ��Ϣ
    std::cout << "----------------------------------------" << std::endl;
    std::cout << "Block Size       : " << blockSize << " bytes" << std::endl;
    std::cout << "Total Blocks     : " << totalBlocks << std::endl;
    std::cout << "Used Blocks      : " << usedBlocks << std::endl;
    std::cout << "Free Blocks      : " << freeBlocks << std::endl;
    std::cout << "Total Size       : " << totalSize << " bytes" << std::endl;
    std::cout << "Used Size        : " << usedSize << " bytes" << std::endl;
    std::cout << "Free Size        : " << freeSize << " bytes" << std::endl;
    std::cout << "----------------------------------------" << std::endl;
    std::cout << "Total Inodes     : " << MAX_INODES << std::endl;
    std::cout << "Allocated Inodes : " << allocatedInodes << std::endl;
    std::cout << "Free Inodes      : " << freeInodes << std::endl;
    std::cout << "----------------------------------------" << std::endl;
    std::cout << "Root Inode Index : " << sb.rootInode << std::endl;
    std::cout << "INode Start Addr : " << sb.inodeStartAddress << std::endl;
    std::cout << "Data Block Start : " << sb.dataBlockStartAddress << std::endl;
    std::cout << "----------------------------------------" << std::endl;
}


void CommandHandler::handleCd(const std::string& path) {
    uint32_t newInodeIndex;
    Directory newDirectory;

    if (!navigateToPath(path, newInodeIndex, newDirectory)) {
        // ����ʧ�ܣ����� navigateToPath �����������Ϣ
        return;
    }

    // ���µ�ǰĿ¼�� inode ����
    currentDirectory = newDirectory;
    currentInodeIndex = newInodeIndex;

    // ���� currentPath
    if (path[0] == '/') {
        // ����·��
        currentPath = path;
    }
    else {
        if (currentPath != "/" && !currentPath.empty()) {
            currentPath += "/";
        }
        currentPath += path;
    }

    // �淶�� currentPath������ `.` �� `..`
    std::vector<std::string> pathComponents = parsePath_normal(currentPath);
    currentPath.clear();
    for (const auto& comp : pathComponents) {
        currentPath += "/";
        currentPath += comp;
    }

    // ���·��Ϊ�գ�����Ϊ��Ŀ¼
    if (currentPath.empty()) {
        currentPath = "/";
    }

    std::cout << "Changed directory to: " << currentPath << std::endl;
}



std::vector<std::string> CommandHandler::parsePath(const std::string& path) {
    
    std::vector<std::string> components;
    if (path.empty()) {
        return components;
    }

    std::istringstream stream(path);
    std::string token;
    while (std::getline(stream, token, '/')) {
        components.push_back(token);
    }
    return components;
}

std::vector<std::string> CommandHandler::parsePath_normal(const std::string& path) {
    std::vector<std::string> components;
    if (path.empty()) {
        return components;
    }

    std::istringstream stream(path);
    std::string token;
    while (std::getline(stream, token, '/')) {
        if (token == "." || token.empty()) {
            continue; // ���Ե�ǰĿ¼���źͿ��ַ���
        }
        else if (token == "..") {
            if (!components.empty()) {
                components.pop_back(); // ������һ��Ŀ¼
            }
        }
        else {
            components.push_back(token);
        }
    }
    return components;
}


void CommandHandler::handleDir(const std::string& path, bool recursive) {
    uint32_t dirInodeIndex;
    Directory dirToDisplay;

    if (!path.empty()) {
        if (!navigateToPath(path, dirInodeIndex, dirToDisplay)) {
            // ����ʧ�ܣ����� navigateToPath �����������Ϣ
            return;
        }
    }
    else {
        dirInodeIndex = currentInodeIndex;
        dirToDisplay = currentDirectory;
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

Directory CommandHandler::loadDirectoryFromINode(const INode& inode) {
    Directory dir;
    char buffer[diskManager.blockSize];
    diskManager.readBlock(inode.blockIndex, buffer);
    dir.deserialize(buffer, diskManager.blockSize);
    return dir;
}

bool CommandHandler::deleteDirectoryRecursively(uint32_t inodeIndex) {
    INode inode = diskManager.readINode(inodeIndex);

    if (inode.type == 1) {
        // �����Ŀ¼���ݹ�ɾ��������
        Directory dir = loadDirectoryFromINode(inode);

        for (const auto& entry : dir.entries) {
            const std::string& name = entry.first;
            uint32_t childInodeIndex = entry.second;

            // ���� "." �� ".."
            if (name == "." || name == "..") {
                continue;
            }

            // �ݹ�ɾ�����ļ�����Ŀ¼
            if (!deleteDirectoryRecursively(childInodeIndex)) {
                return false;
            }
        }
    }

    // �ͷ����ݿ�
    diskManager.freeBlock(inode.blockIndex);

    // ���³�����
    diskManager.superBlock.freeBlocks++;
    diskManager.updateSuperBlock(diskManager.superBlock);

    // �ͷ� inode
    diskManager.freeINode(inodeIndex);

    return true;
}

bool CommandHandler::navigateToPath(const std::string& path, uint32_t& resultInodeIndex, Directory& resultDirectory) {
    if (path.empty()) {
        std::cout << "Path is empty." << std::endl;
        return false;
    }

    // ����·��
    std::vector<std::string> components = parsePath(path);
    uint32_t inodeIndex;
    Directory dir;
    INode inode;

    if (path[0] == '/') {
        // ����·�����Ӹ�Ŀ¼��ʼ
        inodeIndex = diskManager.superBlock.rootInode;
        inode = diskManager.readINode(inodeIndex);
        dir = loadDirectoryFromINode(inode);
    }
    else {
        // ���·�����ӵ�ǰĿ¼��ʼ
        inodeIndex = currentInodeIndex;
        inode = diskManager.readINode(inodeIndex);
        dir = currentDirectory;
    }

    // ����·�����
    for (const auto& component : components) {
        if (component == "..") {
            // ������һ��Ŀ¼
            if (inodeIndex == diskManager.superBlock.rootInode) {
                // �Ѿ��Ǹ�Ŀ¼���޷�������
                continue;
            }
            // ��ȡ��Ŀ¼�� inode
            auto it = dir.entries.find("..");
            if (it == dir.entries.end()) {
                std::cout << "Cannot move to parent directory." << std::endl;
                return false;
            }

            uint32_t parentInodeIndex = it->second;
            INode parentInode = diskManager.readINode(parentInodeIndex);
            dir = loadDirectoryFromINode(parentInode);
            inodeIndex = parentInodeIndex;
        }
        else if (component == ".") {
            // ��ǰĿ¼������
            continue;
        }
        else {
            // ������Ŀ¼
            auto it = dir.entries.find(component);
            if (it == dir.entries.end()) {
                std::cout << "Directory not found: " << component << std::endl;
                return false;
            }

            uint32_t nextInodeIndex = it->second;
            INode nextInode = diskManager.readINode(nextInodeIndex);

            if (nextInode.type != 1) {
                std::cout << component << " is not a directory." << std::endl;
                return false;
            }

            // ������һ��Ŀ¼
            dir = loadDirectoryFromINode(nextInode);
            inodeIndex = nextInodeIndex;
        }
    }

    // �����ɹ������ؽ��
    resultDirectory = dir;
    resultInodeIndex = inodeIndex;
    return true;
}



void CommandHandler::handleMd(const std::string& dirName) {
    // ���Ŀ¼�Ƿ��Ѵ���
    if (currentDirectory.entries.find(dirName) != currentDirectory.entries.end()) {
        std::cout << "Directory '" << dirName << "' already exists." << std::endl;
        return;
    }

    // ����һ���µ� inode
    uint32_t newInodeIndex = diskManager.allocateINode();
    if (newInodeIndex == static_cast<uint32_t>(-1)) {
        std::cout << "Failed to allocate inode for new directory." << std::endl;
        return;
    }

    INode newInode;
    newInode.size = 0;
    newInode.mode = 0755; // Ĭ��Ȩ��
    newInode.type = 1;    // Ŀ¼����
    newInode.inodeIndex = newInodeIndex;

    // ����һ���µ����ݿ����Ŀ¼
    size_t newBlockIndex = diskManager.allocateBlock();
    if (newBlockIndex == static_cast<size_t>(-1)) {
        std::cout << "Failed to allocate block for new directory." << std::endl;
        // �ͷ��ѷ���� inode
        diskManager.freeINode(newInodeIndex);
        return;
    }
    newInode.blockIndex = static_cast<uint32_t>(newBlockIndex);

    // ���³�����
    //diskManager.superBlock.freeBlocks--;
    diskManager.updateSuperBlock(diskManager.superBlock);

    // ����Ŀ¼�� inode д�����
    diskManager.writeINode(newInodeIndex, newInode);

    // ������Ŀ¼��Ŀ¼��
    Directory newDirectory;
    newDirectory.addEntry(".", newInodeIndex);       // ��ǰĿ¼
    newDirectory.addEntry("..", currentInodeIndex);  // ��Ŀ¼

    // ���л���д�����
    std::vector<char> buffer;
    newDirectory.serialize(buffer, diskManager.blockSize);
    diskManager.writeBlock(newInode.blockIndex, buffer.data());

    /*
    ��������������ʵ���л���Ҫ����Ŀ¼��Ŀ¼�£�·����������cd,�����õ�currentDirectory��currentInodeIndex
    */


    // ���µ�ǰĿ¼�� entries
    currentDirectory.addEntry(dirName, newInodeIndex);

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
    std::cout << "[DEBUG] New directory INode: size=" << newInode.size
        << ", mode=" << newInode.mode
        << ", type=" << static_cast<int>(newInode.type)
        << ", blockIndex=" << newInode.blockIndex
        << ", inodeIndex=" << newInode.inodeIndex << std::endl;

    std::cout << "Directory '" << dirName << "' created at block " << newBlockIndex << "." << std::endl;
}




void CommandHandler::handleRd(const std::string& dirName) {
    // �ڵ�ǰĿ¼�в���Ҫɾ����Ŀ¼�� inode ����
    auto it = currentDirectory.entries.find(dirName);
    if (it == currentDirectory.entries.end()) {
        std::cout << "Directory '" << dirName << "' does not exist in the current directory." << std::endl;
        return;
    }

    uint32_t dirInodeIndex = it->second;
    INode dirInode = diskManager.readINode(dirInodeIndex);

    // ���Ҫɾ�����Ƿ���Ŀ¼
    if (dirInode.type != 1) {
        std::cout << "'" << dirName << "' is not a directory." << std::endl;
        return;
    }

    // ����Ҫɾ����Ŀ¼
    Directory dirToDelete = loadDirectoryFromINode(dirInode);

    // ���Ŀ¼��Ϊ�գ���ʾ�û��Ƿ����ɾ��
    if (dirToDelete.entries.size() > 2) { // ���� "." �� ".." ֮�⻹��������Ŀ
        std::cout << "Directory '" << dirName << "' is not empty. Do you want to delete it? (y/n): ";
        std::string choice;
        std::getline(std::cin, choice);
        if (choice != "y" && choice != "Y") {
            std::cout << "Deletion cancelled." << std::endl;
            return;
        }
    }

    // �ݹ�ɾ��Ŀ¼����
    bool success = deleteDirectoryRecursively(dirInodeIndex);
    if (success) {
        // �ӵ�ǰĿ¼��ɾ����Ŀ¼��Ŀ¼��
        currentDirectory.entries.erase(dirName);

        // �����º�ĵ�ǰĿ¼д�ش���
        // ��ȡ��ǰĿ¼�� inode
        INode currentDirInode = diskManager.readINode(currentInodeIndex);

        // ���л���ǰĿ¼��д�����
        std::vector<char> buffer;
        currentDirectory.serialize(buffer, diskManager.blockSize);
        diskManager.writeBlock(currentDirInode.blockIndex, buffer.data());

        // ����ǰĿ¼�� inode д�ش��̣������Ҫ���� size ����Ϣ��
        diskManager.writeINode(currentInodeIndex, currentDirInode);

        std::cout << "Directory '" << dirName << "' deleted successfully." << std::endl;
    }
    else {
        std::cout << "Failed to delete directory '" << dirName << "'." << std::endl;
    }
}


void CommandHandler::handleNewFile(const std::string& fileName) {
    if (fileName.empty()) {
        std::cout << "File name is empty." << std::endl;
        return;
    }

    // 1. ����ļ��Ƿ��Ѵ���
    if (currentDirectory.entries.find(fileName) != currentDirectory.entries.end()) {
        std::cout << "File '" << fileName << "' already exists in the current directory." << std::endl;
        return;
    }

    // 2. �����µ� inode
    uint32_t newInodeIndex = diskManager.allocateINode();
    if (newInodeIndex == static_cast<uint32_t>(-1)) {
        std::cout << "Failed to allocate inode for new file." << std::endl;
        return;
    }

    // 3. ��ʼ�����ļ��� inode
    INode newInode;
    newInode.size = 0;             // �ļ���СΪ 0
    newInode.mode = 0644;          // Ĭ��Ȩ�ޣ��ɸ�����Ҫ����
    newInode.type = 0;             // �ļ����ͣ�0 ��ʾ��ͨ�ļ�
    newInode.inodeIndex = newInodeIndex;

    // 4. ��ѡ���������ݿ飨���ڿ��ļ������Բ����䣩
    // �����ҪԤ�ȷ���һ�����ݿ飬����ȡ������ע��
    /*
    size_t newBlockIndex = diskManager.allocateBlock();
    if (newBlockIndex == static_cast<size_t>(-1)) {
        std::cout << "Failed to allocate block for new file." << std::endl;
        // �ͷ��ѷ���� inode
        diskManager.freeINode(newInodeIndex);
        return;
    }
    newInode.blockIndex = static_cast<uint32_t>(newBlockIndex);
    //diskManager.superBlock.freeBlocks--;
    diskManager.updateSuperBlock(diskManager.superBlock);
    */

    // ���ڿ��ļ������������ݿ飬���� blockIndex Ϊ 0 ������ֵ
    newInode.blockIndex = 0;

    // 5. �����ļ��� inode д�����
    diskManager.writeINode(newInodeIndex, newInode);

    // 6. ���µ�ǰĿ¼�� entries
    currentDirectory.addEntry(fileName, newInodeIndex);

    // �����º�ĵ�ǰĿ¼д�ش���
    // ��ȡ��ǰĿ¼�� inode
    INode currentDirInode = diskManager.readINode(currentInodeIndex);

    // ���л���ǰĿ¼��д�����
    std::vector<char> buffer;
    currentDirectory.serialize(buffer, diskManager.blockSize);
    diskManager.writeBlock(currentDirInode.blockIndex, buffer.data());

    // ����ǰĿ¼�� inode д�ش��̣������Ҫ���� size ����Ϣ��
    diskManager.writeINode(currentInodeIndex, currentDirInode);

    std::cout << "File '" << fileName << "' created successfully in current directory." << std::endl;
}

void CommandHandler::handleWriteFile(const std::string& fileName, bool append) {
    if (fileName.empty()) {
        std::cout << "File name is empty." << std::endl;
        return;
    }

    // 1. ����ļ��Ƿ�����ڵ�ǰĿ¼
    bool fileExists = false;
    uint32_t fileInodeIndex;
    INode fileInode;

    auto it = currentDirectory.entries.find(fileName);
    if (it != currentDirectory.entries.end()) {
        // �ļ�����
        fileExists = true;
        fileInodeIndex = it->second;
        fileInode = diskManager.readINode(fileInodeIndex);

        // ����Ƿ�����ͨ�ļ�
        if (fileInode.type != 0) {
            std::cout << "'" << fileName << "' is not a regular file." << std::endl;
            return;
        }
    }
    else {
        // �ļ������ڣ��ڵ�ǰĿ¼�´������ļ�
        // �����µ� inode
        fileInodeIndex = diskManager.allocateINode();
        if (fileInodeIndex == static_cast<uint32_t>(-1)) {
            std::cout << "Failed to allocate inode for new file." << std::endl;
            return;
        }

        // ��ʼ�����ļ��� inode
        fileInode.size = 0;             // �ļ���СΪ 0
        fileInode.mode = 0644;          // Ĭ��Ȩ�ޣ��ɸ�����Ҫ����
        fileInode.type = 0;             // �ļ����ͣ�0 ��ʾ��ͨ�ļ�
        fileInode.inodeIndex = fileInodeIndex;
        fileInode.blockIndex = 0;       // ��δ�������ݿ�

        // �ڵ�ǰĿ¼��������ļ���Ŀ¼��
        currentDirectory.addEntry(fileName, fileInodeIndex);

        // �����º�ĵ�ǰĿ¼д�ش���
        INode currentDirInode = diskManager.readINode(currentInodeIndex);
        std::vector<char> buffer;
        currentDirectory.serialize(buffer, diskManager.blockSize);
        diskManager.writeBlock(currentDirInode.blockIndex, buffer.data());
        diskManager.writeINode(currentInodeIndex, currentDirInode);
    }

    // 2. ��ȡ�û�������ļ�����
    std::cout << "Enter content for the file (end with a single '.' on a new line):" << std::endl;
    std::string line;
    std::string fileContent;
    while (std::getline(std::cin, line)) {
        if (line == ".") {
            break;
        }
        fileContent += line + "\n";
    }

    // �����׷��ģʽ����ȡԭ�����ݲ�׷��,append�е�Bug��������ά��
    if (append && fileExists && fileInode.size > 0) {
        // ��ȡԭ������
        char* buffer = new char[fileInode.size];
        diskManager.readBlock(fileInode.blockIndex, buffer);
        std::string originalContent(buffer, fileInode.size);
        delete[] buffer;

        // ׷��������
        fileContent = originalContent + fileContent;
    }

    // �����ļ���С
    fileInode.size = static_cast<uint32_t>(fileContent.size());

    // 3. ��������·������ݿ�
    // �����ļ����ݿ��Է���һ�����ݿ���
    if (fileInode.blockIndex == 0) {
        // �ļ���δ�������ݿ飬�����µ����ݿ�
        size_t newBlockIndex = diskManager.allocateBlock();
        if (newBlockIndex == static_cast<size_t>(-1)) {
            std::cout << "Failed to allocate block for file." << std::endl;
            // �ͷ��ѷ���� inode��������´������ļ���
            if (!fileExists) {
                diskManager.freeINode(fileInodeIndex);
                currentDirectory.entries.erase(fileName);
            }
            return;
        }
        fileInode.blockIndex = static_cast<uint32_t>(newBlockIndex);
        diskManager.superBlock.freeBlocks--;
        diskManager.updateSuperBlock(diskManager.superBlock);
    }
    else {
        // �ļ��������ݿ飬�ڱ�ʵ����ֱ�Ӹ���
        // �����Ҫ֧���ļ����ݳ���һ�����ݿ���������Ҫʵ�ֶ�����
    }

    // 4. ���ļ�����д�����ݿ�
    char* buffer = new char[diskManager.blockSize];
    std::memset(buffer, 0, diskManager.blockSize);
    std::memcpy(buffer, fileContent.data(), fileInode.size);
    diskManager.writeBlock(fileInode.blockIndex, buffer);
    delete[] buffer;

    // �����ļ��� inode ��д�ش���
    diskManager.writeINode(fileInodeIndex, fileInode);

    std::cout << "Content written to '" << fileName << "' successfully." << std::endl;
}




void CommandHandler::handleCat(const std::string& filePath) {
    if (filePath.empty()) {
        std::cout << "File path is empty." << std::endl;
        return;
    }

    // 1. ����·������λ���ļ����ڵ�Ŀ¼
    std::string parentPath;
    std::string fileName;
    size_t lastSlash = filePath.find_last_of('/');
    if (lastSlash != std::string::npos) {
        // ����б�ܣ���ָ�·�����ļ���
        parentPath = filePath.substr(0, lastSlash);
        fileName = filePath.substr(lastSlash + 1);

        // �������� "/filename" ���������·��Ϊ���ַ�����Ӧ��Ϊ��Ŀ¼
        if (parentPath.empty()) {
            parentPath = "/";
        }
    }
    else {
        // ������б�ܣ���ʾ�ڵ�ǰĿ¼��
        parentPath = ".";
        fileName = filePath;
    }

    // 2. ʹ�� navigateToPath �������ļ����ڵ�Ŀ¼
    uint32_t parentInodeIndex;
    Directory parentDirectory;
    if (!navigateToPath(parentPath, parentInodeIndex, parentDirectory)) {
        // ����ʧ�ܣ����� navigateToPath �����������Ϣ
        return;
    }

    // 3. ����ļ��Ƿ����
    auto it = parentDirectory.entries.find(fileName);
    if (it == parentDirectory.entries.end()) {
        std::cout << "File '" << fileName << "' does not exist in the specified path." << std::endl;
        return;
    }

    uint32_t fileInodeIndex = it->second;
    INode fileInode = diskManager.readINode(fileInodeIndex);

    // 4. ����Ƿ����ļ�����
    if (fileInode.type != 0) {
        std::cout << "'" << fileName << "' is not a regular file." << std::endl;
        return;
    }

    // 5. ��ȡ�ļ�����
    if (fileInode.size == 0) {
        std::cout << "File '" << fileName << "' is empty." << std::endl;
        return;
    }

    // �����ļ����ݴ洢��ֱ�����ݿ��У������ļ���
    if (fileInode.blockIndex == 0) {
        std::cout << "File '" << fileName << "' has no data block allocated." << std::endl;
        return;
    }

    char* buffer = new char[diskManager.blockSize];
    diskManager.readBlock(fileInode.blockIndex, buffer);

    // 6. ����ļ�����
    std::cout << "=== Content of '" << fileName << "' ===" << std::endl;
    std::cout.write(buffer, fileInode.size);
    std::cout << "\n=============================" << std::endl;

    delete[] buffer;
}


void CommandHandler::handleCopy(const std::string& src, const std::string& dest) {
    // �߼��������ļ�
    std::cout << "Copying from " << src << " to " << dest << std::endl;
}

void CommandHandler::handleDel(const std::string& fileName) {
    if (fileName.empty()) {
        std::cout << "File name is empty." << std::endl;
        return;
    }

    // 1. ����ļ��Ƿ�����ڵ�ǰĿ¼
    auto it = currentDirectory.entries.find(fileName);
    if (it == currentDirectory.entries.end()) {
        std::cout << "File '" << fileName << "' does not exist in the current directory." << std::endl;
        return;
    }

    uint32_t fileInodeIndex = it->second;
    INode fileInode = diskManager.readINode(fileInodeIndex);

    // 2. ����Ƿ�����ͨ�ļ�
    if (fileInode.type != 0) {
        std::cout << "'" << fileName << "' is not a regular file and cannot be deleted using 'del' command." << std::endl;
        return;
    }

    // 3. �ͷ��ļ�ռ�õ���Դ
    // �ͷ����ݿ�
    if (fileInode.blockIndex != 0) {
        diskManager.freeBlock(fileInode.blockIndex);
        diskManager.superBlock.freeBlocks++;
    }

    // �ͷ� inode
    diskManager.freeINode(fileInodeIndex);
    diskManager.superBlock.inodeCount--;

    // 4. �ӵ�ǰĿ¼�� entries ��ɾ���ļ�
    currentDirectory.entries.erase(it);

    // 5. �����º�ĵ�ǰĿ¼д�ش���
    INode currentDirInode = diskManager.readINode(currentInodeIndex);
    std::vector<char> buffer;
    currentDirectory.serialize(buffer, diskManager.blockSize);
    diskManager.writeBlock(currentDirInode.blockIndex, buffer.data());
    diskManager.writeINode(currentInodeIndex, currentDirInode);

    // 6. ���³�����
    diskManager.updateSuperBlock(diskManager.superBlock);

    std::cout << "File '" << fileName << "' deleted successfully from current directory." << std::endl;
}


void CommandHandler::handleCheck() {
    std::cout << "Starting file system check..." << std::endl;

    usedInodes.clear();
    usedBlocks.clear();

    // 1. �Ӹ�Ŀ¼��ʼ�����ļ�ϵͳ
    traverseFileSystem(diskManager.superBlock.rootInode);

    // 2. ����λͼ
    diskManager.loadBitmaps();

    // 3. ��鲢�޸� inode λͼ
    bool inodeBitmapChanged = false;
    for (uint32_t i = 0; i < MAX_INODES; ++i) {
        // **������Ŀ¼�� inode**
        if (i == diskManager.superBlock.rootInode) {
            continue;
        }

        bool isAllocatedInBitmap = diskManager.isINodeAllocated(i);
        bool isUsed = (usedInodes.find(i) != usedInodes.end());

        if (isAllocatedInBitmap && !isUsed) {
            // inode ���Ϊ�ѷ��䵫δʹ��
            std::cout << "Inode " << i << " marked as allocated but not in use. Fixing..." << std::endl;
            diskManager.freeINode(i);
            inodeBitmapChanged = true;
        }
        else if (!isAllocatedInBitmap && isUsed) {
            // inode δ���Ϊ�ѷ��䵫����ʹ��
            std::cout << "Inode " << i << " not marked as allocated but in use. Fixing..." << std::endl;
            diskManager.allocateINodeAtIndex(i);
            inodeBitmapChanged = true;
        }
    }

    // 4. ��鲢�޸����ݿ�λͼ
    bool blockBitmapChanged = false;
    for (size_t i = 0; i < diskManager.totalBlocks; ++i) {
        // **������Ŀ¼�����ݿ�**
        if (i == diskManager.superBlock.rootDataBlock) { //  rootDataBlock ��ʾ��Ŀ¼�����ݿ�����
            continue;
        }

        bool isAllocatedInBitmap = diskManager.isBlockAllocated(i);
        bool isUsed = (usedBlocks.find(i) != usedBlocks.end());

        if (isAllocatedInBitmap && !isUsed) {
            // ���ݿ���Ϊ�ѷ��䵫δʹ��
            std::cout << "Data block " << i << " marked as allocated but not in use. Fixing..." << std::endl;
            diskManager.freeBlock(i);
            blockBitmapChanged = true;
        }
        else if (!isAllocatedInBitmap && isUsed) {
            // ���ݿ�δ���Ϊ�ѷ��䵫����ʹ��
            std::cout << "Data block " << i << " not marked as allocated but in use. Fixing..." << std::endl;
            diskManager.allocateBlockAtIndex(i);
            blockBitmapChanged = true;
        }
    }

    // 5. ���λͼ�и��ģ�����λͼ������
    if (inodeBitmapChanged || blockBitmapChanged) {
        diskManager.updateBitmaps();
    }

    // 6. ���³�������Ϣ
    diskManager.superBlock.freeBlocks = diskManager.calculateFreeBlocks();
    diskManager.superBlock.inodeCount = diskManager.calculateAllocatedInodes();
    diskManager.updateSuperBlock(diskManager.superBlock);

    std::cout << "File system check completed." << std::endl;
}




void CommandHandler::updatePrompt() {
    std::cout << "simdisk:" << (currentPath.empty() ? "/" : currentPath) << "> ";
}

void CommandHandler::traverseFileSystem(uint32_t inodeIndex) {
    // ��ֹѭ������
    if (usedInodes.find(inodeIndex) != usedInodes.end()) {
        return;
    }

    usedInodes.insert(inodeIndex);

    INode inode = diskManager.readINode(inodeIndex);

    // ��¼ inode ʹ�õ����ݿ�
    if (inode.blockIndex != 0) {
        usedBlocks.insert(inode.blockIndex);
        // **������Ǹ�Ŀ¼�� inode����¼�����ݿ�����**
        if (inodeIndex == diskManager.superBlock.rootInode) {
            diskManager.superBlock.rootDataBlock = inode.blockIndex; // �����Ŀ¼�����ݿ�����
        }
    }

    if (inode.type == 1) { // Ŀ¼����
        Directory dir = loadDirectoryFromINode(inode);

        for (const auto& entry : dir.entries) {
            // ���� "." �� ".."
            if (entry.first == "." || entry.first == "..") {
                continue;
            }

            uint32_t childInodeIndex = entry.second;

            // �ݹ������ inode
            traverseFileSystem(childInodeIndex);
        }
    }
    // �����Ҫ�������ļ��������ڴ���Ӵ���
}
