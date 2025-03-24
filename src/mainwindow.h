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
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

signals:
    void signInButtonClicked(const QString &login, const QString &pass);

private:
    Ui::MainWindow *ui;

    LoginProcessor loginProccessor;

    void bindConnects();

private slots:
    void onSwitchToLog();
    void onSwitchToReg();

public slots:
    void firstLaunch(bool isFirtLaunch);
};
#endif // MAINWINDOW_H
