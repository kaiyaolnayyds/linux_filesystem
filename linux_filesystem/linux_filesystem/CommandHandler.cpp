// src/CommandHandler.cpp
#include "CommandHandler.h"
#include <iostream>
#include <sstream>


CommandHandler::CommandHandler(DiskManager& dm) : diskManager(dm) {
    // 加载超级块
    SuperBlock superBlock = diskManager.loadSuperBlock();
    uint32_t rootInodeIndex = superBlock.rootInode;
    currentInodeIndex = rootInodeIndex;

    // 读取根目录的 inode
    INode rootInode = diskManager.readINode(rootInodeIndex);

    // 从磁盘读取根目录数据
    char buffer[diskManager.blockSize];
    diskManager.readBlock(rootInode.blockIndex, buffer);

    // 反序列化根目录
    currentDirectory.deserialize(buffer, diskManager.blockSize);

    // 调试输出
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

        // 解析所有参数
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

    // 打印超级块信息
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

    // 解析路径
    std::vector<std::string> components = parsePath(path);

    // 判断是否为绝对路径
    bool isAbsolutePath = !path.empty() && path[0] == '/';

    uint32_t startInodeIndex;
    Directory startDirectory;

    if (isAbsolutePath) {
        // 从根目录开始
        startInodeIndex = diskManager.superBlock.rootInode;
        if (!getDirectory(startInodeIndex, startDirectory)) {
            std::cout << "Error: Unable to load root directory." << std::endl;
            return;
        }
    }
    else {
        // 从当前目录开始
        startInodeIndex = currentInodeIndex;
        startDirectory = currentDirectory;
    }

    uint32_t targetInodeIndex = startInodeIndex;
    Directory targetDirectory = startDirectory;

    for (const std::string& component : components) {
        if (component == ".") {
            // 当前目录，跳过
            continue;
        }
        else if (component == "..") {
            // 上级目录
            if (targetInodeIndex == diskManager.superBlock.rootInode) {
                // 已经是根目录，无法返回上级
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
                // 未找到目录
                std::cout << "Directory not found: " << component << std::endl;
                return;
            }

            uint32_t inodeIndex = it->second;
            INode inode = diskManager.readINode(inodeIndex);

            if (inode.type != 1) {
                // 不是目录
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

    // 更新当前目录
    currentInodeIndex = targetInodeIndex;
    currentDirectory = targetDirectory;
    std::cout << "Changed directory to: " << path << std::endl;
}



void CommandHandler::handleDir(const std::string& path, bool recursive) {
    // 确定要显示的目录
    Directory dirToDisplay = currentDirectory;
    uint32_t dirInodeIndex = currentInodeIndex;

    if (!path.empty()) {
        // 查找指定的路径
        // 这里需要实现路径解析（例如支持相对路径、绝对路径）
        // 为简化，假设只支持当前目录下的直接子目录
        auto it = currentDirectory.entries.find(path);
        if (it != currentDirectory.entries.end()) {
            uint32_t inodeIndex = it->second;
            INode inode = diskManager.readINode(inodeIndex);
            if (inode.type != 1) {
                std::cout << path << " is not a directory." << std::endl;
                return;
            }
            // 读取目录内容
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

    // 显示目录内容
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

            // 调试输出 inode 信息
            std::cout << "[DEBUG] INode for '" << name << "': size=" << inode.size << ", mode=" << inode.mode
                << ", type=" << static_cast<int>(inode.type) << ", blockIndex=" << inode.blockIndex << ", inodeIndex=" << inode.inodeIndex << std::endl;

            // 显示文件名、物理地址、保护码、文件长度、子目录等
            std::string typeStr = (inode.type == 1) ? "<DIR>" : "<FILE>";
            std::cout << indent << typeStr << " " << name
                << " [Addr: " << inode.blockIndex
                << ", Mode: " << std::oct << inode.mode << std::dec
                << ", Size: " << inode.size << "]" << std::endl;

            // 如果是目录且需要递归
            if (recursive && inode.type == 1) {
                // 读取子目录内容
                Directory subDir;
                char buffer[diskManager.blockSize];
                diskManager.readBlock(inode.blockIndex, buffer);
                subDir.deserialize(buffer, diskManager.blockSize);

                // 递归显示子目录内容
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
            // 当前目录，跳过
            continue;
        }
        else if (component == "..") {
            // 上级目录，暂未实现，简单处理为根目录
            inodeIndex = diskManager.superBlock.rootInode;
            if (!getDirectory(inodeIndex, directory)) {
                return false;
            }
            continue;
        }
        else {
            auto it = directory.entries.find(component);
            if (it == directory.entries.end()) {
                // 未找到目录
                return false;
            }

            inodeIndex = it->second;
            INode inode = diskManager.readINode(inodeIndex);

            if (inode.type != 1) {
                // 不是目录
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
    // 检查目录是否已存在
    if (currentDirectory.entries.find(dirName) != currentDirectory.entries.end()) {
        std::cout << "Directory '" << dirName << "' already exists." << std::endl;
        return;
    }

    // 分配一个新的 inode
    uint32_t newInodeIndex = diskManager.superBlock.inodeCount++;
    INode newInode;
    newInode.size = 0;
    newInode.mode = 0755; // 默认权限
    newInode.type = 1;    // 目录类型
    newInode.inodeIndex = newInodeIndex;

    // 分配一个新的数据块给新目录
    size_t newBlockIndex = diskManager.allocateBlock();
    if (newBlockIndex == static_cast<size_t>(-1)) {
        std::cout << "Failed to allocate block for new directory." << std::endl;
        return;
    }
    newInode.blockIndex = static_cast<uint32_t>(newBlockIndex);

    // 更新超级块
    diskManager.superBlock.freeBlocks--;
    diskManager.updateSuperBlock(diskManager.superBlock);

    // 将新目录的 inode 写入磁盘
    diskManager.writeINode(newInodeIndex, newInode);

    // 创建新目录，添加 `.` 和 `..` 条目
    Directory newDirectory;
    newDirectory.parentInodeIndex = currentInodeIndex; // 设置父目录的 inodeIndex
    newDirectory.entries["."] = newInodeIndex;         // 当前目录
    newDirectory.entries[".."] = currentInodeIndex;    // 父目录

    // 将新目录序列化并写入磁盘
    std::vector<char> buffer;
    newDirectory.serialize(buffer, diskManager.blockSize);
    diskManager.writeBlock(newInode.blockIndex, buffer.data());

    // 更新当前目录的 entries
    currentDirectory.entries[dirName] = newInodeIndex;

    // 将更新后的当前目录写回磁盘
    // 获取当前目录的 inode
    INode currentDirInode = diskManager.readINode(currentInodeIndex);

    // 序列化当前目录并写入磁盘
    buffer.clear();
    currentDirectory.serialize(buffer, diskManager.blockSize);
    diskManager.writeBlock(currentDirInode.blockIndex, buffer.data());

    // 将当前目录的 inode 写回磁盘（如果需要更新 size 等信息）
    diskManager.writeINode(currentInodeIndex, currentDirInode);

    // 调试输出新目录的 inode 信息
    std::cout << "[DEBUG] New directory INode: size=" << newInode.size << ", mode=" << newInode.mode
        << ", type=" << static_cast<int>(newInode.type) << ", blockIndex=" << newInode.blockIndex
        << ", inodeIndex=" << newInode.inodeIndex << std::endl;

    std::cout << "Directory '" << dirName << "' created at block " << newBlockIndex << "." << std::endl;
}





void CommandHandler::handleRd(const std::string& dirName) {
    // 逻辑：删除目录及其内容
    std::cout << "Removing directory: " << dirName << std::endl;
}

void CommandHandler::handleNewFile(const std::string& fileName) {
    // 逻辑：创建新文件
    std::cout << "Creating new file: " << fileName << std::endl;
}

void CommandHandler::handleCat(const std::string& fileName) {
    // 逻辑：显示文件内容
    std::cout << "Displaying file: " << fileName << std::endl;
}

void CommandHandler::handleCopy(const std::string& src, const std::string& dest) {
    // 逻辑：拷贝文件
    std::cout << "Copying from " << src << " to " << dest << std::endl;
}

void CommandHandler::handleDel(const std::string& fileName) {
    // 逻辑：删除文件
    std::cout << "Deleting file: " << fileName << std::endl;
}

void CommandHandler::handleCheck() {
    // 逻辑：检测并恢复文件系统
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
