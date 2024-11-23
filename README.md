# SimDiskFS：模拟 Linux 文件系统

## 目录

- [简介](#%E7%AE%80%E4%BB%8B)
- [功能特性](#%E5%8A%9F%E8%83%BD%E7%89%B9%E6%80%A7)
- [项目架构](#%E9%A1%B9%E7%9B%AE%E6%9E%B6%E6%9E%84)
- [程序流程](#%E7%A8%8B%E5%BA%8F%E6%B5%81%E7%A8%8B)
- [实现策略](#%E5%AE%9E%E7%8E%B0%E7%AD%96%E7%95%A5)
- [安装](#%E5%AE%89%E8%A3%85)
- [使用方法](#%E4%BD%BF%E7%94%A8%E6%96%B9%E6%B3%95)
- [命令列表](#%E5%91%BD%E4%BB%A4%E5%88%97%E8%A1%A8)
- [示例](#%E7%A4%BA%E4%BE%8B)
- [贡献](#%E8%B4%A1%E7%8C%AE)
- [许可证](#%E8%AE%B8%E5%8F%AF%E8%AF%81)
- [致谢](#%E8%87%B4%E8%B0%A2)

---

## 简介

**SimDiskFS** 是一个使用 C++ 编写的项目，旨在模拟一个简化的 Linux 文件系统。它在您的主机系统上创建一个大型虚拟磁盘文件，并提供基本的文件系统功能，如创建、删除、复制文件和目录，以及管理用户权限。该项目旨在帮助理解文件系统和操作系统的内部工作原理。

## 功能特性

- **虚拟磁盘模拟**：创建一个 100MB 的虚拟磁盘文件，模拟磁盘操作。
- **基本文件操作**：支持创建、读取、写入、复制和删除文件和目录。
- **用户管理**：包括用户身份验证和权限管理，区分管理员和普通用户。
- **路径支持**：处理以 `/` 分隔的绝对路径和相对路径，符合 Linux 风格。
- **文件系统命令**：实现了常用的文件系统命令，如 `info`、`cd`、`dir`、`md`、`rd`、`newfile`、`writefile`、`cat`、`copy`、`del` 和 `check`。
- **主机文件系统集成**：支持在主机文件系统和模拟文件系统之间复制文件。

## 项目架构

该项目被模块化为多个组件，每个组件负责特定的功能：

- **DiskManager（磁盘管理器）**：管理虚拟磁盘文件，包括块的分配、释放，以及底层的读/写操作。
- **SuperBlock（超级块）**：存储全局文件系统元数据，如总块数、空闲块数、索引节点信息和根目录的索引节点索引。
- **INode（索引节点）**：表示文件和目录的元数据，包括类型、大小、权限、所有者和数据块指针。
- **Directory（目录）**：管理目录结构，将文件和目录名映射到相应的索引节点索引。
- **UserManager（用户管理器）**：处理用户身份验证、会话管理和权限检查。
- **CommandHandler（命令处理器）**：解析并执行用户命令，根据需要与其他组件交互。

## 程序流程

1. **初始化**：
   
   - 第一次运行时，程序会初始化虚拟磁盘文件（`simdisk.bin`），设置超级块、索引节点区域和根目录。
   - 如果磁盘文件已存在，则加载现有的文件系统结构。

2. **用户身份验证**：
   
   - 提示用户登录。
   - 使用 `UserManager` 验证凭据。
   - 成功认证后，开始用户会话。

3. **命令循环**：
   
   - 显示反映当前用户和目录的命令提示符。
   - 等待用户输入并解析命令。
   - 根据命令，调用 `CommandHandler` 中的适当方法。

4. **命令执行**：
   
   - 执行文件系统操作（如创建、删除、复制），并进行必要的权限检查。
   - 相应地更新虚拟磁盘结构。
   - 向用户提供反馈或错误信息。

5. **终止**：
   
   - 用户可以通过输入 `logout` 注销或 `exit` 退出程序。
   - 在终止前，确保所有数据都已写入虚拟磁盘文件。

## 实现策略

- **虚拟磁盘模拟**：
  
  - 使用一个大型二进制文件模拟磁盘存储。
  - 将磁盘划分为固定大小的块（每个 1KB）。
  - 使用位图管理空闲块，实现高效的分配。

- **文件系统结构**：
  
  - **超级块（SuperBlock）**：放置在磁盘文件的开头，包含关键的元数据。
  - **索引节点（INodes）**：为每个文件和目录存储元数据；包含索引节点位图用于管理。
  - **目录（Directories）**：实现为包含名称到索引节点索引映射的特殊文件。
  - **数据块（Data Blocks）**：存储实际的文件内容。

- **权限管理**：
  
  - 使用类似 Unix 的权限位（读、写、执行）管理所有者和其他用户的权限。
  - 与 `UserManager` 集成，强制执行访问控制。

- **命令处理**：
  
  - `CommandHandler` 类解释用户输入并执行相应的操作。
  - 每个命令封装在自己的方法中，保证代码的清晰和模块化。

- **数据持久化**：
  
  - 所有文件系统更改都会立即写入虚拟磁盘文件。
  - 确保程序执行之间的数据一致性。

## 安装

### 先决条件

- **操作系统**：Linux
- **C++ 编译器**：支持 C++11 或更高版本的 GCC 或 Clang
- **CMake**：版本 3.10 或更高

### 步骤

1. **克隆仓库**
   
   `git clone https://github.com/your_username/simdiskfs.git cd simdiskfs`

2. **创建构建目录并生成 Makefile**
   
   `mkdir build cd build cmake ..`

3. **编译项目**
   
   `make`

4. **运行程序**
   
   `./simdisk`

## 使用方法

### 启动程序

- 运行 `./simdisk` 后，程序会提示您登录。

- 默认的管理员账户：
  
  - **用户名**：`admin`
  - **密码**：`admin123`

- 登录成功后，您将看到命令提示符：
  
  
  
  `admin@simdisk:/>`

### 命令语法

- 在命令提示符下输入命令。
- 路径可以是绝对路径（以 `/` 开头）或相对于当前目录的相对路径。
- 使用 Linux 风格的路径分隔符（`/`）。

## 命令列表

1. **info**
   
   - **描述**：显示文件系统信息。
   - **用法**：`info`

2. **cd**
   
   - **描述**：改变当前工作目录。
   - **用法**：`cd <path>`
   - **示例**：`cd /home/user`

3. **dir**
   
   - **描述**：列出目录内容。
   - **用法**：`dir [options] [path]`
   - **选项**：
     - `/s`：递归列出所有子目录。
   - **示例**：`dir /s /home`

4. **md**
   
   - **描述**：创建新目录。
   - **用法**：`md <dirname>`
   - **示例**：`md documents`

5. **rd**
   
   - **描述**：删除目录。
   - **用法**：`rd <dirname>`
   - **示例**：`rd old_documents`

6. **newfile**
   
   - **描述**：创建新文件。
   - **用法**：`newfile <filename>`
   - **示例**：`newfile notes.txt`

7. **writefile**
   
   - **描述**：向已存在的文件写入内容。
   - **用法**：`writefile <filename>`
   - **示例**：`writefile notes.txt`

8. **cat**
   
   - **描述**：显示文件内容。
   - **用法**：`cat <filepath>`
   - **示例**：`cat notes.txt`

9. **copy**
   
   - **描述**：复制文件。
   - **用法**：`copy <source> <destination>`
   - **示例**：
     - 内部复制：`copy notes.txt backup/notes_backup.txt`
     - 从主机复制到 SimDisk：`copy <host>/home/user/file.txt /simdisk/dir`
     - 从 SimDisk 复制到主机：`copy /simdisk/dir/file.txt <host>/home/user`

10. **del**
    
    - **描述**：删除文件。
    - **用法**：`del <filename>`
    - **示例**：`del old_notes.txt`

11. **check**
    
    - **描述**：检查并修复文件系统。
    - **用法**：`check`

12. **logout**
    
    - **描述**：注销当前用户。
    - **用法**：`logout`

13. **exit**
    
    - **描述**：退出程序。
    - **用法**：`exit`

## 示例

### 创建和导航目录

plaintext

`admin@simdisk:/> md projects 目录 'projects' 创建成功。  admin@simdisk:/> cd projects admin@simdisk:/projects> md simdiskfs 目录 'simdiskfs' 创建成功。  admin@simdisk:/projects> cd simdiskfs admin@simdisk:/projects/simdiskfs>`

### 创建和写入文件

plaintext

`admin@simdisk:/projects/simdiskfs> newfile README.md 请输入新文件的内容（以单独一行的 '.' 结束）： # SimDiskFS 项目 这是一个模拟的文件系统项目。 . 文件 'README.md' 创建成功。`

如果您想向已存在的文件追加或覆盖内容，请使用 `writefile` 命令：

plaintext

`admin@simdisk:/projects/simdiskfs> writefile README.md 请输入要写入文件的内容（以单独一行的 '.' 结束）： ## 功能特性 - 虚拟磁盘模拟 - 基本文件操作 . 内容已成功写入 'README.md'。`

### 查看文件内容

plaintext

`admin@simdisk:/projects/simdiskfs> cat README.md # SimDiskFS 项目 这是一个模拟的文件系统项目。 ## 功能特性 - 虚拟磁盘模拟 - 基本文件操作`

### 复制文件

- **内部复制**
  
  plaintext
  
  `admin@simdisk:/projects/simdiskfs> copy README.md /backup/README_backup.md 已成功将 'README.md' 复制到 '/backup/README_backup.md'。`

- **从主机复制到 SimDisk**
  
  plaintext
  
  `admin@simdisk:/projects/simdiskfs> copy <host>/home/user/document.txt /projects/simdiskfs 已成功将主机文件 '/home/user/document.txt' 复制到 SimDisk 路径 '/projects/simdiskfs'。`

- **从 SimDisk 复制到主机**
  
  plaintext
  
  `admin@simdisk:/projects/simdiskfs> copy README.md <host>/home/user/README_copy.md 已成功将 SimDisk 文件 'README.md' 复制到主机路径 '/home/user/README_copy.md'。`

### 删除文件和目录

plaintext

`admin@simdisk:/projects/simdiskfs> del old_file.txt 文件 'old_file.txt' 已成功删除。  admin@simdisk:/projects> rd simdiskfs 目录 'simdiskfs' 不为空，确定要删除它和其中的所有内容吗？（y/n）：y 目录 'simdiskfs' 已成功删除。`

### 检查文件系统完整性

plaintext

`admin@simdisk:/projects> check 文件系统检查完成，未发现错误。`

## 贡献

欢迎提出意见和建议！请 fork 该仓库并提交 pull request 进行审阅。

## 许可证

本项目采用 MIT 许可证 - 有关详细信息，请参阅 [LICENSE](LICENSE) 文件。

## 致谢

- 感谢所有在本项目开发过程中提供帮助的贡献者和合作伙伴。
- 灵感来源于 Linux 文件系统概念和操作系统课程。
