#include "mainwindow.h"

#include "./ui_mainwindow.h"

#include <QFormLayout>
#include <QInputDialog>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , loginProccessor()
{
    ui->setupUi(this);
    bindConnects();

    // Set welcome page
    ui->mainStackedWidget->setCurrentIndex(0);

    // Check if app was launched for fisrt time or ADMIN pass isn't set
    loginProccessor.firstLaunchCheck();

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
            emit logButtonClicked(ui->userNameLineEdit->text(), ui->passLineEdit->text());
    });
    connect(this, &MainWindow::logButtonClicked, &loginProccessor, &LoginProcessor::logIn);

    // Reg button
    connect(ui->regButton, &QPushButton::clicked, this, [this]() {
        if (!ui->userNameLineEdit->text().isEmpty() && !ui->passLineEdit->text().isEmpty())
            emit regButtonClicked(ui->userNameLineEdit->text(), ui->passLineEdit->text());
    });
    connect(this, &MainWindow::regButtonClicked, &loginProccessor, &LoginProcessor::regUser);

    // Welcome page switch buttons
    connect(ui->logSwitchButton, &QPushButton::clicked, this, [this]() { switchWelcomeView(0); });
    connect(ui->regSwitchButton, &QPushButton::clicked, this, [this]() { switchWelcomeView(1); });

    // First launch handle
    connect(&loginProccessor, &LoginProcessor::firstLaunch, this, &MainWindow::firstLaunch);

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
    oldPass.setEchoMode(QLineEdit::Password);

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
        ui->infoLabel->setText("Register successful");

    } else {
        ui->infoLabel->setText("Register failed");
        ui->userNameLineEdit->clear();
        ui->passLineEdit->clear();
    }
}

void MainWindow::doOnLogInEnd(bool isSuccessLogIn, LoginProcessor::Permission permission)
{
    ui->passLabel->clear();

    if (!isSuccessLogIn) {
        ui->infoLabel->setText("Log in failed");
        return;
    }

    if (permission != LoginProcessor::Permission::banned) {
        ui->mainStackedWidget->setCurrentIndex(1);
        ui->usersListView->setVisible(permission == LoginProcessor::Permission::admin);
        adjustSize();
    } else
        ui->infoLabel->setText("User is banned");
}
void MainWindow::firstLaunch(bool isFirtLaunch)
{
    if (isFirtLaunch) {
        switchWelcomeView(1);
        // ui->regLogStackedWidget->layout()->replaceWidget(ui->regButton, ui->changePassButton);

        qWarning() << "Admin password isn't set yet";
        ui->userNameLineEdit->setText("admin");
        ui->infoLabel->setText("First launch. Set password for admin");

        // lock log in
        ui->logSwitchButton->setDisabled(true);
    } else
        switchWelcomeView(0);
}
