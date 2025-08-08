#include "ClientHandler.h"
#include <QDebug>
#include <QFile>

/*
    Hàm khởi tạo `ClientHandler` nhận `socketDescriptor` và
    gán vào biến thành viên `m_socketDescriptor`.
    Đồng thời gọi hàm khởi tạo của `QObject` với `parent`.
*/
ClientHandler::ClientHandler(qintptr socketDescriptor, QObject *parent)
    : QObject(parent), m_socketDescriptor(socketDescriptor) {
}

/*
    Hàm hủy `~ClientHandler` kiểm tra nếu `m_socket` tồn tại thì
    gọi `deleteLater()` để xóa nó an toàn trong event loop.
*/
ClientHandler::~ClientHandler() {
    if (m_socket) {
        m_socket->deleteLater();
    }
}

/*
    Hàm `ClientHandler::start()` làm 3 việc chính:
    1. Tạo `QTcpSocket` mới gắn với `this`.
    2. Gắn socket descriptor để kết nối với client.
       * Nếu thất bại thì log lỗi và phát tín hiệu `disconnected`.
    3. Kết nối các signal `readyRead` và `disconnected` với slot xử lý tương ứng.

    Nó thay thế cho việc dùng `run()` nếu bạn tách logic xử lý socket ra khỏi `QThread`.
*/
void ClientHandler::start() {
    m_socket = new QTcpSocket(this);

    if (!m_socket->setSocketDescriptor(m_socketDescriptor)) {
        qWarning() << "Không thể set socket descriptor cho client";
        emit disconnected(this);
        return;
    }

    connect(m_socket, &QTcpSocket::readyRead, this, &ClientHandler::onReadyRead);
    connect(m_socket, &QTcpSocket::disconnected, this, &ClientHandler::onDisconnected);
}

/*
    Hàm `onReadyRead()` đọc toàn bộ dữ liệu từ socket khi có dữ liệu đến,
    rồi phát tín hiệu `messageReceived` kèm dữ liệu (chuyển sang `QString`)
    và con trỏ đến chính `ClientHandler`.
*/
// void ClientHandler::onReadyRead() {
    // QByteArray data = m_socket->readAll();
    // // qDebug() << "[DEBUG] Client gửi:" << data; // test trực tiếp
    // emit messageReceived(QString::fromUtf8(data), this);
// }
enum class ReceiveMode { Text, File };

ReceiveMode mode = ReceiveMode::Text;
QString pendingFileName;
qint64 pendingFileSize = 0;
qint64 receivedBytes = 0;
QFile pendingFile;
void ClientHandler::onReadyRead()
{
    QTcpSocket *clientSocket = qobject_cast<QTcpSocket*>(sender());
    if (!clientSocket) return;

    static QMap<QTcpSocket*, qint64> remainingBytes;
    static QMap<QTcpSocket*, QFile*> fileMap;

    while (clientSocket->bytesAvailable() > 0) {
        // Nếu đang nhận file
        if (remainingBytes.contains(clientSocket) && remainingBytes[clientSocket] > 0) {
            QFile *file = fileMap[clientSocket];
            QByteArray data = clientSocket->read(qMin(remainingBytes[clientSocket], (qint64)4096));
            file->write(data);
            remainingBytes[clientSocket] -= data.size();

            if (remainingBytes[clientSocket] <= 0) {
                file->close();
                delete file;
                fileMap.remove(clientSocket);
                remainingBytes.remove(clientSocket);
                qDebug() << "✅ File nhận xong!";
            }
            continue;
        }

        // Nếu chưa ở chế độ nhận file → đọc tin nhắn
        QByteArray message = clientSocket->readLine().trimmed();

        if (message.startsWith("FILE:")) {
            // Định dạng: FILE:<tên_file>:<size>
            QList<QByteArray> parts = message.split(':');
            if (parts.size() == 3) {
                QString fileName = parts[1];
                qint64 fileSize = parts[2].toLongLong();

                QFile *file = new QFile(fileName);
                if (!file->open(QIODevice::WriteOnly)) {
                    qDebug() << "❌ Không thể mở file để ghi:" << fileName;
                        delete file;
                    continue;
                }

                fileMap[clientSocket] = file;
                remainingBytes[clientSocket] = fileSize;
                qDebug() << "📂 Bắt đầu nhận file:" << fileName << "(" << fileSize << "bytes )";
            }
        } else {
            // Tin nhắn thường
            qDebug() << "💬 Tin từ client:" << message;
        }
    }
}

/*
    Hàm `onDisconnected()` phát tín hiệu `disconnected(this)` khi client
    ngắt kết nối, sau đó xóa socket bất đồng bộ bằng `deleteLater()`
    và đặt con trỏ về `nullptr`.
*/
void ClientHandler::onDisconnected() {
    emit disconnected(this);
    m_socket->deleteLater();
    m_socket = nullptr;
}

/*
    Hàm `sendMessage` kiểm tra nếu socket tồn tại và đang kết nối,
    thì gửi chuỗi `message` (chuyển sang UTF-8) qua socket bằng `write()`.
*/
void ClientHandler::sendMessage(const QString &message) {
    if (m_socket && m_socket->state() == QTcpSocket::ConnectedState) {
        m_socket->write(message.toUtf8());
    }
}
