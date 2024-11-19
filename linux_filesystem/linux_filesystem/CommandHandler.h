// include/CommandHandler.h
#ifndef COMMANDHANDLER_H
#define COMMANDHANDLER_H

#include <string>
#include "DiskManager.h"
#include <fstream>
#include "Directory.h"

class CommandHandler {
private:
    DiskManager& diskManager; // ���̹�����
    Directory currentDirectory; // ��ǰĿ¼����

public:
    // ���캯��
   CommandHandler(DiskManager& dm) : diskManager(dm) {
    // ���ظ�Ŀ¼
    SuperBlock superBlock = diskManager.loadSuperBlock();
    uint32_t rootInode = superBlock.rootInode;

    // �Ӵ��̶�ȡ��Ŀ¼����
    char buffer[diskManager.blockSize];
    diskManager.readBlock(rootInode, buffer);

    // �����л���Ŀ¼
    currentDirectory.deserialize(buffer, diskManager.blockSize);
}


    // ��������ķ���
    void handleCommand(const std::string& command);

    // ��������ľ��崦����
    void handleInfo();
    void handleCd(const std::string& path);
    void handleDir(const std::string& path);
    void handleMd(const std::string& dirName);
    void handleRd(const std::string& dirName);
    void handleNewFile(const std::string& fileName);
    void handleCat(const std::string& fileName);
    void handleCopy(const std::string& src, const std::string& dest);
    void handleDel(const std::string& fileName);
    void handleCheck();

    // ���߷���������·��
    std::vector<std::string> parsePath(const std::string& path);
};

#endif // COMMANDHANDLER_H
