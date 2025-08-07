#include "SimpleClient.h"
#include <QDebug>
//#include <iostream>
#include <QTextStream>
//#include <unistd.h> // 👈 để dùng STDIN_FILENO


SimpleClient::SimpleClient(QObject *parent) : QObject(parent) {
    connect(&socket, &QTcpSocket::connected, this, &SimpleClient::onConnected);
    connect(&socket, &QTcpSocket::readyRead, this, &SimpleClient::onReadyRead);
}

void SimpleClient::connectToServer(const QString &host, quint16 port) {
    socket.connectToHost(host, port);
}

void SimpleClient::sendMessage(const QByteArray &msg) {
    if(socket.state() == QTcpSocket::ConnectedState) {
        socket.write(msg);
    }
}

void SimpleClient::onConnected() {
    qDebug() << "Đã kết nối server";
    // Gắn stdin vào notifier để đọc lệnh chat
    stdinNotifier = new QSocketNotifier(0, QSocketNotifier::Read, this);
//    stdinNotifier = new QSocketNotifier(STDIN_FILENO, QSocketNotifier::Read, this);
    connect(stdinNotifier, &QSocketNotifier::activated, this, &SimpleClient::onUserInput);
}
void SimpleClient::onUserInput() {
    QTextStream in(stdin);
    QString line = in.readLine();
    if (!line.isEmpty()) {
        sendMessage(line.toUtf8());
    }
}
void SimpleClient::onReadyRead() {
    QByteArray data = socket.readAll();
    qDebug() << "Server trả lời:" <<QString::fromUtf8(data);
}
