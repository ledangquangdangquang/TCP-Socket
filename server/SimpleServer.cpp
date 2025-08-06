#include "SimpleServer.h"
#include <QDebug>
#include <unistd.h> // 👈 để dùng STDIN_FILENO
#define PORT 1234
SimpleServer::SimpleServer(QObject *parent) : QTcpServer(parent) {
    listen(QHostAddress::Any, PORT);
    qDebug() << "Listening on port" << PORT<< "...";
    connect(this, &QTcpServer::newConnection, this, &SimpleServer::onNewConnection);
}

void SimpleServer::onNewConnection() {
    clientSocket = nextPendingConnection();
    qDebug() << "Client kết nối!";

    connect(clientSocket, &QTcpSocket::readyRead, this, &SimpleServer::onClientReadyRead);
    connect(clientSocket, &QTcpSocket::disconnected, this, &SimpleServer::onClientDisconnected);
    // Add to chat client
    stdinNotifier = new QSocketNotifier(STDIN_FILENO, QSocketNotifier::Read, this);
    connect(stdinNotifier, &QSocketNotifier::activated, this, &SimpleServer::onServerInput);
}
// Add to chat client
void SimpleServer::onServerInput() {
    if (!clientSocket) return;

    QTextStream in(stdin);
    QString line = in.readLine();
    if (!line.isEmpty()) {
        clientSocket->write(line.toUtf8());
    }
}

void SimpleServer::onClientReadyRead() {
    QByteArray data = clientSocket->readAll();
    qDebug() << "Client gửi:" << data;
//                                     clientSocket->write("Server nhận rồi!");
}

void SimpleServer::onClientDisconnected() {
    qDebug() << "Client ngắt kết nối";
    clientSocket->deleteLater();
    clientSocket = nullptr;
}
