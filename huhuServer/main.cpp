// SERVER
#include <QCoreApplication>
#include "SimpleServer.h"
#include <QLoggingCategory>
#include <QFile>
#include <QTextStream>
#include <QDateTime>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    SimpleServer::setupLogging();
    qDebug() << "--- SERVER ---";
    SimpleServer server;
    return a.exec();
}
