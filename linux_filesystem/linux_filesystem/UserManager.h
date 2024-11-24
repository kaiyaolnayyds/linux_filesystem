#ifndef USERMANAGER_H
#define USERMANAGER_H

#include <string>
#include <unordered_map>

class UserManager {
public:
    UserManager();

    /**
     * @brief �û���¼��
     * @param username �û�����
     * @param password ���롣
     * @return �ɹ����� true��ʧ�ܷ��� false��
     */
    bool login(const std::string& username, const std::string& password);

    /**
     * @brief �û�ע����
     */
    void logout();

    /**
     * @brief ��ȡ��ǰ�û��� UID��
     * @return ��ǰ�û��� UID�����δ��¼������ -1��
     */
    int getCurrentUID() const;

    /**
     * @brief ����û��Ƿ��ѵ�¼��
     * @return �ѵ�¼���� true��δ��¼���� false��
     */
    bool isLoggedIn() const;

    /**
     * @brief ��ȡ��ǰ�û�����
     * @return ��ǰ�û��������δ��¼�����ؿ��ַ�����
     */
    std::string getCurrentUsername() const;

    /**
     * @brief ��鵱ǰ�û��Ƿ��ǹ���Ա��admin����
     * @return ����ǹ���Ա������ true�����򷵻� false��
     */
    bool isAdmin() const;

private:
    struct User {
        int uid;
        std::string username;
        std::string password;
        bool isAdmin; // �Ƿ�Ϊ����Ա
    };

    // �û������û���Ϣ��ӳ��
    std::unordered_map<std::string, User> userTable;

    // ��ǰ�û� UID��δ��¼ʱΪ -1
    int currentUID;
};

#endif // USERMANAGER_H
