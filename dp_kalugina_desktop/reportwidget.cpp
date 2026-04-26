#include "reportwidget.h"
#include "ui_reportwidget.h"

reportwidget::reportwidget(SourceType type, int targetId, int userId, QString orgName, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::reportwidget)
    , m_type(type)
    , m_targetId(targetId)
    , m_userId(userId)
    , m_orgName(orgName)
{
    ui->setupUi(this);

    setWindowTitle("Экспорт отчёта");

    fetchUserName();

    ui->lb_val_1->setStyleSheet("font-weight: bold;");
    ui->lb_val_2->setStyleSheet("font-weight: bold;");
    ui->lb_val_3->setStyleSheet("font-weight: bold;");

    connect(ui->pb_export_pdf, &QPushButton::clicked, this, &reportwidget::exportPdf);
    connect(ui->pb_export_excel, &QPushButton::clicked, this, &reportwidget::exportCsv);
    connect(ui->pb_export_txt, &QPushButton::clicked, this, &reportwidget::exportTxt);

    setupInterface();

    if (m_type != CustomReport) {
        loadReportData();
    }
}

reportwidget::~reportwidget()
{
    delete ui;
}

void reportwidget::setupInterface() {
    QSqlDatabase db = QSqlDatabase::database("db_dp_kalugina");
    QSqlQuery query(db);
    QString currentDate = QDate::currentDate().toString("dd.MM.yyyy");

    switch (m_type) {
    case IncomingDoc:
    case OutgoingDoc:
    case InventoryDoc: {
        bool isInc = (m_type == IncomingDoc);
        bool isInv = (m_type == InventoryDoc);

        QString title;
        if (isInv) title = QString("АКТ ИНВЕНТАРИЗАЦИИ №%1").arg(m_targetId);
        else if (isInc) title = QString("ПРИХОДНАЯ НАКЛАДНАЯ №%1").arg(m_targetId);
        else title = QString("АКТ СПИСАНИЯ (РАСХОД) №%1").arg(m_targetId);

        ui->lb_report_title->setText(title + " от " + currentDate);
        ui->tw_report_data->setColumnCount(isInv ? 9 : 5);

        if (isInv) ui->tw_report_data->setHorizontalHeaderLabels({"Категория", "Материал", "Ед. изм.", "Учет", "Факт", "Разница", "Цена", "Сумма", "Причина"});
        else ui->tw_report_data->setHorizontalHeaderLabels({"Материал", "Категория", "Кол-во", "Цена", "Сумма"});

        query.prepare("SELECT d.doc_date, u.login, d.description, s.name FROM documents d "
                      "JOIN users u ON d.user_id = u.id "
                      "LEFT JOIN inventory_transactions it ON it.document_id = d.id "
                      "LEFT JOIN batches b ON it.batch_id = b.id "
                      "LEFT JOIN suppliers s ON b.supplier_id = s.id "
                      "WHERE d.id = :id LIMIT 1");
        query.bindValue(":id", m_targetId);

        if (query.exec() && query.next()) {
            QString supplier = query.value(3).toString();
            setMetadata(
                "Дата документа:", query.value(0).toDateTime().toString("dd.MM.yyyy HH:mm"),
                isInc ? "Поставщик:" : "Ответственный:", isInc ? (supplier.isEmpty() ? "-" : supplier) : query.value(1).toString(),
                isInc ? "Принял:" : "", isInc ? query.value(1).toString() : ""
                );

            QString description = query.value(2).toString().trimmed();
            if (description.isEmpty()) ui->lb_description->setVisible(false);
            else {
                ui->lb_description->setVisible(true);
                ui->lb_description->setText((isInv ? "Примечание: " : "Комментарий: ") + description);
            }
        }
        break;
    }

    case MaterialBatches:
    case MaterialHistory: {
        QString title = (m_type == MaterialBatches) ? "ОТЧЕТ ПО ТЕКУЩИМ ПАРТИЯМ" : "ИСТОРИЯ ДВИЖЕНИЙ МАТЕРИАЛА";
        ui->lb_report_title->setText(title + " на " + currentDate);

        if (m_type == MaterialBatches) {
            ui->tw_report_data->setColumnCount(4);
            ui->tw_report_data->setHorizontalHeaderLabels({"Дата партии", "Поставщик", "Остаток", "Цена закупки"});
        } else {
            ui->tw_report_data->setColumnCount(6);
            ui->tw_report_data->setHorizontalHeaderLabels({"Тип", "Дата и время", "Документ", "Кол-во", "Остаток", "Сотрудник"});
        }

        ui->lb_description->setVisible(false);
        query.prepare("SELECT m.name, c.name, u.name FROM materials m JOIN categories c ON m.category_id = c.id JOIN units u ON m.unit_id = u.id WHERE m.id = :id");
        query.bindValue(":id", m_targetId);
        if (query.exec() && query.next()) {
            setMetadata("Материал:", query.value(0).toString(), "Категория:", query.value(1).toString(), "Ед. изм.:", query.value(2).toString());
        }
        break;
    }
    case CustomReport: {
        ui->lb_description->setVisible(false);
        break;
    }
    }
}

void reportwidget::loadReportData()
{
    QSqlDatabase db = QSqlDatabase::database("db_dp_kalugina");
    if (!db.isValid() || !db.isOpen()) return;

    ui->tw_report_data->setRowCount(0);
    ui->tw_report_data->setSortingEnabled(false);
    QSqlQuery query(db);
    double totalValue = 0.0;
    double runningBalance = 0.0;

    switch (m_type) {
    case IncomingDoc:
    case OutgoingDoc: {
        query.prepare("SELECT m.name, c.name, it.quantity, it.price, (it.quantity * it.price) FROM inventory_transactions it JOIN materials m ON it.material_id = m.id JOIN categories c ON m.category_id = c.id WHERE it.document_id = :id");
        query.bindValue(":id", m_targetId);
        if (query.exec()) {
            while (query.next()) {
                int row = ui->tw_report_data->rowCount();
                ui->tw_report_data->insertRow(row);
                double qty = qAbs(query.value(2).toDouble());
                double sum = qAbs(query.value(4).toDouble());
                ui->tw_report_data->setItem(row, 0, new QTableWidgetItem(query.value(0).toString()));
                ui->tw_report_data->setItem(row, 1, new QTableWidgetItem(query.value(1).toString()));
                ui->tw_report_data->setItem(row, 2, new QTableWidgetItem(QString::number(qty, 'f', 3)));
                ui->tw_report_data->setItem(row, 3, new QTableWidgetItem(QString::number(query.value(3).toDouble(), 'f', 2)));
                ui->tw_report_data->setItem(row, 4, new QTableWidgetItem(QString::number(sum, 'f', 2)));
                for(int i=2; i<=4; ++i) ui->tw_report_data->item(row, i)->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
                totalValue += sum;
            }
        }
        ui->lb_total_info->setText((m_type == IncomingDoc ? "ИТОГО ПО НАКЛАДНОЙ: " : "ИТОГО СПИСАНО: ") + QString::number(totalValue, 'f', 2));
        break;
    }
    case InventoryDoc: {
        query.prepare("SELECT c.name, m.name, u.name, it.expected_quantity, it.actual_quantity, it.quantity, it.price, (it.quantity * it.price), it.reason FROM inventory_transactions it JOIN materials m ON it.material_id = m.id JOIN categories c ON m.category_id = c.id JOIN units u ON m.unit_id = u.id WHERE it.document_id = :id");
        query.bindValue(":id", m_targetId);
        if (query.exec()) {
            while (query.next()) {
                int row = ui->tw_report_data->rowCount();
                ui->tw_report_data->insertRow(row);
                double diff = query.value(5).toDouble();
                double sSum = query.value(7).toDouble();
                ui->tw_report_data->setItem(row, 0, new QTableWidgetItem(query.value(0).toString()));
                ui->tw_report_data->setItem(row, 1, new QTableWidgetItem(query.value(1).toString()));
                ui->tw_report_data->setItem(row, 2, new QTableWidgetItem(query.value(2).toString()));
                ui->tw_report_data->setItem(row, 3, new QTableWidgetItem(QString::number(query.value(3).toDouble(), 'f', 3)));
                ui->tw_report_data->setItem(row, 4, new QTableWidgetItem(QString::number(query.value(4).toDouble(), 'f', 3)));
                ui->tw_report_data->setItem(row, 5, new QTableWidgetItem(QString::number(diff, 'f', 3)));
                ui->tw_report_data->setItem(row, 6, new QTableWidgetItem(QString::number(query.value(6).toDouble(), 'f', 2)));
                ui->tw_report_data->setItem(row, 7, new QTableWidgetItem(QString::number(sSum, 'f', 2)));
                ui->tw_report_data->setItem(row, 8, new QTableWidgetItem(query.value(8).toString().isEmpty() ? "-" : query.value(8).toString()));
                if (diff < -0.0001) ui->tw_report_data->item(row, 5)->setForeground(Qt::red);
                else if (diff > 0.0001) ui->tw_report_data->item(row, 5)->setForeground(QColor(0, 150, 0));
                for(int i=3; i<=7; ++i) ui->tw_report_data->item(row, i)->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
                totalValue += sSum;
            }
        }
        ui->lb_total_info->setText("ИТОГОВЫЙ РЕЗУЛЬТАТ (РАЗНИЦА): " + QString::number(totalValue, 'f', 2));
        break;
    }
    case MaterialBatches: {
        query.prepare("SELECT b.incoming_date, s.name, b.current_quantity, b.purchase_price FROM batches b LEFT JOIN suppliers s ON b.supplier_id = s.id WHERE b.material_id = :id AND b.current_quantity > 0.0001 ORDER BY b.incoming_date ASC");
        query.bindValue(":id", m_targetId);
        if (query.exec()) {
            while (query.next()) {
                int row = ui->tw_report_data->rowCount();
                ui->tw_report_data->insertRow(row);
                ui->tw_report_data->setItem(row, 0, new QTableWidgetItem(query.value(0).toDateTime().toString("dd.MM.yyyy HH:mm")));
                ui->tw_report_data->setItem(row, 1, new QTableWidgetItem(query.value(1).toString().isEmpty() ? "Инвентаризация" : query.value(1).toString()));
                ui->tw_report_data->setItem(row, 2, new QTableWidgetItem(QString::number(query.value(2).toDouble(), 'f', 3)));
                ui->tw_report_data->setItem(row, 3, new QTableWidgetItem(QString::number(query.value(3).toDouble(), 'f', 2)));
                ui->tw_report_data->item(row, 2)->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
                ui->tw_report_data->item(row, 3)->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
                totalValue += query.value(2).toDouble();
            }
        }
        ui->lb_total_info->setText(QString("ОБЩИЙ ОСТАТОК: %1").arg(QString::number(totalValue, 'f', 3)));
        break;
    }
    case MaterialHistory: {
        query.prepare("SELECT d.doc_type, d.doc_date, d.id, it.quantity, u.login FROM inventory_transactions it JOIN documents d ON it.document_id = d.id JOIN users u ON d.user_id = u.id WHERE it.material_id = :id ORDER BY d.doc_date ASC");
        query.bindValue(":id", m_targetId);
        if (query.exec()) {
            while (query.next()) {
                int row = ui->tw_report_data->rowCount();
                ui->tw_report_data->insertRow(row);
                QString type = query.value(0).toString();
                double qty = query.value(3).toDouble();
                QString disp;
                if (type == "incoming") { disp = "Приход"; runningBalance += qty; }
                else if (type == "outgoing") { disp = "Расход"; runningBalance -= qAbs(qty); qty = -qAbs(qty); }
                else { disp = "Инвент."; runningBalance += qty; }
                ui->tw_report_data->setItem(row, 0, new QTableWidgetItem(disp));
                ui->tw_report_data->setItem(row, 1, new QTableWidgetItem(query.value(1).toDateTime().toString("dd.MM.yy HH:mm")));
                ui->tw_report_data->setItem(row, 2, new QTableWidgetItem(QString("Док. №%1").arg(query.value(2).toInt())));
                QTableWidgetItem *iQty = new QTableWidgetItem(QString::number(qty, 'f', 3));
                QTableWidgetItem *iBal = new QTableWidgetItem(QString::number(runningBalance, 'f', 3));
                iQty->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
                iBal->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
                if (qty < 0) iQty->setForeground(Qt::red);
                ui->tw_report_data->setItem(row, 3, iQty);
                ui->tw_report_data->setItem(row, 4, iBal);
                ui->tw_report_data->setItem(row, 5, new QTableWidgetItem(query.value(4).toString()));
            }
        }
        ui->lb_total_info->setText(QString("ТЕКУЩИЙ ОСТАТОК: %1").arg(QString::number(runningBalance, 'f', 3)));
        break;
    }
    case CustomReport: break;
    }

    QHeaderView *header = ui->tw_report_data->horizontalHeader();
    header->setDefaultAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    if (ui->tw_report_data->columnCount() > 0) {
        header->setSectionResizeMode(0, QHeaderView::ResizeToContents);
        for (int i = 1; i < ui->tw_report_data->columnCount(); ++i) header->setSectionResizeMode(i, QHeaderView::Stretch);
    }
    ui->tw_report_data->setSortingEnabled(true);
}

void reportwidget::setMetadata(QString k1, QString v1, QString k2, QString v2, QString k3, QString v3)
{
    ui->lb_key_1->setText(k1); ui->lb_val_1->setText(v1);
    ui->lb_key_2->setText(k2); ui->lb_val_2->setText(v2);
    ui->lb_key_3->setText(k3); ui->lb_val_3->setText(v3);

    ui->lb_key_1->setVisible(!k1.isEmpty()); ui->lb_val_1->setVisible(!k1.isEmpty());
    ui->lb_key_2->setVisible(!k2.isEmpty()); ui->lb_val_2->setVisible(!k2.isEmpty());
    ui->lb_key_3->setVisible(!k3.isEmpty()); ui->lb_val_3->setVisible(!k3.isEmpty());
}

void reportwidget::exportPdf()
{
    QString sanitizedTitle = ui->lb_report_title->text().replace(" ", "_").replace("№", "N");
    QString timestamp = QDateTime::currentDateTime().toString("dd_MM_yyyy_HHmm");
    QString fileName = QFileDialog::getSaveFileName(this, "Сохранить как PDF",
                                                    sanitizedTitle + "_" + timestamp, "PDF файлы (*.pdf)");

    if (fileName.isEmpty()) return;
    if (QFileInfo(fileName).suffix().isEmpty()) fileName += ".pdf";

    QString html = "<html><head><style>"
                   "body { margin: 0; padding: 5px; font-family: sans-serif; }"
                   "table { border-collapse: collapse; width: 100%; }"
                   "th, td { border: 1px solid black; padding: 5px; text-align: left; font-size: 9pt; }"
                   "th { background-color: #f2f2f2; }"
                   ".header-top { text-align: right; font-size: 8pt; color: #555; margin-bottom: 2px; }"
                   ".company-name { font-size: 11pt; font-weight: bold; margin-bottom: 10px; }"
                   ".report-title { text-align: center; font-size: 15pt; font-weight: bold; margin: 0; }"
                   ".info-table { border: none; margin: 0; }"
                   ".info-table td { border: none; padding: 1px; font-size: 10pt; }"
                   ".total { text-align: left; font-size: 12pt; font-weight: bold; margin-top: 10px; border-top: 1px solid #000; padding-top: 5px; }"
                   ".description { font-size: 10pt; margin: 0; }"

                   ".signature-section { margin-top: 30px; width: 100%; border: none; border-collapse: collapse; }"
                   ".signature-label { border: none !important; width: 1%; white-space: nowrap; font-size: 10pt; padding-right: 10px; }"
                   ".signature-line { border: none !important; border-bottom: 1px solid black !important; width: 35%; }"
                   ".signature-spacer { border: none !important; width: 20%; }"

                   ".gen-info { margin-top: 20px; font-size: 8pt; color: #666; border-top: 0.5px solid #ccc; padding-top: 3px; }"
                   "</style></head><body>";

    html += "<div class='header-top'>Система 'SoundStream' v1.0</div>";
    html += "<div class='company-name'>" + m_orgName + "</div>";
    html += "<div class='report-title'>" + ui->lb_report_title->text() + "</div><br>";

    if (ui->lb_key_1->isVisible() || ui->lb_key_2->isVisible() || ui->lb_key_3->isVisible()) {
        html += "<table class='info-table'>";
        if (ui->lb_key_1->isVisible()) html += QString("<tr><td><b>%1</b> %2</td></tr>").arg(ui->lb_key_1->text(), ui->lb_val_1->text());
        if (ui->lb_key_2->isVisible()) html += QString("<tr><td><b>%1</b> %2</td></tr>").arg(ui->lb_key_2->text(), ui->lb_val_2->text());
        if (ui->lb_key_3->isVisible()) html += QString("<tr><td><b>%1</b> %2</td></tr>").arg(ui->lb_key_3->text(), ui->lb_val_3->text());
        html += "</table><br>";
    }

    if (ui->lb_description->isVisible()) html += "<div class='description'><b>" + ui->lb_description->text() + "</b></div><br>";

    html += "<table><thead><tr>";
    for (int i = 0; i < ui->tw_report_data->columnCount(); ++i)
        html += "<th>" + ui->tw_report_data->horizontalHeaderItem(i)->text() + "</th>";
    html += "</tr></thead><tbody>";

    for (int r = 0; r < ui->tw_report_data->rowCount(); ++r) {
        html += "<tr>";
        for (int c = 0; c < ui->tw_report_data->columnCount(); ++c)
            html += "<td>" + (ui->tw_report_data->item(r, c) ? ui->tw_report_data->item(r, c)->text() : "") + "</td>";
        html += "</tr>";
    }
    html += "</tbody></table>";

    html += "<div class='total'>" + ui->lb_total_info->text() + "</div>";

    if (m_type == IncomingDoc || m_type == OutgoingDoc || m_type == InventoryDoc) {
        QString longLine = "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"
                           "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"
                           "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;";

        html += "<table class='signature-section'><tr>"
                "<td class='signature-label'>Сдал:</td>"
                "<td class='signature-line'>" + longLine + "</td>"
                             "<td class='signature-spacer'></td>"
                             "<td class='signature-label'>Принял:</td>"
                             "<td class='signature-line'>" + longLine + "</td>"
                             "</tr></table>";
    }

    html += "<div class='gen-info'>Сформировал: " + m_userName + "<br>Дата: " + QDateTime::currentDateTime().toString("dd.MM.yyyy HH:mm:ss") + "</div>";
    html += "</body></html>";

    QTextDocument doc;
    doc.setHtml(html);

    doc.setDocumentMargin(0);

    QPrinter printer(QPrinter::PrinterResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setPageSize(QPageSize(QPageSize::A4));
    printer.setOutputFileName(fileName);

    printer.setPageMargins(QMarginsF(2, 2, 2, 2), QPageLayout::Millimeter);

    doc.print(&printer);
    emit fileSaved("PDF", fileName);
    QMessageBox::information(this, "Успех", "PDF файл сохранён.");
}

void reportwidget::exportCsv()
{
    QString sanitizedTitle = ui->lb_report_title->text().replace(" ", "_").replace("№", "N");
    QString fileName = QFileDialog::getSaveFileName(this, "Сохранить для Excel (CSV)",
                                                    sanitizedTitle, "CSV файлы (*.csv)");
    if (fileName.isEmpty()) return;
    if (QFileInfo(fileName).suffix().isEmpty()) fileName += ".csv";

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "Ошибка", "Не удалось создать файл для записи.");
        return;
    }

    QTextStream out(&file);
    out.setGenerateByteOrderMark(true);
    QString sep = ";";

    out << m_orgName << "\n";
    out << ui->lb_report_title->text() << "\n\n";

    if (ui->lb_key_1->isVisible()) out << ui->lb_key_1->text() << sep << ui->lb_val_1->text() << "\n";
    if (ui->lb_key_2->isVisible()) out << ui->lb_key_2->text() << sep << ui->lb_val_2->text() << "\n";
    if (ui->lb_key_3->isVisible()) out << ui->lb_key_3->text() << sep << ui->lb_val_3->text() << "\n";
    if (ui->lb_description->isVisible()) out << ui->lb_description->text() << "\n";
    out << "\n";

    for (int i = 0; i < ui->tw_report_data->columnCount(); ++i) {
        out << ui->tw_report_data->horizontalHeaderItem(i)->text() << (i == ui->tw_report_data->columnCount() - 1 ? "" : sep);
    }
    out << "\n";

    for (int row = 0; row < ui->tw_report_data->rowCount(); ++row) {
        for (int col = 0; col < ui->tw_report_data->columnCount(); ++col) {
            QString val = ui->tw_report_data->item(row, col) ? ui->tw_report_data->item(row, col)->text() : "";
            out << "\"" << val << "\"" << (col == ui->tw_report_data->columnCount() - 1 ? "" : sep);
        }
        out << "\n";
    }

    out << "\n" << ui->lb_total_info->text() << "\n";
    out << "Сформировал: " << m_userName << sep << "Дата: " << QDateTime::currentDateTime().toString("dd.MM.yyyy HH:mm") << "\n";

    file.close();
    emit fileSaved("CSV", fileName);
    QMessageBox::information(this, "Успех", "Отчёт успешно экспортирован в CSV (Excel).");
}

void reportwidget::exportTxt()
{
    QString sanitizedTitle = ui->lb_report_title->text().replace(" ", "_").replace("№", "N");
    QString fileName = QFileDialog::getSaveFileName(this, "Сохранить как текст", sanitizedTitle, "Текстовые файлы (*.txt)");

    if (fileName.isEmpty()) return;
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) return;

    QTextStream out(&file);
    out.setEncoding(QStringConverter::Utf8);

    out << m_orgName << "\n";
    out << "\n";
    out << QString("=").repeated(60) << "\n";
    out << ui->lb_report_title->text().toUpper() << "\n";
    out << QString("=").repeated(60) << "\n\n";

    if (ui->lb_key_1->isVisible()) out << ui->lb_key_1->text() << " " << ui->lb_val_1->text() << "\n";
    if (ui->lb_key_2->isVisible()) out << ui->lb_key_2->text() << " " << ui->lb_val_2->text() << "\n";
    if (ui->lb_key_3->isVisible()) out << ui->lb_key_3->text() << " " << ui->lb_val_3->text() << "\n";
    if (ui->lb_description->isVisible()) out << "\n" << ui->lb_description->text() << "\n";

    out << "\n";

    QList<int> widths;
    for (int c = 0; c < ui->tw_report_data->columnCount(); ++c) {
        int w = ui->tw_report_data->horizontalHeaderItem(c)->text().length();
        for (int r = 0; r < ui->tw_report_data->rowCount(); ++r) {
            int cellW = ui->tw_report_data->item(r, c) ? ui->tw_report_data->item(r, c)->text().length() : 0;
        }
    }

    for (int c = 0; c < ui->tw_report_data->columnCount(); ++c) {
        int w = ui->tw_report_data->horizontalHeaderItem(c)->text().length();
        for (int r = 0; r < ui->tw_report_data->rowCount(); ++r) {
            if (ui->tw_report_data->item(r, c)) {
                int cellW = ui->tw_report_data->item(r, c)->text().length();
                if (cellW > w) w = cellW;
            }
        }
        widths << w + 2;
    }

    for (int c = 0; c < ui->tw_report_data->columnCount(); ++c) {
        out << ui->tw_report_data->horizontalHeaderItem(c)->text().leftJustified(widths[c], ' ');
    }
    out << "\n" << QString("-").repeated(60) << "\n";

    for (int r = 0; r < ui->tw_report_data->rowCount(); ++r) {
        for (int c = 0; c < ui->tw_report_data->columnCount(); ++c) {
            QString txt = ui->tw_report_data->item(r, c) ? ui->tw_report_data->item(r, c)->text() : "";
            out << txt.leftJustified(widths[c], ' ');
        }
        out << "\n";
    }

    out << QString("-").repeated(60) << "\n";
    out << ui->lb_total_info->text() << "\n\n";
    out << "Сформировал: " << m_userName << "\n";
    out << "Дата печати: " << QDateTime::currentDateTime().toString("dd.MM.yyyy HH:mm:ss") << "\n";

    file.close();
    emit fileSaved("TXT", fileName);
    QMessageBox::information(this, "Успех", "Отчёт сохранён в TXT");
}

void reportwidget::copyDataFromTable(QTableWidget *sourceTable, const QString &title, const QString &summary)
{
    if (!sourceTable) return;
    QString currentDate = QDate::currentDate().toString("dd.MM.yyyy");
    ui->lb_report_title->setText(title + " от " + currentDate);
    ui->lb_total_info->setText(summary);

    int colCount = sourceTable->columnCount();
    ui->tw_report_data->setColumnCount(colCount);
    QStringList headers;
    for (int i = 0; i < colCount; ++i) headers << sourceTable->horizontalHeaderItem(i)->text();
    ui->tw_report_data->setHorizontalHeaderLabels(headers);
    ui->tw_report_data->setRowCount(0);
    for (int row = 0; row < sourceTable->rowCount(); ++row) {
        ui->tw_report_data->insertRow(row);
        for (int col = 0; col < colCount; ++col) {
            QTableWidgetItem *sourceItem = sourceTable->item(row, col);
            if (sourceItem) {
                QTableWidgetItem *newItem = new QTableWidgetItem(sourceItem->text());
                newItem->setTextAlignment(sourceItem->textAlignment());
                newItem->setForeground(sourceItem->foreground());
                ui->tw_report_data->setItem(row, col, newItem);
            }
        }
    }
    QHeaderView *header = ui->tw_report_data->horizontalHeader();
    header->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    for (int i = 1; i < colCount; ++i) header->setSectionResizeMode(i, QHeaderView::Stretch);
}

void reportwidget::fetchUserName()
{
    QSqlDatabase db = QSqlDatabase::database("db_dp_kalugina");
    QSqlQuery query(db);
    query.prepare("SELECT login FROM users WHERE id = :id");
    query.bindValue(":id", m_userId);

    if (query.exec() && query.next()) {
        m_userName = query.value(0).toString();
    } else {
        m_userName = "ID: " + QString::number(m_userId);
    }
}
