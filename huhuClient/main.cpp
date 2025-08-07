// CLIENT
#include <QCoreApplication>
#include "SimpleClient.h"
#include <QTimer>
#include <QDebug>
int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    qDebug() << "--- CLIENT ---";
    SimpleClient client;
    client.connectToServer("127.0.0.1", 12345);


    return a.exec();
}
