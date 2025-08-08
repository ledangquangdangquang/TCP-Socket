// CLIENT
#include <QCoreApplication>
#include "SimpleClient.h"
#include <QTimer>
#include <QDebug>
#include <QDir>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    SimpleClient::setupLogging();

    qDebug() << "--- CLIENT RASPBERRY PI ---";
    qDebug() << "pwd:" << QDir::currentPath();

    SimpleClient client;
    client.connectToServer("192.168.30.77", 12345);

    return a.exec();
}
