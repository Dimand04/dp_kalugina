#ifndef EDIT_USER_H
#define EDIT_USER_H

#include <QDialog>
#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>
#include <QCryptographicHash>

namespace Ui {
class edit_user;
}

class edit_user : public QDialog
{
    Q_OBJECT

public:
    explicit edit_user(const int& userId, QWidget *parent = nullptr);
    ~edit_user();

signals:
    void actionLogged(const QString &type, const QString &description, int targetId);

private slots:
    void loadRoles();
    void checkId();
    void loadUser();
    void createUser();
    void pb_ok_clicked();
    void updateUser();

private:
    Ui::edit_user *ui;
    int userId;
};

#endif // EDIT_USER_H
