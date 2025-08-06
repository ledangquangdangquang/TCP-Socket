#ifndef SIMPLESERVER_H
#define SIMPLESERVER_H

#include <QTcpServer>
#include <QTcpSocket>
#include <QSocketNotifier>

class SimpleServer : public QTcpServer {
    Q_OBJECT
public:
    explicit SimpleServer(QObject *parent = nullptr);

private slots:
    void onNewConnection();
    void onClientReadyRead();
    void onClientDisconnected();
    void onServerInput();

private:
    QTcpSocket *clientSocket = nullptr;
    QSocketNotifier *stdinNotifier = nullptr;

};

#endif // SIMPLESERVER_H
