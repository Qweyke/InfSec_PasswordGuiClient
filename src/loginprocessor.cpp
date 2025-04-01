#include "loginprocessor.h"

namespace {
inline QString appPathConst()
{
    return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    // + "/PasswordGuiClient";
}

inline QString filePathConst()
{
    return appPathConst() + "/db.json";
}

inline QString adminConst()
{
    return "admin";
}

} // namespace

LoginProcessor::LoginProcessor()
    : dbFile(filePathConst())
{
    qInfo() << "App working folder path:" << appPathConst();

    QDir appDir(appPathConst());
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
        qDebug() << "Data base first launch";
        QJsonObject adminUser;
        adminUser["password"] = "";
        adminUser["permissions"] = Permission::admin;

        // Adding admin to array of users
        dbUsers[adminConst()] = adminUser;

        dumpData();
        currentUserName = adminConst();
    }
}

void LoginProcessor::firstLaunchCheck()
{
    if (dbUsers.contains(adminConst())) {
        QJsonObject currentUser = dbUsers.value(adminConst()).toObject();

        if (!currentUser.contains("password") || currentUser["password"].toString().isEmpty()) {
            currentUserName = adminConst();
            emit firstLaunch(true);
        } else
            emit firstLaunch(false);
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
        qDebug() << "Dump data success";

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
        qDebug() << "Load data success";

    } else {
        qCritical() << "Open db file error";
    }
}

void LoginProcessor::registerUser(const QString &login, const QString &pass)
{
    if (login.isEmpty() || login.isNull()) {
        emit onRegisterEnd(false);
        return;
    }

    if (dbUsers.contains(login)) {
        qWarning() << "User already exists";
        emit onRegisterEnd(false);
        return;
    }

    if (login == adminConst()) {
        changePass("", pass);
        qInfo() << "Set password for admin success";

    } else {
        QJsonObject newUser;
        newUser["password"] = pass;
        newUser["permissions"] = Permission::user;

        dbUsers[login] = newUser;
        dumpData();
        qInfo() << "Register user success";
    }

    emit onRegisterEnd(true);
}

bool LoginProcessor::changePass(const QString &oldPass, const QString &newPass)
{
    if (dbUsers.contains(currentUserName)) {
        QJsonObject userToHandle = dbUsers.value(currentUserName).toObject();

        if (oldPass == userToHandle.value("password").toString()
            || currentUserName == adminConst()) {
            QJsonObject updatedUser = dbUsers.value(currentUserName).toObject();
            updatedUser["password"] = newPass;

            dbUsers[currentUserName] = updatedUser;
            dumpData();

            if (currentUserName != adminConst())
                qInfo() << "Change password success";
            return true;
        } else
            qWarning() << "Wrong password, aborting password change";
    } else
        qCritical() << "Current user object is missing in db";

    return false;
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
