#ifndef USERMANAGER_H
#define USERMANAGER_H

#include <string>
#include <unordered_map>

class UserManager {
public:
    UserManager();

    /**
     * @brief 用户登录。
     * @param username 用户名。
     * @param password 密码。
     * @return 成功返回 true，失败返回 false。
     */
    bool login(const std::string& username, const std::string& password);

    /**
     * @brief 用户注销。
     */
    void logout();

    /**
     * @brief 获取当前用户的 UID。
     * @return 当前用户的 UID，如果未登录，返回 -1。
     */
    int getCurrentUID() const;

    /**
     * @brief 检查用户是否已登录。
     * @return 已登录返回 true，未登录返回 false。
     */
    bool isLoggedIn() const;

    /**
     * @brief 获取当前用户名。
     * @return 当前用户名，如果未登录，返回空字符串。
     */
    std::string getCurrentUsername() const;

    /**
     * @brief 检查当前用户是否是管理员（admin）。
     * @return 如果是管理员，返回 true；否则返回 false。
     */
    bool isAdmin() const;

private:
    struct User {
        int uid;
        std::string username;
        std::string password;
        bool isAdmin; // 是否为管理员
    };

    // 用户名到用户信息的映射
    std::unordered_map<std::string, User> userTable;

    // 当前用户 UID，未登录时为 -1
    int currentUID;
};

#endif // USERMANAGER_H
