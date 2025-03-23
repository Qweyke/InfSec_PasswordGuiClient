#include "loginprocessor.h"

namespace {
QString APP_PATH = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation)
                   + "/PasswordGuiClient";
QString FILE_PATH = APP_PATH + "/db.json";

} // namespace

LoginProcessor::LoginProcessor()
    : dbFile(FILE_PATH)
{
    qInfo() << "App working folder path:" << APP_PATH;

    QDir appDir(APP_PATH);
    if (!appDir.exists()) {
        if (!appDir.mkdir(".")) {
            qCritical() << "Creating app dir error";
            return;
        }
    }

    if (dbFile.exists())
        readData();

    // If db doesn't exist yet - create it
    else {
        qWarning() << "First launch";
        QJsonObject adminUser;
        adminUser["password"] = "";
        adminUser["permissions"] = Permission::admin;

        // Adding admin to array of users
        dbUsers["admin"] = adminUser;

        dumpData();
        currentUserName = "admin";
    }
}

void LoginProcessor::firstLaunchCheck()
{
    if (dbUsers.contains("admin")) {
        QJsonObject currentUser = dbUsers.value("admin").toObject();

        if (currentUser["password"].isNull()) {
            currentUserName = "admin";
            emit firstLaunch();
        }
    }
}

void LoginProcessor::dumpData()
{
    if (dbFile.open(QIODevice::WriteOnly)) {
        //Reformat json db obj to json doc

        QJsonObject root;
        root["users"] = dbUsers;

        QJsonDocument dbJsonDoc(root);
        dbFile.write(dbJsonDoc.toJson());

        dbFile.close();
        qInfo() << "Dump data success";

    } else {
        qCritical() << "Open db file error";
    }
}

void LoginProcessor::readData()
{
    if (dbFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QByteArray dbFileData = dbFile.readAll();
        dbFile.close();

        QJsonDocument dbJsonDoc = QJsonDocument::fromJson(dbFileData);
        if (dbJsonDoc.isNull()) {
            qCritical() << "Db file parsing error";
        }

        QJsonObject dbObject = dbJsonDoc.object();

        if (dbObject.contains("users") && dbObject["users"].isObject()) {
            dbUsers = dbObject["users"].toObject();
        } else {
            qCritical() << "Unexpected db file content";
        }

        dbFile.close();
        qInfo() << "Load data success";

    } else {
        qCritical() << "Open db file error";
    }
}

void LoginProcessor::registerUser(const QString &login, const QString &pass)
{
    if (!dbUsers.contains(login)) {
        QJsonObject newUser;
        newUser["password"] = pass;
        newUser["permissions"] = Permission::user;

        dbUsers[login] = newUser;
        dumpData();
        qInfo() << "Register user success";
    } else
        qWarning() << "User already exists";
}

void LoginProcessor::changePass(const QString &oldPass, const QString &newPass)
{
    if (dbUsers.contains(currentUserName)) {
        QJsonObject updatedUser = dbUsers.value(currentUserName).toObject();
        updatedUser["password"] = newPass;

        dbUsers[currentUserName] = updatedUser;
        dumpData();
    } else
        qCritical() << "Current user object is missing in db";
}

void LoginProcessor::signIn(const QString &login, const QString &pass)
{
    if (dbUsers.contains(login)) {
        QJsonObject currentUser = dbUsers.value(login).toObject();

        if (currentUser["password"].toString() == pass) {
            currentUserName = login;
            qInfo() << "User" << login << "log in success";
        }

    } else
        qWarning() << "No such user" << login << "in db";
}

void LoginProcessor::setPass(const QString &login, const QString &pass)
{
    if (dbUsers.contains(login)) {
        QJsonObject currentUser = dbUsers.value(login).toObject();

        currentUser["password"] = pass;

        qInfo() << "Password for" << login << "set success";

    } else
        qWarning() << "No such user" << login << "in db";
}
