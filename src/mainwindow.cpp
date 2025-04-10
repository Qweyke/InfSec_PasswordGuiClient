#include "mainwindow.h"

#include "./ui_mainwindow.h"

#include <QComboBox>
#include <QDialog>
#include <QFormLayout>
#include <QMenu>

MainWindow::MainWindow(LoginProcessor &loginProcessor, QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , loginProcessor(loginProcessor)
{
    ui->setupUi(this);
    adjustSize();
    bindConnects();

    // Set welcome page as first
    ui->mainStackedWidget->setCurrentIndex(0);

    // Set login page on welcome page
    switchWelcomeView(0);

    // Set model for view
    ui->usersListView->setModel(loginProcessor.getUsersListModel());
    ui->usersListView->setContextMenuPolicy(Qt::CustomContextMenu);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::bindConnects()
{
    // Log button
    connect(ui->logInButton, &QPushButton::clicked, this, [this]() {
        if (!ui->userNameLineEdit->text().isEmpty() && !ui->passLineEdit->text().isEmpty())
            loginProcessor.logIn(ui->userNameLineEdit->text(), ui->passLineEdit->text());
    });

    // Reg button
    connect(ui->regButton, &QPushButton::clicked, this, [this]() {
        if (!ui->userNameLineEdit->text().isEmpty() && !ui->passLineEdit->text().isEmpty())
            loginProcessor.regUser(ui->userNameLineEdit->text(), ui->passLineEdit->text());
    });

    // Welcome page switch buttons
    connect(ui->logSwitchButton, &QPushButton::clicked, this, [this]() { switchWelcomeView(0); });
    connect(ui->regSwitchButton, &QPushButton::clicked, this, [this]() { switchWelcomeView(1); });

    // Welcome page actions done
    connect(&loginProcessor, &LoginProcessor::onRegEnd, this, &MainWindow::doOnRegEnd);
    connect(&loginProcessor, &LoginProcessor::onLogIn, this, &MainWindow::doOnLogInEnd);

    // Change password
    connect(ui->changePassButton, &QPushButton::clicked, this, &MainWindow::doChangePassPressed);

    // Log out
    connect(ui->logOutButton, &QPushButton::clicked, this, [this]() {
        loginProcessor.logOut();
        ui->mainStackedWidget->setCurrentIndex(0);
        // adjustSize();
    });

    connect(ui->usersListView,
            &QListView::customContextMenuRequested,
            this,
            &MainWindow::onListViewContextMenu);
}

void MainWindow::onListViewContextMenu(const QPoint &pos)
{
    QModelIndex index = ui->usersListView->indexAt(pos);
    QMenu contextMenu(this);
    if (index.isValid()) {
        QAction *actionEdit = new QAction("Change permission", &contextMenu);
        QAction *actionDelete = new QAction("Delete", &contextMenu);

        contextMenu.addAction(actionEdit);
        contextMenu.addAction(actionDelete);

        connect(actionEdit, &QAction::triggered, this, [this, index]() {
            qDebug() << "Change permission triggered on user:" << index.data().toString();

            QDialog permissionDialog;
            QFormLayout dialogLayout(&permissionDialog);

            permissionDialog.setWindowTitle("Change Permission");

            QComboBox *permissionComboBox = new QComboBox(&permissionDialog);

            permissionComboBox->addItem("User",
                                        QVariant::fromValue(LoginProcessor::Permission::user));
            permissionComboBox->addItem("Banned",
                                        QVariant::fromValue(LoginProcessor::Permission::banned));

            dialogLayout.addRow("Permission: ", permissionComboBox);

            QPushButton confirmButton("Confirm");
            dialogLayout.addRow(&confirmButton);

            connect(&confirmButton, &QPushButton::clicked, &permissionDialog, &QDialog::accept);

            if (permissionDialog.exec() == QDialog::Accepted) {
                LoginProcessor::Permission selectedPermission
                    = permissionComboBox->currentData().value<LoginProcessor::Permission>();

                loginProcessor.changeUserPermission(index.data().toString(), selectedPermission);
            }
        });

        connect(actionDelete, &QAction::triggered, this, [this, index]() {
            qDebug() << "Delete triggered on user:" << index.data().toString();
        });

    } else {
        QAction *actionAddUser = new QAction("Add New User", &contextMenu);
        contextMenu.addAction(actionAddUser);

        connect(actionAddUser, &QAction::triggered, this, [this]() {
            qDebug() << "Add New User triggered";

            QDialog changePassDialog;
            QFormLayout dialogLayout(&changePassDialog);

            changePassDialog.setWindowTitle("Add New User");

            QLineEdit newPass, newLogin;
            newPass.setEchoMode(QLineEdit::Password);

            dialogLayout.addRow("Username: ", &newLogin);
            dialogLayout.addRow("Password: ", &newPass);

            QPushButton confirmButton("Confirm");
            dialogLayout.addRow(&confirmButton);

            connect(&confirmButton, &QPushButton::clicked, &changePassDialog, &QDialog::accept);

            if (changePassDialog.exec() == QDialog::Accepted) {
                loginProcessor.regUser(newLogin.text(), newPass.text());
            }
        });
    }
    contextMenu.exec(ui->usersListView->viewport()->mapToGlobal(pos));
}

void MainWindow::switchWelcomeView(quint16 pageIndex)
{
    ui->passLineEdit->clear();
    ui->regLogStackedWidget->setCurrentIndex(pageIndex);

    if (pageIndex == 0) {
        ui->logSwitchButton->setDisabled(true);
        ui->regSwitchButton->setDisabled(false);

    } else if (pageIndex == 1) {
        ui->regSwitchButton->setDisabled(true);
        ui->logSwitchButton->setDisabled(false);
    }
}

void MainWindow::doChangePassPressed()
{
    QDialog changePassDialog;
    QFormLayout dialogLayout(&changePassDialog);

    changePassDialog.setWindowTitle("Change password");

    QLineEdit oldPass, newPass;
    oldPass.setEchoMode(QLineEdit::Password);
    newPass.setEchoMode(QLineEdit::Password);

    dialogLayout.addRow("Old password: ", &oldPass);
    dialogLayout.addRow("New password: ", &newPass);

    QPushButton confirmButton("Confirm");
    dialogLayout.addRow(&confirmButton);

    connect(&confirmButton, &QPushButton::clicked, &changePassDialog, &QDialog::accept);

    if (changePassDialog.exec() == QDialog::Accepted) {
        loginProcessor.changePass(oldPass.text(), newPass.text());
    }
}

void MainWindow::doOnRegEnd(bool isSuccessReg)
{
    if (isSuccessReg) {
        switchWelcomeView(0);

    } else {
        ui->userNameLineEdit->clear();
        ui->passLineEdit->clear();
    }
}

void MainWindow::doOnLogInEnd(bool isSuccessLogIn, LoginProcessor::Permission permission)
{
    ui->passLineEdit->clear();

    if (!isSuccessLogIn) {
        return;
    }

    if (permission != LoginProcessor::Permission::banned) {
        ui->userNameLineEdit->clear();
        ui->mainStackedWidget->setCurrentIndex(1);
        ui->usersListView->setVisible(permission == LoginProcessor::Permission::admin);
    }
}
