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
    connect(ui->signInButton, &QPushButton::clicked, this, [this]() {
        if (!ui->loginLineEdit->text().isNull() && !ui->passLineEdit->text().isNull())
            emit signInButtonClicked(ui->loginLineEdit->text(), ui->passLineEdit->text());
    });
    connect(this, &MainWindow::signInButtonClicked, &loginProccessor, &LoginProcessor::signIn);

    // First launch
    connect(&loginProccessor, &LoginProcessor::firstLaunch, this, &MainWindow::firstLaunch);
}

void MainWindow::firstLaunch()
{
    ui->signInButton->setDisabled(true);
    ui->loginLineEdit->setText("admin");
}
