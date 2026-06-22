#include "networkmanager.h"
#include <QNetworkRequest>
#include <QUrl>
#include <QDebug>

NetworkManager::NetworkManager(QObject *parent)
    : QObject(parent), m_manager(new QNetworkAccessManager(this))
{

}

void NetworkManager::fetchMaterials(const QString &ipAddress, int port)
{
    QString urlString = QString("http://%1:%2/api/materials").arg(ipAddress).arg(port);
    QUrl url(urlString);
    QNetworkRequest request(url);

    qDebug() << "Отправка запроса на:" << urlString;

    QNetworkReply *reply = m_manager->get(request);

    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        onFetchMaterialsFinished(reply);
    });
}

void NetworkManager::onFetchMaterialsFinished(QNetworkReply *reply)
{
    if (reply->error() == QNetworkReply::NoError) {
        QByteArray responseData = reply->readAll();
        qDebug() << "Получены данные от сервера:" << responseData;

        emit materialsReceived(QString::fromUtf8(responseData));
    } else {
        QString errorMsg = reply->errorString();
        qWarning() << "Сетевая ошибка:" << errorMsg;
        emit errorOccurred(errorMsg);
    }

    reply->deleteLater();
}

void NetworkManager::fetchWarehouse(const QString &ipAddress, int port)
{
    QString urlString = QString("http://%1:%2/api/warehouse").arg(ipAddress).arg(port);
    QNetworkRequest request((QUrl(urlString)));

    QNetworkReply *reply = m_manager->get(request);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        if (reply->error() == QNetworkReply::NoError) {
            QByteArray responseData = reply->readAll();
            emit warehouseReceived(QString::fromUtf8(responseData));
        } else {
            emit errorOccurred(reply->errorString());
        }
        reply->deleteLater();
    });
}

void NetworkManager::sendInventory(const QString &ipAddress, int port, const QString &jsonPayload)
{
    QString urlString = QString("http://%1:%2/api/inventory").arg(ipAddress).arg(port);
    QNetworkRequest request((QUrl(urlString)));

    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QNetworkReply *reply = m_manager->post(request, jsonPayload.toUtf8());

    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        if (reply->error() == QNetworkReply::NoError) {
            emit inventorySentSuccess();
        } else {
            emit errorOccurred("Ошибка отправки: " + reply->errorString());
        }
        reply->deleteLater();
    });
}
