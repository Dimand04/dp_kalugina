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
    else if(index == 4) {
        tabw_administration_change(0);
    }
}

void MainWidget::tabw_administration_change(int index)
{
    if(index == 0) {
        load_categories_table();
    }
    else if(index == 1) {
        load_users_table();
        load_roles_table();
        loadRoles();
    }
    else if(index == 2) {

    }
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
