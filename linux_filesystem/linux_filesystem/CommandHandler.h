#ifndef COMMANDHANDLER_H
#define COMMANDHANDLER_H

#include <string>
#include "DiskManager.h"
#include <fstream>
#include "Directory.h"
#include "SuperBlock.h"
#include "INode.h" // ���� INode.h �ļ������ڴ��� inode ��ز���

/**
 * @class CommandHandler
 * @brief ���ڴ����ļ�ϵͳ������ࡣ
 */
class CommandHandler {
public:
    DiskManager& diskManager;        ///< ���̹�������������ã����ڹ�����̲���
    Directory currentDirectory;     ///< ��ǰ���ڵ�Ŀ¼����
    uint32_t currentInodeIndex;     ///< ��ǰĿ¼��Ӧ�� inode ����
    std::string currentPath;        ///< ��ǰ���ڵ�·�����ַ�����ʾ��

    /**
     * @brief ���캯������ʼ�� CommandHandler ����
     * @param dm ���̹�������������á�
     */
    CommandHandler(DiskManager& dm);

    /**
     * @brief ���������������ö�Ӧ�Ĵ�������
     * @param command �û�����������ַ�����
     */
    void handleCommand(const std::string& command);

    /**
     * @brief ��ʾ�ļ�ϵͳ��������Ϣ��
     */
    void handleInfo();

    /**
     * @brief ���ĵ�ǰ����Ŀ¼��
     * @param path Ҫ���ĵ���Ŀ��Ŀ¼·����
     */
    void handleCd(const std::string& path);

    /**
     * @brief ��ʾĿ¼���ݡ�
     * @param path Ŀ��Ŀ¼·����Ĭ����ʾ��ǰĿ¼����
     * @param recursive �Ƿ�ݹ���ʾ��Ŀ¼���ݡ�
     */
    void handleDir(const std::string& path = "", bool recursive = false);

    /**
     * @brief ������Ŀ¼��
     * @param dirName ��Ŀ¼�����ơ�
     */
    void handleMd(const std::string& dirName);

    /**
     * @brief ɾ��ָ��Ŀ¼��
     * @param dirName Ҫɾ����Ŀ¼���ơ�
     */
    void handleRd(const std::string& dirName);

    /**
     * @brief �������ļ���
     * @param fileName ���ļ������ơ�
     */
    void handleNewFile(const std::string& fileName);

    /**
     * @brief ��ʾ�ļ����ݡ�
     * @param fileName Ҫ��ʾ���ݵ��ļ����ơ�
     */
    void handleCat(const std::string& fileName);

    /**
     * @brief �����ļ���
     * @param src Դ�ļ�·����
     * @param dest Ŀ���ļ�·����
     */
    void handleCopy(const std::string& src, const std::string& dest);

    /**
     * @brief ɾ��ָ���ļ���
     * @param fileName Ҫɾ�����ļ����ơ�
     */
    void handleDel(const std::string& fileName);

    /**
     * @brief ����ļ�ϵͳ��״̬�������޸���
     */
    void handleCheck();

    /**
     * @brief ����·���ַ������ֽ�Ϊ·������б�
     * @param path Ҫ������·���ַ�����
     * @return ����·�������ֵ��ַ����б�
     */
    std::vector<std::string> parsePath(const std::string& path);

    /**
     * @brief ����������ʾ���Է�ӳ��ǰ·����
     */
    void updatePrompt();

    /**
     * @brief ��ʾĿ¼���ݣ������ļ�����Ŀ¼��
     * @param dir Ҫ��ʾ��Ŀ¼����
     * @param dirInodeIndex Ŀ¼��Ӧ�� inode ������
     * @param recursive �Ƿ�ݹ���ʾ��Ŀ¼���ݡ�
     * @param indent �����ַ��������ڵݹ���ʾʱ�Ĳ㼶�ṹ��
     */
    void displayDirectoryContents(const Directory& dir, uint32_t dirInodeIndex, bool recursive, const std::string& indent);

    /**
     * @brief ���� inode ���ض�Ӧ��Ŀ¼����
     * @param inode Ҫ���ص� inode ����
     * @return ���غ��Ŀ¼����
     */
    Directory loadDirectoryFromINode(const INode& inode);

    /**
     * @brief �ݹ�ɾ��Ŀ¼�����������ݡ�
     * @param inodeIndex Ҫɾ����Ŀ¼��Ӧ�� inode ������
     * @return ɾ���Ƿ�ɹ���
     */
    bool deleteDirectoryRecursively(uint32_t inodeIndex);
};

#endif // COMMANDHANDLER_H
