#include "edit_role.h"
#include "ui_edit_role.h"

edit_role::edit_role(const int& roleId, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::edit_role)
    , roleId(roleId)
{
    ui->setupUi(this);

    connect(ui->treeWidget, &QTreeWidget::itemChanged, this, &edit_role::onPermissionChanged);
    connect(ui->pb_ok, &QPushButton::clicked, this, &edit_role::pb_ok_clicked);

    buildPermissionsTree();
    checkId();
}

edit_role::~edit_role()
{
    delete ui;
}

void edit_role::buildPermissionsTree()
{
    ui->treeWidget->clear();
    ui->treeWidget->setColumnCount(1);

    QSqlDatabase db = QSqlDatabase::database("db_dp_kalugina");
    if (!db.isValid() || !db.isOpen()) return;

    QList<PermData> allPerms;
    QSqlQuery query(db);
    query.prepare("SELECT id, parent_id, name, slug FROM permissions");

    if (!query.exec()) {
        qCritical() << "Ошибка загрузки прав:" << query.lastError().text();
        return;
    }

    while (query.next()) {
        PermData p;
        p.id = query.value("id").toInt();
        p.parentId = query.value("parent_id").isNull() ? 0 : query.value("parent_id").toInt();
        p.name = query.value("name").toString();
        p.slug = query.value("slug").toString();
        allPerms.append(p);
    }

    addRecursiveChild(nullptr, 0, allPerms);
}

void edit_role::addRecursiveChild(QTreeWidgetItem *parentItem, int parentId, const QList<PermData> &allPerms)
{
    for (const auto &perm : allPerms) {
        if (perm.parentId == parentId) {
            QTreeWidgetItem *item = (parentItem == nullptr)
            ? new QTreeWidgetItem(ui->treeWidget)
            : new QTreeWidgetItem(parentItem);

            item->setText(0, perm.name);
            item->setCheckState(0, Qt::Unchecked);
            item->setData(0, Qt::UserRole, perm.id);
            item->setExpanded(true);

            if (parentItem && parentItem->checkState(0) == Qt::Unchecked) {
                item->setFlags(item->flags() & ~Qt::ItemIsEnabled);
            } else {
                item->setFlags(item->flags() | Qt::ItemIsEnabled);
            }

            addRecursiveChild(item, perm.id, allPerms);
        }
    }
}

void edit_role::onPermissionChanged(QTreeWidgetItem *item, int column)
{
    if (m_isUpdatingTree) return;
    m_isUpdatingTree = true;

    if (item->checkState(column) == Qt::Checked) {
        QTreeWidgetItem *parent = item->parent();
        while (parent) {
            parent->setCheckState(column, Qt::Checked);
            parent->setFlags(parent->flags() | Qt::ItemIsEnabled);
            parent = parent->parent();
        }

        for (int i = 0; i < item->childCount(); ++i) {
            item->child(i)->setFlags(item->child(i)->flags() | Qt::ItemIsEnabled);
        }
    }
    else {
        QList<QTreeWidgetItem*> stack;
        stack.append(item);

        while (!stack.isEmpty()) {
            QTreeWidgetItem *current = stack.takeFirst();
            if (current != item) {
                current->setCheckState(column, Qt::Unchecked);
                current->setFlags(current->flags() & ~Qt::ItemIsEnabled);
            }
            for (int i = 0; i < current->childCount(); ++i) {
                stack.append(current->child(i));
            }
        }
    }

    m_isUpdatingTree = false;
}

void edit_role::checkId()
{
    qDebug()<<"ID роли: "<<roleId;
    if(roleId != 0) {
        setWindowTitle("Редактирование роли");
        ui->pb_ok->setText("Обновить");
        loadRole();
    }
    else {
        setWindowTitle("Создание новой роли");
        ui->pb_ok->setText("Создать");
    }
}

void edit_role::loadRole()
{
    QSqlDatabase db = QSqlDatabase::database("db_dp_kalugina");
    if (!db.isValid() || !db.isOpen()) {
        qCritical() << "БД не доступна";
        return;
    }
    QSqlQuery nameQuery(db);
    nameQuery.prepare("select name from roles where id = ?");
    nameQuery.addBindValue(roleId);
    if(nameQuery.exec()) {
        if(nameQuery.next()) {
            ui->le_name->setText(nameQuery.value(0).toString());
        }
    }

    QSet<int> assignedIds;
    QSqlQuery query(db);
    query.prepare("SELECT permission_id FROM role_permissions WHERE role_id = :roleId");
    query.bindValue(":roleId", roleId);

    if (!query.exec()) {
        qCritical() << "Ошибка при загрузке прав роли:" << query.lastError().text();
        return;
    }

    while (query.next()) {
        assignedIds.insert(query.value(0).toInt());
    }

    m_isUpdatingTree = true;

    QTreeWidgetItemIterator it(ui->treeWidget);
    while (*it) {
        QTreeWidgetItem *item = *it;
        int id = item->data(0, Qt::UserRole).toInt();

        if (assignedIds.contains(id)) {
            item->setCheckState(0, Qt::Checked);
            item->setFlags(item->flags() | Qt::ItemIsEnabled);
        } else {
            item->setCheckState(0, Qt::Unchecked);
            if (item->parent() && item->parent()->checkState(0) == Qt::Unchecked) {
                item->setFlags(item->flags() & ~Qt::ItemIsEnabled);
            } else {
                item->setFlags(item->flags() | Qt::ItemIsEnabled);
            }
        }
        ++it;
    }

    m_isUpdatingTree = false;
}

void edit_role::createRole()
{
    QString roleName = ui->le_name->text().trimmed();

    if (roleName.isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Введите название роли!");
        return;
    }

    QSqlDatabase db = QSqlDatabase::database("db_dp_kalugina");
    if (!db.isValid() || !db.isOpen()) return;

    db.transaction();

    QSqlQuery query(db);

    query.prepare("SELECT id FROM roles WHERE name = :name");
    query.bindValue(":name", roleName);
    query.exec();
    if (query.next()) {
        db.rollback();
        QMessageBox::warning(this, "Ошибка", "Роль с таким названием уже существует!");
        return;
    }

    query.prepare("INSERT INTO roles (name) VALUES (:name)");
    query.bindValue(":name", roleName);

    if (!query.exec()) {
        db.rollback();
        qCritical() << "Ошибка при создании роли:" << query.lastError().text();
        return;
    }

    int newRoleId = query.lastInsertId().toInt();

    QTreeWidgetItemIterator it(ui->treeWidget);
    while (*it) {
        QTreeWidgetItem *item = *it;

        if (item->checkState(0) == Qt::Checked) {
            int permId = item->data(0, Qt::UserRole).toInt();

            QSqlQuery linkQuery(db);
            linkQuery.prepare("INSERT INTO role_permissions (role_id, permission_id) "
                              "VALUES (:roleId, :permId)");
            linkQuery.bindValue(":roleId", newRoleId);
            linkQuery.bindValue(":permId", permId);

            if (!linkQuery.exec()) {
                db.rollback();
                qCritical() << "Ошибка при сохранении прав:" << linkQuery.lastError().text();
                return;
            }
        }
        ++it;
    }

    if (db.commit()) {
        emit actionLogged("UserAdmin", QString("Создана новая роль: '%1'").arg(roleName), newRoleId);

        QMessageBox::information(this, "Успех", "Роль успешно создана!");
        this->accept();
    } else {
        db.rollback();
        QMessageBox::critical(this, "Ошибка", "Не удалось зафиксировать изменения в базе.");
    }
}

void edit_role::pb_ok_clicked()
{
    if (roleId == 0) {
        createRole();
    } else {
        updateRole();
    }
}

void edit_role::updateRole()
{
    QString newName = ui->le_name->text().trimmed();
    if (newName.isEmpty()) return;

    QSqlDatabase db = QSqlDatabase::database("db_dp_kalugina");
    if (!db.isValid() || !db.isOpen()) return;

    QString oldName;
    QSet<int> oldPermissions;

    QSqlQuery roleQuery(db);
    roleQuery.prepare("SELECT name FROM roles WHERE id = :id");
    roleQuery.bindValue(":id", roleId);
    if (roleQuery.exec() && roleQuery.next()) {
        oldName = roleQuery.value(0).toString();
    }

    QSqlQuery permsQuery(db);
    permsQuery.prepare("SELECT permission_id FROM role_permissions WHERE role_id = :id");
    permsQuery.bindValue(":id", roleId);
    if (permsQuery.exec()) {
        while (permsQuery.next()) {
            oldPermissions.insert(permsQuery.value(0).toInt());
        }
    }

    QSet<int> newPermissions;
    QTreeWidgetItemIterator it(ui->treeWidget);
    while (*it) {
        if ((*it)->checkState(0) == Qt::Checked) {
            newPermissions.insert((*it)->data(0, Qt::UserRole).toInt());
        }
        ++it;
    }

    bool nameChanged = (oldName != newName);
    bool rightsChanged = (oldPermissions != newPermissions);

    if (!nameChanged && !rightsChanged) {
        this->reject();
        return;
    }

    db.transaction();
    QSqlQuery query(db);

    query.prepare("UPDATE roles SET name = :name WHERE id = :id");
    query.bindValue(":name", newName);
    query.bindValue(":id", roleId);

    if (!query.exec()) {
        db.rollback();
        return;
    }

    if (rightsChanged) {
        QSqlQuery delQuery(db);
        delQuery.prepare("DELETE FROM role_permissions WHERE role_id = :roleId");
        delQuery.bindValue(":roleId", roleId);
        delQuery.exec();

        for (int permId : newPermissions) {
            QSqlQuery insQuery(db);
            insQuery.prepare("INSERT INTO role_permissions (role_id, permission_id) VALUES (:rid, :pid)");
            insQuery.bindValue(":rid", roleId);
            insQuery.bindValue(":pid", permId);
            insQuery.exec();
        }
    }

    if (db.commit()) {
        QString logDescription;

        if (nameChanged && rightsChanged) {
            logDescription = QString("Роль '%1' переименована в '%2'. Права доступа изменены.")
                                 .arg(oldName, newName);
        }
        else if (nameChanged) {
            logDescription = QString("Роль '%1' переименована в '%2' (права не менялись).")
                                 .arg(oldName, newName);
        }
        else if (rightsChanged) {
            logDescription = QString("Обновлены права доступа роли '%1'.")
                                 .arg(newName);
        }

        emit actionLogged("UserAdmin", logDescription, roleId);

        QMessageBox::information(this, "Успех", "Данные роли обновлены!");
        this->accept();
    } else {
        db.rollback();
    }
}
