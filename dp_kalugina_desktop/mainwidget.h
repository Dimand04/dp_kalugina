#ifndef MAINWIDGET_H
#define MAINWIDGET_H

#include <QWidget>
#include <QToolButton>
#include <QTabBar>
#include <QMessageBox>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDateTime>
#include <QTableWidgetItem>
#include <QDoubleSpinBox>
#include <QComboBox>
#include <QCompleter>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWidget;
}
QT_END_NAMESPACE

class MainWidget : public QWidget
{
    Q_OBJECT

struct MaterialEntry {
    int id;
    QString category;
    QString unit;
};
QMap<int, MaterialEntry> m_materialsCache;

public:
    MainWidget(int userId, int userRole, QWidget *parent = nullptr);
    ~MainWidget();

private slots:
    void adminFunction();
    void logout();

    void tabw_main_change(int index);
    void tabw_administration_change(int index);
    void tabw_warehouse_change();
    void tabw_incoming_change();

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
    void table_warehouseBatches_clicked(int row);
    void loadWarehouseBachesTable();
    void loadWarehouseMovementsTable();

    void loadInventoryHistoryTable();
    void filter_inventory();
    void history_period_currentIndexChanged(int index);
    void table_inventory_clicked(int row);
    void loadInventoryDetails();
    void showInventory();
    void on_tw_inventoryDetails_itemChanged(QTableWidgetItem *item);
    void inventory_cancel();
    void closeInventoryWorkArea();
    void saveInventory();

    void loadIncomingSuppliers();
    void incoming_period_currentIndexChanged(int index);
    void filter_incoming();
    void loadIncomingHistoryTable();
    void toggleIncomingLayout();
    void table_incoming_clicked(int row);
    void loadIncomingDetails();
    void showIncoming();
    void incoming_cancel();
    void closeIncomingWorkArea();
    void loadAddIncomingSuppliers();
    void loadAddIncomingMaterials();
    void on_cb_inc_add_material_currentIndexChanged(int index);
    void calculateCurrentRowSum();
    void insertIncomingRow();
    void clearIncomingInputBar();
    void deleteIncomingRow(int row);
    void updateIncomingTotalSum();
    void saveIncoming();

private:
    Ui::MainWidget *ui;
    int userId;
    int userRole;
    int currentUserId;
    int currentRoleId;
    int currentCategoryId;
    int currentMaterialId;
    int currentSupplierId;
    int currentMaterialBatchesId;
    int currentInventoryId;
    int currentIncomingId;

    bool m_isWarehouseDetailsOpened = false;
    bool m_isFirstWarehouseOpen = true;
    bool m_isInventoryLoading = false;
    bool m_isIncomingDetailsOpened = false;
    bool m_isFirstIncomingOpen = true;

    QDate getEarliestInventoryDate();
    QDate getEarliestIncomingDate();
};
#endif // MAINWIDGET_H
