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
        InventoryDoc
    };

    explicit reportwidget(SourceType type, int targetId, QWidget *parent = nullptr);
    ~reportwidget();

private:
    Ui::reportwidget *ui;
    SourceType m_type;
    int m_targetId;

    void setupInterface();
    void loadReportData();
    void setMetadata(QString k1, QString v1, QString k2, QString v2, QString k3, QString v3);
    void exportPdf();
    void exportCsv();
    void exportTxt();
};

#endif // REPORTWIDGET_H
