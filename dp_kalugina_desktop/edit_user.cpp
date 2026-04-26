#include "edit_user.h"
#include "ui_edit_user.h"

edit_user::edit_user(const int& userId, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::edit_user)
    , userId(userId)
{
    ui->setupUi(this);

    connect(ui->pb_ok, &QPushButton::clicked, this, &edit_user::pb_ok_clicked);

    loadRoles();
    checkId();
}

edit_user::~edit_user()
{
    delete ui;
}

void edit_user::loadRoles()
{
    ui->cb_roles->clear();

    QSqlDatabase db = QSqlDatabase::database("db_dp_kalugina");
    if (!db.isValid() || !db.isOpen()) return;

    QSqlQuery query(db);
    if (query.exec("SELECT id, name FROM roles ORDER BY name")) {
        while (query.next()) {
            int id = query.value(0).toInt();
            QString name = query.value(1).toString();

            ui->cb_roles->addItem(name, id);
        }
    }
}

void edit_user::checkId()
{
    qDebug()<<"ID пользователя: "<<userId;
    if(userId != 0) {
        setWindowTitle("Редактирование пользователя");
        ui->pb_ok->setText("Обновить");
        loadUser();
    }
    else {
        setWindowTitle("Создание нового пользователя");
        ui->pb_ok->setText("Создать");
    }
}

void edit_user::loadUser()
{
    QSqlDatabase db = QSqlDatabase::database("db_dp_kalugina");
    if (!db.isValid() || !db.isOpen()) return;

    QSqlQuery query(db);
    query.prepare("SELECT login, role_id FROM users WHERE id = :id");
    query.bindValue(":id", userId);

    if (query.exec()) {
        if (query.next()) {
            QString login = query.value("login").toString();
            int roleIdFromDb = query.value("role_id").toInt();

            ui->le_login->setText(login);

            int index = ui->cb_roles->findData(roleIdFromDb);
            if (index != -1) {
                ui->cb_roles->setCurrentIndex(index);
            }

            ui->le_password->clear();
            ui->le_confirmPassword->clear();

            ui->le_password->setPlaceholderText("Введите новый пароль, чтобы изменить его");
            ui->le_confirmPassword->setPlaceholderText("Повторите новый пароль");
        }
    }
}

void edit_user::createUser()
{
    QString login = ui->le_login->text().remove(" ");
    QString pass = ui->le_password->text().remove(" ");
    QString confirm = ui->le_confirmPassword->text().remove(" ");
    int selectedRoleId = ui->cb_roles->currentData().toInt();

    if (login.isEmpty() || pass.isEmpty() || confirm.isEmpty()) {
        QMessageBox::warning(this, "Внимание", "Логин и пароль не могут быть пустыми!");
        return;
    }

    if (selectedRoleId <= 0) {
        QMessageBox::warning(this, "Внимание", "Пожалуйста, выберите роль пользователя!");
        return;
    }

    if (pass != confirm) {
        QMessageBox::warning(this, "Внимание", "Пароли не совпадают!");
        return;
    }

    QSqlDatabase db = QSqlDatabase::database("db_dp_kalugina");
    if (!db.isOpen()) {
        qCritical() << "База данных не открыта!";
        return;
    }

    db.transaction();
    QSqlQuery query(db);

    query.prepare("SELECT id FROM users WHERE login = ?");
    query.addBindValue(login);
    query.exec();

    if (query.next()) {
        db.rollback();
        return;
    }

    QString salt = "my_very_secure_salt_2026";
    QByteArray hashedPass = QCryptographicHash::hash((pass + salt).toUtf8(), QCryptographicHash::Sha256).toHex();

    query.prepare("INSERT INTO users (login, role_id) VALUES (?, ?)");
    query.addBindValue(login);
    query.addBindValue(selectedRoleId);

    if (!query.exec()) {
        db.rollback();
        qDebug() << query.lastError().text();
        return;
    }

    int newUserId = query.lastInsertId().toInt();

    query.prepare("INSERT INTO authorization (user_id, password) VALUES (?, ?)");
    query.addBindValue(newUserId);
    query.addBindValue(QString(hashedPass));

    if (!query.exec()) {
        db.rollback();
        qDebug() << query.lastError().text();
    } else {
        if (db.commit()) {
            emit actionLogged("UserAdmin", QString("Создан новый пользователь: '%1'").arg(login), newUserId);

            QMessageBox::information(this, "Успех", "Пользователь успешно создан!");
            this->accept();
        } else {
            db.rollback();
        }
    }
}

void edit_user::pb_ok_clicked()
{
    if (userId == 0) {
        createUser();
    } else {
        updateUser();
    }
}

void edit_user::updateUser()
{
    QString login = ui->le_login->text().remove(" ");
    QString pass = ui->le_password->text().remove(" ");
    QString confirm = ui->le_confirmPassword->text().remove(" ");
    int selectedRoleId = ui->cb_roles->currentData().toInt();

    QSqlDatabase db = QSqlDatabase::database("db_dp_kalugina");
    db.transaction();
    QSqlQuery query(db);

    query.prepare("UPDATE users SET login = ?, role_id = ? WHERE id = ?");
    query.addBindValue(login);
    query.addBindValue(selectedRoleId);
    query.addBindValue(userId);

    if (!query.exec()) { db.rollback(); return; }

    if (!pass.isEmpty()) {
        if (pass != confirm) {
            db.rollback();
            return;
        }

        QString salt = "my_very_secure_salt_2026";
        QByteArray hashedPass = QCryptographicHash::hash((pass + salt).toUtf8(), QCryptographicHash::Sha256).toHex();

        QSqlQuery authQuery(db);
        authQuery.prepare("UPDATE authorization SET password = ? WHERE user_id = ?");
        authQuery.addBindValue(QString(hashedPass));
        authQuery.addBindValue(userId);

        if (!authQuery.exec()) { db.rollback(); return; }
    }

    if (db.commit()) {
        emit actionLogged("UserAdmin", QString("Обновлены данные пользователя: '%1' (ID: %2)").arg(login).arg(userId), userId);

        QMessageBox::information(this, "Успех", "Данные пользователя обновлены");
        this->accept();
    }
}
