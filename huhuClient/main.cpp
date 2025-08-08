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

    qDebug() << "--- CLIENT ---";
    qDebug() << "pwd:" << QDir::currentPath();

    SimpleClient client;
    client.connectToServer("127.0.0.1", 12345);

    return a.exec();
}
