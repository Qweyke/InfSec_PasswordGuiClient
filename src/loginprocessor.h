#ifndef LOGINPROCESSOR_H
#define LOGINPROCESSOR_H

#include <QDebug>
#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStandardItemModel>
#include <QStandardPaths>
#include <QString>

class LoginProcessor : public QObject
{
    Q_OBJECT

public:
    enum Permission { admin = 0, user = 1, banned = 2 };
    LoginProcessor();
    bool firstLaunchCheck();
    QStandardItemModel *getUsersListModel();

public slots:
    void regUser(const QString &login, const QString &pass);
    bool changePass(const QString &oldPass, const QString &newPass);
    void logIn(const QString &login, const QString &pass);
    void logOut();
    void setPass(const QString &login, const QString &pass);

signals:
    void onRegEnd(bool isSuccessReg);
    void onLogIn(bool isSuccessLogIn, Permission userPermission);

private:
    QFile dbFile;
    QJsonObject dbUsers;
    QStandardItemModel *usersListModel;

    QString currentUserName;

    void readData();
    void dumpData();

    bool setPassForAdmin();

    QString convertPermissionToString(Permission userPermission);
    Permission convertStringToPermission(QString userPermission);

    void refreshUsersListModel();
};

#endif // LOGINPROCESSOR_H
