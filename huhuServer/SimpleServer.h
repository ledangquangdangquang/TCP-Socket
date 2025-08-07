#ifndef SIMPLESERVER_H
#define SIMPLESERVER_H

#include <QTcpServer>
#include <QTcpSocket>
#include <QSocketNotifier>
#include "ClientHandler.h"

class SimpleServer : public QTcpServer {
    Q_OBJECT
public:
    explicit SimpleServer(QObject *parent = nullptr);
protected:
    // ADD multi
    void incomingConnection(qintptr handle) override;
    // ----------------------------------
private slots:
    void onNewConnection();
    void onClientReadyRead();
    void onClientDisconnected();
    void onServerInput();
    // ADD multi
    void broadcastMessage(const QString &message, ClientHandler* sender);
    void removeClient(ClientHandler* client);
    // ----------------------------------
private:
    QTcpSocket *clientSocket = nullptr;
    QSocketNotifier *stdinNotifier = nullptr;
    // ADD multi
    QList<ClientHandler*> clients;
    // ----------------------------------
};

#endif // SIMPLESERVER_H
