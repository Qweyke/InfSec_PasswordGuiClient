#include "mainwindow.h"

#include "./ui_mainwindow.h"
#include "passanalyzer.h"

#include <QCheckBox>
#include <QComboBox>
#include <QDialog>
#include <QFormLayout>
#include <QGroupBox>
#include <QMenu>
#include <QMessageBox>

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

    // Suggest pass
    connect(ui->usersListView,
            &QListView::customContextMenuRequested,
            this,
            &MainWindow::onListViewContextMenu);

    connect(ui->checkStrengthAction, &QAction::triggered, this, &MainWindow::checkPassStrength);
}

void MainWindow::onListViewContextMenu(const QPoint &pos)
{
    QModelIndex index = ui->usersListView->indexAt(pos);
    QMenu contextMenu(this);
    if (index.isValid()) {
        QAction *actionEdit = new QAction("Change permission", &contextMenu);
        QAction* actionFordbid = new QAction("Forbid password", &contextMenu);

        contextMenu.addAction(actionEdit);
        contextMenu.addAction(actionFordbid);

        connect(actionEdit, &QAction::triggered, this, [this, index]() {

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

        connect(
            actionFordbid,
            &QAction::triggered,
            this,
            [this, index]() {
                QDialog forbidPasswordsDialog;
                forbidPasswordsDialog.setWindowTitle("Set required character groups");

                QFormLayout* dialogLayout = new QFormLayout(&forbidPasswordsDialog);

                QGroupBox* charGroup = new QGroupBox("Required character groups:");
                QVBoxLayout* groupLayout = new QVBoxLayout();

                QCheckBox* cbDigits = new QCheckBox("Digits [0–9]");
                QCheckBox* cbUpperLat = new QCheckBox("Upper Latin");
                QCheckBox* cbLowerLat = new QCheckBox("Lower Latin");
                QCheckBox* cbSpecial = new QCheckBox("Special characters (!@#$...)");
                QCheckBox* cbUpperCyr = new QCheckBox("Upper Cyrillic");
                QCheckBox* cbLowerCyr = new QCheckBox("Lower Cyrillic");

                QList<QCheckBox*> checks
                    = {cbDigits, cbUpperLat, cbLowerLat, cbSpecial, cbUpperCyr, cbLowerCyr};

                for (auto* cb : checks)
                    groupLayout->addWidget(cb);

                charGroup->setLayout(groupLayout);
                dialogLayout->addRow(charGroup);

                QPushButton* confirmButton = new QPushButton("Confirm");
                dialogLayout->addRow(confirmButton);

                connect(confirmButton, &QPushButton::clicked, this, [&]() {
                    QVector<int> requiredGroups;
                    for (int i = 0; i < checks.size(); ++i) {
                        if (checks[i]->isChecked())
                            requiredGroups.append(i);
                    }

                    if (requiredGroups.isEmpty()) {
                        QMessageBox::warning(&forbidPasswordsDialog,
                                             "Error",
                                             "Choose at least one option.");
                        return;
                    }

                    forbidPasswordsDialog.accept();
                });

                if (forbidPasswordsDialog.exec() == QDialog::Accepted) {
                    QString username = index.data().toString();

                    QVector<int> requiredGroups;
                    for (int i = 0; i < checks.size(); ++i) {
                        if (checks[i]->isChecked())
                            requiredGroups.append(i);
                    }

                    loginProcessor.setPasswordRequirements(username, requiredGroups);
                }
            },
            Qt::QueuedConnection);

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

void MainWindow::checkPassStrength()
{
    QDialog checkDialog;
    checkDialog.setWindowTitle("Check password strength");
    QFormLayout dialogLayout(&checkDialog);

    QLineEdit pass;
    QPushButton confirmButton("Check");

    dialogLayout.addRow("Check password: ", &pass);
    dialogLayout.addRow(&confirmButton);

    connect(&confirmButton, &QPushButton::clicked, &checkDialog, &QDialog::accept);

    if (checkDialog.exec() == QDialog::Accepted) {
        qDebug() << "Calculating password strength";
        checkPswdStrength(pass.text());
        checkPassStrength();
    }
}
