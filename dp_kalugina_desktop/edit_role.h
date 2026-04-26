#ifndef EDIT_ROLE_H
#define EDIT_ROLE_H

#include <QDialog>
#include <QSqlQuery>
#include <QSqlError>
#include <QMap>
#include <QTreeWidgetItem>
#include <QMessageBox>

namespace Ui {
class edit_role;
}

class edit_role : public QDialog
{
    Q_OBJECT

public:
    explicit edit_role(const int& roleId, QWidget *parent = nullptr);
    ~edit_role();

signals:
    void actionLogged(const QString &type, const QString &description, int targetId);

private slots:
    void buildPermissionsTree();
    void onPermissionChanged(QTreeWidgetItem *item, int column);
    void checkId();
    void loadRole();
    void createRole();
    void pb_ok_clicked();
    void updateRole();

private:
    Ui::edit_role *ui;
    int roleId;

    struct PermData {
        int id;
        int parentId;
        QString name;
        QString slug;
    };
    void addRecursiveChild(QTreeWidgetItem *parentItem, int parentId, const QList<PermData> &allPerms);

    bool m_isUpdatingTree = false;
    void updateChildrenStatus(QTreeWidgetItem *parent);
};

#endif // EDIT_ROLE_H
