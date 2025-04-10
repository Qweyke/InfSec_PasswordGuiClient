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
    , usersListModel(new QStandardItemModel(this))
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
        adminUser["permission"] = convertPermissionToString(Permission::admin);

        // Adding admin to array of users
        dbUsers[adminConst()] = adminUser;

        dumpData();
        currentUserName = adminConst();
    }

    // Setup users list model
    usersListModel->setHorizontalHeaderLabels({"Username"});
    refreshUsersListModel();
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

void LoginProcessor::refreshUsersListModel()
{
    usersListModel->clear();

    for (QJsonObject::Iterator userObjIterator = dbUsers.begin(); userObjIterator != dbUsers.end();
         ++userObjIterator) {
        QString userName = userObjIterator.key();
        QStandardItem *userItem = new QStandardItem(userName);

        usersListModel->appendRow(userItem);
    }
}

QStandardItemModel *LoginProcessor::getUsersListModel()
{
    return usersListModel;
}

QString LoginProcessor::convertPermissionToString(Permission userPermission)
{
    switch (userPermission) {
    case Permission::admin:
        return "adm";
    case Permission::user:
        return "usr";
    case Permission::banned:
    default:
        return "ban";
    }
}

LoginProcessor::Permission LoginProcessor::convertStringToPermission(QString userPermission)
{
    if (userPermission == "adm")
        return Permission::admin;
    else if (userPermission == "usr")
        return Permission::user;
    else
        return Permission::banned;
}

void LoginProcessor::regUser(const QString &login, const QString &pass)
{
    // If no login - return
    if (login.isEmpty() || login.isNull()) {
        emit onRegEnd(false);

    } else if (dbUsers.contains(login)) {
        // If user is ADMIN - emit true
        // if (login == adminConst()) {
        //     changePass("", pass);
        //     qInfo() << "Set password for" << login << "success";
        //     emit onRegEnd(true);
        //     return;
        // }
        qWarning() << "User" << login << "already exists";
        emit onRegEnd(false);

    } else {
        QJsonObject newUser;
        newUser["password"] = pass;
        newUser["permission"] = convertPermissionToString(Permission::user);

        dbUsers[login] = newUser;
        dumpData();
        qInfo() << "Register" << login << "user success";

        emit onRegEnd(true);
    }
}

bool LoginProcessor::changePass(const QString &oldPass, const QString &newPass)
{
    if (dbUsers.contains(currentUserName)) {
        QJsonObject userToHandle = dbUsers.value(currentUserName).toObject();

        if (oldPass == userToHandle.value("password").toString()) {
            QJsonObject updatedUser = dbUsers.value(currentUserName).toObject();
            updatedUser["password"] = newPass;

            dbUsers[currentUserName] = updatedUser;
            dumpData();
            qInfo() << "Change password success";

            return true;
        } else
            qWarning() << "Wrong password, aborting password change";
    } else
        qCritical() << "Current user object is missing in db";

    return false;
}

void LoginProcessor::logIn(const QString &login, const QString &pass)
{
    if (dbUsers.contains(login)) {
        QJsonObject currentUser = dbUsers.value(login).toObject();

        if (currentUser["password"].toString() == pass) {
            Permission permission = convertStringToPermission(
                currentUser.value("permission").toString());

            currentUserName = login;

            qInfo() << "User" << login << "log in success";
            emit onLogIn(true, permission);

        } else {
            qInfo() << "User" << login << "password is wrong";
            emit onLogIn(false, Permission::banned);
        }

    } else {
        qWarning() << "No such user" << login << "in db";
        emit onLogIn(false, Permission::banned);
    }
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
