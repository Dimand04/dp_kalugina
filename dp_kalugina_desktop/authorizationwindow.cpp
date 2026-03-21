#include "authorizationwindow.h"
#include "ui_authorizationwindow.h"

AuthorizationWindow::AuthorizationWindow(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::AuthorizationWindow)
{
    ui->setupUi(this);
    setWindowTitle("Авторизация");
    setWindowFlags(Qt::WindowCloseButtonHint);
    this->resize(350,200);

    this->setStyleSheet(
        "QLineEdit { "
        "   border: 1px solid #ccc; "
        "   border-radius: 4px; "
        "   padding: 2px; "
        "   background: white; "
        "} "
        "QLineEdit[error='true'] { "
        "   border: 1px solid red; "
        "   background-color: #fff0f0; "
        "} "
        "QLineEdit[error='true']::placeholder { "
        "   color: red; "
        "}"
        );

    ui->lb_log_error->setVisible(false);
    ui->le_log_login->setFocus();

    connect(ui->pb_login, &QPushButton::clicked, this, &AuthorizationWindow::tryLogin);

    connect(ui->le_log_login, &QLineEdit::returnPressed, this, [this]() {
        if (!ui->le_log_login->text().trimmed().isEmpty()) {
            ui->le_log_password->setFocus();
        }
    });
    connect(ui->le_log_password, &QLineEdit::returnPressed, this, [this]() {
        if (!ui->le_log_password->text().trimmed().isEmpty()) {
            tryLogin();
        }
    });

    connect(ui->le_log_login, &QLineEdit::textChanged, this, [this]() {
        setErrorState(ui->le_log_login, false);
        ui->lb_log_error->setVisible(false);
    });
    connect(ui->le_log_password, &QLineEdit::textChanged, this, [this]() {
        setErrorState(ui->le_log_password, false);
        ui->lb_log_error->setVisible(false);
    });
}

AuthorizationWindow::~AuthorizationWindow()
{
    delete ui;
}

void AuthorizationWindow::setErrorState(QLineEdit *edit, bool hasError) {
    edit->setProperty("error", hasError);
    edit->style()->unpolish(edit);
    edit->style()->polish(edit);
}

void AuthorizationWindow::tryLogin()
{
    QString login = ui->le_log_login->text().remove(" ");
    QString password = ui->le_log_password->text().remove(" ");

    if (login.isEmpty() || password.isEmpty()) {
        if (login.isEmpty()) {
            ui->le_log_login->setFocus();
            setErrorState(ui->le_log_login, true);
        }
        if (password.isEmpty()) {
            ui->le_log_password->setFocus();
            setErrorState(ui->le_log_password, true);
        }
        return;
    }

    QString salt = "my_very_secure_salt_2026";
    QByteArray hashedInput = QCryptographicHash::hash((password + salt).toUtf8(), QCryptographicHash::Sha256).toHex();

    QSqlDatabase db = QSqlDatabase::database("db_dp_kalugina");
    if (db.isValid() && db.isOpen()) {
        QSqlQuery query(db);
        query.prepare("SELECT users.id, role_id FROM users "
                      "INNER JOIN authorization ON users.id = authorization.user_id "
                      "WHERE users.login = ? AND authorization.password = ?;");
        query.addBindValue(login);
        query.addBindValue(QString(hashedInput));
        query.exec();

        if(query.next()) {
            userId = query.value(0).toInt();
            userRole = query.value(1).toInt();
            accept();
        }
        else {
            ui->le_log_login->clear();
            ui->le_log_password->clear();

            setErrorState(ui->le_log_login, true);
            setErrorState(ui->le_log_password, true);

            ui->lb_log_error->setVisible(true);

            ui->le_log_login->setFocus();
        }
    }
}
