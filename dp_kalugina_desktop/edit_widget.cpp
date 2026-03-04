#include "edit_widget.h"
#include "ui_edit_widget.h"

edit_widget::edit_widget(Mode mode, int id, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::edit_widget)
    , m_mode(mode)
    , m_id(id)
{
    ui->setupUi(this);

    switch (m_mode) {
    case CategoryMode:
        setWindowTitle(m_id == 0 ? "Создание категории" : "Редактирование категории");
        ui->pb_ok->setText(m_id == 0 ? "Создать" : "Обновить");
        ui->le_name->setPlaceholderText("Название категории");
        break;
    case StatusMode:
        setWindowTitle("Настройка статуса");
        break;
    }

    if (m_id != 0) loadData();

    connect(ui->pb_ok, &QPushButton::clicked, this, &edit_widget::saveData);
}

edit_widget::~edit_widget()
{
    delete ui;
}

void edit_widget::loadData()
{
    QSqlDatabase db = QSqlDatabase::database("db_dp_kalugina");
    QSqlQuery query(db);

    QString tableName;
    switch (m_mode) {
    case CategoryMode: tableName = "categories"; break;
    // case StatusMode:   tableName = "statuses";   break;
    // case UnitMode:     tableName = "units";      break;
    }

    query.prepare(QString("SELECT name FROM %1 WHERE id = :id").arg(tableName));
    query.bindValue(":id", m_id);

    if (query.exec() && query.next()) {
        ui->le_name->setText(query.value(0).toString());
    }
}

void edit_widget::saveData()
{
    QString name = ui->le_name->text().trimmed();
    if (name.isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Поле не может быть пустым!");
        return;
    }

    QSqlDatabase db = QSqlDatabase::database("db_dp_kalugina");
    QSqlQuery query(db);

    QString tableName;
    switch (m_mode) {
    case CategoryMode: tableName = "categories"; break;
    case StatusMode:   tableName = "statuses";   break;
    case UnitMode:     tableName = "units";      break;
    }

    if (m_id == 0) {
        query.prepare(QString("INSERT INTO %1 (name) VALUES (:name)").arg(tableName));
    } else {
        query.prepare(QString("UPDATE %1 SET name = :name WHERE id = :id").arg(tableName));
        query.bindValue(":id", m_id);
    }

    query.bindValue(":name", name);

    if (!query.exec()) {
        QMessageBox::critical(this, "Ошибка БД", query.lastError().text());
        return;
    }
    this->accept();
}
