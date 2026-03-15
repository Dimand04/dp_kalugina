#include "mainwidget.h"
#include "ui_mainwidget.h"
#include "authorizationwindow.h"
#include "edit_role.h"
#include "edit_user.h"
#include "edit_widget.h"

MainWidget::MainWidget(int userId, int userRole, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::MainWidget)
    , userId(userId)
    , userRole(userRole)
{
    ui->setupUi(this);

    adminFunction();

    QToolButton *logoutBtn = new QToolButton(this);
    logoutBtn->setText("Выход");
    logoutBtn->setAutoRaise(true);
    ui->tabw_main->setCornerWidget(logoutBtn, Qt::TopRightCorner);

    connect(logoutBtn, &QToolButton::clicked, this, [this](){
        logout();
    });

    connect(ui->tabw_main, &QTabWidget::currentChanged, this, &MainWidget::tabw_main_change);
    connect(ui->tabw_administration, &QTabWidget::currentChanged, this, &MainWidget::tabw_administration_change);
    connect(ui->le_admin_users_search, &QLineEdit::textChanged,this, &MainWidget::filter_users);
    connect(ui->tw_admin_users, &QTableWidget::cellClicked, this, &MainWidget::table_users_clicked);
    connect(ui->pb_admin_addUser,&QPushButton::clicked, this, &MainWidget::show_edit_user);
    connect(ui->pb_admin_editUser,&QPushButton::clicked, this, &MainWidget::show_edit_user);

    connect(ui->le_admin_roles_search, &QLineEdit::textChanged,this, &MainWidget::filter_roles);
    connect(ui->tw_admin_roles, &QTableWidget::cellClicked, this, &MainWidget::table_roles_clicked);
    connect(ui->pb_admin_addRole,&QPushButton::clicked, this, &MainWidget::show_edit_role);
    connect(ui->pb_admin_editRole,&QPushButton::clicked, this, &MainWidget::show_edit_role);
    connect(ui->cb_admin_roles, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWidget::filter_users);

    connect(ui->le_admin_categories_search, &QLineEdit::textChanged,this, &MainWidget::filter_categories);
    connect(ui->pb_admin_addCategory,&QPushButton::clicked, this, &MainWidget::show_edit_category);
    connect(ui->pb_admin_editCategory,&QPushButton::clicked, this, &MainWidget::show_edit_category);
    connect(ui->tw_admin_categories, &QTableWidget::cellClicked, this, &MainWidget::table_categories_clicked);

    connect(ui->le_admin_materials_search, &QLineEdit::textChanged,this, &MainWidget::filter_materials);
    connect(ui->pb_admin_addMaterial,&QPushButton::clicked, this, &MainWidget::show_edit_material);
    connect(ui->pb_admin_editMaterial,&QPushButton::clicked, this, &MainWidget::show_edit_material);
    connect(ui->tw_admin_materials, &QTableWidget::cellClicked, this, &MainWidget::table_materials_clicked);

    connect(ui->le_admin_suppliers_search, &QLineEdit::textChanged,this, &MainWidget::filter_suppliers);
    connect(ui->pb_admin_addSupplier,&QPushButton::clicked, this, &MainWidget::show_edit_supplier);
    connect(ui->pb_admin_editSupplier,&QPushButton::clicked, this, &MainWidget::show_edit_supplier);
    connect(ui->tw_admin_suppliers, &QTableWidget::cellClicked, this, &MainWidget::table_suppliers_clicked);

    connect(ui->le_warehouse_materials_search, &QLineEdit::textChanged,this, &MainWidget::filter_warehouse);
    connect(ui->cb_warehouse_categories, QOverload<int>::of(&QComboBox::currentIndexChanged),this, &MainWidget::filter_warehouse);
    connect(ui->lb_change_layout,&QPushLabel::clicked, this, &MainWidget::toggleWarehouseLayout);
    connect(ui->tw_warehouse, &QTableWidget::cellClicked, this, &MainWidget::table_warehouseBatches_clicked);

    QHeaderView *usersheader = ui->tw_admin_users->horizontalHeader();
    usersheader->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    usersheader->setSectionResizeMode(1, QHeaderView::Stretch);
    usersheader->setSectionResizeMode(2, QHeaderView::Stretch);
    ui->tw_admin_users->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    QHeaderView *rolesheader = ui->tw_admin_roles->horizontalHeader();
    rolesheader->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    rolesheader->setSectionResizeMode(1, QHeaderView::Stretch);
    ui->tw_admin_roles->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    QHeaderView *categoriesheader = ui->tw_admin_categories->horizontalHeader();
    categoriesheader->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    categoriesheader->setSectionResizeMode(1, QHeaderView::Stretch);
    ui->tw_admin_categories->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    QHeaderView *materialsheader = ui->tw_admin_materials->horizontalHeader();
    materialsheader->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    materialsheader->setSectionResizeMode(1, QHeaderView::Stretch);
    ui->tw_admin_materials->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    QHeaderView *suppliersheader = ui->tw_admin_suppliers->horizontalHeader();
    suppliersheader->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    suppliersheader->setSectionResizeMode(1, QHeaderView::Stretch);
    ui->tw_admin_suppliers->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    QHeaderView *warehouseheader = ui->tw_warehouse->horizontalHeader();
    warehouseheader->setSectionResizeMode(1, QHeaderView::Stretch);

    //this->showMaximized();
    ui->tabw_main->setCurrentIndex(0);
    ui->tabw_administration->setCurrentIndex(0);
}

MainWidget::~MainWidget()
{
    delete ui;
}

void MainWidget::adminFunction()
{
    if (userRole == 1) {
        ui->tabw_main->insertTab(4, ui->tab_5, "Администрирование");
    }
    else {
        ui->tabw_main->removeTab(4);
    }
}

void MainWidget::logout()
{
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Подтверждение выхода", "Вы действительно хотите выйти из учётной записи?",
                                  QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::Yes)
    {
        userId = 0;
        userRole = 0;
        this->close();

        AuthorizationWindow *auth = new AuthorizationWindow();
        if (auth->exec() == QDialog::Accepted)
        {
            int newUserId = auth->getUserId();
            int newUserRole = auth->getUserRole();

            MainWidget *mainWidget = new MainWidget(newUserId, newUserRole);
            mainWidget->show();
        }
    }
}

void MainWidget::tabw_main_change(int index)
{
    if(index == 0) {

    }
    else if(index == 2) {
        tabw_warehouse_change();
    }
    else if(index == 4) {
        tabw_administration_change(0);
    }
}

void MainWidget::tabw_administration_change(int index)
{
    if(index == 0) {
        load_categories_table();
        load_materials_table();
        load_suppliers_table();
    }
    else if(index == 1) {
        load_users_table();
        load_roles_table();
        loadRoles();
    }
    else if(index == 2) {

    }
}

void MainWidget::tabw_warehouse_change()
{
    ui->splitter->setOrientation(Qt::Vertical);
    ui->lb_change_layout->setPixmap(QPixmap(":/res/edithlayoutsplit.png"));
    ui->splitter->setSizes({1, 0});
    m_isWarehouseDetailsOpened = false;
    currentMaterialBatchesId = 0;
    loadWarehouseCategories();
    loadWarehouseTable();
}

void MainWidget::load_users_table()
{
    reset_users_ui();

    QSqlDatabase db = QSqlDatabase::database("db_dp_kalugina");
    if (!db.isValid() || !db.isOpen()) return;

    QSqlQuery query(db);
    QString sql = "SELECT users.id, users.login, roles.name FROM users "
                  "JOIN roles ON roles.id = users.role_id";
    if (query.exec(sql)) {
        ui->tw_admin_users->setRowCount(0);

        int row = 0;
        while (query.next()) {
            ui->tw_admin_users->insertRow(row);
            ui->tw_admin_users->setItem(row, 0, new QTableWidgetItem(query.value(0).toString()));
            ui->tw_admin_users->setItem(row, 1, new QTableWidgetItem(query.value(1).toString()));
            ui->tw_admin_users->setItem(row, 2, new QTableWidgetItem(query.value(2).toString()));
            row++;
        }
    }
}

void MainWidget::filter_users()
{
    QString searchText = ui->le_admin_users_search->text();
    QString selectedRole = ui->cb_admin_roles->currentText();
    int selectedRoleId = ui->cb_admin_roles->currentData().toInt();

    for (int row = 0; row < ui->tw_admin_users->rowCount(); ++row) {
        QTableWidgetItem *itemId = ui->tw_admin_users->item(row, 0);
        QTableWidgetItem *itemName = ui->tw_admin_users->item(row, 1);
        QTableWidgetItem *itemRole = ui->tw_admin_users->item(row, 2);

        QString idText = itemId ? itemId->text() : "";
        QString nameText = itemName ? itemName->text() : "";
        QString roleText = itemRole ? itemRole->text() : "";

        bool textMatch = idText.contains(searchText, Qt::CaseInsensitive) ||
                         nameText.contains(searchText, Qt::CaseInsensitive);

        bool roleMatch = (selectedRoleId == 0) || (roleText == selectedRole);

        ui->tw_admin_users->setRowHidden(row, !(textMatch && roleMatch));
    }
}

void MainWidget::show_edit_user()
{
    edit_user eu(currentUserId);
    eu.setWindowFlags(Qt::Window | Qt::WindowCloseButtonHint);
    if (eu.exec() == QDialog::Accepted) {
        load_users_table();
    } else {
        reset_users_ui();
    }
}

void MainWidget::table_users_clicked(int row)
{
    currentUserId = 0;
    QTableWidgetItem *item = ui->tw_admin_users->item(row, 0);
    if (item) {
        currentUserId = item->text().toInt();
        ui->pb_admin_addUser->setEnabled(false);
        ui->pb_admin_editUser->setEnabled(true);
        ui->pb_admin_deleteUser->setEnabled(true);
        qDebug()<<"ID пользователя: "<<currentUserId;
    }
}

void MainWidget::reset_users_ui()
{
    currentUserId = 0;
    ui->pb_admin_addUser->setEnabled(true);
    ui->pb_admin_editUser->setEnabled(false);
    ui->pb_admin_deleteUser->setEnabled(false);
    ui->tw_admin_users->clearSelection();
}

void MainWidget::load_roles_table()
{
    reset_roles_ui();

    QSqlDatabase db = QSqlDatabase::database("db_dp_kalugina");
    if (!db.isValid() || !db.isOpen()) return;

    QSqlQuery query(db);
    if (query.exec("SELECT id, name FROM roles")) {
        ui->tw_admin_roles->setRowCount(0);

        int row = 0;
        while (query.next()) {
            ui->tw_admin_roles->insertRow(row);
            ui->tw_admin_roles->setItem(row, 0, new QTableWidgetItem(query.value(0).toString()));
            ui->tw_admin_roles->setItem(row, 1, new QTableWidgetItem(query.value(1).toString()));
            row++;
        }
    }
}

void MainWidget::filter_roles(const QString &searchText)
{
    for (int row = 0; row < ui->tw_admin_roles->rowCount(); ++row) {
        QTableWidgetItem *itemId = ui->tw_admin_roles->item(row, 0);
        QTableWidgetItem *itemName = ui->tw_admin_roles->item(row, 1);
        QString idText = itemId ? itemId->text() : "";
        QString nameText = itemName ? itemName->text() : "";
        bool match = idText.contains(searchText, Qt::CaseInsensitive) ||
                     nameText.contains(searchText, Qt::CaseInsensitive);
        ui->tw_admin_roles->setRowHidden(row, !match);
    }
}

void MainWidget::show_edit_role()
{
    edit_role er(currentRoleId);
    er.setWindowFlags(Qt::Window | Qt::WindowCloseButtonHint);
    if (er.exec() == QDialog::Accepted) {
        load_roles_table();
        loadRoles();
    } else {
        reset_roles_ui();
    }
}

void MainWidget::table_roles_clicked(int row)
{
    currentRoleId = 0;
    QTableWidgetItem *item = ui->tw_admin_roles->item(row, 0);
    if (item) {
        currentRoleId = item->text().toInt();
        ui->pb_admin_addRole->setEnabled(false);
        ui->pb_admin_editRole->setEnabled(true);
        ui->pb_admin_deleteRole->setEnabled(true);
        qDebug()<<"ID роли: "<<currentRoleId;
    }
}

void MainWidget::reset_roles_ui()
{
    currentRoleId = 0;
    ui->pb_admin_addRole->setEnabled(true);
    ui->pb_admin_editRole->setEnabled(false);
    ui->pb_admin_deleteRole->setEnabled(false);
    ui->tw_admin_roles->clearSelection();
}

void MainWidget::loadRoles()
{
    ui->cb_admin_roles->clear();

    ui->cb_admin_roles->addItem("Все роли", 0);

    QSqlDatabase db = QSqlDatabase::database("db_dp_kalugina");
    if (!db.isValid() || !db.isOpen()) return;

    QSqlQuery query(db);
    if (query.exec("SELECT id, name FROM roles ORDER BY name")) {
        while (query.next()) {
            ui->cb_admin_roles->addItem(query.value(1).toString(), query.value(0).toInt());
        }
    }
}

void MainWidget::load_categories_table()
{
    reset_categories_ui();

    QSqlDatabase db = QSqlDatabase::database("db_dp_kalugina");
    if (!db.isValid() || !db.isOpen()) return;

    QSqlQuery query(db);
    if (query.exec("SELECT id, name FROM categories")) {
        ui->tw_admin_categories->setRowCount(0);

        int row = 0;
        while (query.next()) {
            ui->tw_admin_categories->insertRow(row);
            ui->tw_admin_categories->setItem(row, 0, new QTableWidgetItem(query.value(0).toString()));
            ui->tw_admin_categories->setItem(row, 1, new QTableWidgetItem(query.value(1).toString()));
            row++;
        }
    }
}

void MainWidget::reset_categories_ui()
{
    currentCategoryId = 0;
    ui->pb_admin_addCategory->setEnabled(true);
    ui->pb_admin_editCategory->setEnabled(false);
    ui->pb_admin_deleteCategory->setEnabled(false);
    ui->tw_admin_categories->clearSelection();
}

void MainWidget::filter_categories(const QString &searchText)
{
    for (int row = 0; row < ui->tw_admin_categories->rowCount(); ++row) {
        QTableWidgetItem *itemId = ui->tw_admin_categories->item(row, 0);
        QTableWidgetItem *itemName = ui->tw_admin_categories->item(row, 1);
        QString idText = itemId ? itemId->text() : "";
        QString nameText = itemName ? itemName->text() : "";
        bool match = idText.contains(searchText, Qt::CaseInsensitive) ||
                     nameText.contains(searchText, Qt::CaseInsensitive);
        ui->tw_admin_categories->setRowHidden(row, !match);
    }
}

void MainWidget::show_edit_category()
{
    edit_widget ew(edit_widget::CategoryMode, currentCategoryId, this);

    if (ew.exec() == QDialog::Accepted) {
        load_categories_table();
    } else {
        reset_categories_ui();
    }
}

void MainWidget::table_categories_clicked(int row)
{
    currentCategoryId = 0;
    QTableWidgetItem *item = ui->tw_admin_categories->item(row, 0);
    if (item) {
        currentCategoryId = item->text().toInt();
        ui->pb_admin_addCategory->setEnabled(false);
        ui->pb_admin_editCategory->setEnabled(true);
        ui->pb_admin_deleteCategory->setEnabled(true);
        qDebug()<<"ID категории: "<<currentCategoryId;
    }
}

void MainWidget::load_materials_table()
{
    reset_materials_ui();

    QSqlDatabase db = QSqlDatabase::database("db_dp_kalugina");
    if (!db.isValid() || !db.isOpen()) return;

    QSqlQuery query(db);
    if (query.exec("SELECT id, name FROM materials")) {
        ui->tw_admin_materials->setRowCount(0);

        int row = 0;
        while (query.next()) {
            ui->tw_admin_materials->insertRow(row);
            ui->tw_admin_materials->setItem(row, 0, new QTableWidgetItem(query.value(0).toString()));
            ui->tw_admin_materials->setItem(row, 1, new QTableWidgetItem(query.value(1).toString()));
            row++;
        }
    }
}

void MainWidget::reset_materials_ui()
{
    currentMaterialId = 0;
    ui->pb_admin_addMaterial->setEnabled(true);
    ui->pb_admin_editMaterial->setEnabled(false);
    ui->pb_admin_deleteMaterial->setEnabled(false);
    ui->tw_admin_materials->clearSelection();
}

void MainWidget::filter_materials(const QString &searchText)
{
    for (int row = 0; row < ui->tw_admin_materials->rowCount(); ++row) {
        QTableWidgetItem *itemId = ui->tw_admin_materials->item(row, 0);
        QTableWidgetItem *itemName = ui->tw_admin_materials->item(row, 1);
        QString idText = itemId ? itemId->text() : "";
        QString nameText = itemName ? itemName->text() : "";
        bool match = idText.contains(searchText, Qt::CaseInsensitive) ||
                     nameText.contains(searchText, Qt::CaseInsensitive);
        ui->tw_admin_materials->setRowHidden(row, !match);
    }
}

void MainWidget::show_edit_material()
{
    edit_widget ew(edit_widget::MaterialMode, currentMaterialId, this);

    if (ew.exec() == QDialog::Accepted) {
        load_materials_table();
    } else {
        reset_materials_ui();
    }
}

void MainWidget::table_materials_clicked(int row)
{
    currentMaterialId = 0;
    QTableWidgetItem *item = ui->tw_admin_materials->item(row, 0);
    if (item) {
        currentMaterialId = item->text().toInt();
        ui->pb_admin_addMaterial->setEnabled(false);
        ui->pb_admin_editMaterial->setEnabled(true);
        ui->pb_admin_deleteMaterial->setEnabled(true);
        qDebug()<<"ID материала: "<<currentMaterialId;
    }
}

void MainWidget::load_suppliers_table()
{
    reset_suppliers_ui();

    QSqlDatabase db = QSqlDatabase::database("db_dp_kalugina");
    if (!db.isValid() || !db.isOpen()) return;

    QSqlQuery query(db);
    if (query.exec("SELECT id, name FROM suppliers")) {
        ui->tw_admin_suppliers->setRowCount(0);

        int row = 0;
        while (query.next()) {
            ui->tw_admin_suppliers->insertRow(row);
            ui->tw_admin_suppliers->setItem(row, 0, new QTableWidgetItem(query.value(0).toString()));
            ui->tw_admin_suppliers->setItem(row, 1, new QTableWidgetItem(query.value(1).toString()));
            row++;
        }
    }
}

void MainWidget::reset_suppliers_ui()
{
    currentSupplierId = 0;
    ui->pb_admin_addSupplier->setEnabled(true);
    ui->pb_admin_editSupplier->setEnabled(false);
    ui->pb_admin_deleteSupplier->setEnabled(false);
    ui->tw_admin_suppliers->clearSelection();
}

void MainWidget::filter_suppliers(const QString &searchText)
{
    for (int row = 0; row < ui->tw_admin_suppliers->rowCount(); ++row) {
        QTableWidgetItem *itemId = ui->tw_admin_suppliers->item(row, 0);
        QTableWidgetItem *itemName = ui->tw_admin_suppliers->item(row, 1);
        QString idText = itemId ? itemId->text() : "";
        QString nameText = itemName ? itemName->text() : "";
        bool match = idText.contains(searchText, Qt::CaseInsensitive) ||
                     nameText.contains(searchText, Qt::CaseInsensitive);
        ui->tw_admin_suppliers->setRowHidden(row, !match);
    }
}

void MainWidget::show_edit_supplier()
{
    edit_widget ew(edit_widget::SupplierMode, currentSupplierId, this);

    if (ew.exec() == QDialog::Accepted) {
        load_suppliers_table();
    } else {
        reset_suppliers_ui();
    }
}

void MainWidget::table_suppliers_clicked(int row)
{
    currentSupplierId = 0;
    QTableWidgetItem *item = ui->tw_admin_suppliers->item(row, 0);
    if (item) {
        currentSupplierId = item->text().toInt();
        ui->pb_admin_addSupplier->setEnabled(false);
        ui->pb_admin_editSupplier->setEnabled(true);
        ui->pb_admin_deleteSupplier->setEnabled(true);
        qDebug()<<"ID поставщика: "<<currentSupplierId;
    }
}

void MainWidget::loadWarehouseTable()
{
    QSqlDatabase db = QSqlDatabase::database("db_dp_kalugina");
    if (!db.isValid() || !db.isOpen()) return;

    QSqlQuery query(db);

    QString sql =
        "SELECT "
        "  m.id, "
        "  c.name AS category_name, "
        "  m.name AS material_name, "
        "  SUM(b.current_quantity) AS total_quantity, "
        "  u.name AS unit_name, "
        "  ROUND(SUM(b.current_quantity * b.purchase_price) / SUM(b.current_quantity), 2) AS avg_price, "
        "  ROUND(SUM(b.current_quantity * b.purchase_price), 2) AS total_sum "
        "FROM materials m "
        "JOIN categories c ON m.category_id = c.id "
        "JOIN units u ON m.unit_id = u.id "
        "JOIN batches b ON m.id = b.material_id "
        "WHERE b.current_quantity > 0 "
        "GROUP BY m.id, c.name, m.name, u.name "
        "ORDER BY c.name ASC, m.name ASC";

    if (query.exec(sql)) {
        ui->tw_warehouse->setRowCount(0);
        ui->tw_warehouse->setSortingEnabled(false);

        int row = 0;
        while (query.next()) {
            ui->tw_warehouse->insertRow(row);

            int materialId = query.value(0).toInt();
            QString category = query.value(1).toString();
            QString material = query.value(2).toString();
            double quantity = query.value(3).toDouble();
            QString unit = query.value(4).toString();
            double avgPrice = query.value(5).toDouble();
            double totalSum = query.value(6).toDouble();

            QTableWidgetItem *item_cat = new QTableWidgetItem(category);
            item_cat->setData(Qt::UserRole, materialId);

            QTableWidgetItem *item_name = new QTableWidgetItem(material);

            QTableWidgetItem *item_qty = new QTableWidgetItem(QString::number(quantity, 'f', 3));
            item_qty->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);

            QTableWidgetItem *item_unit = new QTableWidgetItem(unit);

            QTableWidgetItem *item_price = new QTableWidgetItem(QString::number(avgPrice, 'f', 2));
            item_price->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);

            QTableWidgetItem *item_sum = new QTableWidgetItem(QString::number(totalSum, 'f', 2));
            item_sum->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);

            ui->tw_warehouse->setItem(row, 0, item_cat);
            ui->tw_warehouse->setItem(row, 1, item_name);
            ui->tw_warehouse->setItem(row, 2, item_qty);
            ui->tw_warehouse->setItem(row, 3, item_unit);
            ui->tw_warehouse->setItem(row, 4, item_price);
            ui->tw_warehouse->setItem(row, 5, item_sum);

            row++;
        }
        ui->tw_warehouse->setSortingEnabled(true);
    } else {
        qCritical() << "Ошибка загрузки склада:" << query.lastError().text();
    }
    //ui->splitter->setStretchFactor(0, 1);
}

void MainWidget::filter_warehouse()
{
    QString searchText = ui->le_warehouse_materials_search->text();
    QString selectedCategory = ui->cb_warehouse_categories->currentText();
    int selectedCategoryId = ui->cb_warehouse_categories->currentData().toInt();

    for (int row = 0; row < ui->tw_warehouse->rowCount(); ++row) {
        QTableWidgetItem *itemCat = ui->tw_warehouse->item(row, 0);
        QTableWidgetItem *itemName = ui->tw_warehouse->item(row, 1);

        QString catText = itemCat ? itemCat->text() : "";
        QString nameText = itemName ? itemName->text() : "";

        bool textMatch = nameText.contains(searchText, Qt::CaseInsensitive);

        bool categoryMatch = (selectedCategoryId == 0) || (catText == selectedCategory);

        ui->tw_warehouse->setRowHidden(row, !(textMatch && categoryMatch));
    }
}

void MainWidget::loadWarehouseCategories()
{
    ui->cb_warehouse_categories->clear();
    ui->cb_warehouse_categories->addItem("Все категории", 0);
    QSqlDatabase db = QSqlDatabase::database("db_dp_kalugina");
    QSqlQuery query(db);

    if (query.exec("SELECT id, name FROM categories ORDER BY name")) {
        while (query.next()) {
            ui->cb_warehouse_categories->addItem(query.value(1).toString(), query.value(0).toInt());
        }
    }
}

void MainWidget::toggleWarehouseLayout()
{
    if (ui->splitter->orientation() == Qt::Horizontal) {
        ui->splitter->setOrientation(Qt::Vertical);
        ui->lb_change_layout->setPixmap(QPixmap(":/res/edithlayoutsplit.png"));
        //loadWarehouseTable();
    } else {
        ui->splitter->setOrientation(Qt::Horizontal);
        ui->lb_change_layout->setPixmap(QPixmap(":/res/editvlayoutsplit.png"));
        //loadWarehouseTable();
    }
}

void MainWidget::table_warehouseBatches_clicked(int row)
{
    currentMaterialBatchesId = 0;
    QTableWidgetItem *item = ui->tw_warehouse->item(row, 0);
    if (item) {
        currentMaterialBatchesId = item->data(Qt::UserRole).toInt();
        if (!m_isWarehouseDetailsOpened) {
            ui->splitter->setSizes({1, 1});
            m_isWarehouseDetailsOpened = true;
        }
        qDebug()<<"ID записи: "<<currentMaterialBatchesId;
        loadWarehouseBachesTable();
        loadWarehouseMovementsTable();
    }
}

void MainWidget::loadWarehouseBachesTable()
{
    ui->tw_warehouse_batches->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    QSqlDatabase db = QSqlDatabase::database("db_dp_kalugina");
    if (!db.isValid() || !db.isOpen()) return;

    QSqlQuery query(db);

    QString sql =
        "SELECT "
        "  b.incoming_date, "
        "  s.name AS supplier_name, "
        "  b.current_quantity, "
        "  b.purchase_price, "
        "  DATEDIFF(NOW(), b.incoming_date) AS days_on_stock "
        "FROM batches b "
        "JOIN suppliers s ON b.supplier_id = s.id "
        "WHERE b.material_id = :matId AND b.current_quantity > 0 "
        "ORDER BY b.incoming_date ASC";

    query.prepare(sql);
    query.bindValue(":matId", currentMaterialBatchesId);

    if (query.exec()) {
        ui->tw_warehouse_batches->setRowCount(0);
        int row = 0;

        while (query.next()) {
            ui->tw_warehouse_batches->insertRow(row);

            QDateTime incomingDate = query.value(0).toDateTime();
            QString supplier = query.value(1).toString();
            double quantity = query.value(2).toDouble();
            double price = query.value(3).toDouble();
            int daysOnStock = query.value(4).toInt();

            QTableWidgetItem *itemDate = new QTableWidgetItem(incomingDate.toString("dd.MM.yyyy HH:mm"));
            QTableWidgetItem *itemSupplier = new QTableWidgetItem(supplier);

            QTableWidgetItem *itemQty = new QTableWidgetItem(QString::number(quantity, 'f', 3));
            itemQty->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);

            QTableWidgetItem *itemPrice = new QTableWidgetItem(QString::number(price, 'f', 2));
            itemPrice->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);

            QTableWidgetItem *itemDays = new QTableWidgetItem(QString::number(daysOnStock));
            itemDays->setTextAlignment(Qt::AlignCenter);

            ui->tw_warehouse_batches->setItem(row, 0, itemDate);
            ui->tw_warehouse_batches->setItem(row, 1, itemSupplier);
            ui->tw_warehouse_batches->setItem(row, 2, itemQty);
            ui->tw_warehouse_batches->setItem(row, 3, itemPrice);
            ui->tw_warehouse_batches->setItem(row, 4, itemDays);

            row++;
        }
    }
}

void MainWidget::loadWarehouseMovementsTable()
{
    ui->tw_warehouse_movements->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    QSqlDatabase db = QSqlDatabase::database("db_dp_kalugina");
    if (!db.isValid() || !db.isOpen()) return;

    QSqlQuery query(db);

    QString sql =
        "SELECT "
        "  d.doc_date, "
        "  d.doc_type, "
        "  d.id AS doc_id, "
        "  it.quantity, "
        "  u.login "
        "FROM inventory_transactions it "
        "JOIN documents d ON it.document_id = d.id "
        "JOIN users u ON d.user_id = u.id "
        "WHERE it.material_id = :matId "
        "ORDER BY d.doc_date ASC";

    query.prepare(sql);
    query.bindValue(":matId", currentMaterialBatchesId);

    if (query.exec()) {
        ui->tw_warehouse_movements->setRowCount(0);
        ui->tw_warehouse_movements->setSortingEnabled(false);

        double runningBalance = 0.0;
        int row = 0;

        while (query.next()) {
            ui->tw_warehouse_movements->insertRow(row);

            QDateTime dateTime = query.value(0).toDateTime();
            QString typeStr = query.value(1).toString();
            int docId = query.value(2).toInt();
            double qty = query.value(3).toDouble();
            QString user = query.value(4).toString();

            QString displayType;
            QColor rowColor;

            if (typeStr == "incoming") {
                displayType = "Приход";
                rowColor = QColor(200, 245, 200);
                runningBalance += qty;
            } else {
                displayType = "Расход";
                rowColor = QColor(255, 210, 210);
                runningBalance -= qty;
                qty = -qty;
            }

            QTableWidgetItem *itemDate    = new QTableWidgetItem(dateTime.toString("dd.MM.yy HH:mm"));
            QTableWidgetItem *itemType    = new QTableWidgetItem(displayType);
            QTableWidgetItem *itemDoc     = new QTableWidgetItem(QString("Док. №%1").arg(docId));
            QTableWidgetItem *itemQty     = new QTableWidgetItem(QString::number(qty, 'f', 3));
            QTableWidgetItem *itemBalance = new QTableWidgetItem(QString::number(runningBalance, 'f', 3));
            QTableWidgetItem *itemUser    = new QTableWidgetItem(user);

            itemQty->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
            itemBalance->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
            itemBalance->setFont(QFont("Segoe UI", -1, QFont::Bold));

            QList<QTableWidgetItem*> allItems = {itemDate, itemType, itemDoc, itemQty, itemBalance, itemUser};

            for (int col = 0; col < allItems.size(); ++col) {
                QTableWidgetItem *item = allItems.at(col);
                item->setBackground(rowColor);
                ui->tw_warehouse_movements->setItem(row, col, item);
            }

            row++;
        }
        ui->tw_warehouse_movements->setSortingEnabled(true);
        ui->tw_warehouse_movements->scrollToBottom();
    }
}
