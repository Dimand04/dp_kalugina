#ifndef MAINWIDGET_H
#define MAINWIDGET_H

#include <QWidget>
#include <QToolButton>
#include <QTabBar>
#include <QMessageBox>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWidget;
}
QT_END_NAMESPACE

class MainWidget : public QWidget
{
    Q_OBJECT

public:
    MainWidget(int userId, int userRole, QWidget *parent = nullptr);
    ~MainWidget();

private slots:
    void adminFunction();
    void logout();

    void tabw_main_change(int index);
    void tabw_administration_change(int index);
    void tabw_warehouse_change();

    void load_users_table();
    void filter_users();
    void show_edit_user();
    void table_users_clicked(int row);
    void reset_users_ui();

    void load_roles_table();
    void filter_roles(const QString &searchText);
    void show_edit_role();
    void table_roles_clicked(int row);
    void reset_roles_ui();
    void loadRoles();

    void load_categories_table();
    void reset_categories_ui();
    void filter_categories(const QString &searchText);
    void show_edit_category();
    void table_categories_clicked(int row);

    void load_materials_table();
    void reset_materials_ui();
    void filter_materials(const QString &searchText);
    void show_edit_material();
    void table_materials_clicked(int row);

    void load_suppliers_table();
    void reset_suppliers_ui();
    void filter_suppliers(const QString &searchText);
    void show_edit_supplier();
    void table_suppliers_clicked(int row);

    void loadWarehouseTable();
    void filter_warehouse();
    void loadWarehouseCategories();
    void toggleWarehouseLayout();

private:
    Ui::MainWidget *ui;
    int userId;
    int userRole;
    int currentUserId;
    int currentRoleId;
    int currentCategoryId;
    int currentMaterialId;
    int currentSupplierId;
};
#endif // MAINWIDGET_H
