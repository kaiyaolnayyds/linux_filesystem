// include/CommandHandler.h
#ifndef COMMANDHANDLER_H
#define COMMANDHANDLER_H

#include <string>
#include "DiskManager.h"
#include <fstream>
#include "Directory.h"
#include "SuperBlock.h"  
#include "INode.h"       // ���� INode.h


class CommandHandler {
public:
    DiskManager& diskManager; // ���̹�����
    Directory currentDirectory; // ��ǰĿ¼����
    uint32_t currentInodeIndex; // ��ǰĿ¼�� inodeIndex

    // ���캯��
    CommandHandler(DiskManager& dm);

    // ��������ķ���
    void handleCommand(const std::string& command);

    // ��������ľ��崦����
    void handleInfo();
    void handleCd(const std::string& path);
    void handleDir(const std::string& path = "", bool recursive = false);
    void handleMd(const std::string& dirName);
    void handleRd(const std::string& dirName);
    void handleNewFile(const std::string& fileName);
    void handleCat(const std::string& fileName);
    void handleCopy(const std::string& src, const std::string& dest);
    void handleDel(const std::string& fileName);
    void handleCheck();

    // ���߷���������·��
    std::vector<std::string> parsePath(const std::string& path);

    // ��ʾĿ¼����
    void displayDirectoryContents(const Directory& dir, uint32_t dirInodeIndex, bool recursive, const std::string& indent);

    // ����������·������Ŀ¼
    bool findDirectory(const std::vector<std::string>& pathComponents, uint32_t& resultInodeIndex, Directory& resultDirectory);

    // ��������ȡָ�� inodeIndex ��Ŀ¼
    bool getDirectory(uint32_t inodeIndex, Directory& directory);
};

#endif // COMMANDHANDLER_H
