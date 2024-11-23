# linux_filesystem

OS course design

Understood, here's a comprehensive `README.md` document that includes detailed information about the program flow, implementation strategies, project architecture, and the complete command list including the `writefile` command.

---

# SimDiskFS: A Simulated Linux File System

## Table of Contents

- [Introduction](#introduction)
- [Features](#features)
- [Project Architecture](#project-architecture)
- [Program Flow](#program-flow)
- [Implementation Strategies](#implementation-strategies)
- [Installation](#installation)
- [Usage](#usage)
- [Command List](#command-list)
- [Examples](#examples)
- [Contributing](#contributing)
- [License](#license)
- [Acknowledgements](#acknowledgements)

---

## Introduction

**SimDiskFS** is a C++ project that simulates a simplified Linux file system. It creates a virtual disk within a single large file on your host system and provides basic file system functionalities such as creating, deleting, copying files and directories, and managing user permissions. This project is designed to help understand the inner workings of file systems and operating systems.

## Features

- **Virtual Disk Simulation**: Creates a 100MB virtual disk file to simulate disk operations.
- **Basic File Operations**: Supports creating, reading, writing, copying, and deleting files and directories.
- **User Management**: Includes user authentication and permission management, distinguishing between administrators and regular users.
- **Path Support**: Handles both absolute and relative paths with Linux-style `/` separators.
- **File System Commands**: Implements common file system commands like `info`, `cd`, `dir`, `md`, `rd`, `newfile`, `writefile`, `cat`, `copy`, `del`, and `check`.
- **Host File System Integration**: Supports copying files between the host file system and the simulated file system.

## Project Architecture

The project is modularized into several components, each responsible for specific functionalities:

- **DiskManager**: Manages the virtual disk file, including block allocation, deallocation, and low-level read/write operations.
- **SuperBlock**: Stores global file system metadata such as total blocks, free blocks, inode information, and the root directory inode index.
- **INode**: Represents the metadata of files and directories, including type, size, permissions, owner, and pointers to data blocks.
- **Directory**: Manages directory structures, mapping file and directory names to their corresponding inode indices.
- **UserManager**: Handles user authentication, session management, and permission checks.
- **CommandHandler**: Parses and executes user commands, interacting with other components as needed.

## Program Flow

1. **Initialization**:
   
   - On first run, the program initializes the virtual disk file (`simdisk.bin`), setting up the superblock, inode area, and root directory.
   - Loads existing file system structures if the disk file already exists.

2. **User Authentication**:
   
   - Prompts the user to log in.
   - Validates credentials using the `UserManager`.
   - Starts a user session upon successful authentication.

3. **Command Loop**:
   
   - Displays a command prompt reflecting the current user and directory.
   - Waits for user input and parses the command.
   - Invokes the appropriate method in `CommandHandler` based on the command.

4. **Command Execution**:
   
   - Executes file system operations (e.g., create, delete, copy) with necessary permission checks.
   - Updates the virtual disk structures accordingly.
   - Provides feedback or error messages to the user.

5. **Termination**:
   
   - The user can log out or exit the program.
   - Ensures all data is flushed to the virtual disk file before termination.

## Implementation Strategies

- **Virtual Disk Simulation**:
  
  - Uses a large binary file to simulate disk storage.
  - Divides the disk into fixed-size blocks (1KB each).
  - Manages free blocks using a bitmap for efficient allocation.

- **File System Structures**:
  
  - **SuperBlock**: Placed at the beginning of the disk file, contains essential metadata.
  - **INodes**: Stores metadata for each file and directory; includes an inode bitmap for management.
  - **Directories**: Implemented as special files containing entries that map names to inode indices.
  - **Data Blocks**: Store actual file content.

- **Permission Management**:
  
  - Uses Unix-like permission bits (read, write, execute) for owner and others.
  - Integrates with `UserManager` to enforce access control.

- **Command Handling**:
  
  - The `CommandHandler` class interprets user input and executes corresponding operations.
  - Each command is encapsulated in its own method for clarity and modularity.

- **Data Persistence**:
  
  - All file system changes are immediately written to the virtual disk file.
  - Ensures data consistency across program executions.

## Installation

### Prerequisites

- **Operating System**: Linux
- **C++ Compiler**: GCC or Clang supporting C++11 or higher
- **CMake**: Version 3.10 or higher

### Steps

1. **Clone the Repository**
   
   bash
   
   复制代码
   
   `git clone https://github.com/your_username/simdiskfs.git cd simdiskfs`

2. **Create a Build Directory and Generate Makefile**
   
   bash
   
   复制代码
   
   `mkdir build cd build cmake ..`

3. **Compile the Project**
   
   bash
   
   复制代码
   
   `make`

4. **Run the Program**
   
   bash
   
   复制代码
   
   `./simdisk`

## Usage

### Starting the Program

- Upon running `./simdisk`, you will be prompted to log in.

- Default administrator credentials:
  
  - **Username**: `admin`
  - **Password**: `admin123`

- After logging in, you'll see a command prompt:
  
  ruby
  
  复制代码
  
  `admin@simdisk:/>`

### Command Syntax

- Commands are entered in the command prompt.
- Paths can be absolute (starting with `/`) or relative to the current directory.
- Use Linux-style path separators (`/`).

## Command List

1. **info**
   
   - **Description**: Displays file system information.
   - **Usage**: `info`

2. **cd**
   
   - **Description**: Changes the current working directory.
   - **Usage**: `cd <path>`
   - **Example**: `cd /home/user`

3. **dir**
   
   - **Description**: Lists the contents of a directory.
   - **Usage**: `dir [options] [path]`
   - **Options**:
     - `/s`: Recursively lists all subdirectories.
   - **Example**: `dir /s /home`

4. **md**
   
   - **Description**: Creates a new directory.
   - **Usage**: `md <dirname>`
   - **Example**: `md documents`

5. **rd**
   
   - **Description**: Deletes a directory.
   - **Usage**: `rd <dirname>`
   - **Example**: `rd old_documents`

6. **newfile**
   
   - **Description**: Creates a new file.
   - **Usage**: `newfile <filename>`
   - **Example**: `newfile notes.txt`

7. **writefile**
   
   - **Description**: Writes content to an existing file.
   - **Usage**: `writefile <filename>`
   - **Example**: `writefile notes.txt`

8. **cat**
   
   - **Description**: Displays the content of a file.
   - **Usage**: `cat <filepath>`
   - **Example**: `cat notes.txt`

9. **copy**
   
   - **Description**: Copies a file.
   - **Usage**: `copy <source> <destination>`
   - **Examples**:
     - Internal copy: `copy notes.txt backup/notes_backup.txt`
     - Copy from host to SimDisk: `copy <host>/home/user/file.txt /simdisk/dir`
     - Copy from SimDisk to host: `copy /simdisk/dir/file.txt <host>/home/user`

10. **del**
    
    - **Description**: Deletes a file.
    - **Usage**: `del <filename>`
    - **Example**: `del old_notes.txt`

11. **check**
    
    - **Description**: Checks and repairs the file system.
    - **Usage**: `check`

12. **logout**
    
    - **Description**: Logs out the current user.
    - **Usage**: `logout`

13. **exit**
    
    - **Description**: Exits the program.
    - **Usage**: `exit`

## Examples

### Creating and Navigating Directories

plaintext

复制代码

`admin@simdisk:/> md projects Directory 'projects' created successfully.  admin@simdisk:/> cd projects admin@simdisk:/projects> md simdiskfs Directory 'simdiskfs' created successfully.  admin@simdisk:/projects> cd simdiskfs admin@simdisk:/projects/simdiskfs>`

### Creating and Writing to a File

plaintext

复制代码

`admin@simdisk:/projects/simdiskfs> newfile README.md Enter content for the new file (end with a single '.' on a new line): # SimDiskFS Project This is a simulated file system project. . File 'README.md' created successfully.`

If you want to append or overwrite content in an existing file, use the `writefile` command:

plaintext

复制代码

`admin@simdisk:/projects/simdiskfs> writefile README.md Enter content to write to the file (end with a single '.' on a new line): ## Features - Virtual Disk Simulation - Basic File Operations . Content written to 'README.md' successfully.`

### Viewing File Content

plaintext

复制代码

`admin@simdisk:/projects/simdiskfs> cat README.md # SimDiskFS Project This is a simulated file system project. ## Features - Virtual Disk Simulation - Basic File Operations`

### Copying Files

- **Internal Copy**
  
  plaintext
  
  复制代码
  
  `admin@simdisk:/projects/simdiskfs> copy README.md /backup/README_backup.md Copied 'README.md' to '/backup/README_backup.md' successfully.`

- **Copy from Host to SimDisk**
  
  plaintext
  
  复制代码
  
  `admin@simdisk:/projects/simdiskfs> copy <host>/home/user/document.txt /projects/simdiskfs Copied host file '/home/user/document.txt' to SimDisk path '/projects/simdiskfs' successfully.`

- **Copy from SimDisk to Host**
  
  plaintext
  
  复制代码
  
  `admin@simdisk:/projects/simdiskfs> copy README.md <host>/home/user/README_copy.md Copied SimDisk file 'README.md' to host path '/home/user/README_copy.md' successfully.`

### Deleting Files and Directories

plaintext

复制代码

`admin@simdisk:/projects/simdiskfs> del old_file.txt File 'old_file.txt' deleted successfully.  admin@simdisk:/projects> rd simdiskfs Are you sure you want to delete directory 'simdiskfs' and all its contents? (y/n): y Directory 'simdiskfs' deleted successfully.`

### Checking File System Integrity

plaintext

复制代码

`admin@simdisk:/projects> check File system check completed. No errors found.`

## Contributing

Contributions are welcome! Please fork the repository and submit a pull request for review.

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Acknowledgements

- Thanks to all contributors and collaborators who have helped in the development of this project.
- Inspired by Linux file system concepts and operating system coursework.
