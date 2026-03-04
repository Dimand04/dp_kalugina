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
        StatusMode,
        UnitMode
    };

    explicit edit_widget(Mode mode, int id = 0, QWidget *parent = nullptr);
    ~edit_widget();

private slots:
    void loadData();
    void saveData();

private:
    Ui::edit_widget *ui;
    Mode m_mode;
    int m_id;
};

#endif // EDIT_WIDGET_H
