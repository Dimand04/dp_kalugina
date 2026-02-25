#include "mainwidget.h"
#include <QSqlDatabase>
#include <QMessageBox>
#include <QApplication>
#include "authorizationwindow.h"

bool setupDatabaseConnection() {
    QSqlDatabase db = QSqlDatabase::addDatabase("QMYSQL", "db_dp_kalugina");
    db.setHostName("127.0.0.1");
    db.setPort(3306);
    db.setDatabaseName("db_dp_kalugina");
    db.setUserName("root");
    db.setPassword("1234");

    if (!db.open()) {
        QMessageBox::critical(nullptr, "Ошибка подключения", "Не удалось подключиться к базе данных!");
        return false;
    }
    return true;
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    if (!setupDatabaseConnection())
        return 1;

    AuthorizationWindow auth;
    if (auth.exec() == QDialog::Accepted) {
        int userId = auth.getUserId();
        int userRole = auth.getUserRole();

        MainWidget w(userId, userRole);
        w.show();
        return a.exec();
    }

    return 0;
}
