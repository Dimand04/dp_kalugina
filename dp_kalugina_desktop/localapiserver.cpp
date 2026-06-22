#include "localapiserver.h"
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <QStringList>
#include <QDebug>

LocalApiServer::LocalApiServer(quint16 port, QObject *parent)
    : QObject(parent)
    , m_tcpServer(new QTcpServer(this))
{
    connect(m_tcpServer, &QTcpServer::newConnection, this, &LocalApiServer::onNewConnection);

    if (m_tcpServer->listen(QHostAddress::AnyIPv4, port)) {
        qDebug() << "Локальный API Сервер успешно запущен на порту:" << port;
    } else {
        qCritical() << "Не удалось запустить сетевой сервер:" << m_tcpServer->errorString();
    }
}

LocalApiServer::~LocalApiServer()
{
    m_tcpServer->close();
}

void LocalApiServer::onNewConnection()
{
    while (m_tcpServer->hasPendingConnections()) {
        QTcpSocket *clientSocket = m_tcpServer->nextPendingConnection();
        connect(clientSocket, &QTcpSocket::readyRead, this, &LocalApiServer::onReadyRead);
        connect(clientSocket, &QTcpSocket::disconnected, this, &LocalApiServer::onDisconnected);
    }
}

void LocalApiServer::onReadyRead()
{
    QTcpSocket *socket = qobject_cast<QTcpSocket*>(sender());
    if (!socket) return;

    QByteArray requestData = socket->readAll();

    int bodyStart = requestData.indexOf("\r\n\r\n");
    QByteArray body;
    if (bodyStart != -1) {
        body = requestData.mid(bodyStart + 4);
    }

    QString requestStr = QString::fromUtf8(requestData);
    QStringList lines = requestStr.split(QRegularExpression("[\r\n]+"), Qt::SkipEmptyParts);
    if (lines.isEmpty()) return;

    QStringList tokens = lines.first().split(" ");
    if (tokens.size() < 2) return;

    QString method = tokens[0];
    QString path = tokens[1];

    if (method == "GET" && path == "/api/materials") {
        handleGetMaterials(socket);
    } else if (method == "GET" && path == "/api/warehouse") {
        handleGetWarehouse(socket);
    } else if (method == "POST" && path == "/api/inventory") {
        handlePostInventory(socket, body);
    } else {
        sendJsonResponse(socket, "{\"error\":\"Маршрут не найден\"}", 404);
    }
}

void LocalApiServer::onDisconnected()
{
    QTcpSocket *socket = qobject_cast<QTcpSocket*>(sender());
    if (socket) {
        socket->deleteLater();
    }
}

void LocalApiServer::handleGetMaterials(QTcpSocket *socket)
{
    QSqlDatabase db = QSqlDatabase::database("db_dp_kalugina");
    if (!db.isValid() || !db.isOpen()) {
        sendJsonResponse(socket, "{\"error\":\"База данных недоступна на десктопе\"}", 500);
        return;
    }

    QJsonArray materialsArray;
    QSqlQuery query(db);

    if (query.exec("SELECT id, name FROM materials ORDER BY name")) {
        while (query.next()) {
            QJsonObject materialObject;
            materialObject["id"] = query.value(0).toInt();
            materialObject["name"] = query.value(1).toString();
            materialsArray.append(materialObject);
        }
    }

    QJsonDocument doc(materialsArray);
    sendJsonResponse(socket, doc.toJson(QJsonDocument::Compact));
}

void LocalApiServer::handleGetWarehouse(QTcpSocket *socket)
{
    QSqlDatabase db = QSqlDatabase::database("db_dp_kalugina");
    if (!db.isValid() || !db.isOpen()) {
        sendJsonResponse(socket, "{\"error\":\"База данных недоступна\"}", 500);
        return;
    }

    QJsonArray warehouseArray;
    QSqlQuery query(db);

    QString sql =
        "SELECT m.id, c.name AS category_name, m.name AS material_name, "
        "SUM(b.current_quantity) AS total_quantity, u.name AS unit_name, "
        "ROUND(SUM(b.current_quantity * b.purchase_price) / SUM(b.current_quantity), 2) AS avg_price, "
        "ROUND(SUM(b.current_quantity * b.purchase_price), 2) AS total_sum "
        "FROM materials m "
        "JOIN categories c ON m.category_id = c.id "
        "JOIN units u ON m.unit_id = u.id "
        "JOIN batches b ON m.id = b.material_id "
        "WHERE b.current_quantity > 0 "
        "GROUP BY m.id, c.name, m.name, u.name "
        "ORDER BY c.name ASC, m.name ASC";

    if (query.exec(sql)) {
        while (query.next()) {
            QJsonObject row;
            row["material_id"] = query.value(0).toInt();
            row["category"] = query.value(1).toString();
            row["name"] = query.value(2).toString();
            row["quantity"] = query.value(3).toDouble();
            row["unit"] = query.value(4).toString();
            row["avg_price"] = query.value(5).toDouble();
            row["total_sum"] = query.value(6).toDouble();
            warehouseArray.append(row);
        }
    }

    QJsonDocument doc(warehouseArray);
    sendJsonResponse(socket, doc.toJson(QJsonDocument::Compact));
}

void LocalApiServer::sendJsonResponse(QTcpSocket *socket, const QByteArray &jsonBytes, int statusCode)
{
    QString statusStr = (statusCode == 200) ? "200 OK" :
                            (statusCode == 404) ? "404 Not Found" :
                            (statusCode == 405) ? "405 Method Not Allowed" : "500 Internal Server Error";

    QByteArray response;
    response.append("HTTP/1.1 " + statusStr.toUtf8() + "\r\n");
    response.append("Content-Type: application/json; charset=utf-8\r\n");
    response.append("Content-Length: " + QByteArray::number(jsonBytes.size()) + "\r\n");

    response.append("Access-Control-Allow-Origin: *\r\n");
    response.append("Connection: close\r\n\r\n");
    response.append(jsonBytes);

    socket->write(response);
    socket->flush();
    socket->disconnectFromHost();
}

void LocalApiServer::handlePostInventory(QTcpSocket *socket, const QByteArray &body)
{
    QJsonDocument doc = QJsonDocument::fromJson(body);
    if (!doc.isArray()) {
        sendJsonResponse(socket, "{\"error\":\"Ожидался массив данных\"}", 400);
        return;
    }

    QJsonArray items = doc.array();
    if (items.isEmpty()) {
        sendJsonResponse(socket, "{\"status\":\"empty\"}", 200);
        return;
    }

    QSqlDatabase db = QSqlDatabase::database("db_dp_kalugina");
    if (!db.transaction()) {
        qCritical() << "БД заблокирована:" << db.lastError().text();
        sendJsonResponse(socket, "{\"error\":\"БД заблокирована\"}", 500);
        return;
    }

    int validUserId = -1;
    QSqlQuery userQuery(db);
    if (userQuery.exec("SELECT id FROM users LIMIT 1") && userQuery.next()) {
        validUserId = userQuery.value(0).toInt();
    } else {
        qCritical() << "КРИТИЧЕСКАЯ ОШИБКА: В таблице users нет ни одного пользователя!";
        db.rollback();
        sendJsonResponse(socket, "{\"error\":\"Нет пользователей в БД\"}", 500);
        return;
    }

    QSqlQuery query(db);
    query.prepare("INSERT INTO documents (doc_type, doc_date, user_id, description) "
                  "VALUES ('inventory', NOW(), :userId, 'Инвентаризация через ТСД')");
    query.bindValue(":userId", validUserId);

    if (!query.exec()) {
        qCritical() << "Ошибка SQL при создании документа:" << query.lastError().text();
        db.rollback();
        sendJsonResponse(socket, "{\"error\":\"Ошибка создания документа\"}", 500);
        return;
    }

    int docId = query.lastInsertId().toInt();

    for (int i = 0; i < items.size(); ++i) {
        QJsonObject obj = items[i].toObject();
        int matId = obj["material_id"].toInt();
        double actual = obj["actual_quantity"].toDouble();

        QSqlQuery calcQuery(db);
        calcQuery.prepare("SELECT COALESCE(SUM(current_quantity), 0), "
                          "COALESCE(ROUND(SUM(current_quantity * purchase_price) / SUM(current_quantity), 2), 0) "
                          "FROM batches WHERE material_id = :mid AND current_quantity > 0");
        calcQuery.bindValue(":mid", matId);
        calcQuery.exec();

        double expected = 0;
        double price = 0;
        if (calcQuery.next()) {
            expected = calcQuery.value(0).toDouble();
            price = calcQuery.value(1).toDouble();
        }

        double diff = actual - expected;

        QSqlQuery transQuery(db);
        transQuery.prepare("INSERT INTO inventory_transactions "
                           "(document_id, material_id, quantity, price, expected_quantity, actual_quantity, reason) "
                           "VALUES (:docId, :matId, :qty, :price, :exp, :act, 'Сверка ТСД')");
        transQuery.bindValue(":docId", docId);
        transQuery.bindValue(":matId", matId);
        transQuery.bindValue(":qty", diff);
        transQuery.bindValue(":price", price);
        transQuery.bindValue(":exp", expected);
        transQuery.bindValue(":act", actual);

        if (!transQuery.exec()) {
            qCritical() << "Ошибка SQL в транзакции инвентаризации:" << transQuery.lastError().text();
            db.rollback();
            sendJsonResponse(socket, "{\"error\":\"Ошибка записи транзакции\"}", 500);
            return;
        }

        if (diff < 0) {
            double toSubtract = qAbs(diff);
            QSqlQuery batchQuery(db);
            batchQuery.prepare("SELECT id, current_quantity FROM batches WHERE material_id = :matId AND current_quantity > 0 ORDER BY incoming_date ASC");
            batchQuery.bindValue(":matId", matId);
            batchQuery.exec();

            while (batchQuery.next() && toSubtract > 0.0001) {
                int batchId = batchQuery.value(0).toInt();
                double batchQty = batchQuery.value(1).toDouble();
                double amountToTake = qMin(toSubtract, batchQty);

                QSqlQuery updateBatch(db);
                updateBatch.prepare("UPDATE batches SET current_quantity = current_quantity - :take WHERE id = :id");
                updateBatch.bindValue(":take", amountToTake);
                updateBatch.bindValue(":id", batchId);
                updateBatch.exec();
                toSubtract -= amountToTake;
            }
        } else if (diff > 0) {
            QSqlQuery addBatch(db);
            addBatch.prepare("INSERT INTO batches (material_id, incoming_date, initial_quantity, current_quantity, purchase_price) "
                             "VALUES (:matId, NOW(), :qty, :qty, :price)");
            addBatch.bindValue(":matId", matId);
            addBatch.bindValue(":qty", diff);
            addBatch.bindValue(":price", price);
            if (!addBatch.exec()) {
                qCritical() << "Ошибка SQL при добавлении партии:" << addBatch.lastError().text();
            }
        }
    }

    db.commit();
    sendJsonResponse(socket, "{\"status\":\"success\"}", 200);
}
