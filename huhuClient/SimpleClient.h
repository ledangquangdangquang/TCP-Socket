#ifndef SIMPLECLIENT_H
#define SIMPLECLIENT_H

#include <QTcpSocket>
#include <QSocketNotifier>
class SimpleClient : public QObject {
    Q_OBJECT
public:
    explicit SimpleClient(QObject *parent = nullptr);
    void connectToServer(const QString &host, quint16 port);
    void sendMessage(const QByteArray &msg);
    void sendFile(const QString &filePath);
    static void setupLogging();  // Log file
private slots:
    void onConnected();
    void onReadyRead();
    void onUserInput();  // 👈 thêm hàm đọc stdin
private:
    QTcpSocket socket;
    QSocketNotifier *stdinNotifier = nullptr;  // 👈 theo dõi stdin
};

#endif // SIMPLECLIENT_H
