// SERVER
#include <QCoreApplication>
#include "SimpleServer.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    qDebug() << "--- SERVER ---";
    SimpleServer server;
    return a.exec();
}
