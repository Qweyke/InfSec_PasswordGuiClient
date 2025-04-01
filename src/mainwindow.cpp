#include "mainwindow.h"

#include "./ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , loginProccessor()
{
    ui->setupUi(this);
    bindConnects();
    loginProccessor.firstLaunchCheck();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::bindConnects()
{
    // Sign in button
    connect(ui->logButton, &QPushButton::clicked, this, [this]() {
        if (!ui->loginLineEdit->text().isEmpty() && !ui->passLineEdit->text().isEmpty())
            emit logButtonClicked(ui->loginLineEdit->text(), ui->passLineEdit->text());
    });
    connect(this, &MainWindow::logButtonClicked, &loginProccessor, &LoginProcessor::signIn);

    connect(ui->regButton, &QPushButton::clicked, this, [this]() {
        if (!ui->loginLineEdit->text().isEmpty() && !ui->passLineEdit->text().isEmpty())
            emit regButtonClicked(ui->loginLineEdit->text(), ui->passLineEdit->text());
    });
    connect(this, &MainWindow::regButtonClicked, &loginProccessor, &LoginProcessor::registerUser);

    // First launch
    connect(&loginProccessor, &LoginProcessor::firstLaunch, this, &MainWindow::firstLaunch);

    connect(ui->logSwitchButton, &QPushButton::clicked, this, &MainWindow::onSwitchToLog);
    connect(ui->regSwitchButton, &QPushButton::clicked, this, &MainWindow::onSwitchToReg);

    connect(&loginProccessor, &LoginProcessor::onRegisterEnd, this, &MainWindow::onRegEnd);
}

void MainWindow::onSwitchToLog()
{
    ui->switchFormsWidget->setCurrentIndex(0);
    ui->logSwitchButton->setDisabled(true);
    ui->regSwitchButton->setDisabled(false);

    ui->logSwitchButton->setStyleSheet("color: blue; border: none; border-bottom: 2px solid blue;");
    ui->regSwitchButton->setStyleSheet("color: grey; ");
}

void MainWindow::onSwitchToReg()
{
    ui->switchFormsWidget->setCurrentIndex(1);
    ui->regSwitchButton->setDisabled(true);
    ui->logSwitchButton->setDisabled(false);

    ui->regSwitchButton->setStyleSheet(
        "color: green; border: none; border-bottom: 2px solid green;");
    ui->logSwitchButton->setStyleSheet("color: grey; ");
}

void MainWindow::onRegEnd(bool isSuccessReg)
{
    if (isSuccessReg) {
        // if first launch
        ui->logSwitchButton->setDisabled(false);

        ui->loginLabel->setText("");
        ui->passLabel->setText("");
        ui->infoLabel->setText("Register successful");

    } else {
        ui->passLabel->setText("");
        ui->infoLabel->setText("Register failed");
    }
}

void MainWindow::firstLaunch(bool isFirtLaunch)
{
    if (isFirtLaunch) {
        qWarning() << "Admin password isn't set yet";
        onSwitchToReg();
        ui->loginLineEdit->setText("admin");
        ui->infoLabel->setText("First launch. Set password for admin");
        // lock log in
        ui->logSwitchButton->setDisabled(true);

    } else {
        onSwitchToLog();
    }
}
