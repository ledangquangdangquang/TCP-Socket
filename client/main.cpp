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
    client.connectToServer("127.0.0.1", 1234);
    QTimer::singleShot(1000, &a, [&client]() {
        client.sendMessage("Hello server!");
    });
    return a.exec();
}
