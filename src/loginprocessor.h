#ifndef LOGINPROCESSOR_H
#define LOGINPROCESSOR_H

#include <QDebug>
#include <QFile>
#include <QJsonDocument>
#include <QString>

class LoginProcessor
{
public:
    LoginProcessor();
    void loginUser(const QString& name);

private:
    QString dbPath;
};

#endif // LOGINPROCESSOR_H
