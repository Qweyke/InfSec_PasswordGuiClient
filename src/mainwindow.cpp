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
        if (!ui->logNameLineEdit->text().isNull() && !ui->logPassLineEdit->text().isNull())
            emit signInButtonClicked(ui->logNameLineEdit->text(), ui->logPassLineEdit->text());
    });
    connect(this, &MainWindow::signInButtonClicked, &loginProccessor, &LoginProcessor::signIn);

    // First launch
    connect(&loginProccessor, &LoginProcessor::firstLaunch, this, &MainWindow::firstLaunch);

    connect(ui->logSwitchButton, &QPushButton::clicked, this, &MainWindow::onSwitchToLog);
    connect(ui->regSwitchButton, &QPushButton::clicked, this, &MainWindow::onSwitchToReg);
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

void MainWindow::firstLaunch(bool isFirtLaunch)
{
    if (isFirtLaunch) {
        onSwitchToReg();
        ui->logNameLineEdit->setText("admin");
    } else {
        onSwitchToLog();
    }
}
