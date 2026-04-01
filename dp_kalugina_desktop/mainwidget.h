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
#include <QDateEdit>
#include <QStackedWidget>

struct MaterialEntry {
    int id;
    QString category;
    QString unit;
};

enum DocMode { Incoming, Outgoing };

struct DocUI {
    QTableWidget *historyTable;
    QTableWidget *detailsTable;
    QLineEdit *searchLe;
    QComboBox *periodCb;
    QComboBox *counterpartyFilterCb;
    QDateEdit *deStart;
    QDateEdit *deEnd;
    QStackedWidget *stack;
};

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
    void tabw_incoming_change();
    void tabw_outgoing_change();

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
    void updateDocPeriod(DocMode mode, int index);
    void filterDocs(DocMode mode);
    void loadHistoryTable(DocMode mode);
    void toggleDocLayout(DocMode mode);
    void handleDocClick(DocMode mode, int row);
    void loadDocDetails(DocMode mode);
    void prepareDocEditor(DocMode mode);
    void cancelDocOperation(DocMode mode);
    void closeDocWorkArea(DocMode mode);
    void loadAddIncomingSuppliers();
    void loadAddDocMaterials(DocMode mode);
    void handleMaterialSelectionChanged(DocMode mode);
    void calculateCurrentRowSum(DocMode mode);
    void addDocRow(DocMode mode);
    void clearDocInputBar(DocMode mode);
    void deleteDocRow(DocMode mode, int row);
    void updateDocTotalSum(DocMode mode);
    void saveIncoming();
    void saveOutgoing();

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
    int currentOutgoingId;

    bool m_isWarehouseDetailsOpened = false;
    bool m_isFirstWarehouseOpen = true;
    bool m_isInventoryLoading = false;
    bool m_isIncomingDetailsOpened = false;
    bool m_isFirstIncomingOpen = true;
    bool m_isOutgoingDetailsOpened = false;
    bool m_isFirstOutgoingOpen = true;

    QDate getEarliestInventoryDate();
    QDate getEarliestDocDate(QString type);
    double getFifoPrice(int materialId);
    double getAvailableStock(int materialId);

    DocUI getDocUI(DocMode mode);

    QMap<int, MaterialEntry> m_materialsCache;
};
#endif // MAINWIDGET_H
