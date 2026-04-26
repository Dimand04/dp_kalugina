#ifndef EDIT_WIDGET_H
#define EDIT_WIDGET_H

#include <QDialog>
#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>

namespace Ui {
class edit_widget;
}

class edit_widget : public QDialog
{
    Q_OBJECT

public:
    enum Mode {
        CategoryMode,
        MaterialMode,
        SupplierMode
    };

    explicit edit_widget(Mode mode, int id = 0, QWidget *parent = nullptr);
    ~edit_widget();

signals:
    void actionLogged(const QString &type, const QString &description, int targetId);

private slots:
    void loadData();
    void saveData();
    void loadCategories();

private:
    Ui::edit_widget *ui;
    Mode m_mode;
    int m_id;
};

#endif // EDIT_WIDGET_H
