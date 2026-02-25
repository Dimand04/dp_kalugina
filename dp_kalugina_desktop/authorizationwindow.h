#ifndef AUTHORIZATIONWINDOW_H
#define AUTHORIZATIONWINDOW_H

#include <QDialog>
#include <QMessageBox>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QStyle>
#include <QCryptographicHash>
#include <QSqlError>

namespace Ui {
class AuthorizationWindow;
}

class AuthorizationWindow : public QDialog
{
    Q_OBJECT

public:
    explicit AuthorizationWindow(QWidget *parent = nullptr);
    ~AuthorizationWindow();

    int getUserId() const { return userId; }
    int getUserRole() const { return userRole; }

private slots:
    void tryLogin();
    void setErrorState(class QLineEdit *edit, bool hasError);

private:
    Ui::AuthorizationWindow *ui;
    int userId;
    int userRole;
};

#endif // AUTHORIZATIONWINDOW_H
