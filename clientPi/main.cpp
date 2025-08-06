// CLIENT
#include <QCoreApplication>
#include "SimpleClient.h"
#include <QTimer>
#include <QDebug>
int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    qDebug() << "--- CLIENT RASPBERRY PI ---";
    SimpleClient client;
    client.connectToServer("192.168.30.111", 1234);
    QTimer::singleShot(1000, &a, [&client]() {
        client.sendMessage("Hello server!");
    });
    return a.exec();
}
