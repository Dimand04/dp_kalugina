#include "edit_widget.h"
#include "ui_edit_widget.h"

edit_widget::edit_widget(Mode mode, int id, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::edit_widget)
    , m_mode(mode)
    , m_id(id)
{
    ui->setupUi(this);

    ui->cb_categories->setVisible(false);
    ui->lb_category->setVisible(false);

    switch (m_mode) {
    case CategoryMode:
        setWindowTitle(m_id == 0 ? "Создание категории" : "Редактирование категории");
        ui->pb_ok->setText(m_id == 0 ? "Создать" : "Обновить");
        ui->le_name->setPlaceholderText("Название категории");
        break;
    case MaterialMode:
        setWindowTitle(m_id == 0 ? "Создание материала" : "Редактирование материала");
        ui->pb_ok->setText(m_id == 0 ? "Создать" : "Обновить");
        ui->le_name->setPlaceholderText("Название материала");

        ui->cb_categories->setVisible(true);
        ui->lb_category->setVisible(true);

        loadCategories();
        break;
    case SupplierMode:
        setWindowTitle(m_id == 0 ? "Создание поставщика" : "Редактирование поставщика");
        ui->pb_ok->setText(m_id == 0 ? "Создать" : "Обновить");
        ui->le_name->setPlaceholderText("Название поставщика");
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
    if (m_mode == CategoryMode) tableName = "categories";
    else if (m_mode == MaterialMode) tableName = "materials";
    else if (m_mode == SupplierMode) tableName = "suppliers";

    QString sql = (m_mode == MaterialMode)
                      ? QString("SELECT name, category_id FROM materials WHERE id = :id")
                      : QString("SELECT name FROM %1 WHERE id = :id").arg(tableName);

    query.prepare(sql);
    query.bindValue(":id", m_id);

    if (query.exec() && query.next()) {
        ui->le_name->setText(query.value(0).toString());

        if (m_mode == MaterialMode) {
            int catId = query.value(1).toInt();
            int index = ui->cb_categories->findData(catId);
            if (index != -1) ui->cb_categories->setCurrentIndex(index);
        }
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

    if (m_mode == CategoryMode) {
        if (m_id == 0) query.prepare("INSERT INTO categories (name) VALUES (:name)");
        else {
            query.prepare("UPDATE categories SET name = :name WHERE id = :id");
            query.bindValue(":id", m_id);
        }
        query.bindValue(":name", name);
    }
    else if (m_mode == SupplierMode) {
        if (m_id == 0) query.prepare("INSERT INTO suppliers (name) VALUES (:name)");
        else {
            query.prepare("UPDATE suppliers SET name = :name WHERE id = :id");
            query.bindValue(":id", m_id);
        }
        query.bindValue(":name", name);
    }
    else if (m_mode == MaterialMode) {
        int catId = ui->cb_categories->currentData().toInt();
        if (catId <= 0) {
            QMessageBox::warning(this, "Ошибка", "Выберите категорию!");
            return;
        }

        if (m_id == 0) {
            query.prepare("INSERT INTO materials (name, category_id) VALUES (:name, :catId)");
        } else {
            query.prepare("UPDATE materials SET name = :name, category_id = :catId WHERE id = :id");
            query.bindValue(":id", m_id);
        }
        query.bindValue(":name", name);
        query.bindValue(":catId", catId);
    }

    if(query.exec()) {
        int finalId = (m_id == 0) ? query.lastInsertId().toInt() : m_id;

        QString typeName;
        if (m_mode == CategoryMode) typeName = "категория";
        else if (m_mode == SupplierMode) typeName = "поставщик";
        else if (m_mode == MaterialMode) typeName = "материал";

        QString actionText = (m_id == 0) ? "Создан(а)" : "Изменен(а)";
        QString logMessage = QString("%1 %2: '%3'").arg(actionText, typeName, name);

        emit actionLogged("RefData", logMessage, finalId);
    }
    else {
        QMessageBox::critical(this, "Ошибка БД", query.lastError().text());
        return;
    }
    this->accept();
}

void edit_widget::loadCategories()
{
    ui->cb_categories->clear();
    QSqlDatabase db = QSqlDatabase::database("db_dp_kalugina");
    QSqlQuery query(db);

    if (query.exec("SELECT id, name FROM categories ORDER BY name")) {
        while (query.next()) {
            ui->cb_categories->addItem(query.value(1).toString(), query.value(0).toInt());
        }
    }
}
