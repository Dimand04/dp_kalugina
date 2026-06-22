#ifndef NETWORKMANAGER_H
#define NETWORKMANAGER_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>

class NetworkManager : public QObject
{
    Q_OBJECT
public:
    explicit NetworkManager(QObject *parent = nullptr);

    Q_INVOKABLE void fetchMaterials(const QString &ipAddress, int port = 54321);
    Q_INVOKABLE void fetchWarehouse(const QString &ipAddress, int port = 54321);
    Q_INVOKABLE void sendInventory(const QString &ipAddress, int port, const QString &jsonPayload);

signals:
    void materialsReceived(const QString &jsonData);
    void errorOccurred(const QString &errorMsg);
    void warehouseReceived(const QString &jsonData);
    void inventorySentSuccess();

private slots:
    void onFetchMaterialsFinished(QNetworkReply *reply);

private:
    QNetworkAccessManager *m_manager;
};

#endif // NETWORKMANAGER_H
