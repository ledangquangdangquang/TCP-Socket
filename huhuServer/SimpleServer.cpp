#include "SimpleServer.h"
#include "ClientHandler.h"
#include <QDebug>
#include <unistd.h> // 👈 để dùng STDIN_FILENO
#include <QThread>

#define PORT 1234
// ADD multi
/*
    Hàm khởi tạo `SimpleServer`:
    * Gọi `listen()` để bắt đầu lắng nghe trên cổng 12345.
    * Nếu thành công, in ra thông báo; nếu không, báo lỗi.
    * Tạo `QSocketNotifier` để theo dõi stdin (bàn phím).
    * Kết nối tín hiệu `activated` từ `stdinNotifier` tới slot `onServerInput()`
       để xử lý input từ người dùng nhập vào terminal.
*/
SimpleServer::SimpleServer(QObject *parent) : QTcpServer(parent) {
    if (listen(QHostAddress::Any, 12345)) {
        qDebug() << "Server đang chạy tại cổng 12345...";
    } else {
        qDebug() << "Không thể lắng nghe trên cổng 12345";
    }
    stdinNotifier = new QSocketNotifier(fileno(stdin), QSocketNotifier::Read, this);
    connect(stdinNotifier, &QSocketNotifier::activated, this, &SimpleServer::onServerInput);

}

/*
    Hàm `incomingConnection` xử lý khi có client mới kết nối:
    * Tạo `ClientHandler` với `socketDescriptor` của client.
    * Tạo `QThread` mới, chuyển `handler` sang thread đó.
    * Khi thread bắt đầu (`started`), gọi `handler->start()` để thiết lập socket.
    * Kết nối tín hiệu `disconnected` để:
      * Xóa `handler` khỏi danh sách client.
      * Giải phóng bộ nhớ `handler` và dừng + xóa thread.
    * Kết nối `messageReceived` để chuyển tiếp tin nhắn tới hàm `broadcastMessage`.
    * Thêm `handler` vào danh sách `clients`.
    * Bắt đầu thread.
*/
void SimpleServer::incomingConnection(qintptr socketDescriptor) {
    auto *handler = new ClientHandler(socketDescriptor);
    auto *thread = new QThread;

    handler->moveToThread(thread);
    connect(thread, &QThread::started, handler, &ClientHandler::start);
    connect(handler, &ClientHandler::disconnected, this, [=]() {
        clients.removeOne(handler);
        handler->deleteLater();
        thread->quit();
        thread->deleteLater();
    });

    connect(handler, &ClientHandler::messageReceived, this, &SimpleServer::broadcastMessage);
    clients.append(handler);
    thread->start();
}
/*
    Hàm `broadcastMessage` nhận tin từ một client (`sender`) và gửi lại cho tất cả client khác:
    * In nội dung tin nhận được.
    * Duyệt qua danh sách `clients`.
    * Gửi tin nhắn cho mọi client **khác** với người gửi (`sender`).
*/
void SimpleServer::broadcastMessage(const QString &message, ClientHandler* sender) {
    qDebug() << "Phát tin từ client:" << message.trimmed();

    for (ClientHandler* client : clients) {
        if (client != sender) {
            client->sendMessage(message);
        }
    }
}

/*
    Hàm `onServerInput` xử lý lệnh từ bàn phím:
    * Đọc 1 dòng từ `stdin`, loại bỏ khoảng trắng.
    * Nếu không rỗng, phát tin tới **tất cả client** (vì `sender = nullptr`).
    * Dạng tin: `[Server] nội_dung`.
*/
void SimpleServer::onServerInput() {
    QString line = QTextStream(stdin).readLine().trimmed();

    if (!line.isEmpty())
        broadcastMessage("[Server] " + line, nullptr);
}

/*
    Hàm `removeClient`:
    * In thông báo khi client ngắt kết nối.
    * Xóa client khỏi danh sách `clients`.
    * Gọi `deleteLater()` để giải phóng bộ nhớ client an toàn trong event loop.
*/
void SimpleServer::removeClient(ClientHandler* client) {
    qDebug() << "Client ngắt kết nối";
    clients.removeOne(client);
    client->deleteLater();
}
// ---------------------------------

// Server mở cổng PORT chờ client kết nối, khi có kết nối mới gọi onNewConnection.
//SimpleServer::SimpleServer(QObject *parent) : QTcpServer(parent) {
//    listen(QHostAddress::Any, PORT);
//    qDebug() << "Listening on port" << PORT<< "...";
//    connect(this, &QTcpServer::newConnection, this, &SimpleServer::onNewConnection);
//}

/*
    Có client kết nối, lấy socket đó, bắt sự kiện khi client
    gửi dữ liệu (readyRead) hoặc ngắt kết nối (disconnected).
    Đồng thời tạo notifier đọc dữ liệu từ bàn phím server (stdin) để gửi tin.
*/
void SimpleServer::onNewConnection() {
    clientSocket = nextPendingConnection();
    qDebug() << "Client kết nối!";

    connect(clientSocket, &QTcpSocket::readyRead, this, &SimpleServer::onClientReadyRead);
    connect(clientSocket, &QTcpSocket::disconnected, this, &SimpleServer::onClientDisconnected);
    // Add to chat client
    stdinNotifier = new QSocketNotifier(STDIN_FILENO, QSocketNotifier::Read, this);
    connect(stdinNotifier, &QSocketNotifier::activated, this, &SimpleServer::onServerInput);
}
/*
    Hàm này đọc dòng từ stdin (bàn phím server),
    nếu không rỗng thì gửi qua clientSocket.
    → Cho phép server chủ động gửi tin nhắn cho client.
*/
//void SimpleServer::onServerInput() {
//    if (!clientSocket) return;

//    QTextStream in(stdin);
//    QString line = in.readLine();
//    if (!line.isEmpty()) {
//        clientSocket->write(line.toUtf8());
//    }
//}
/*
    Đọc toàn bộ dữ liệu client gửi (readAll()).
    In ra console nội dung nhận được từ client.
*/
void SimpleServer::onClientReadyRead() {
    QByteArray data = clientSocket->readAll();
    qDebug() << "Client gửi:" << data;
}
/*
    In thông báo ngắt kết nối.
    Giải phóng clientSocket (sử dụng deleteLater() để xoá sau event loop).
    Gán nullptr để tránh dùng socket cũ nữa.
*/
void SimpleServer::onClientDisconnected() {
    qDebug() << "Client ngắt kết nối";
    clientSocket->deleteLater();
    clientSocket = nullptr;
}
