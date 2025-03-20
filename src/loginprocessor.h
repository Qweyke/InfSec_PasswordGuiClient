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
public:
    LoginProcessor();
    void dumpData();

private:
    QFile dbFile;
    QJsonObject dbUsers;
};

#endif // LOGINPROCESSOR_H
