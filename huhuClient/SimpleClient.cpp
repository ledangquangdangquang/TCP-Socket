#include "SimpleClient.h"
#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <iostream>
#include <QTextStream>
#include <unistd.h> // 👈 để dùng STDIN_FILENO
#include <QLoggingCategory>
#include <QFile>
#include <QTextStream>
#include <QDateTime>
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
void SimpleClient::sendFile(const QString &filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "❌ Không mở được file";
        return;
    }
    qint64 size = file.size();
    QString header = QString("FILE:%1:%2\n").arg(QFileInfo(filePath).fileName()).arg(size);
    socket.write(header.toUtf8());
    socket.flush();

    while (!file.atEnd()) {
        socket.write(file.read(4096));
        socket.flush();
    }
    file.close();
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
// void SimpleClient::onUserInput() {
    // QTextStream in(stdin);
    // QString line = in.readLine();
    // if (!line.isEmpty()) {
         // sendMessage("[Client] " + line.toUtf8());
    // }
// }
void SimpleClient::onUserInput() {
    QTextStream in(stdin);
    QString line = in.readLine().trimmed();
    if (line.isEmpty()) return;

    if (line.startsWith("file ")) {
         QString filePath = line.mid(5).trimmed();
         sendFile(filePath);
    } else {
         sendMessage(("[Client] " + line).toUtf8() + "\n");
    }
}

//  Khi có dữ liệu từ server, đọc và in ra.
void SimpleClient::onReadyRead() {
    QByteArray data = socket.readAll();
    qDebug() <<QString::fromUtf8(data);
}


void SimpleClient::setupLogging()
{
    QLoggingCategory::setFilterRules(QStringLiteral("*.debug=true\n"));
    static QFile logFile("../app.log");
    if (logFile.open(QIODevice::Append | QIODevice::Text)) {
        qInstallMessageHandler([](QtMsgType type, const QMessageLogContext &context, const QString &msg){
            static QFile& file = logFile;
            QTextStream out(&file);
            QString level;

            switch(type) {
            case QtDebugMsg: level = "DEBUG"; break;
            case QtInfoMsg: level = "INFO"; break;
            case QtWarningMsg: level = "WARN"; break;
            case QtCriticalMsg: level = "CRIT"; break;
            case QtFatalMsg: level = "FATAL"; break;
            }

            QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm");
            out << timestamp << " [" << level << "] "
                << context.file << ":" << context.line << " "
                << context.function << " - " << msg << "\n";
            out.flush();

            fprintf(stderr, "%s\n", msg.toLocal8Bit().constData());
            fflush(stderr);

            if(type == QtFatalMsg)
                abort();
        });
    }
}
