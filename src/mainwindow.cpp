#include "mainwindow.h"

#include "./ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , loginProccessor()
{
    ui->setupUi(this);
    bindConnects();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::bindConnects()
{
    connect(ui->signInButton, &QPushButton::clicked, this, [this]() {
        emit signInButtonClicked(ui->loginLineEdit->text(), ui->passLineEdit->text());
    });
    connect(this, &MainWindow::signInButtonClicked, &loginProccessor, &LoginProcessor::signIn);
}
