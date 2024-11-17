// include/CommandHandler.h
#ifndef COMMANDHANDLER_H
#define COMMANDHANDLER_H

#include <string>
#include "DiskManager.h"

class CommandHandler {
private:
    DiskManager& diskManager;

public:
    // ���캯��
    CommandHandler(DiskManager& dm) : diskManager(dm) {}

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
};

#endif // COMMANDHANDLER_H
