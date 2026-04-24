#include "mainwidget.h"
#include "ui_mainwidget.h"
#include "authorizationwindow.h"
#include "edit_role.h"
#include "edit_user.h"
#include "edit_widget.h"
#include "reportwidget.h"

MainWidget::MainWidget(int userId, int userRole, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::MainWidget)
    , userId(userId)
    , userRole(userRole)
{
    ui->setupUi(this);

    // --- 1. СИСТЕМНЫЕ ФУНКЦИИ И ВЫХОД ---
    adminFunction();

    QToolButton *logoutBtn = new QToolButton(this);
    logoutBtn->setText("Выход");
    logoutBtn->setAutoRaise(true);
    ui->tabw_main->setCornerWidget(logoutBtn, Qt::TopRightCorner); //

    connect(logoutBtn, &QToolButton::clicked, this, &MainWidget::logout);

    // --- 2. ОБЩАЯ НАВИГАЦИЯ ---
    connect(ui->tabw_main, &QTabWidget::currentChanged, this, &MainWidget::tabw_main_change);
    connect(ui->tabw_administration, &QTabWidget::currentChanged, this, &MainWidget::tabw_administration_change);

    // --- 3. АДМИНИСТРИРОВАНИЕ ---
    connect(ui->le_admin_users_search, &QLineEdit::textChanged, this, &MainWidget::filter_users);
    connect(ui->cb_admin_roles, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWidget::filter_users);
    connect(ui->tw_admin_users, &QTableWidget::cellClicked, this, &MainWidget::table_users_clicked);
    connect(ui->pb_admin_addUser, &QPushButton::clicked, this, &MainWidget::show_edit_user);
    connect(ui->pb_admin_editUser, &QPushButton::clicked, this, &MainWidget::show_edit_user);

    connect(ui->le_admin_roles_search, &QLineEdit::textChanged, this, &MainWidget::filter_roles);
    connect(ui->tw_admin_roles, &QTableWidget::cellClicked, this, &MainWidget::table_roles_clicked);
    connect(ui->pb_admin_addRole, &QPushButton::clicked, this, &MainWidget::show_edit_role);
    connect(ui->pb_admin_editRole, &QPushButton::clicked, this, &MainWidget::show_edit_role);

    connect(ui->le_admin_categories_search, &QLineEdit::textChanged, this, &MainWidget::filter_categories);
    connect(ui->pb_admin_addCategory, &QPushButton::clicked, this, &MainWidget::show_edit_category);
    connect(ui->pb_admin_editCategory, &QPushButton::clicked, this, &MainWidget::show_edit_category);
    connect(ui->tw_admin_categories, &QTableWidget::cellClicked, this, &MainWidget::table_categories_clicked);

    connect(ui->le_admin_materials_search, &QLineEdit::textChanged, this, &MainWidget::filter_materials);
    connect(ui->pb_admin_addMaterial, &QPushButton::clicked, this, &MainWidget::show_edit_material);
    connect(ui->pb_admin_editMaterial, &QPushButton::clicked, this, &MainWidget::show_edit_material);
    connect(ui->tw_admin_materials, &QTableWidget::cellClicked, this, &MainWidget::table_materials_clicked);

    connect(ui->le_admin_suppliers_search, &QLineEdit::textChanged, this, &MainWidget::filter_suppliers);
    connect(ui->pb_admin_addSupplier, &QPushButton::clicked, this, &MainWidget::show_edit_supplier);
    connect(ui->pb_admin_editSupplier, &QPushButton::clicked, this, &MainWidget::show_edit_supplier);
    connect(ui->tw_admin_suppliers, &QTableWidget::cellClicked, this, &MainWidget::table_suppliers_clicked);

    // --- 4. СКЛАД ---
    connect(ui->le_warehouse_materials_search, &QLineEdit::textChanged, this, &MainWidget::filter_warehouse);
    connect(ui->cb_warehouse_categories, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWidget::filter_warehouse);
    connect(ui->lb_change_layout, &QPushLabel::clicked, this, &MainWidget::toggleWarehouseLayout);
    connect(ui->tw_warehouse, &QTableWidget::cellClicked, this, &MainWidget::table_warehouseBatches_clicked);
    connect(ui->pb_baches_report, &QPushButton::clicked, this, &MainWidget::reportBaches);
    connect(ui->pb_movements_report, &QPushButton::clicked, this, &MainWidget::reportMovements);

    // --- 5. ИНВЕНТАРИЗАЦИЯ ---
    ui->de_history_start->setCalendarPopup(true);
    ui->de_history_end->setCalendarPopup(true);

    connect(ui->le_history_search, &QLineEdit::textChanged, this, &MainWidget::filter_inventory);
    connect(ui->de_history_start, &QDateEdit::dateChanged, this, &MainWidget::filter_inventory);
    connect(ui->de_history_end, &QDateEdit::dateChanged, this, &MainWidget::filter_inventory);
    connect(ui->cb_history_period, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWidget::history_period_currentIndexChanged);

    auto setCustomInvPeriod = [this](){
        if (ui->cb_history_period->currentIndex() != 4) {
            ui->cb_history_period->blockSignals(true);
            ui->cb_history_period->setCurrentIndex(4);
            ui->cb_history_period->blockSignals(false);
        }
    };
    connect(ui->de_history_start, &QDateEdit::userDateChanged, this, setCustomInvPeriod);
    connect(ui->de_history_end, &QDateEdit::userDateChanged, this, setCustomInvPeriod);
    connect(ui->de_history_start, &QDateEdit::dateChanged, this, [this](QDate date){ ui->de_history_end->setMinimumDate(date); });

    connect(ui->tw_inventory_history, &QTableWidget::cellClicked, this, &MainWidget::table_inventory_clicked);
    connect(ui->pb_inventoryDetailsBack, &QPushButton::clicked, this, &MainWidget::closeInventoryWorkArea);
    connect(ui->pb_inventorization, &QPushButton::clicked, this, &MainWidget::showInventory);
    connect(ui->pb_inventory_cancel, &QPushButton::clicked, this, &MainWidget::inventory_cancel);
    connect(ui->pb_inventory_ok, &QPushButton::clicked, this, &MainWidget::saveInventory);
    connect(ui->pb_inventory_report, &QPushButton::clicked, this, &MainWidget::reportInventory);

    // --- 6. ПОСТУПЛЕНИЯ УНИВЕРСАЛЬНЫЕ ФУНКЦИИ ---
    ui->de_incoming_start->setCalendarPopup(true);
    ui->de_incoming_end->setCalendarPopup(true);
    ui->de_new_incoming_date->setCalendarPopup(true);

    connect(ui->le_incoming_search, &QLineEdit::textChanged, this, [this](){ filterDocs(Incoming); });
    connect(ui->cb_incoming_filter_counterparty, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](){ filterDocs(Incoming); });
    connect(ui->de_incoming_start, &QDateEdit::dateChanged, this, [this](){ filterDocs(Incoming); });
    connect(ui->de_incoming_end, &QDateEdit::dateChanged, this, [this](){ filterDocs(Incoming); });
    connect(ui->cb_incoming_period, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int index){ updateDocPeriod(Incoming, index); });

    auto setCustomIncPeriod = [this](){
        if (ui->cb_incoming_period->currentIndex() != 4) {
            ui->cb_incoming_period->blockSignals(true);
            ui->cb_incoming_period->setCurrentIndex(4);
            ui->cb_incoming_period->blockSignals(false);
        }
    };
    connect(ui->de_incoming_start, &QDateEdit::userDateChanged, this, setCustomIncPeriod);
    connect(ui->de_incoming_end, &QDateEdit::userDateChanged, this, setCustomIncPeriod);
    connect(ui->de_incoming_start, &QDateEdit::dateChanged, this, [this](QDate d){ ui->de_incoming_end->setMinimumDate(d); });

    connect(ui->tw_incoming_history, &QTableWidget::cellClicked, this, [this](int r){ handleDocClick(Incoming, r); });
    connect(ui->pb_incoming_add, &QPushButton::clicked, this, [this](){ prepareDocEditor(Incoming); });
    connect(ui->pb_new_incoming_cancel, &QPushButton::clicked, this, [this](){ cancelDocOperation(Incoming); });
    connect(ui->pb_new_incoming_save, &QPushButton::clicked, this, &MainWidget::saveIncoming);

    connect(ui->pb_incoming_report, &QPushButton::clicked, this, &MainWidget::reportIncoming);
    connect(ui->pb_outgoing_report, &QPushButton::clicked, this, &MainWidget::reportOutgoing);

    // Ввод данных в строку поступления
    connect(ui->cb_inc_add_material, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](){ handleMaterialSelectionChanged(Incoming); });
    connect(ui->dsb_inc_add_qty, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [this](){ calculateCurrentRowSum(Incoming); });
    connect(ui->dsb_inc_add_price, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [this](){ calculateCurrentRowSum(Incoming); });
    connect(ui->pb_inc_add_row, &QPushButton::clicked, this, [this](){ addDocRow(Incoming); });
    connect(ui->pb_inc_clear_fields, &QPushButton::clicked, this, [this](){ clearDocInputBar(Incoming); });
    connect(ui->tw_incoming_editor, &QTableWidget::cellClicked, this, [this](int r){ deleteDocRow(Incoming, r); });
    connect(ui->lb_change_layout_incoming, &QPushLabel::clicked, this, [this](){
        toggleDocLayout(Incoming);
    });

    // --- 7. СПИСАНИЯ УНИВЕРСАЛЬНЫЕ ФУНКЦИИ ---
    ui->de_outgoing_start->setCalendarPopup(true);
    ui->de_outgoing_end->setCalendarPopup(true);
    ui->de_new_outgoing_date->setCalendarPopup(true);

    connect(ui->le_outgoing_search, &QLineEdit::textChanged, this, [this](){ filterDocs(Outgoing); });
    connect(ui->de_outgoing_start, &QDateEdit::dateChanged, this, [this](){ filterDocs(Outgoing); });
    connect(ui->de_outgoing_end, &QDateEdit::dateChanged, this, [this](){ filterDocs(Outgoing); });
    connect(ui->cb_outgoing_period, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int index){ updateDocPeriod(Outgoing, index); });

    auto setCustomOutPeriod = [this](){
        if (ui->cb_outgoing_period->currentIndex() != 4) {
            ui->cb_outgoing_period->blockSignals(true);
            ui->cb_outgoing_period->setCurrentIndex(4);
            ui->cb_outgoing_period->blockSignals(false);
        }
    };
    connect(ui->de_outgoing_start, &QDateEdit::userDateChanged, this, setCustomOutPeriod);
    connect(ui->de_outgoing_end, &QDateEdit::userDateChanged, this, setCustomOutPeriod);
    connect(ui->de_outgoing_start, &QDateEdit::dateChanged, this, [this](QDate d){ ui->de_outgoing_end->setMinimumDate(d); });

    connect(ui->tw_outgoing_history, &QTableWidget::cellClicked, this, [this](int r){ handleDocClick(Outgoing, r); });
    connect(ui->pb_outgoing_add, &QPushButton::clicked, this, [this](){ prepareDocEditor(Outgoing); });
    connect(ui->pb_new_outgoing_cancel, &QPushButton::clicked, this, [this](){ cancelDocOperation(Outgoing); });
    connect(ui->pb_new_outgoing_save, &QPushButton::clicked, this, &MainWidget::saveOutgoing);

    // Ввод данных в строку списания
    connect(ui->cb_out_add_material, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](){ handleMaterialSelectionChanged(Outgoing); });
    connect(ui->dsb_out_add_qty, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [this](){ calculateCurrentRowSum(Outgoing); });
    connect(ui->dsb_out_add_price, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [this](){ calculateCurrentRowSum(Outgoing); });
    connect(ui->pb_out_add_row, &QPushButton::clicked, this, [this](){ addDocRow(Outgoing); });
    connect(ui->pb_out_clear_fields, &QPushButton::clicked, this, [this](){ clearDocInputBar(Outgoing); });
    connect(ui->tw_outgoing_editor, &QTableWidget::cellClicked, this, [this](int r){ deleteDocRow(Outgoing, r); });
    connect(ui->lb_change_layout_outgoing, &QPushLabel::clicked, this, [this](){
        toggleDocLayout(Outgoing);
    });

    connect(ui->pb_period_reset, &QPushButton::clicked, this, &MainWidget::setupReportDates);
    connect(ui->pb_report_reset, &QPushButton::clicked, this, &MainWidget::tabw_report_change);
    connect(ui->list_report_types, &QListWidget::currentRowChanged, this, &MainWidget::onReportTypeChanged);

    // --- 8. НАСТРОЙКА ГЕОМЕТРИИ И ФИЛЬТРОВ СОБЫТИЙ ---
    auto setupHeader = [](QTableWidget* tw) {
        tw->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        tw->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
        for(int i = 1; i < tw->columnCount(); ++i)
            tw->horizontalHeader()->setSectionResizeMode(i, QHeaderView::Stretch);
    };

    setupHeader(ui->tw_admin_users);
    setupHeader(ui->tw_admin_roles);
    setupHeader(ui->tw_admin_categories);
    setupHeader(ui->tw_admin_materials);
    setupHeader(ui->tw_admin_suppliers);
    ui->tw_warehouse->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);

    // Event Filters для полей ввода (selectAll при клике)
    ui->dsb_inc_add_qty->installEventFilter(this);
    ui->dsb_inc_add_price->installEventFilter(this);
    ui->dsb_out_add_qty->installEventFilter(this);
    ui->dsb_out_add_price->installEventFilter(this);

    // --- 9. НАЧАЛЬНОЕ СОСТОЯНИЕ ---
    ui->tabw_main->setCurrentIndex(0);
    ui->tabw_administration->setCurrentIndex(0);
    tabw_incoming_change();

    showMaximized();
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
    QMessageBox msgBox(
        QMessageBox::Question,
        "Подтверждение выхода",
        "Вы действительно хотите выйти из учётной записи?",
        QMessageBox::Yes | QMessageBox::No,
        this
        );

    msgBox.setButtonText(QMessageBox::Yes, "Да");
    msgBox.setButtonText(QMessageBox::No, "Нет");

    if (msgBox.exec() == QMessageBox::Yes) {
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
        tabw_incoming_change();
    }
    else if(index == 1) {
        tabw_outgoing_change();
    }
    else if(index == 2) {
        tabw_warehouse_change();
    }
    else if(index == 3) {
        tabw_report_change();
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
    if (m_isFirstWarehouseOpen) {
        ui->splitter->setOrientation(Qt::Vertical);
        ui->lb_change_layout->setPixmap(QPixmap(":/res/edithlayoutsplit.png"));
        ui->splitter->setSizes({1, 0});

        m_isWarehouseDetailsOpened = false;
        currentMaterialBatchesId = 0;

        ui->cb_history_period->setCurrentIndex(-1);
        ui->cb_history_period->setCurrentIndex(0);

        m_isFirstWarehouseOpen = false;
    }

    loadWarehouseCategories();
    loadWarehouseTable();
    loadInventoryHistoryTable();

    if (ui->sw_inventarization->currentIndex() == 0) {
        if (currentMaterialBatchesId != 0) {
            loadWarehouseBachesTable();
            loadWarehouseMovementsTable();
        }
    }
}

void MainWidget::tabw_incoming_change()
{
    if (m_isFirstIncomingOpen) {
        ui->splitter_2->setOrientation(Qt::Vertical);
        ui->lb_change_layout_incoming->setPixmap(QPixmap(":/res/edithlayoutsplit.png"));
        ui->splitter_2->setSizes({1, 0});

        m_isIncomingDetailsOpened = false;
        currentIncomingId = 0;

        ui->cb_incoming_period->setCurrentIndex(-1);
        ui->cb_incoming_period->setCurrentIndex(0);

        m_isFirstIncomingOpen = false;
    }

    loadIncomingSuppliers();
    loadHistoryTable(Incoming);

    if (currentIncomingId != 0) {
        loadDocDetails(Incoming);
    }
}

void MainWidget::tabw_outgoing_change()
{
    if (m_isFirstOutgoingOpen) {
        ui->splitter_3->setOrientation(Qt::Vertical);
        ui->lb_change_layout_outgoing->setPixmap(QPixmap(":/res/edithlayoutsplit.png"));
        ui->splitter_3->setSizes({1, 0});

        m_isOutgoingDetailsOpened = false;
        currentOutgoingId = 0;

        ui->cb_outgoing_period->setCurrentIndex(-1);
        ui->cb_outgoing_period->setCurrentIndex(0);

        m_isFirstOutgoingOpen = false;
    }

    loadHistoryTable(Outgoing);

    if (currentOutgoingId != 0) {
        loadDocDetails(Outgoing);
    }
}

void MainWidget::tabw_report_change()
{
    fillReportTypes();
    fillReportFilters();
    setupReportDates();

    if (ui->list_report_types->count() > 0) {
        ui->list_report_types->setCurrentRow(0);
    }

    ui->lb_report_summary->setText("Настройте фильтры и нажмите 'Применить'");
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

        if (currentMaterialBatchesId != 0) {
            for (int row = 0; row < ui->tw_warehouse->rowCount(); ++row) {
                if (ui->tw_warehouse->item(row, 0)->data(Qt::UserRole).toInt() == currentMaterialBatchesId) {
                    ui->tw_warehouse->selectRow(row);
                    break;
                }
            }
        }
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
    } else {
        ui->splitter->setOrientation(Qt::Horizontal);
        ui->lb_change_layout->setPixmap(QPixmap(":/res/editvlayoutsplit.png"));
    }
}

void MainWidget::table_warehouseBatches_clicked(int row)
{
    currentMaterialBatchesId = 0;
    QTableWidgetItem *item = ui->tw_warehouse->item(row, 0);

    if (item) {
        currentMaterialBatchesId = item->data(Qt::UserRole).toInt();
        if (ui->splitter->sizes().at(1) == 0) {
            ui->splitter->setSizes({1, 1});
        }

        qDebug() << "ID записи: " << currentMaterialBatchesId;

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
        "LEFT JOIN suppliers s ON b.supplier_id = s.id "
        "WHERE b.material_id = :matId AND b.current_quantity > 0.0001 "
        "ORDER BY b.incoming_date ASC";

    query.prepare(sql);
    query.bindValue(":matId", currentMaterialBatchesId);

    if (query.exec()) {
        ui->tw_warehouse_batches->setRowCount(0);
        ui->tw_warehouse_batches->setSortingEnabled(false);
        int row = 0;

        while (query.next()) {
            ui->tw_warehouse_batches->insertRow(row);

            QDateTime incomingDate = query.value(0).toDateTime();

            QString supplier = query.value(1).toString();
            if (supplier.isEmpty()) {
                supplier = "Инвентаризация";
            }

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
        ui->tw_warehouse_batches->setSortingEnabled(true);
    } else {
        qCritical() << "Ошибка загрузки партий:" << query.lastError().text();
    }
}

void MainWidget::loadWarehouseMovementsTable()
{
    ui->tw_warehouse_movements->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    QSqlDatabase db = QSqlDatabase::database("db_dp_kalugina");
    if (!db.isValid() || !db.isOpen()) return;

    QSqlQuery query(db);

    QString sql =
        "SELECT d.doc_date, d.doc_type, d.id, it.quantity, u.login "
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
            }
            else if (typeStr == "outgoing") {
                displayType = "Расход";
                rowColor = QColor(255, 210, 210);
                runningBalance -= qAbs(qty);
                qty = -qAbs(qty);
            }
            else if (typeStr == "inventory") {
                displayType = "Инвентар.";
                runningBalance += qty;
                rowColor = (qty >= 0) ? QColor(240, 240, 240) : QColor(255, 235, 235);
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
            for (auto item : allItems) {
                item->setBackground(rowColor);
                ui->tw_warehouse_movements->setItem(row, allItems.indexOf(item), item);
            }
            row++;
        }
        ui->tw_warehouse_movements->setSortingEnabled(true);
        ui->tw_warehouse_movements->scrollToBottom();
    }
}

void MainWidget::reportBaches()
{
    if (currentMaterialBatchesId <= 0) {
        QMessageBox::warning(this, "Внимание", "Сначала выберите материал в таблице склада!");
        return;
    }

    reportwidget *rw = new reportwidget(reportwidget::MaterialBatches, currentMaterialBatchesId, this);
    rw->setAttribute(Qt::WA_DeleteOnClose);
    rw->exec();
}

void MainWidget::reportMovements()
{
    if (currentMaterialBatchesId <= 0) {
        QMessageBox::warning(this, "Внимание", "Сначала выберите материал в таблице склада!");
        return;
    }

    reportwidget *rw = new reportwidget(reportwidget::MaterialHistory, currentMaterialBatchesId, this);
    rw->setAttribute(Qt::WA_DeleteOnClose);
    rw->show();
}

void MainWidget::loadInventoryHistoryTable()
{
    ui->tw_inventory_history->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tw_inventory_history->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);

    QSqlDatabase db = QSqlDatabase::database("db_dp_kalugina");
    if (!db.isValid() || !db.isOpen()) return;

    QSqlQuery query(db);

    QString sql =
        "SELECT "
        "  d.id, "
        "  d.doc_date, "
        "  u.login, "
        "  COUNT(it.id) AS items_count, "
        "  SUM(it.quantity * it.price) AS total_diff "
        "FROM documents d "
        "JOIN users u ON d.user_id = u.id "
        "LEFT JOIN inventory_transactions it ON d.id = it.document_id "
        "WHERE d.doc_type = 'inventory' "
        "GROUP BY d.id, d.doc_date, u.login "
        "ORDER BY d.doc_date DESC";

    if (query.exec(sql)) {
        ui->tw_inventory_history->setRowCount(0);
        ui->tw_inventory_history->setSortingEnabled(false);

        int row = 0;
        while (query.next()) {
            ui->tw_inventory_history->insertRow(row);

            int docId = query.value(0).toInt();
            QDateTime date = query.value(1).toDateTime();
            QString user = query.value(2).toString();
            int itemsCount = query.value(3).toInt();
            double totalDiff = query.value(4).toDouble();

            QTableWidgetItem *itemNo = new QTableWidgetItem(QString::number(docId));

            itemNo->setData(Qt::UserRole, docId);

            QTableWidgetItem *itemDate = new QTableWidgetItem(date.toString("dd.MM.yyyy HH:mm"));
            QTableWidgetItem *itemUser = new QTableWidgetItem(user);

            QTableWidgetItem *itemCount = new QTableWidgetItem(QString::number(itemsCount));
            itemCount->setTextAlignment(Qt::AlignCenter);

            QTableWidgetItem *itemSum = new QTableWidgetItem(QString::number(totalDiff, 'f', 2));
            itemSum->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);

            if (totalDiff < 0) {
                itemSum->setForeground(Qt::red);
            } else if (totalDiff > 0) {
                itemSum->setForeground(QColor(0, 150, 0));
            }

            ui->tw_inventory_history->setItem(row, 0, itemNo);
            ui->tw_inventory_history->setItem(row, 1, itemDate);
            ui->tw_inventory_history->setItem(row, 2, itemUser);
            ui->tw_inventory_history->setItem(row, 3, itemCount);
            ui->tw_inventory_history->setItem(row, 4, itemSum);

            row++;
        }
        ui->tw_inventory_history->setSortingEnabled(true);
    } else {
        qCritical() << "Ошибка загрузки истории инвентаризаций:" << query.lastError().text();
    }
}

void MainWidget::filter_inventory()
{
    QString searchText = ui->le_history_search->text().trimmed();
    QDate startDate = ui->de_history_start->date();
    QDate endDate = ui->de_history_end->date();

    for (int row = 0; row < ui->tw_inventory_history->rowCount(); ++row) {
        QTableWidgetItem *itemNo = ui->tw_inventory_history->item(row, 0);
        QTableWidgetItem *itemDate = ui->tw_inventory_history->item(row, 1);
        QTableWidgetItem *itemUser = ui->tw_inventory_history->item(row, 2);

        QString idText = itemNo ? itemNo->text() : "";
        QString dateText = itemDate ? itemDate->text() : "";
        QString userText = itemUser ? itemUser->text() : "";

        bool textMatch = idText.contains(searchText, Qt::CaseInsensitive) ||
                         userText.contains(searchText, Qt::CaseInsensitive);

        QDate rowDate = QDate::fromString(dateText.left(10), "dd.MM.yyyy");
        bool dateMatch = (rowDate >= startDate && rowDate <= endDate);

        ui->tw_inventory_history->setRowHidden(row, !(textMatch && dateMatch));
    }
}

void MainWidget::history_period_currentIndexChanged(int index)
{
    QDate today = QDate::currentDate();
    QDate start;
    QDate end = today;

    ui->de_history_start->blockSignals(true);
    ui->de_history_end->blockSignals(true);

    ui->de_history_start->setMaximumDate(QDate(9999, 1, 1));
    ui->de_history_end->setMinimumDate(QDate(1900, 1, 1));

    switch (index) {
    case 0:
        start = getEarliestInventoryDate();
        break;
    case 1:
        start = today.addDays(-(today.dayOfWeek() - 1));
        break;
    case 2:
        start = QDate(today.year(), today.month(), 1);
        break;
    case 3:
        start = QDate(today.year(), 1, 1);
        break;
    case 4:
        ui->de_history_start->blockSignals(false);
        ui->de_history_end->blockSignals(false);
        return;
    }

    ui->de_history_end->setDate(end);
    ui->de_history_start->setDate(start);

    ui->de_history_end->setMinimumDate(start);
    ui->de_history_start->setMaximumDate(end);

    ui->de_history_start->blockSignals(false);
    ui->de_history_end->blockSignals(false);

    filter_inventory();
}

void MainWidget::table_inventory_clicked(int row)
{
    currentInventoryId = 0;
    QTableWidgetItem *item = ui->tw_inventory_history->item(row, 0);
    if (item) {
        currentInventoryId = item->data(Qt::UserRole).toInt();
        qDebug()<<"ID записи: "<<currentInventoryId;
        ui->sw_inventarization->setCurrentIndex(1);
        ui->pb_inventoryDetailsBack->setVisible(true);
        ui->widget->setVisible(false);
        loadInventoryDetails();
    }
}

void MainWidget::loadInventoryDetails()
{
    m_isInventoryLoading = true;

    ui->tw_inventoryDetails->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tw_inventoryDetails->setRowCount(0);
    ui->tw_inventoryDetails->setSortingEnabled(false);
    ui->lb_inventory_title->setVisible(false);
    ui->pb_inventory_report->setVisible(true);

    QHeaderView *header = ui->tw_inventoryDetails->horizontalHeader();
    header->setDefaultAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    header->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    header->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    header->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    header->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    header->setSectionResizeMode(5, QHeaderView::ResizeToContents);
    header->setSectionResizeMode(6, QHeaderView::ResizeToContents);
    header->setSectionResizeMode(7, QHeaderView::ResizeToContents);

    header->setSectionResizeMode(4, QHeaderView::Stretch);
    header->setSectionResizeMode(8, QHeaderView::Stretch);

    QSqlDatabase db = QSqlDatabase::database("db_dp_kalugina");
    if (!db.isValid() || !db.isOpen()) {
        m_isInventoryLoading = false;
        return;
    }

    QSqlQuery query(db);
    QString sql =
        "SELECT "
        "  c.name, m.name, u.name, it.expected_quantity, it.actual_quantity, "
        "  it.quantity, it.price, (it.quantity * it.price), m.id, "
        "  it.reason "
        "FROM inventory_transactions it "
        "JOIN materials m ON it.material_id = m.id "
        "JOIN categories c ON m.category_id = c.id "
        "JOIN units u ON m.unit_id = u.id "
        "WHERE it.document_id = :docId";

    query.prepare(sql);
    query.bindValue(":docId", currentInventoryId);

    if (query.exec()) {
        int row = 0;
        while (query.next()) {
            ui->tw_inventoryDetails->insertRow(row);

            QString category = query.value(0).toString();
            QString material = query.value(1).toString();
            QString unit     = query.value(2).toString();
            double expected  = query.value(3).toDouble();
            double actual    = query.value(4).toDouble();
            double diffQty   = query.value(5).toDouble();
            double price     = query.value(6).toDouble();
            double diffSum   = query.value(7).toDouble();
            int materialId   = query.value(8).toInt();
            QString reason   = query.value(9).toString();
            if (reason.isEmpty()) reason = "-";

            QTableWidgetItem *itemCat      = new QTableWidgetItem(category);
            QTableWidgetItem *itemMaterial = new QTableWidgetItem(material);
            QTableWidgetItem *itemUnit     = new QTableWidgetItem(unit);
            QTableWidgetItem *itemExpt     = new QTableWidgetItem(QString::number(expected, 'f', 3));
            QTableWidgetItem *itemActl     = new QTableWidgetItem(QString::number(actual, 'f', 3));
            QTableWidgetItem *itemDiff     = new QTableWidgetItem(QString::number(diffQty, 'f', 3));
            QTableWidgetItem *itemPrice    = new QTableWidgetItem(QString::number(price, 'f', 2));
            QTableWidgetItem *itemSum      = new QTableWidgetItem(QString::number(diffSum, 'f', 2));
            QTableWidgetItem *itemReason   = new QTableWidgetItem(reason);

            itemExpt->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
            itemActl->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
            itemDiff->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
            itemPrice->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
            itemSum->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);

            if (diffQty < 0) {
                itemDiff->setForeground(Qt::red);
                itemSum->setForeground(Qt::red);
            } else if (diffQty > 0) {
                itemDiff->setForeground(QColor(0, 150, 0));
                itemSum->setForeground(QColor(0, 150, 0));
            }

            itemCat->setData(Qt::UserRole, materialId);

            itemCat->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
            itemActl->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
            itemReason->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);

            ui->tw_inventoryDetails->setItem(row, 0, itemCat);
            ui->tw_inventoryDetails->setItem(row, 1, itemMaterial);
            ui->tw_inventoryDetails->setItem(row, 2, itemUnit);
            ui->tw_inventoryDetails->setItem(row, 3, itemExpt);
            ui->tw_inventoryDetails->setItem(row, 4, itemActl);
            ui->tw_inventoryDetails->setItem(row, 5, itemDiff);
            ui->tw_inventoryDetails->setItem(row, 6, itemPrice);
            ui->tw_inventoryDetails->setItem(row, 7, itemSum);
            ui->tw_inventoryDetails->setItem(row, 8, itemReason);

            row++;
        }
        ui->tw_inventoryDetails->setSortingEnabled(true);
    }

    m_isInventoryLoading = false;
}

void MainWidget::showInventory()
{
    ui->sw_inventarization->setCurrentIndex(1);
    ui->pb_inventoryDetailsBack->setVisible(false);
    ui->widget->setVisible(true);
    ui->lb_inventory_title->setVisible(true);
    ui->pb_inventory_report->setVisible(false);

    m_isInventoryLoading = true;

    ui->tw_inventoryDetails->setRowCount(0);
    ui->tw_inventoryDetails->setColumnCount(9);
    ui->tw_inventoryDetails->setHorizontalHeaderLabels({
        "Категория", "Материал", "Ед. изм.", "Учет (Система)", "Факт", "Разница", "Цена сред.", "Сумма разницы", "Причина"
    });

    ui->tw_inventoryDetails->setEditTriggers(QAbstractItemView::DoubleClicked |
                                             QAbstractItemView::SelectedClicked |
                                             QAbstractItemView::AnyKeyPressed);

    QHeaderView *header = ui->tw_inventoryDetails->horizontalHeader();
    header->setDefaultAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    header->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    header->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    header->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    header->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    header->setSectionResizeMode(4, QHeaderView::Stretch);
    header->setSectionResizeMode(5, QHeaderView::ResizeToContents);
    header->setSectionResizeMode(6, QHeaderView::ResizeToContents);
    header->setSectionResizeMode(7, QHeaderView::ResizeToContents);
    header->setSectionResizeMode(8, QHeaderView::Stretch);

    QSqlDatabase db = QSqlDatabase::database("db_dp_kalugina");
    if (!db.isValid() || !db.isOpen()) {
        m_isInventoryLoading = false;
        return;
    }

    QSqlQuery query(db);
    QString sql =
        "SELECT c.name, m.name, u.name, "
        "COALESCE(SUM(b.current_quantity), 0) as expected, m.id, "
        "COALESCE(ROUND(SUM(b.current_quantity * b.purchase_price) / SUM(b.current_quantity), 3), 0) as avg_price "
        "FROM materials m "
        "JOIN categories c ON m.category_id = c.id "
        "JOIN units u ON m.unit_id = u.id "
        "LEFT JOIN batches b ON m.id = b.material_id "
        "GROUP BY m.id, c.name, m.name, u.name "
        "HAVING expected > 0.0001";

    if (query.exec(sql)) {
        int row = 0;
        ui->tw_inventoryDetails->setSortingEnabled(false);

        while (query.next()) {
            ui->tw_inventoryDetails->insertRow(row);

            QString unit     = query.value(2).toString();
            double expected  = query.value(3).toDouble();
            int matId        = query.value(4).toInt();
            double avgPrice  = query.value(5).toDouble();

            bool isPieces = (unit.toLower() == "шт" || unit.toLower() == "шт.");

            QTableWidgetItem *itemCat     = new QTableWidgetItem(query.value(0).toString());
            QTableWidgetItem *itemMat     = new QTableWidgetItem(query.value(1).toString());
            QTableWidgetItem *itemUnit    = new QTableWidgetItem(unit);

            QString expStr = isPieces ? QString::number(expected, 'f', 0) : QString::number(expected, 'f', 3);
            QTableWidgetItem *itemExp     = new QTableWidgetItem(expStr);
            QTableWidgetItem *itemFact    = new QTableWidgetItem("");
            QTableWidgetItem *itemDiff    = new QTableWidgetItem("-");
            QTableWidgetItem *itemPrice   = new QTableWidgetItem(QString::number(avgPrice, 'f', 2));
            QTableWidgetItem *itemSum     = new QTableWidgetItem("-");
            QTableWidgetItem *itemReason  = new QTableWidgetItem("");

            itemCat->setData(Qt::UserRole, matId);
            itemPrice->setData(Qt::UserRole, avgPrice);

            Qt::ItemFlags readOnly = Qt::ItemIsEnabled | Qt::ItemIsSelectable;
            Qt::ItemFlags editable = Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable;

            itemCat->setFlags(readOnly);
            itemMat->setFlags(readOnly);
            itemUnit->setFlags(readOnly);
            itemExp->setFlags(readOnly);
            itemFact->setFlags(editable);
            itemDiff->setFlags(readOnly);
            itemPrice->setFlags(readOnly);
            itemSum->setFlags(readOnly);
            itemReason->setFlags(editable);

            itemFact->setBackground(QColor(255, 255, 225));

            QList<QTableWidgetItem*> numericItems = {itemExp, itemFact, itemDiff, itemPrice, itemSum};
            for(auto i : numericItems) i->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);

            ui->tw_inventoryDetails->setItem(row, 0, itemCat);
            ui->tw_inventoryDetails->setItem(row, 1, itemMat);
            ui->tw_inventoryDetails->setItem(row, 2, itemUnit);
            ui->tw_inventoryDetails->setItem(row, 3, itemExp);
            ui->tw_inventoryDetails->setItem(row, 4, itemFact);
            ui->tw_inventoryDetails->setItem(row, 5, itemDiff);
            ui->tw_inventoryDetails->setItem(row, 6, itemPrice);
            ui->tw_inventoryDetails->setItem(row, 7, itemSum);
            ui->tw_inventoryDetails->setItem(row, 8, itemReason);

            row++;
        }
        ui->tw_inventoryDetails->setSortingEnabled(true);
    } else {
        qCritical() << "Ошибка SQL в showInventory:" << query.lastError().text();
    }

    m_isInventoryLoading = false;
}

QDate MainWidget::getEarliestInventoryDate()
{
    QSqlDatabase db = QSqlDatabase::database("db_dp_kalugina");
    QSqlQuery query(db);
    query.exec("SELECT MIN(doc_date) FROM documents WHERE doc_type = 'inventory'");

    if (query.next() && !query.value(0).isNull()) {
        return query.value(0).toDateTime().date();
    }

    return QDate(2020, 1, 1);
}

void MainWidget::on_tw_inventoryDetails_itemChanged(QTableWidgetItem *item)
{
    if (m_isInventoryLoading || item->column() != 4) return;

    int row = item->row();
    QString factText = item->text().trimmed();

    QTableWidgetItem *itemDiff = ui->tw_inventoryDetails->item(row, 5);
    QTableWidgetItem *itemSum  = ui->tw_inventoryDetails->item(row, 7);
    QString unit = ui->tw_inventoryDetails->item(row, 2)->text();
    bool isPieces = (unit.toLower() == "шт" || unit.toLower() == "шт.");

    if (factText.isEmpty()) {
        m_isInventoryLoading = true;
        itemDiff->setText("-");
        itemSum->setText("-");
        itemDiff->setForeground(Qt::black);
        m_isInventoryLoading = false;
        return;
    }

    bool ok;
    double actual = factText.toDouble(&ok);
    if (!ok) {
        QMessageBox::warning(this, "Ошибка", "Введите корректное числовое значение!");
        item->setText("");
        return;
    }

    double expected = ui->tw_inventoryDetails->item(row, 3)->text().toDouble();
    double price = ui->tw_inventoryDetails->item(row, 6)->data(Qt::UserRole).toDouble();

    double diff = actual - expected;
    double diffSum = diff * price;

    m_isInventoryLoading = true;

    itemDiff->setText(isPieces ? QString::number(diff, 'f', 0) : QString::number(diff, 'f', 3));
    itemSum->setText(QString::number(diffSum, 'f', 2));

    if (diff < 0) itemDiff->setForeground(Qt::red);
    else if (diff > 0) itemDiff->setForeground(QColor(0, 150, 0));
    else itemDiff->setForeground(Qt::black);

    m_isInventoryLoading = false;
}

void MainWidget::inventory_cancel()
{
    QMessageBox msgBox(
        QMessageBox::Question,
        "Отмена инвентаризации",
        "Вы действительно хотите прервать проведение инвентаризации?\n"
        "Все введённые фактические остатки будут удалены без сохранения.",
        QMessageBox::Yes | QMessageBox::No,
        this
        );

    msgBox.setButtonText(QMessageBox::Yes, "Да, отменить");
    msgBox.setButtonText(QMessageBox::No, "Нет, продолжить");

    if (msgBox.exec() == QMessageBox::Yes) {
        closeInventoryWorkArea();
        qDebug() << "Инвентаризация отменена.";
    }
}

void MainWidget::closeInventoryWorkArea()
{
    currentInventoryId = 0;

    ui->sw_inventarization->setCurrentIndex(0);

    ui->pb_inventoryDetailsBack->setVisible(false);
    ui->widget->setVisible(false);

    ui->tw_inventoryDetails->setRowCount(0);

    loadInventoryHistoryTable();
}

void MainWidget::saveInventory()
{
    if (ui->tw_inventoryDetails->rowCount() == 0) return;

    QMessageBox::StandardButton res = QMessageBox::question(this, "Подтверждение",
                                                            "Завершить инвентаризацию и обновить остатки на складе?",
                                                            QMessageBox::Yes | QMessageBox::No);
    if (res != QMessageBox::Yes) return;

    QSqlDatabase db = QSqlDatabase::database("db_dp_kalugina");
    if (!db.transaction()) {
        qCritical() << "Не удалось начать транзакцию:" << db.lastError().text();
        return;
    }

    QSqlQuery query(db);

    query.prepare("INSERT INTO documents (doc_type, doc_date, user_id, description) "
                  "VALUES ('inventory', NOW(), :userId, 'Инвентаризация склада')");
    query.bindValue(":userId", userId);

    if (!query.exec()) {
        db.rollback();
        QMessageBox::critical(this, "Ошибка", "Не удалось создать документ!");
        return;
    }

    int docId = query.lastInsertId().toInt();

    for (int row = 0; row < ui->tw_inventoryDetails->rowCount(); ++row) {

        QTableWidgetItem *itemFact = ui->tw_inventoryDetails->item(row, 4);
        if (!itemFact || itemFact->text().isEmpty()) continue;

        int matId      = ui->tw_inventoryDetails->item(row, 0)->data(Qt::UserRole).toInt();
        double expected = ui->tw_inventoryDetails->item(row, 3)->text().toDouble();
        double actual   = itemFact->text().toDouble();
        double diff     = actual - expected;
        double price    = ui->tw_inventoryDetails->item(row, 6)->data(Qt::UserRole).toDouble();
        QString reason  = ui->tw_inventoryDetails->item(row, 8)->text();

        QSqlQuery transQuery(db);
        transQuery.prepare("INSERT INTO inventory_transactions "
                           "(document_id, material_id, quantity, price, expected_quantity, actual_quantity, reason) "
                           "VALUES (:docId, :matId, :qty, :price, :exp, :act, :reason)");
        transQuery.bindValue(":docId", docId);
        transQuery.bindValue(":matId", matId);
        transQuery.bindValue(":qty", diff);
        transQuery.bindValue(":price", price);
        transQuery.bindValue(":exp", expected);
        transQuery.bindValue(":act", actual);
        transQuery.bindValue(":reason", reason);

        if (!transQuery.exec()) {
            db.rollback();
            QMessageBox::critical(this, "Ошибка", "Ошибка записи транзакции по строке " + QString::number(row+1));
            return;
        }

        if (diff < 0) {
            double toSubtract = qAbs(diff);

            QSqlQuery batchQuery(db);
            batchQuery.prepare("SELECT id, current_quantity FROM batches "
                               "WHERE material_id = :matId AND current_quantity > 0 "
                               "ORDER BY incoming_date ASC");
            batchQuery.bindValue(":matId", matId);
            batchQuery.exec();

            while (batchQuery.next() && toSubtract > 0.0001) {
                int batchId = batchQuery.value(0).toInt();
                double batchQty = batchQuery.value(1).toDouble();

                double amountToTake = qMin(toSubtract, batchQty);

                QSqlQuery updateBatch(db);
                updateBatch.prepare("UPDATE batches SET current_quantity = current_quantity - :take "
                                    "WHERE id = :id");
                updateBatch.bindValue(":take", amountToTake);
                updateBatch.bindValue(":id", batchId);
                updateBatch.exec();

                toSubtract -= amountToTake;
            }
        }
        else if (diff > 0) {
            QSqlQuery addBatch(db);
            addBatch.prepare("INSERT INTO batches (material_id, supplier_id, incoming_date, "
                             "initial_quantity, current_quantity, purchase_price) "
                             "VALUES (:matId, NULL, NOW(), :qty, :qty, :price)");
            addBatch.bindValue(":matId", matId);
            addBatch.bindValue(":qty", diff);
            addBatch.bindValue(":price", price);
            addBatch.exec();
        }
    }

    if (db.commit()) {
        QMessageBox::information(this, "Успех", "Инвентаризация успешно проведена!");

        closeInventoryWorkArea();
    } else {
        db.rollback();
        QMessageBox::critical(this, "Ошибка", "Не удалось сохранить данные в БД.");
    }
}

void MainWidget::reportInventory()
{
    if (currentInventoryId <= 0) {
        QMessageBox::warning(this, "Внимание", "Сначала выберите материал в таблице склада!");
        return;
    }

    reportwidget *rw = new reportwidget(reportwidget::InventoryDoc, currentInventoryId, this);
    rw->setAttribute(Qt::WA_DeleteOnClose);
    rw->show();
}

void MainWidget::loadIncomingSuppliers()
{
    ui->cb_incoming_filter_counterparty->clear();
    ui->cb_incoming_filter_counterparty->addItem("Все поставщики", 0);
    QSqlDatabase db = QSqlDatabase::database("db_dp_kalugina");
    QSqlQuery query(db);

    if (query.exec("SELECT id, name FROM suppliers ORDER BY name")) {
        while (query.next()) {
            ui->cb_incoming_filter_counterparty->addItem(query.value(1).toString(), query.value(0).toInt());
        }
    }
}

void MainWidget::updateDocPeriod(DocMode mode, int index)
{
    QDateEdit *deStart = (mode == Incoming) ? ui->de_incoming_start : ui->de_outgoing_start;
    QDateEdit *deEnd   = (mode == Incoming) ? ui->de_incoming_end   : ui->de_outgoing_end;
    QString docType    = (mode == Incoming) ? "incoming"            : "outgoing";

    QDate today = QDate::currentDate();
    QDate start;
    QDate end = today;

    deStart->blockSignals(true);
    deEnd->blockSignals(true);

    deStart->setMaximumDate(QDate(9999, 1, 1));
    deEnd->setMinimumDate(QDate(1900, 1, 1));

    switch (index) {
    case 0:
        start = getEarliestDocDate(docType);
        break;
    case 1:
        start = today.addDays(-(today.dayOfWeek() - 1));
        break;
    case 2:
        start = QDate(today.year(), today.month(), 1);
        break;
    case 3:
        start = QDate(today.year(), 1, 1);
        break;
    case 4:
        deStart->blockSignals(false);
        deEnd->blockSignals(false);
        return;
    }

    deEnd->setDate(end);
    deStart->setDate(start);

    deEnd->setMinimumDate(start);
    deStart->setMaximumDate(end);

    deStart->blockSignals(false);
    deEnd->blockSignals(false);

    filterDocs(mode);
}

QDate MainWidget::getEarliestDocDate(QString type)
{
    QSqlDatabase db = QSqlDatabase::database("db_dp_kalugina");
    QSqlQuery query(db);

    query.prepare("SELECT MIN(doc_date) FROM documents WHERE doc_type = :type");
    query.bindValue(":type", type);

    if (query.exec() && query.next() && !query.value(0).isNull()) {
        return query.value(0).toDateTime().date();
    }

    return QDate::currentDate().addYears(-1);
}

void MainWidget::filterDocs(DocMode mode)
{
    DocUI d = getDocUI(mode);

    QString searchText = d.searchLe->text().trimmed();
    QDate startDate = d.deStart->date();
    QDate endDate = d.deEnd->date();

    QString selectedCpText = "";
    int selectedCpId = 0;
    if (d.counterpartyFilterCb) {
        selectedCpId = d.counterpartyFilterCb->currentData().toInt();
        selectedCpText = d.counterpartyFilterCb->currentText();
    }

    for (int row = 0; row < d.historyTable->rowCount(); ++row) {
        QTableWidgetItem *itemNo    = d.historyTable->item(row, 0);
        QTableWidgetItem *itemDate  = d.historyTable->item(row, 1);
        QTableWidgetItem *itemCp    = d.historyTable->item(row, 2);
        QTableWidgetItem *itemStaff = d.historyTable->item(row, 4);

        QString idText    = itemNo ? itemNo->text() : "";
        QString dateText  = itemDate ? itemDate->text() : "";
        QString cpText    = itemCp ? itemCp->text() : "";
        QString staffText = itemStaff ? itemStaff->text() : "";

        bool textMatch = idText.contains(searchText, Qt::CaseInsensitive) ||
                         staffText.contains(searchText, Qt::CaseInsensitive);

        QDate rowDate = QDate::fromString(dateText.left(10), "dd.MM.yyyy");
        bool dateMatch = (rowDate >= startDate && rowDate <= endDate);

        bool cpMatch = true;
        if (d.counterpartyFilterCb) {
            cpMatch = (selectedCpId == 0) || (cpText == selectedCpText);
        }

        d.historyTable->setRowHidden(row, !(textMatch && dateMatch && cpMatch));
    }
}

void MainWidget::loadHistoryTable(DocMode mode)
{
    QTableWidget *table;
    int *currentId;
    QString typeStr;
    QColor sumColor;

    if (mode == Incoming) {
        table = ui->tw_incoming_history;
        currentId = &currentIncomingId;
        typeStr = "incoming";
        sumColor = QColor(0, 100, 0);
    } else {
        table = ui->tw_outgoing_history;
        currentId = &currentOutgoingId;
        typeStr = "outgoing";
        sumColor = QColor(150, 0, 0);
    }

    QHeaderView *header = table->horizontalHeader();
    header->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    header->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    header->setSectionResizeMode(2, QHeaderView::Stretch);
    header->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    header->setSectionResizeMode(4, QHeaderView::ResizeToContents);
    header->setSectionResizeMode(5, QHeaderView::Stretch);

    QSqlDatabase db = QSqlDatabase::database("db_dp_kalugina");
    if (!db.isValid() || !db.isOpen()) return;

    QSqlQuery query(db);

    QString sql =
        "SELECT "
        "  d.id, "
        "  d.doc_date, "
        "  MAX(s.name) AS counterparty_name, "
        "  SUM(it.quantity * it.price) AS total_sum, "
        "  u.login, "
        "  d.description "
        "FROM documents d "
        "JOIN users u ON d.user_id = u.id "
        "LEFT JOIN inventory_transactions it ON d.id = it.document_id "
        "LEFT JOIN batches b ON it.batch_id = b.id "
        "LEFT JOIN suppliers s ON b.supplier_id = s.id "
        "WHERE d.doc_type = :type "
        "GROUP BY d.id, d.doc_date, u.login, d.description "
        "ORDER BY d.doc_date DESC";

    query.prepare(sql);
    query.bindValue(":type", typeStr);

    if (query.exec()) {
        table->setRowCount(0);
        table->setSortingEnabled(false);

        int row = 0;
        while (query.next()) {
            table->insertRow(row);

            int docId = query.value(0).toInt();
            QDateTime date = query.value(1).toDateTime();
            QString cpName = query.value(2).toString();
            double totalSum = query.value(3).toDouble();
            QString employee = query.value(4).toString();
            QString desc = query.value(5).toString();

            QTableWidgetItem *itemNo = new QTableWidgetItem(QString::number(docId));
            itemNo->setData(Qt::UserRole, docId);

            QTableWidgetItem *itemDate = new QTableWidgetItem(date.toString("dd.MM.yyyy HH:mm"));
            QTableWidgetItem *itemCp   = new QTableWidgetItem(cpName.isEmpty() ? "-" : cpName);

            QTableWidgetItem *itemSum  = new QTableWidgetItem(QString::number(qAbs(totalSum), 'f', 2));
            itemSum->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
            itemSum->setForeground(sumColor);

            QTableWidgetItem *itemUser = new QTableWidgetItem(employee);
            QTableWidgetItem *itemDesc = new QTableWidgetItem(desc);

            table->setItem(row, 0, itemNo);
            table->setItem(row, 1, itemDate);
            table->setItem(row, 2, itemCp);
            table->setItem(row, 3, itemSum);
            table->setItem(row, 4, itemUser);
            table->setItem(row, 5, itemDesc);

            row++;
        }
        table->setSortingEnabled(true);

        if (*currentId != 0) {
            for (int r = 0; r < table->rowCount(); ++r) {
                if (table->item(r, 0)->data(Qt::UserRole).toInt() == *currentId) {
                    table->selectRow(r);
                    break;
                }
            }
        }
    } else {
        qCritical() << "Ошибка загрузки истории:" << query.lastError().text();
    }
}

void MainWidget::toggleDocLayout(DocMode mode)
{
    QSplitter *splitter = (mode == Incoming) ? ui->splitter_2 : ui->splitter_3;
    QPushLabel *label   = (mode == Incoming) ? ui->lb_change_layout_incoming : ui->lb_change_layout_outgoing;

    if (splitter->orientation() == Qt::Horizontal) {
        splitter->setOrientation(Qt::Vertical);
        label->setPixmap(QPixmap(":/res/edithlayoutsplit.png"));
    } else {
        splitter->setOrientation(Qt::Horizontal);
        label->setPixmap(QPixmap(":/res/editvlayoutsplit.png"));
    }
}

void MainWidget::handleDocClick(DocMode mode, int row)
{
    QTableWidget *historyTable = (mode == Incoming) ? ui->tw_incoming_history : ui->tw_outgoing_history;
    QSplitter *splitter        = (mode == Incoming) ? ui->splitter_2 : ui->splitter_3;
    int *currentId             = (mode == Incoming) ? &currentIncomingId : &currentOutgoingId;

    QTableWidgetItem *item = historyTable->item(row, 0);
    if (item) {
        *currentId = item->data(Qt::UserRole).toInt();

        if (splitter->sizes().at(1) == 0) {
            splitter->setSizes({1, 1});
        }

        qDebug() << (mode == Incoming ? "Приход" : "Расход") << " ID:" << *currentId;

        loadDocDetails(mode);
    }
}

void MainWidget::loadDocDetails(DocMode mode)
{
    QTableWidget *detailsTable = (mode == Incoming) ? ui->tw_incoming_details : ui->tw_outgoing_details;
    int docId                  = (mode == Incoming) ? currentIncomingId : currentOutgoingId;
    QColor sumColor            = (mode == Incoming) ? QColor(0, 120, 0) : QColor(180, 0, 0);

    if (docId <= 0) return;

    detailsTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    QSqlDatabase db = QSqlDatabase::database("db_dp_kalugina");
    if (!db.isValid() || !db.isOpen()) return;

    QSqlQuery query(db);

    QString sql =
        "SELECT "
        "  c.name AS category_name, m.name AS material_name, it.quantity, "
        "  u.name AS unit_name, it.price, (it.quantity * it.price) AS row_sum, "
        "  it.batch_id, m.id "
        "FROM inventory_transactions it "
        "JOIN materials m ON it.material_id = m.id "
        "JOIN categories c ON m.category_id = c.id "
        "JOIN units u ON m.unit_id = u.id "
        "WHERE it.document_id = :docId";

    query.prepare(sql);
    query.bindValue(":docId", docId);

    if (query.exec()) {
        detailsTable->setRowCount(0);
        detailsTable->setSortingEnabled(false);
        int row = 0;

        while (query.next()) {
            detailsTable->insertRow(row);

            QString category = query.value(0).toString();
            QString material = query.value(1).toString();
            double quantity  = qAbs(query.value(2).toDouble());
            QString unit     = query.value(3).toString();
            double price     = query.value(4).toDouble();
            double sum       = qAbs(query.value(5).toDouble());
            int batchId      = query.value(6).toInt();
            int materialId   = query.value(7).toInt();

            QTableWidgetItem *itemCat   = new QTableWidgetItem(category);
            QTableWidgetItem *itemMat   = new QTableWidgetItem(material);
            QTableWidgetItem *itemUnit  = new QTableWidgetItem(unit);
            QTableWidgetItem *itemQty   = new QTableWidgetItem(QString::number(quantity, 'f', 3));
            QTableWidgetItem *itemPrice = new QTableWidgetItem(QString::number(price, 'f', 2));
            QTableWidgetItem *itemSum   = new QTableWidgetItem(QString::number(sum, 'f', 2));
            QTableWidgetItem *itemBatch = new QTableWidgetItem(QString::number(batchId));

            itemCat->setData(Qt::UserRole, materialId);

            QList<QTableWidgetItem*> numericItems = {itemQty, itemPrice, itemSum, itemBatch};
            for(auto item : numericItems) {
                item->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
            }

            itemSum->setForeground(sumColor);
            itemSum->setFont(QFont("Segoe UI", -1, QFont::Bold));

            detailsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);

            detailsTable->setItem(row, 0, itemCat);
            detailsTable->setItem(row, 1, itemMat);
            detailsTable->setItem(row, 2, itemQty);
            detailsTable->setItem(row, 3, itemUnit);
            detailsTable->setItem(row, 4, itemPrice);
            detailsTable->setItem(row, 5, itemSum);
            detailsTable->setItem(row, 6, itemBatch);

            row++;
        }
        detailsTable->setSortingEnabled(true);
    }
}

void MainWidget::prepareDocEditor(DocMode mode)
{
    QStackedWidget *stack;
    QLabel *totalLb;
    QDateEdit *dateDe;
    QTableWidget *table;
    QLineEdit *descLe;

    if (mode == Incoming) {
        stack   = ui->sw_incoming;
        totalLb = ui->lb_new_incoming_total_value;
        dateDe  = ui->de_new_incoming_date;
        table   = ui->tw_incoming_editor;
        descLe  = ui->le_new_incoming_description;

        loadAddIncomingSuppliers();
    } else {
        stack   = ui->sw_outgoing;
        totalLb = ui->lb_new_outgoing_total_value;
        dateDe  = ui->de_new_outgoing_date;
        table   = ui->tw_outgoing_editor;
        descLe  = ui->le_new_outgoing_description;
    }

    stack->setCurrentIndex(1);
    totalLb->setText("0.00");
    totalLb->setStyleSheet("color: black; font-size: 13pt;");
    dateDe->setDate(QDate::currentDate());
    descLe->clear();
    table->setRowCount(0);

    table->setColumnCount(6);
    table->setHorizontalHeaderLabels({
        "Материал", "Категория", "Кол-во", "Ед. изм.", "Цена", "Сумма"
    });

    QHeaderView *header = table->horizontalHeader();
    header->setSectionResizeMode(0, QHeaderView::Stretch);
    header->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    header->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    header->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    header->setSectionResizeMode(4, QHeaderView::ResizeToContents);
    header->setSectionResizeMode(5, QHeaderView::ResizeToContents);

    loadAddDocMaterials(mode);

    QSqlDatabase db = QSqlDatabase::database("db_dp_kalugina");
    if (db.isOpen()) {
        m_materialsCache.clear();
        QSqlQuery q(db);
        QString sql = "SELECT m.id, c.name, u.name FROM materials m "
                      "JOIN categories c ON m.category_id = c.id "
                      "JOIN units u ON m.unit_id = u.id";
        if (q.exec(sql)) {
            while(q.next()) {
                MaterialEntry entry;
                entry.id = q.value(0).toInt();
                entry.category = q.value(1).toString();
                entry.unit = q.value(2).toString();
                m_materialsCache.insert(entry.id, entry);
            }
        }
    }
    clearDocInputBar(mode);
}

void MainWidget::cancelDocOperation(DocMode mode)
{
    QString title = (mode == Incoming) ? "Отмена поступления" : "Отмена списания";
    QString text = (mode == Incoming)
                       ? "Вы действительно хотите прервать проведение поступления?\nВсе позиции будут удалены без сохранения."
                       : "Вы действительно хотите прервать оформление списания?\nВсе позиции будут удалены без сохранения.";

    QMessageBox msgBox(
        QMessageBox::Question,
        title,
        text,
        QMessageBox::Yes | QMessageBox::No,
        this
        );

    msgBox.setButtonText(QMessageBox::Yes, "Да, отменить");
    msgBox.setButtonText(QMessageBox::No, "Нет, продолжить");

    if (msgBox.exec() == QMessageBox::Yes) {
        closeDocWorkArea(mode);
        qDebug() << title << "подтверждена пользователем.";
    }
}

void MainWidget::closeDocWorkArea(DocMode mode)
{
    if (mode == Incoming) {
        currentIncomingId = 0;
        ui->sw_incoming->setCurrentIndex(0);
        ui->tw_incoming_details->setRowCount(0);
        ui->splitter_2->setSizes({1, 0});
        m_isIncomingDetailsOpened = false;
    } else {
        currentOutgoingId = 0;
        ui->sw_outgoing->setCurrentIndex(0);
        ui->tw_outgoing_details->setRowCount(0);
        ui->splitter_3->setSizes({1, 0});
        m_isOutgoingDetailsOpened = false;
    }

    loadHistoryTable(mode);
}

void MainWidget::loadAddIncomingSuppliers()
{
    ui->cb_new_incoming_supplier->clear();

    ui->cb_new_incoming_supplier->addItem("Выберите поставщика", 0);

    QSqlDatabase db = QSqlDatabase::database("db_dp_kalugina");
    if (!db.isValid() || !db.isOpen()) return;

    QSqlQuery query(db);
    if (query.exec("SELECT id, name FROM suppliers ORDER BY name")) {
        while (query.next()) {
            ui->cb_new_incoming_supplier->addItem(query.value(1).toString(), query.value(0).toInt());
        }
    }
    ui->cb_new_incoming_supplier->setCurrentIndex(0);
}

void MainWidget::loadAddDocMaterials(DocMode mode)
{
    QComboBox *cb = (mode == Incoming) ? ui->cb_inc_add_material : ui->cb_out_add_material;

    cb->clear();
    cb->addItem("Выберите материал", 0);

    QSqlDatabase db = QSqlDatabase::database("db_dp_kalugina");
    if (!db.isValid() || !db.isOpen()) return;

    QSqlQuery query(db);
    if (query.exec("SELECT id, name FROM materials ORDER BY name")) {
        while (query.next()) {
            cb->addItem(query.value(1).toString(), query.value(0).toInt());
        }
    }
    cb->setCurrentIndex(0);
}

void MainWidget::handleMaterialSelectionChanged(DocMode mode)
{
    QComboBox *cb       = (mode == Incoming) ? ui->cb_inc_add_material : ui->cb_out_add_material;
    QLineEdit *leCat    = (mode == Incoming) ? ui->le_inc_add_category : ui->le_out_add_category;
    QLineEdit *leUnit   = (mode == Incoming) ? ui->le_inc_add_unit     : ui->le_out_add_unit;
    QDoubleSpinBox *sbQty = (mode == Incoming) ? ui->dsb_inc_add_qty   : ui->dsb_out_add_qty;
    QDoubleSpinBox *sbPrice = (mode == Incoming) ? ui->dsb_inc_add_price : ui->dsb_out_add_price;
    QLineEdit *leSum    = (mode == Incoming) ? ui->le_inc_add_sum      : ui->le_out_add_sum;

    int materialId = cb->currentData().toInt();

    if (materialId <= 0) {
        leCat->clear(); leUnit->clear();
        sbQty->setValue(0); sbPrice->setValue(0);
        leSum->setText("0.00");
        if (mode == Outgoing) ui->lb_out_available_qty->clear();
        return;
    }

    if (m_materialsCache.contains(materialId)) {
        MaterialEntry entry = m_materialsCache.value(materialId);
        leCat->setText(entry.category);
        leUnit->setText(entry.unit);
        sbQty->setValue(1.000);

        if (mode == Outgoing) {
            double stock = getAvailableStock(materialId);

            double price = getFifoPrice(materialId);

            ui->lb_out_available_qty->setText(QString("На складе: %1 %2")
                                                  .arg(QString::number(stock, 'f', 3))
                                                  .arg(entry.unit));

            if (stock <= 0) ui->lb_out_available_qty->setStyleSheet("color: red; font-weight: bold;");
            else ui->lb_out_available_qty->setStyleSheet("color: blue;");

            sbPrice->setValue(price);
        } else {
            sbPrice->setValue(0.00);
        }

        calculateCurrentRowSum(mode);
    }
}

void MainWidget::calculateCurrentRowSum(DocMode mode)
{
    QDoubleSpinBox *sbQty   = (mode == Incoming) ? ui->dsb_inc_add_qty   : ui->dsb_out_add_qty;
    QDoubleSpinBox *sbPrice = (mode == Incoming) ? ui->dsb_inc_add_price : ui->dsb_out_add_price;
    QLineEdit *leSum        = (mode == Incoming) ? ui->le_inc_add_sum     : ui->le_out_add_sum;

    double total = sbQty->value() * sbPrice->value();
    leSum->setText(QString::number(total, 'f', 2));
}

void MainWidget::addDocRow(DocMode mode)
{
    QComboBox *cb       = (mode == Incoming) ? ui->cb_inc_add_material : ui->cb_out_add_material;
    QLineEdit *leCat    = (mode == Incoming) ? ui->le_inc_add_category : ui->le_out_add_category;
    QLineEdit *leUnit   = (mode == Incoming) ? ui->le_inc_add_unit     : ui->le_out_add_unit;
    QDoubleSpinBox *sbQty = (mode == Incoming) ? ui->dsb_inc_add_qty   : ui->dsb_out_add_qty;
    QDoubleSpinBox *sbPrice = (mode == Incoming) ? ui->dsb_inc_add_price : ui->dsb_out_add_price;
    QTableWidget *table = (mode == Incoming) ? ui->tw_incoming_editor : ui->tw_outgoing_editor;

    int matId = cb->currentData().toInt();
    QString matName = cb->currentText();
    QString category = leCat->text();
    QString unit = leUnit->text();
    double qty = sbQty->value();
    double price = sbPrice->value();
    double sum = qty * price;

    if (matId <= 0) {
        QMessageBox::warning(this, "Внимание", "Выберите материал из списка!");
        cb->setFocus();
        return;
    }
    if (qty <= 0.0001) {
        QMessageBox::warning(this, "Внимание", "Количество должно быть больше нуля!");
        sbQty->setFocus();
        return;
    }
    if (price <= 0.001) {
        QString msg = (mode == Incoming) ? "Укажите цену закупки!" : "Укажите цену списания!";
        QMessageBox::warning(this, "Внимание", msg);
        sbPrice->setFocus();
        return;
    }

    if (mode == Outgoing) {
        double available = getAvailableStock(matId);
        if (qty > available + 0.0001) {
            QMessageBox::warning(this, "Ошибка остатков",
                                 QString("Недостаточно товара на складе!\n"
                                         "Вы пытаетесь списать: %1\n"
                                         "Доступно: %2").arg(qty).arg(available));
            return;
        }
    }

    int row = 0;
    table->insertRow(row);

    QTableWidgetItem *itemMat  = new QTableWidgetItem(matName);
    QTableWidgetItem *itemCat  = new QTableWidgetItem(category);
    QTableWidgetItem *itemQty  = new QTableWidgetItem(QString::number(qty, 'f', 3));
    QTableWidgetItem *itemUnit = new QTableWidgetItem(unit);
    QTableWidgetItem *itemPrice = new QTableWidgetItem(QString::number(price, 'f', 2));
    QTableWidgetItem *itemSum  = new QTableWidgetItem(QString::number(sum, 'f', 2));

    itemMat->setData(Qt::UserRole, matId);

    QList<QTableWidgetItem*> allItems = {itemMat, itemCat, itemQty, itemUnit, itemPrice, itemSum};
    for (auto item : allItems) {
        item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
    }
    itemQty->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
    itemPrice->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
    itemSum->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);

    table->setItem(row, 0, itemMat);
    table->setItem(row, 1, itemCat);
    table->setItem(row, 2, itemQty);
    table->setItem(row, 3, itemUnit);
    table->setItem(row, 4, itemPrice);
    table->setItem(row, 5, itemSum);

    clearDocInputBar(mode);
    updateDocTotalSum(mode);

    qDebug() << (mode == Incoming ? "Приход:" : "Расход:") << matName << "добавлен в список.";
}

void MainWidget::clearDocInputBar(DocMode mode)
{
    QComboBox *cb       = (mode == Incoming) ? ui->cb_inc_add_material : ui->cb_out_add_material;
    QLineEdit *leCat    = (mode == Incoming) ? ui->le_inc_add_category : ui->le_out_add_category;
    QLineEdit *leUnit   = (mode == Incoming) ? ui->le_inc_add_unit     : ui->le_out_add_unit;
    QDoubleSpinBox *sbQty = (mode == Incoming) ? ui->dsb_inc_add_qty   : ui->dsb_out_add_qty;
    QDoubleSpinBox *sbPrice = (mode == Incoming) ? ui->dsb_inc_add_price : ui->dsb_out_add_price;
    QLineEdit *leSum    = (mode == Incoming) ? ui->le_inc_add_sum      : ui->le_out_add_sum;

    cb->setCurrentIndex(0);
    leCat->clear();
    leUnit->clear();
    sbQty->setValue(0);
    sbPrice->setValue(0);
    leSum->setText("0.00");
    cb->setFocus();
}

void MainWidget::deleteDocRow(DocMode mode, int row)
{
    QTableWidget *table = (mode == Incoming) ? ui->tw_incoming_editor : ui->tw_outgoing_editor;

    if (row < 0 || row >= table->rowCount()) return;

    QString matName = table->item(row, 0)->text();

    QMessageBox msgBox(
        QMessageBox::Question,
        "Удаление позиции",
        QString("Вы действительно хотите удалить '%1' из списка?").arg(matName),
        QMessageBox::Yes | QMessageBox::No,
        this
        );
    msgBox.setButtonText(QMessageBox::Yes, "Удалить");
    msgBox.setButtonText(QMessageBox::No, "Отмена");

    if (msgBox.exec() == QMessageBox::Yes) {
        table->removeRow(row);

        updateDocTotalSum(mode);

        qDebug() << (mode == Incoming ? "Приход:" : "Расход:") << "строка" << row << "удалена.";
    }
}

void MainWidget::updateDocTotalSum(DocMode mode)
{
    QTableWidget *table = (mode == Incoming) ? ui->tw_incoming_editor : ui->tw_outgoing_editor;
    QLabel *totalLb     = (mode == Incoming) ? ui->lb_new_incoming_total_value : ui->lb_new_outgoing_total_value;

    double total = 0.0;
    for (int i = 0; i < table->rowCount(); ++i) {
        total += table->item(i, 5)->text().toDouble();
    }

    totalLb->setText(QString::number(total, 'f', 2));
}

void MainWidget::saveIncoming()
{
    int supplierId = ui->cb_new_incoming_supplier->currentData().toInt();
    if (supplierId <= 0) {
        QMessageBox::warning(this, "Внимание", "Пожалуйста, выберите поставщика!");
        return;
    }

    int rows = ui->tw_incoming_editor->rowCount();
    if (rows == 0) {
        QMessageBox::warning(this, "Внимание", "Таблица товаров пуста!");
        return;
    }

    QMessageBox::StandardButton res = QMessageBox::question(this, "Подтверждение",
                                                            "Провести приходную накладную и обновить остатки?",
                                                            QMessageBox::Yes | QMessageBox::No);
    if (res != QMessageBox::Yes) return;

    QSqlDatabase db = QSqlDatabase::database("db_dp_kalugina");
    if (!db.transaction()) {
        qCritical() << "Не удалось начать транзакцию";
        return;
    }

    QSqlQuery query(db);

    QDateTime finalDateTime(ui->de_new_incoming_date->date(), QTime::currentTime());

    query.prepare("INSERT INTO documents (doc_type, doc_date, user_id, description) "
                  "VALUES ('incoming', :date, :userId, :desc)");
    query.bindValue(":date", finalDateTime);
    query.bindValue(":userId", userId);
    query.bindValue(":desc", ui->le_new_incoming_description->text().trimmed());

    if (!query.exec()) {
        db.rollback();
        QMessageBox::critical(this, "Ошибка", "Не удалось создать заголовок документа.");
        return;
    }

    int docId = query.lastInsertId().toInt();

    for (int row = 0; row < rows; ++row) {
        int matId = ui->tw_incoming_editor->item(row, 0)->data(Qt::UserRole).toInt();
        double qty = ui->tw_incoming_editor->item(row, 2)->text().toDouble();
        double price = ui->tw_incoming_editor->item(row, 4)->text().toDouble();

        QSqlQuery qBatch(db);
        qBatch.prepare("INSERT INTO batches (material_id, supplier_id, incoming_date, "
                       "initial_quantity, current_quantity, purchase_price) "
                       "VALUES (:matId, :supId, :date, :iniQty, :curQty, :price)");
        qBatch.bindValue(":matId", matId);
        qBatch.bindValue(":supId", supplierId);
        qBatch.bindValue(":date", finalDateTime);
        qBatch.bindValue(":iniQty", qty);
        qBatch.bindValue(":curQty", qty);
        qBatch.bindValue(":price", price);

        if (!qBatch.exec()) {
            db.rollback();
            QMessageBox::critical(this, "Ошибка", "Ошибка при создании партии в строке " + QString::number(row+1));
            return;
        }

        int batchId = qBatch.lastInsertId().toInt();

        QSqlQuery qTrans(db);
        qTrans.prepare("INSERT INTO inventory_transactions (document_id, material_id, batch_id, quantity, price) "
                       "VALUES (:docId, :matId, :batchId, :qty, :price)");
        qTrans.bindValue(":docId", docId);
        qTrans.bindValue(":matId", matId);
        qTrans.bindValue(":batchId", batchId);
        qTrans.bindValue(":qty", qty);
        qTrans.bindValue(":price", price);

        if (!qTrans.exec()) {
            db.rollback();
            QMessageBox::critical(this, "Ошибка", "Ошибка при записи транзакции в строке " + QString::number(row+1));
            return;
        }
    }

    if (db.commit()) {
        QMessageBox::information(this, "Успех", "Поступление успешно проведено.");

        closeDocWorkArea(Incoming);

        loadWarehouseTable();
    } else {
        db.rollback();
        QMessageBox::critical(this, "Ошибка", "Критическая ошибка фиксации данных в БД.");
    }
}

DocUI MainWidget::getDocUI(DocMode mode) {
    if (mode == Incoming) {
        return {
            ui->tw_incoming_history,
            ui->tw_incoming_details,
            ui->le_incoming_search,
            ui->cb_incoming_period,
            ui->cb_incoming_filter_counterparty,
            ui->de_incoming_start,
            ui->de_incoming_end,
            ui->sw_incoming
        };
    } else {
        return {
            ui->tw_outgoing_history,
            ui->tw_outgoing_details,
            ui->le_outgoing_search,
            ui->cb_outgoing_period,
            nullptr,
            ui->de_outgoing_start,
            ui->de_outgoing_end,
            ui->sw_outgoing
        };
    }
}

void MainWidget::saveOutgoing()
{
    int rows = ui->tw_outgoing_editor->rowCount();
    if (rows == 0) {
        QMessageBox::warning(this, "Внимание", "Таблица списания пуста!");
        return;
    }

    QMessageBox::StandardButton res = QMessageBox::question(this, "Подтверждение",
                                                            "Провести списание материалов со склада?",
                                                            QMessageBox::Yes | QMessageBox::No);
    if (res != QMessageBox::Yes) return;

    QSqlDatabase db = QSqlDatabase::database("db_dp_kalugina");
    if (!db.transaction()) return;

    QSqlQuery query(db);
    QDateTime finalDateTime(ui->de_new_outgoing_date->date(), QTime::currentTime());

    query.prepare("INSERT INTO documents (doc_type, doc_date, user_id, description) "
                  "VALUES ('outgoing', :date, :userId, :desc)");
    query.bindValue(":date", finalDateTime);
    query.bindValue(":userId", userId);
    query.bindValue(":desc", ui->le_new_outgoing_description->text().trimmed());

    if (!query.exec()) {
        db.rollback();
        return;
    }

    int docId = query.lastInsertId().toInt();

    for (int row = 0; row < rows; ++row) {
        int matId = ui->tw_outgoing_editor->item(row, 0)->data(Qt::UserRole).toInt();
        double qtyToSubtract = ui->tw_outgoing_editor->item(row, 2)->text().toDouble();

        QSqlQuery batchQuery(db);
        batchQuery.prepare("SELECT id, current_quantity, purchase_price FROM batches "
                           "WHERE material_id = :matId AND current_quantity > 0 "
                           "ORDER BY incoming_date ASC");
        batchQuery.bindValue(":matId", matId);
        batchQuery.exec();

        double remainingInRequest = qtyToSubtract;

        while (batchQuery.next() && remainingInRequest > 0.0001) {
            int batchId = batchQuery.value(0).toInt();
            double batchQty = batchQuery.value(1).toDouble();
            double buyPrice = batchQuery.value(2).toDouble();

            double canTake = qMin(remainingInRequest, batchQty);

            QSqlQuery updateBatch(db);
            updateBatch.prepare("UPDATE batches SET current_quantity = current_quantity - :take "
                                "WHERE id = :id");
            updateBatch.bindValue(":take", canTake);
            updateBatch.bindValue(":id", batchId);
            updateBatch.exec();

            QSqlQuery qTrans(db);
            qTrans.prepare("INSERT INTO inventory_transactions (document_id, material_id, batch_id, quantity, price) "
                           "VALUES (:docId, :matId, :batchId, :qty, :price)");
            qTrans.bindValue(":docId", docId);
            qTrans.bindValue(":matId", matId);
            qTrans.bindValue(":batchId", batchId);

            qTrans.bindValue(":qty", -canTake);

            qTrans.bindValue(":price", buyPrice);
            qTrans.exec();

            remainingInRequest -= canTake;
        }

        if (remainingInRequest > 0.0001) {
            db.rollback();
            QMessageBox::warning(this, "Ошибка остатков",
                                 QString("Недостаточно товара '%1' на складе!").arg(ui->tw_outgoing_editor->item(row, 0)->text()));
            return;
        }
    }

    if (db.commit()) {
        QMessageBox::information(this, "Успех", "Списание успешно проведено.");
        closeDocWorkArea(Outgoing);
        loadWarehouseTable();
    } else {
        db.rollback();
    }
}

void MainWidget::reportIncoming()
{
    if (currentIncomingId <= 0) {
        QMessageBox::warning(this, "Отчет", "Сначала выберите документ в таблице!");
        return;
    }

    reportwidget *rw = new reportwidget(reportwidget::IncomingDoc, currentIncomingId, this);
    rw->setAttribute(Qt::WA_DeleteOnClose);
    rw->exec();
}

void MainWidget::reportOutgoing()
{
    if (currentOutgoingId <= 0) {
        QMessageBox::warning(this, "Отчет", "Сначала выберите документ в таблице!");
        return;
    }

    reportwidget *rw = new reportwidget(reportwidget::OutgoingDoc, currentOutgoingId, this);
    rw->setAttribute(Qt::WA_DeleteOnClose);
    rw->exec();
}

double MainWidget::getFifoPrice(int materialId)
{
    QSqlDatabase db = QSqlDatabase::database("db_dp_kalugina");
    QSqlQuery query(db);

    query.prepare("SELECT purchase_price FROM batches "
                  "WHERE material_id = :matId AND current_quantity > 0 "
                  "ORDER BY incoming_date ASC LIMIT 1");
    query.bindValue(":matId", materialId);

    if (query.exec() && query.next()) {
        return query.value(0).toDouble();
    }
    return 0.0;
}

double MainWidget::getAvailableStock(int materialId)
{
    QSqlDatabase db = QSqlDatabase::database("db_dp_kalugina");
    QSqlQuery query(db);

    query.prepare("SELECT SUM(current_quantity) FROM batches WHERE material_id = :id");
    query.bindValue(":id", materialId);

    if (query.exec() && query.next()) {
        return query.value(0).toDouble();
    }
    return 0.0;
}

void MainWidget::fillReportTypes()
{
    ui->list_report_types->clear();

    QMap<QString, QString> reports;
    reports.insert("rep_stock", "📦 Ведомость текущих остатков");
    reports.insert("rep_osv", "📊 Оборотно-сальдовая ведомость");
    reports.insert("rep_purchases", "🚚 Закупки по поставщикам");
    reports.insert("rep_outgoing", "📉 Отчет по списаниям (расход)");
    reports.insert("rep_inventory", "📋 Реестр инвентаризаций");

    for (auto it = reports.begin(); it != reports.end(); ++it) {
        QListWidgetItem *item = new QListWidgetItem(it.value());
        item->setData(Qt::UserRole, it.key());
        ui->list_report_types->addItem(item);
    }
}

void MainWidget::fillReportFilters()
{
    QSqlDatabase db = QSqlDatabase::database("db_dp_kalugina");
    QSqlQuery query(db);

    ui->cb_filter_category->clear();
    ui->cb_filter_category->addItem("Все категории", 0);
    if (query.exec("SELECT id, name FROM categories ORDER BY name")) {
        while (query.next()) {
            ui->cb_filter_category->addItem(query.value(1).toString(), query.value(0).toInt());
        }
    }

    ui->cb_filter_supplier->clear();
    ui->cb_filter_supplier->addItem("Все поставщики", 0);
    if (query.exec("SELECT id, name FROM suppliers ORDER BY name")) {
        while (query.next()) {
            ui->cb_filter_supplier->addItem(query.value(1).toString(), query.value(0).toInt());
        }
    }
}

void MainWidget::setupReportDates()
{
    QSqlDatabase db = QSqlDatabase::database("db_dp_kalugina");
    QSqlQuery query(db);

    QDate minDate = QDate::currentDate().addYears(-1);
    QDate maxDate = QDate::currentDate();

    if (query.exec("SELECT MIN(doc_date) FROM documents") && query.next()) {
        if (!query.value(0).isNull()) {
            minDate = query.value(0).toDateTime().date();
        }
    }

    ui->de_start->setCalendarPopup(true);
    ui->de_end->setCalendarPopup(true);

    ui->de_start->setDate(minDate);
    ui->de_end->setDate(maxDate);

    ui->de_end->setMaximumDate(maxDate);
}

void MainWidget::onReportTypeChanged()
{
    QListWidgetItem *current = ui->list_report_types->currentItem();
    if (!current) return;

    QString reportSlug = current->data(Qt::UserRole).toString();

    ui->de_start->setEnabled(true);
    ui->de_end->setEnabled(true);
    ui->pb_period_reset->setEnabled(true);
    ui->cb_filter_category->setEnabled(true);
    ui->cb_filter_supplier->setEnabled(true);

    if (reportSlug == "rep_stock") {
        ui->de_start->setEnabled(false);
        ui->de_end->setEnabled(false);
        ui->pb_period_reset->setEnabled(false);
        ui->cb_filter_supplier->setEnabled(false);
    }
    else if (reportSlug == "rep_osv") {
        ui->cb_filter_supplier->setEnabled(false);
    }
    else if (reportSlug == "rep_purchases") {
        ui->cb_filter_category->setEnabled(false);
    }
    else if (reportSlug == "rep_outgoing") {
        ui->cb_filter_supplier->setEnabled(false);
    }
    else if (reportSlug == "rep_inventory") {
        ui->cb_filter_category->setEnabled(false);
        ui->cb_filter_supplier->setEnabled(false);
    }

    ui->tw_report_main->setRowCount(0);
    ui->lb_report_summary->setText("Нажмите 'Применить' для формирования отчёта");
}
