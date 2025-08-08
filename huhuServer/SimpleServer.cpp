#include "SimpleServer.h"
#include <QDebug>
#include <QThread>
#include <QTextStream>
#include <unistd.h> // STDIN_FILENO
#include <QLoggingCategory>
#include <QFile>
#include <QTextStream>
#include <QDateTime>

SimpleServer::SimpleServer(QObject *parent) : QTcpServer(parent) {
    if (listen(QHostAddress::Any, 12345)) {
        qInfo() << "Server đang chạy tại cổng 12345...";
    } else {
        qFatal() << "Không thể lắng nghe trên cổng 12345";
    }

    // Chỉ tạo stdinNotifier 1 lần
    stdinNotifier = new QSocketNotifier(STDIN_FILENO, QSocketNotifier::Read, this);
    connect(stdinNotifier, &QSocketNotifier::activated, this, &SimpleServer::onServerInput);
}

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
        qWarning() << "Client đã ngắt kết nối";
    });

    connect(handler, &ClientHandler::messageReceived,
            this, &SimpleServer::broadcastMessage);

    clients.append(handler);
    thread->start();
}

void SimpleServer::broadcastMessage(const QString &message, ClientHandler* sender) {
    qDebug() << "Tin từ" << (sender ? "client" : "server") << ":" << message.trimmed();

    for (ClientHandler* client : clients) {
        if (client != sender) {
            client->sendMessage(message);
        }
    }
}

void SimpleServer::onServerInput() {
    QString line = QTextStream(stdin).readLine().trimmed();
    if (!line.isEmpty()) {
        broadcastMessage("[Server] " + line, nullptr);
    }
}

void SimpleServer::removeClient(ClientHandler* client) {
    clients.removeOne(client);
    client->deleteLater();
}

void SimpleServer::setupLogging()
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
