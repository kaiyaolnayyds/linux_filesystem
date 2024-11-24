#ifndef COMMANDHANDLER_H
#define COMMANDHANDLER_H

#include <string>
#include "DiskManager.h"
#include <fstream>
#include <unordered_set>
#include "Directory.h"
#include "SuperBlock.h"
#include "INode.h" 
#include "UserManager.h"

/**
 * @class CommandHandler
 * @brief ���ڴ����ļ�ϵͳ������ࡣ
 */
class CommandHandler {
public:
    DiskManager& diskManager;        ///< ���̹�������������ã����ڹ�����̲���
    UserManager& userManager;        //<�û������������ã����ڹ����û�����
    Directory currentDirectory;     ///< ��ǰ���ڵ�Ŀ¼����
    uint32_t currentInodeIndex;     ///< ��ǰĿ¼��Ӧ�� inode ����
    std::string currentPath;        ///< ��ǰ���ڵ�·�����ַ�����ʾ��

    // ���ڸ���ʹ�õ� inode �����ݿ�
    std::unordered_set<uint32_t> usedInodes;  //��ʹ��inodes
    std::unordered_set<uint32_t> usedBlocks;  //��ʹ�����ݿ�

    // �洢���һ����������
    std::string lastOutput;
  
    // ��ȡ���һ����������
    std::string getLastOutput() const;

    // �������һ��������������ջ����ã�
    void setLastOutput(const std::string& output);


    /**
     * @brief ���캯������ʼ�� CommandHandler ����
     * @param dm ���̹�������������á�
     */
    CommandHandler(DiskManager& diskManager, UserManager& userManager);

    /**
     * @brief ���������������ö�Ӧ�Ĵ�������
     * @param command �û�����������ַ�����
     */
    bool handleCommand(const std::string& command);

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
 * @brief д���ļ����ݡ�
 * @param filePath Ҫд����ļ�·����
 * @param append �Ƿ���׷��ģʽд�룬Ĭ��Ϊ false������д�룩��
 */
    void handleWriteFile(const std::string& filePath, bool append = false);


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
    std::vector<std::string> parsePath(const std::string& path);        //·����������������ȥ��path�е�"/",�Ӷ�ʹ����ڴ���

    /**
    * @brief ����·������ȡ��·�����ļ�����
    * @param fullPath ������·����
    * @param parentPath ������������ظ�·����
    * @param fileName ��������������ļ�����
    */
    void parsePath(const std::string& fullPath, std::string& parentPath, std::string& fileName);

    /**
   * @brief ����·���ַ������ֽ�Ϊ·������б�
   * @param path Ҫ������·���ַ�����
   * @return ����·�������ֵ��ַ����б�
   */
    std::vector<std::string> parsePath_normal(const std::string& path);      //·�����캯������������path���й淶������ȥ��path�е�"."��".."

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

    /**
 * @brief ���ݸ�����·����������Ŀ��Ŀ¼�����ض�Ӧ�� inode ������Ŀ¼����
 * @param path Ҫ������·���������Ǿ���·�������·����
 * @param resultInodeIndex �������������Ŀ��Ŀ¼�� inode ������
 * @param resultDirectory �������������Ŀ��Ŀ¼�� Directory ����
 * @return �ɹ����� true��ʧ�ܷ��� false��
 */
    bool navigateToPath(const std::string& path, uint32_t& resultInodeIndex, Directory& resultDirectory);

    /**
       * @brief �ݹ�����ļ�ϵͳ����ָ���� inode ��ʼ��
       * @param inodeIndex ��ʼ inode ��������
       * �ú������ռ���������ʹ�õ� inode �����ݿ���Ϣ��
       */
    void traverseFileSystem(uint32_t inodeIndex);

    /**
    * @brief ��鵱ǰ�û��Ƿ��ж�ָ�� inode ����ָ��������Ȩ�ޡ�
    * @param inode Ҫ���� inode��
    * @param permissionType �������ͣ�"r"����ȡ��"w"��д�룬"x"��ִ�С�
    * @return ��Ȩ�޷��� true����Ȩ�޷��� false��
    */
    bool checkPermission(const INode& inode, char permissionType);

    /**
    * @brief ��ģ���ļ�ϵͳ�ڲ������ļ���
    * @param srcPath Դ�ļ���·����
    * @param destPath Ŀ���ļ���·����
    */
    void copyWithinSimDisk(const std::string& srcPath, const std::string& destPath);

    /**
     * @brief �������ļ�ϵͳ�����ļ���ģ���ļ�ϵͳ��
     * @param hostPath �����ļ�ϵͳ�е��ļ�·����
     * @param simDiskPath ģ���ļ�ϵͳ�е�Ŀ��·����
     */
    void copyFromHostToSimDisk(const std::string& hostPath, const std::string& simDiskPath);

    /**
     * @brief ��ģ���ļ�ϵͳ�����ļ��������ļ�ϵͳ��
     * @param simDiskPath ģ���ļ�ϵͳ�е��ļ�·����
     * @param hostPath �����ļ�ϵͳ�е�Ŀ��·����
     */
    void copyFromSimDiskToHost(const std::string& simDiskPath, const std::string& hostPath);

    /**
    * @brief �����ļ�·����ȡ��Ӧ�� inode��
    * @param filePath �ļ���·����
    * @param inodeIndex ������������� inode ��������
    * @param inode ������������� inode ����
    * @return ����ɹ��ҵ��ļ�����ȡ inode������ true�����򷵻� false��
    */
    bool getFileINode(const std::string& filePath, uint32_t& inodeIndex, INode& inode);
};

#endif // COMMANDHANDLER_H
