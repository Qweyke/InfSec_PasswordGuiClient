#include "loginprocessor.h"
#include <QDialog>
#include <QFormLayout>
#include <QLineEdit>
#include <QPushButton>

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
    , currentUserName("")
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

bool LoginProcessor::firstLaunchCheck()
{
    if (dbUsers.contains(adminConst())) {
        QJsonObject currentUser = dbUsers.value(adminConst()).toObject();

        if (!currentUser.contains("password") || currentUser["password"].toString().isEmpty()) {
            currentUserName = adminConst();

            if (!setPassForAdmin())
                return false;
        }

        return true;
    }

    return false;
}

bool LoginProcessor::setPassForAdmin()
{
    qWarning() << "Admin password isn't set yet";

    QDialog changePassDialog;
    QFormLayout dialogLayout(&changePassDialog);

    changePassDialog.setWindowTitle("Set password for admin");

    QLineEdit newPass;
    newPass.setEchoMode(QLineEdit::Password);

    dialogLayout.addRow("Password: ", &newPass);

    QPushButton confirmButton("Confirm");
    dialogLayout.addRow(&confirmButton);

    QPushButton exitButton("Exit");
    dialogLayout.addRow(&exitButton);

    connect(&confirmButton, &QPushButton::clicked, &changePassDialog, &QDialog::accept);
    connect(&exitButton, &QPushButton::clicked, &changePassDialog, &QDialog::reject);

    if (changePassDialog.exec() == QDialog::Accepted) {
        changePass("", newPass.text());
        return true;
    } else
        return false;
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
        refreshUsersListModel();

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
        refreshUsersListModel();

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

        if (userName == adminConst())
            continue;

        QJsonObject userObj = userObjIterator.value().toObject();
        Permission permission = convertStringToPermission(userObj.value("permission").toString());
        QStandardItem *userItem = new QStandardItem(userName);

        if (permission == Permission::banned)
            userItem->setForeground(QColor(Qt::red));
        else if (permission == Permission::user)
            userItem->setForeground(QColor(Qt::green));

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
        QJsonArray forbiddenPasswords = userToHandle.value("forbidden passwords").toArray();

        if (!validatePassword(currentUserName, newPass)) {
            qWarning() << "Password isn't strong enough, aborting password change";
            return false;
        }

        if (oldPass == userToHandle.value("password").toString()
            || currentUserName == adminConst()) {
            QJsonObject updatedUser = dbUsers.value(currentUserName).toObject();
            updatedUser["password"] = newPass;

            dbUsers[currentUserName] = updatedUser;
            dumpData();
            qInfo() << "Change password success";

            return true;
        } else {
            qWarning() << "Wrong password, aborting password change";
        }
    } else {
        qCritical() << "Current user object is missing in db";
    }

    return false;
}

void LoginProcessor::logIn(const QString &login, const QString &pass)
{
    if (dbUsers.contains(login)) {
        QJsonObject currentUser = dbUsers.value(login).toObject();
        Permission permission = convertStringToPermission(
            currentUser.value("permission").toString());

        if (permission == Permission::banned) {
            qWarning() << "User" << login << "is banned";
            emit onLogIn(false, Permission::banned);
        }

        else if (currentUser["password"].toString() == pass) {
            currentUserName = login;

            qInfo() << "User" << login << "log in success";
            emit onLogIn(true, permission);

        } else {
            qWarning() << "User" << login << "password" << pass << "is wrong";
            emit onLogIn(false, Permission::banned);
        }

    } else {
        qWarning() << "No such user" << login << "in db";
        emit onLogIn(false, Permission::banned);
    }
}

void LoginProcessor::logOut()
{
    currentUserName = "";
}

void LoginProcessor::setPasswordRequirements(const QString &username,
                                             const QVector<int> &requiredGroups)
{
    if (!dbUsers.contains(username)) {
        qWarning() << "User not found!";
        return;
    }

    QJsonObject userToHandle = dbUsers.value(username).toObject();
    QJsonArray requiredGroupsArray;

    for (int group : requiredGroups) {
        requiredGroupsArray.append(group);
    }

    userToHandle["requiredGroups"] = requiredGroupsArray;
    dbUsers[username] = userToHandle;
    dumpData();
}

bool LoginProcessor::validatePassword(const QString &username, const QString &newPassword)
{
    if (!dbUsers.contains(username)) {
        qWarning() << "User not found!";
        return false;
    }

    QJsonObject userToHandle = dbUsers.value(username).toObject();
    QJsonArray requiredGroupsArray = userToHandle.value("requiredGroups").toArray();

    auto matchesCategory = [](QChar ch, int groupIndex) -> bool {
        ushort u = ch.unicode();
        switch (groupIndex) {
        case 0: // Digits [0-9]
            return ch.isDigit();
        case 1: // Upper Latin Letters [A-Z]
            return ch.isUpper() && ch.isLetter();
        case 2: // Lower Latin Letters [a-z]
            return ch.isLower() && ch.isLetter();
        case 3: // Special characters (!@#$...)
            return (u >= 33 && u <= 47) || (u >= 58 && u <= 64) || (u >= 91 && u <= 96)
                   || (u >= 123 && u <= 126);
        case 4: // Upper Cyrillic Letters
            return (u >= 0x0410 && u <= 0x042F) || u == 0x0401;
        case 5: // Lower Cyrillic Letters
            return (u >= 0x0430 && u <= 0x044F) || u == 0x0451;
        default:
            return false;
        }
    };

    for (int i = 0; i < requiredGroupsArray.size(); ++i) {
        int groupIndex = requiredGroupsArray[i].toInt();
        bool groupFound = false;

        for (const QChar &ch : newPassword) {
            if (matchesCategory(ch, groupIndex)) {
                groupFound = true;
                break;
            }
        }

        if (!groupFound) {
            qWarning() << "Password must contain a character from the required group:"
                       << groupIndex;
            return false;
        }
    }
    return true;
}

void LoginProcessor::changeUserPermission(const QString &login, Permission permission)
{
    if (dbUsers.contains(login)) {
        QJsonObject currentUser = dbUsers.value(login).toObject();

        currentUser["permission"] = convertPermissionToString(permission);
        qInfo() << "Change permission for user" << login << "successful";

        dbUsers[login] = currentUser;
        dumpData();

    } else {
        qWarning() << "No such user" << login << "in db";
    }
}
