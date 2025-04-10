#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <QMainWindow>
#include "loginprocessor.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(LoginProcessor &loginProccessor, QWidget *parent = nullptr);
    ~MainWindow();

signals:
    void logButtonClicked(const QString &login, const QString &pass);
    void regButtonClicked(const QString &login, const QString &pass);

private:
    Ui::MainWindow *ui;

    LoginProcessor &loginProccessor;

    void bindConnects();

private slots:
    void switchWelcomeView(quint16 pageIndex);

    void doOnRegEnd(bool isSuccessReg);
    void doOnLogInEnd(bool isSuccessLogIn, LoginProcessor::Permission permission);
    void doChangePassPressed();
};
#endif // MAINWINDOW_H
