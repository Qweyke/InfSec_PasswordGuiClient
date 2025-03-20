#include "loginprocessor.h"

#define FILE_PATH "good"

LoginProcessor::LoginProcessor() {}

void LoginProcessor::loginUser(const QString& name)
{
    QFile dbFile(dbPath);

    if (!dbFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        //warning
        qWarning() << "Open file err";
    }

    QByteArray fileData = dbFile.readAll();
    dbFile.close();

    QJsonDocument dbJson = QJsonDocument::fromJson(fileData);
}
