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

class LoginProcessor
{
    enum Permission { admin = 0, user = 1 };

public:
    LoginProcessor();
    void registerUser(const QString &login, const QString &pass);
    void changePass(const QString &oldPass, const QString &newPass);

private:
    QFile dbFile;
    QJsonObject dbUsers;
    void readData();
    void dumpData();
};

#endif // LOGINPROCESSOR_H
