#include "mainwindow.h"

#include <QApplication>

void logHandlerFunc(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    QString msgType;
    const char *color;

    switch (type) {
    case QtDebugMsg:
        msgType = "DEBUG";
        color = "\033[37m";
        break;
    case QtWarningMsg:
        msgType = "WARNING";
        color = "\033[33m";
        break;

    case QtCriticalMsg:
        msgType = "CRITICAL";
        color = "\033[31m";
        break;

    case QtInfoMsg:
        msgType = "INFO";
        color = "\033[32m";
        break;

    default:
        msgType = "UNKNWN";
        color = "\033[35m";
        break;
    }

    QString timeStamp = QDateTime::currentDateTime().toString("HH:mm:ss");
    QString outMsg = QString("[%1]-%2: %3").arg(timeStamp, msgType, msg);

    fprintf(stderr, "%s%s\033[0m\n", color, outMsg.toUtf8().constData());
}

int main(int argc, char *argv[])
{
    qInstallMessageHandler(logHandlerFunc);
    QApplication app(argc, argv);
    MainWindow window;
    window.show();

    return app.exec();
}
