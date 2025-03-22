#include "loginprocessor.h"

namespace {
QString APP_PATH = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
QString FILE_PATH = APP_PATH + "/PasswordGuiClient/db.json";

} // namespace

LoginProcessor::LoginProcessor()
    : dbFile(FILE_PATH)
{
    qInfo() << "Data base path";

    QDir appDir(APP_PATH);
    if (!appDir.exists()) {
        if (!appDir.mkdir(".")) {
            qWarning() << "Mkdir err";
            return;
        }
    }

    if (dbFile.exists())
        readData();

    else {
        QJsonObject adminUser;
        adminUser["password"] = "";
        adminUser["permissions"] = Permission::admin;

        // Adding admin to array of users
        dbUsers["admin"] = adminUser;

        dumpData();
    }
}

void LoginProcessor::dumpData()
{
    if (dbFile.open(QIODevice::WriteOnly)) {
        //Reformat json db obj to json doc
        QJsonDocument dbJsonDoc(dbUsers);
        dbFile.write(dbJsonDoc.toJson());
        dbFile.close();
    } else {
        qWarning() << "Open file err";
        return;
    }
}

void LoginProcessor::readData()
{
    if (dbFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QByteArray dbFileData = dbFile.readAll();
        qDebug() << "File has been read";
        dbFile.close();

        QJsonDocument dbJsonDoc = QJsonDocument::fromJson(dbFileData);
        if (dbJsonDoc.isNull()) {
            qWarning() << "Parse json err";
            return;
        }

        qDebug() << "Document parsed";
        QJsonObject dbObject = dbJsonDoc.object();

        if (dbObject.contains("users") && dbObject["users"].isObject()) {
            dbUsers = dbObject;
            qDebug() << "Users loaded";
        } else {
            qWarning() << "Unexpected file content";
            return;
        }
    } else {
        qWarning() << "Open file err";
        return;
    }
}

void LoginProcessor::registerUser(const QString &login, const QString &pass)
{
    if (!dbUsers.contains(login)) {
        QJsonObject newUser;
        newUser["password"] = pass;
        newUser["permissions"] = Permission::user;
    }
}

void LoginProcessor::changePass(const QString &oldPass, const QString &newPass) {}
