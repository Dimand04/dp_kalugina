#ifndef REPORTWIDGET_H
#define REPORTWIDGET_H

#include <QDialog>
#include <QFileDialog>
#include <QStandardPaths>
#include <QMessageBox>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDateTime>
#include <QTextDocument>
#include <QMessageBox>
#include <QPrinter>
#include <QFile>
#include <QTextStream>
#include <QTableWidget>

namespace Ui {
class reportwidget;
}

class reportwidget : public QDialog
{
    Q_OBJECT

public:
    enum SourceType {
        IncomingDoc,
        OutgoingDoc,
        MaterialBatches,
        MaterialHistory,
        InventoryDoc,
        CustomReport
    };

    explicit reportwidget(SourceType type, int targetId, int userId, QString orgName, QWidget *parent = nullptr);
    ~reportwidget();

signals:
    void fileSaved(QString type, QString path);

public slots:
    void copyDataFromTable(QTableWidget *sourceTable, const QString &title, const QString &summary);
    void setMetadata(QString k1, QString v1, QString k2, QString v2, QString k3, QString v3);

private:
    Ui::reportwidget *ui;
    SourceType m_type;
    int m_targetId;
    int m_userId;
    QString m_orgName;
    QString m_userName;

    void setupInterface();
    void loadReportData();
    void exportPdf();
    void exportCsv();
    void exportTxt();
    void fetchUserName();
};

#endif // REPORTWIDGET_H
