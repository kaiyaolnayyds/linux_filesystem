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
 * @brief 用于处理文件系统命令的类。
 */
class CommandHandler {
public:
    DiskManager& diskManager;        ///< 磁盘管理器对象的引用，用于管理磁盘操作
    UserManager& userManager;        //<用户管理对象的引用，用于管理用户操作
    Directory currentDirectory;     ///< 当前所在的目录对象
    uint32_t currentInodeIndex;     ///< 当前目录对应的 inode 索引
    std::string currentPath;        ///< 当前所在的路径（字符串表示）

    // 用于跟踪使用的 inode 和数据块
    std::unordered_set<uint32_t> usedInodes;  //已使用inodes
    std::unordered_set<uint32_t> usedBlocks;  //已使用数据块

    // 存储最后一次命令的输出
    std::string lastOutput;
  
    // 获取最后一次命令的输出
    std::string getLastOutput() const;

    // 设置最后一次命令的输出（清空或设置）
    void setLastOutput(const std::string& output);


    /**
     * @brief 构造函数，初始化 CommandHandler 对象。
     * @param dm 磁盘管理器对象的引用。
     */
    CommandHandler(DiskManager& diskManager, UserManager& userManager);

    /**
     * @brief 处理输入的命令并调用对应的处理函数。
     * @param command 用户输入的命令字符串。
     */
    bool handleCommand(const std::string& command);

    /**
     * @brief 显示文件系统的整体信息。
     */
    void handleInfo();

    /**
     * @brief 更改当前工作目录。
     * @param path 要更改到的目标目录路径。
     */
    void handleCd(const std::string& path);

    /**
     * @brief 显示目录内容。
     * @param path 目标目录路径（默认显示当前目录）。
     * @param recursive 是否递归显示子目录内容。
     */
    void handleDir(const std::string& path = "", bool recursive = false);

    /**
     * @brief 创建新目录。
     * @param dirName 新目录的名称。
     */
    void handleMd(const std::string& dirName);

    /**
     * @brief 删除指定目录。
     * @param dirName 要删除的目录名称。
     */
    void handleRd(const std::string& dirName);

    /**
     * @brief 创建新文件。
     * @param fileName 新文件的名称。
     */
    void handleNewFile(const std::string& fileName);

    /**
 * @brief 写入文件内容。
 * @param filePath 要写入的文件路径。
 * @param append 是否以追加模式写入，默认为 false（覆盖写入）。
 */
    void handleWriteFile(const std::string& filePath, bool append = false);


    /**
     * @brief 显示文件内容。
     * @param fileName 要显示内容的文件名称。
     */
    void handleCat(const std::string& fileName);

    /**
     * @brief 拷贝文件。
     * @param src 源文件路径。
     * @param dest 目标文件路径。
     */
    void handleCopy(const std::string& src, const std::string& dest);

    /**
     * @brief 删除指定文件。
     * @param fileName 要删除的文件名称。
     */
    void handleDel(const std::string& fileName);

    /**
     * @brief 检测文件系统的状态并尝试修复。
     */
    void handleCheck();

    /**
     * @brief 解析路径字符串，分解为路径组件列表。
     * @param path 要解析的路径字符串。
     * @return 包含路径各部分的字符串列表。
     */
    std::vector<std::string> parsePath(const std::string& path);        //路径解析函数，用于去掉path中的"/",从而使其便于处理

    /**
    * @brief 解析路径，获取父路径和文件名。
    * @param fullPath 完整的路径。
    * @param parentPath 输出参数，返回父路径。
    * @param fileName 输出参数，返回文件名。
    */
    void parsePath(const std::string& fullPath, std::string& parentPath, std::string& fileName);

    /**
   * @brief 解析路径字符串，分解为路径组件列表。
   * @param path 要解析的路径字符串。
   * @return 包含路径各部分的字符串列表。
   */
    std::vector<std::string> parsePath_normal(const std::string& path);      //路径构造函数，用于最后对path进行规范化处理，去除path中的"."和".."

    /**
     * @brief 更新命令提示符以反映当前路径。
     */
    void updatePrompt();

    /**
     * @brief 显示目录内容，包括文件和子目录。
     * @param dir 要显示的目录对象。
     * @param dirInodeIndex 目录对应的 inode 索引。
     * @param recursive 是否递归显示子目录内容。
     * @param indent 缩进字符串，用于递归显示时的层级结构。
     */
    void displayDirectoryContents(const Directory& dir, uint32_t dirInodeIndex, bool recursive, const std::string& indent);

    /**
     * @brief 根据 inode 加载对应的目录对象。
     * @param inode 要加载的 inode 对象。
     * @return 加载后的目录对象。
     */
    Directory loadDirectoryFromINode(const INode& inode);

    /**
     * @brief 递归删除目录及其所有内容。
     * @param inodeIndex 要删除的目录对应的 inode 索引。
     * @return 删除是否成功。
     */
    bool deleteDirectoryRecursively(uint32_t inodeIndex);

    /**
 * @brief 根据给定的路径，导航到目标目录，返回对应的 inode 索引和目录对象。
 * @param path 要导航的路径，可以是绝对路径或相对路径。
 * @param resultInodeIndex 输出参数，返回目标目录的 inode 索引。
 * @param resultDirectory 输出参数，返回目标目录的 Directory 对象。
 * @return 成功返回 true，失败返回 false。
 */
    bool navigateToPath(const std::string& path, uint32_t& resultInodeIndex, Directory& resultDirectory);

    /**
       * @brief 递归遍历文件系统，从指定的 inode 开始。
       * @param inodeIndex 起始 inode 的索引。
       * 该函数会收集所有正在使用的 inode 和数据块信息。
       */
    void traverseFileSystem(uint32_t inodeIndex);

    /**
    * @brief 检查当前用户是否有对指定 inode 进行指定操作的权限。
    * @param inode 要检查的 inode。
    * @param permissionType 操作类型，"r"：读取，"w"：写入，"x"：执行。
    * @return 有权限返回 true，无权限返回 false。
    */
    bool checkPermission(const INode& inode, char permissionType);

    /**
    * @brief 在模拟文件系统内部复制文件。
    * @param srcPath 源文件的路径。
    * @param destPath 目标文件的路径。
    */
    void copyWithinSimDisk(const std::string& srcPath, const std::string& destPath);

    /**
     * @brief 从主机文件系统复制文件到模拟文件系统。
     * @param hostPath 主机文件系统中的文件路径。
     * @param simDiskPath 模拟文件系统中的目标路径。
     */
    void copyFromHostToSimDisk(const std::string& hostPath, const std::string& simDiskPath);

    /**
     * @brief 从模拟文件系统复制文件到主机文件系统。
     * @param simDiskPath 模拟文件系统中的文件路径。
     * @param hostPath 主机文件系统中的目标路径。
     */
    void copyFromSimDiskToHost(const std::string& simDiskPath, const std::string& hostPath);

    /**
    * @brief 根据文件路径获取对应的 inode。
    * @param filePath 文件的路径。
    * @param inodeIndex 输出参数，返回 inode 的索引。
    * @param inode 输出参数，返回 inode 对象。
    * @return 如果成功找到文件并获取 inode，返回 true；否则返回 false。
    */
    bool getFileINode(const std::string& filePath, uint32_t& inodeIndex, INode& inode);
};

#endif // COMMANDHANDLER_H
