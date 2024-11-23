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

    // 初始化当前路径为根目录
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
    uint32_t newInodeIndex;
    Directory newDirectory;

    if (!navigateToPath(path, newInodeIndex, newDirectory)) {
        // 导航失败，已在 navigateToPath 中输出错误信息
        return;
    }

    // 更新当前目录和 inode 索引
    currentDirectory = newDirectory;
    currentInodeIndex = newInodeIndex;

    // 更新 currentPath
    if (path[0] == '/') {
        // 绝对路径
        currentPath = path;
    }
    else {
        if (currentPath != "/" && !currentPath.empty()) {
            currentPath += "/";
        }
        currentPath += path;
    }

    // 规范化 currentPath，处理 `.` 和 `..`
    std::vector<std::string> pathComponents = parsePath_normal(currentPath);
    currentPath.clear();
    for (const auto& comp : pathComponents) {
        currentPath += "/";
        currentPath += comp;
    }

    // 如果路径为空，设置为根目录
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
            continue; // 忽略当前目录符号和空字符串
        }
        else if (token == "..") {
            if (!components.empty()) {
                components.pop_back(); // 返回上一级目录
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
            // 导航失败，已在 navigateToPath 中输出错误信息
            return;
        }
    }
    else {
        dirInodeIndex = currentInodeIndex;
        dirToDisplay = currentDirectory;
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
        // 如果是目录，递归删除其内容
        Directory dir = loadDirectoryFromINode(inode);

        for (const auto& entry : dir.entries) {
            const std::string& name = entry.first;
            uint32_t childInodeIndex = entry.second;

            // 跳过 "." 和 ".."
            if (name == "." || name == "..") {
                continue;
            }

            // 递归删除子文件或子目录
            if (!deleteDirectoryRecursively(childInodeIndex)) {
                return false;
            }
        }
    }

    // 释放数据块
    diskManager.freeBlock(inode.blockIndex);

    // 更新超级块
    diskManager.superBlock.freeBlocks++;
    diskManager.updateSuperBlock(diskManager.superBlock);

    // 释放 inode
    diskManager.freeINode(inodeIndex);

    return true;
}

bool CommandHandler::navigateToPath(const std::string& path, uint32_t& resultInodeIndex, Directory& resultDirectory) {
    if (path.empty()) {
        std::cout << "Path is empty." << std::endl;
        return false;
    }

    // 解析路径
    std::vector<std::string> components = parsePath(path);
    uint32_t inodeIndex;
    Directory dir;
    INode inode;

    if (path[0] == '/') {
        // 绝对路径，从根目录开始
        inodeIndex = diskManager.superBlock.rootInode;
        inode = diskManager.readINode(inodeIndex);
        dir = loadDirectoryFromINode(inode);
    }
    else {
        // 相对路径，从当前目录开始
        inodeIndex = currentInodeIndex;
        inode = diskManager.readINode(inodeIndex);
        dir = currentDirectory;
    }

    // 遍历路径组件
    for (const auto& component : components) {
        if (component == "..") {
            // 返回上一级目录
            if (inodeIndex == diskManager.superBlock.rootInode) {
                // 已经是根目录，无法再向上
                continue;
            }
            // 获取父目录的 inode
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
            // 当前目录，跳过
            continue;
        }
        else {
            // 查找子目录
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

            // 加载下一级目录
            dir = loadDirectoryFromINode(nextInode);
            inodeIndex = nextInodeIndex;
        }
    }

    // 导航成功，返回结果
    resultDirectory = dir;
    resultInodeIndex = inodeIndex;
    return true;
}



void CommandHandler::handleMd(const std::string& dirName) {
    // 检查目录是否已存在
    if (currentDirectory.entries.find(dirName) != currentDirectory.entries.end()) {
        std::cout << "Directory '" << dirName << "' already exists." << std::endl;
        return;
    }

    // 分配一个新的 inode
    uint32_t newInodeIndex = diskManager.allocateINode();
    if (newInodeIndex == static_cast<uint32_t>(-1)) {
        std::cout << "Failed to allocate inode for new directory." << std::endl;
        return;
    }

    INode newInode;
    newInode.size = 0;
    newInode.mode = 0755; // 默认权限
    newInode.type = 1;    // 目录类型
    newInode.inodeIndex = newInodeIndex;

    // 分配一个新的数据块给新目录
    size_t newBlockIndex = diskManager.allocateBlock();
    if (newBlockIndex == static_cast<size_t>(-1)) {
        std::cout << "Failed to allocate block for new directory." << std::endl;
        // 释放已分配的 inode
        diskManager.freeINode(newInodeIndex);
        return;
    }
    newInode.blockIndex = static_cast<uint32_t>(newBlockIndex);

    // 更新超级块
    //diskManager.superBlock.freeBlocks--;
    diskManager.updateSuperBlock(diskManager.superBlock);

    // 将新目录的 inode 写入磁盘
    diskManager.writeINode(newInodeIndex, newInode);

    // 创建新目录的目录项
    Directory newDirectory;
    newDirectory.addEntry(".", newInodeIndex);       // 当前目录
    newDirectory.addEntry("..", currentInodeIndex);  // 父目录

    // 序列化并写入磁盘
    std::vector<char> buffer;
    newDirectory.serialize(buffer, diskManager.blockSize);
    diskManager.writeBlock(newInode.blockIndex, buffer.data());

    /*
    （待开发）这里实现切换到要创建目录的目录下，路径解析类似cd,最终拿到currentDirectory和currentInodeIndex
    */


    // 更新当前目录的 entries
    currentDirectory.addEntry(dirName, newInodeIndex);

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
    std::cout << "[DEBUG] New directory INode: size=" << newInode.size
        << ", mode=" << newInode.mode
        << ", type=" << static_cast<int>(newInode.type)
        << ", blockIndex=" << newInode.blockIndex
        << ", inodeIndex=" << newInode.inodeIndex << std::endl;

    std::cout << "Directory '" << dirName << "' created at block " << newBlockIndex << "." << std::endl;
}




void CommandHandler::handleRd(const std::string& dirName) {
    // 在当前目录中查找要删除的目录的 inode 索引
    auto it = currentDirectory.entries.find(dirName);
    if (it == currentDirectory.entries.end()) {
        std::cout << "Directory '" << dirName << "' does not exist in the current directory." << std::endl;
        return;
    }

    uint32_t dirInodeIndex = it->second;
    INode dirInode = diskManager.readINode(dirInodeIndex);

    // 检查要删除的是否是目录
    if (dirInode.type != 1) {
        std::cout << "'" << dirName << "' is not a directory." << std::endl;
        return;
    }

    // 加载要删除的目录
    Directory dirToDelete = loadDirectoryFromINode(dirInode);

    // 如果目录不为空，提示用户是否继续删除
    if (dirToDelete.entries.size() > 2) { // 除了 "." 和 ".." 之外还有其他条目
        std::cout << "Directory '" << dirName << "' is not empty. Do you want to delete it? (y/n): ";
        std::string choice;
        std::getline(std::cin, choice);
        if (choice != "y" && choice != "Y") {
            std::cout << "Deletion cancelled." << std::endl;
            return;
        }
    }

    // 递归删除目录内容
    bool success = deleteDirectoryRecursively(dirInodeIndex);
    if (success) {
        // 从当前目录中删除该目录的目录项
        currentDirectory.entries.erase(dirName);

        // 将更新后的当前目录写回磁盘
        // 获取当前目录的 inode
        INode currentDirInode = diskManager.readINode(currentInodeIndex);

        // 序列化当前目录并写入磁盘
        std::vector<char> buffer;
        currentDirectory.serialize(buffer, diskManager.blockSize);
        diskManager.writeBlock(currentDirInode.blockIndex, buffer.data());

        // 将当前目录的 inode 写回磁盘（如果需要更新 size 等信息）
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

    // 1. 检查文件是否已存在
    if (currentDirectory.entries.find(fileName) != currentDirectory.entries.end()) {
        std::cout << "File '" << fileName << "' already exists in the current directory." << std::endl;
        return;
    }

    // 2. 分配新的 inode
    uint32_t newInodeIndex = diskManager.allocateINode();
    if (newInodeIndex == static_cast<uint32_t>(-1)) {
        std::cout << "Failed to allocate inode for new file." << std::endl;
        return;
    }

    // 3. 初始化新文件的 inode
    INode newInode;
    newInode.size = 0;             // 文件大小为 0
    newInode.mode = 0644;          // 默认权限，可根据需要调整
    newInode.type = 0;             // 文件类型，0 表示普通文件
    newInode.inodeIndex = newInodeIndex;

    // 4. 可选：分配数据块（对于空文件，可以不分配）
    // 如果需要预先分配一个数据块，可以取消以下注释
    /*
    size_t newBlockIndex = diskManager.allocateBlock();
    if (newBlockIndex == static_cast<size_t>(-1)) {
        std::cout << "Failed to allocate block for new file." << std::endl;
        // 释放已分配的 inode
        diskManager.freeINode(newInodeIndex);
        return;
    }
    newInode.blockIndex = static_cast<uint32_t>(newBlockIndex);
    //diskManager.superBlock.freeBlocks--;
    diskManager.updateSuperBlock(diskManager.superBlock);
    */

    // 对于空文件，不分配数据块，设置 blockIndex 为 0 或特殊值
    newInode.blockIndex = 0;

    // 5. 将新文件的 inode 写入磁盘
    diskManager.writeINode(newInodeIndex, newInode);

    // 6. 更新当前目录的 entries
    currentDirectory.addEntry(fileName, newInodeIndex);

    // 将更新后的当前目录写回磁盘
    // 获取当前目录的 inode
    INode currentDirInode = diskManager.readINode(currentInodeIndex);

    // 序列化当前目录并写入磁盘
    std::vector<char> buffer;
    currentDirectory.serialize(buffer, diskManager.blockSize);
    diskManager.writeBlock(currentDirInode.blockIndex, buffer.data());

    // 将当前目录的 inode 写回磁盘（如果需要更新 size 等信息）
    diskManager.writeINode(currentInodeIndex, currentDirInode);

    std::cout << "File '" << fileName << "' created successfully in current directory." << std::endl;
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



void CommandHandler::updatePrompt() {
    std::cout << "simdisk:" << (currentPath.empty() ? "/" : currentPath) << "> ";
}
