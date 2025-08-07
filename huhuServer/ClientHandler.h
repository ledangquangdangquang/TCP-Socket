#ifndef CLIENTHANDLER_H
#define CLIENTHANDLER_H

#include <QObject>
#include <QTcpSocket>

class ClientHandler : public QObject {
    Q_OBJECT
public:
    explicit ClientHandler(qintptr socketDescriptor, QObject *parent = nullptr);
    ~ClientHandler();

signals:
    void messageReceived(const QString &message, ClientHandler* sender);
    void disconnected(ClientHandler* handler);

public slots:
    void start(); // Khởi động xử lý socket
    void sendMessage(const QString &message);

private slots:
    void onReadyRead();
    void onDisconnected();

private:
    qintptr m_socketDescriptor;
    QTcpSocket *m_socket = nullptr;
};

#endif // CLIENTHANDLER_H
