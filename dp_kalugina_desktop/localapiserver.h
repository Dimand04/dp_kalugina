#ifndef LOCALAPISERVER_H
#define LOCALAPISERVER_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>

class LocalApiServer : public QObject
{
    Q_OBJECT
public:
    explicit LocalApiServer(quint16 port = 8080, QObject *parent = nullptr);
    ~LocalApiServer();

private slots:
    void onNewConnection();
    void onReadyRead();
    void onDisconnected();

private:
    QTcpServer *m_tcpServer;

    void handleGetMaterials(QTcpSocket *socket);
    void handleGetWarehouse(QTcpSocket *socket);
    void handlePostInventory(QTcpSocket *socket, const QByteArray &body);

    void sendJsonResponse(QTcpSocket *socket, const QByteArray &jsonBytes, int statusCode = 200);
};

#endif // LOCALAPISERVER_H
