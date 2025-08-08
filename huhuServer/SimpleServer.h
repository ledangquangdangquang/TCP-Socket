#pragma once
#include <QTcpServer>
#include <QList>
#include <QSocketNotifier>
#include "ClientHandler.h"

class SimpleServer : public QTcpServer {
    Q_OBJECT
public:
    explicit SimpleServer(QObject *parent = nullptr);
    static void setupLogging();  // Log file
protected:
    void incomingConnection(qintptr socketDescriptor) override;

private slots:
    void broadcastMessage(const QString &message, ClientHandler* sender);
    void onServerInput();
    void removeClient(ClientHandler* client);

private:
    QList<ClientHandler*> clients;
    QSocketNotifier *stdinNotifier;
};
