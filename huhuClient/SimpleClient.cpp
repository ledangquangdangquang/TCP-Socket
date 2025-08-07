#include "SimpleClient.h"
#include <QDebug>
#include <iostream>
#include <QTextStream>
#include <unistd.h> // 👈 để dùng STDIN_FILENO

/*
    Ham khoi tao
    Gán parent nếu có.
    Gắn connected → gọi onConnected() khi kết nối thành công.
    Gắn readyRead → gọi onReadyRead() khi có dữ liệu từ server.
*/
SimpleClient::SimpleClient(QObject *parent) : QObject(parent) {
    connect(&socket, &QTcpSocket::connected, this, &SimpleClient::onConnected);
    connect(&socket, &QTcpSocket::readyRead, this, &SimpleClient::onReadyRead);
}

/*
    Gọi hàm này để kết nối tới server tại địa chỉ host và cổng port.
    👉 Đây là bước khởi động kết nối TCP.
*/
void SimpleClient::connectToServer(const QString &host, quint16 port) {
    socket.connectToHost(host, port);
}

/*
    🔹 Gửi msg tới server nếu socket đang kết nối (ConnectedState).
    👉 Đảm bảo chỉ gửi khi đã kết nối thành công.
*/
void SimpleClient::sendMessage(const QByteArray &msg) {
    if(socket.state() == QTcpSocket::ConnectedState) {
        socket.write(msg);
    }
}

/*
    🔹 Khi client kết nối thành công, bật QSocketNotifier để nghe nhập từ bàn phím (stdin).
    🔹 Khi có input, gọi onUserInput() để gửi tin nhắn.
*/
void SimpleClient::onConnected() {
    qDebug() << "Đã kết nối server";
    sendMessage("Hello server!");  // Gửi ngay sau khi kết nối thành công
    // Gắn stdin vào notifier để đọc lệnh chat
    stdinNotifier = new QSocketNotifier(STDIN_FILENO, QSocketNotifier::Read, this);
    connect(stdinNotifier, &QSocketNotifier::activated, this, &SimpleClient::onUserInput);
}

// Đọc dòng từ bàn phím → nếu không rỗng → gửi tới server.
void SimpleClient::onUserInput() {
    QTextStream in(stdin);
    QString line = in.readLine();
    if (!line.isEmpty()) {
        sendMessage(line.toUtf8());
    }
}

//  Khi có dữ liệu từ server, đọc và in ra.
void SimpleClient::onReadyRead() {
    QByteArray data = socket.readAll();
    qDebug() << "Server trả lời:" <<QString::fromUtf8(data);
}
