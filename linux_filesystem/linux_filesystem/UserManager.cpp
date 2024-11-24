#include "UserManager.h"

UserManager::UserManager() : currentUID(-1) {
    // 初始化用户表，可以预置一些用户
    User admin = { 0, "admin", "admin123", true }; // admin 是管理员
    User user1 = { 1, "user1", "password1", false };
    User user2 = { 2, "user2", "password2", false };

    userTable[admin.username] = admin;
    userTable[user1.username] = user1;
    userTable[user2.username] = user2;
}

bool UserManager::login(const std::string& username, const std::string& password) {
    auto it = userTable.find(username);
    if (it != userTable.end() && it->second.password == password) {
        currentUID = it->second.uid;
        return true;
    }
    return false;
}

void UserManager::logout() {
    currentUID = -1;
}

int UserManager::getCurrentUID() const {
    return currentUID;
}

bool UserManager::isLoggedIn() const {
    return currentUID != -1;
}

std::string UserManager::getCurrentUsername() const {
    for (const auto& entry : userTable) {
        if (entry.second.uid == currentUID) {
            return entry.second.username;
        }
    }
    return "";
}

bool UserManager::isAdmin() const {
    for (const auto& entry : userTable) {
        if (entry.second.uid == currentUID) {
            return entry.second.isAdmin;
        }
    }
    return false;
}
