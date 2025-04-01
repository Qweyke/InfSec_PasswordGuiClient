#ifndef LOGINPROCESSOR_H
#define LOGINPROCESSOR_H

#include <QDebug>
#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStandardPaths>
#include <QString>

class LoginProcessor : public QObject
{
    Q_OBJECT

    enum Permission { admin = 0, user = 1, banned = 2 };

signals:
    void firstLaunch(bool isFirtLaunch);
    void onRegisterEnd(bool isSuccessReg);

public:
    LoginProcessor();
    void firstLaunchCheck();

public slots:
    void registerUser(const QString &login, const QString &pass);
    bool changePass(const QString &oldPass, const QString &newPass);
    void signIn(const QString &login, const QString &pass);
    void setPass(const QString &login, const QString &pass);

private:
    QFile dbFile;
    QJsonObject dbUsers;

    QString currentUserName;

    void readData();
    void dumpData();
};

#endif // LOGINPROCESSOR_H
