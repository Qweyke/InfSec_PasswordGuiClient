#include "mainwindow.h"

#include "./ui_mainwindow.h"

#include <QDialog>
#include <QFormLayout>

MainWindow::MainWindow(LoginProcessor &loginProccessor, QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , loginProccessor(loginProccessor)
{
    ui->setupUi(this);
    bindConnects();

    // Set welcome page as first
    ui->mainStackedWidget->setCurrentIndex(0);

    // Set login page on welcome page
    switchWelcomeView(0);

    // Set model for view
    ui->usersListView->setModel(loginProccessor.getUsersListModel());
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
            loginProccessor.logIn(ui->userNameLineEdit->text(), ui->passLineEdit->text());
    });

    // Reg button
    connect(ui->regButton, &QPushButton::clicked, this, [this]() {
        if (!ui->userNameLineEdit->text().isEmpty() && !ui->passLineEdit->text().isEmpty())
            loginProccessor.regUser(ui->userNameLineEdit->text(), ui->passLineEdit->text());
    });

    // Welcome page switch buttons
    connect(ui->logSwitchButton, &QPushButton::clicked, this, [this]() { switchWelcomeView(0); });
    connect(ui->regSwitchButton, &QPushButton::clicked, this, [this]() { switchWelcomeView(1); });

    // Welcome page actions done
    connect(&loginProccessor, &LoginProcessor::onRegEnd, this, &MainWindow::doOnRegEnd);
    connect(&loginProccessor, &LoginProcessor::onLogIn, this, &MainWindow::doOnLogInEnd);

    // Change password
    connect(ui->changePassButton, &QPushButton::clicked, this, &MainWindow::doChangePassPressed);
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
        loginProccessor.changePass(oldPass.text(), newPass.text());
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
    ui->passLabel->clear();

    if (!isSuccessLogIn) {
        return;
    }

    if (permission != LoginProcessor::Permission::banned) {
        ui->mainStackedWidget->setCurrentIndex(1);
        ui->usersListView->setVisible(permission == LoginProcessor::Permission::admin);
        adjustSize();
    }
}
