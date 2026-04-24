#include "reportwidget.h"
#include "ui_reportwidget.h"

reportwidget::reportwidget(SourceType type, int targetId, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::reportwidget)
    , m_type(type)
    , m_targetId(targetId)
{
    ui->setupUi(this);

    setWindowTitle("Экспорт отчёта");

    ui->lb_val_1->setStyleSheet("font-weight: bold;");
    ui->lb_val_2->setStyleSheet("font-weight: bold;");
    ui->lb_val_3->setStyleSheet("font-weight: bold;");

    connect(ui->pb_export_pdf, &QPushButton::clicked, this, &reportwidget::exportPdf);
    connect(ui->pb_export_excel, &QPushButton::clicked, this, &reportwidget::exportCsv);
    connect(ui->pb_export_txt, &QPushButton::clicked, this, &reportwidget::exportTxt);

    setupInterface();

    loadReportData();
}

reportwidget::~reportwidget()
{
    delete ui;
}

void reportwidget::setupInterface() {
    QSqlDatabase db = QSqlDatabase::database("db_dp_kalugina");
    QSqlQuery query(db);

    switch (m_type) {
    case IncomingDoc:
    case OutgoingDoc:
    case InventoryDoc: {
        bool isInc = (m_type == IncomingDoc);
        bool isInv = (m_type == InventoryDoc);

        if (isInv) {
            ui->lb_report_title->setText(QString("АКТ ИНВЕНТАРИЗАЦИИ №%1").arg(m_targetId));
            ui->tw_report_data->setColumnCount(9);
            ui->tw_report_data->setHorizontalHeaderLabels({
                "Категория", "Материал", "Ед. изм.", "Учет", "Факт", "Разница", "Цена", "Сумма", "Причина"
            });
        } else {
            ui->lb_report_title->setText(isInc ? QString("ПРИХОДНАЯ НАКЛАДНАЯ №%1").arg(m_targetId)
                                               : QString("АКТ СПИСАНИЯ (РАСХОД) №%1").arg(m_targetId));
            ui->tw_report_data->setColumnCount(5);
            ui->tw_report_data->setHorizontalHeaderLabels({"Материал", "Категория", "Кол-во", "Цена", "Сумма"});
        }

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
                "Дата:", query.value(0).toDateTime().toString("dd.MM.yyyy HH:mm"),
                isInc ? "Поставщик:" : "Ответственный:", isInc ? (supplier.isEmpty() ? "-" : supplier) : query.value(1).toString(),
                isInc ? "Принял:" : "", isInc ? query.value(1).toString() : ""
                );

            QString description = query.value(2).toString().trimmed();
            if (description.isEmpty()) {
                ui->lb_description->setVisible(false);
            } else {
                ui->lb_description->setVisible(true);
                ui->lb_description->setText((isInv ? "Примечание: " : "Комментарий: ") + description);
            }
        }
        break;
    }

    case MaterialBatches:
    case MaterialHistory: {
        if (m_type == MaterialBatches) {
            ui->lb_report_title->setText("ОТЧЕТ ПО ТЕКУЩИМ ПАРТИЯМ");
            ui->tw_report_data->setColumnCount(4);
            ui->tw_report_data->setHorizontalHeaderLabels({"Дата партии", "Поставщик", "Остаток", "Цена закупки"});
        } else {
            ui->lb_report_title->setText("ИСТОРИЯ ДВИЖЕНИЙ МАТЕРИАЛА");
            ui->tw_report_data->setColumnCount(6);
            ui->tw_report_data->setHorizontalHeaderLabels({"Тип", "Дата и время", "Документ", "Кол-во", "Остаток", "Сотрудник"});
        }
        ui->lb_description->setVisible(false);
        query.prepare("SELECT m.name, c.name, u.name FROM materials m "
                      "JOIN categories c ON m.category_id = c.id "
                      "JOIN units u ON m.unit_id = u.id "
                      "WHERE m.id = :id");
        query.bindValue(":id", m_targetId);
        if (query.exec() && query.next()) {
            setMetadata("Материал:", query.value(0).toString(), "Категория:", query.value(1).toString(), "Ед. изм.:", query.value(2).toString());
        }
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
        query.prepare("SELECT m.name, c.name, it.quantity, it.price, (it.quantity * it.price) "
                      "FROM inventory_transactions it "
                      "JOIN materials m ON it.material_id = m.id "
                      "JOIN categories c ON m.category_id = c.id "
                      "WHERE it.document_id = :id");
        query.bindValue(":id", m_targetId);

        if (query.exec()) {
            while (query.next()) {
                int row = ui->tw_report_data->rowCount();
                ui->tw_report_data->insertRow(row);
                double qty = qAbs(query.value(2).toDouble());
                double price = query.value(3).toDouble();
                double sum = qAbs(query.value(4).toDouble());

                ui->tw_report_data->setItem(row, 0, new QTableWidgetItem(query.value(0).toString()));
                ui->tw_report_data->setItem(row, 1, new QTableWidgetItem(query.value(1).toString()));
                ui->tw_report_data->setItem(row, 2, new QTableWidgetItem(QString::number(qty, 'f', 3)));
                ui->tw_report_data->setItem(row, 3, new QTableWidgetItem(QString::number(price, 'f', 2)));
                ui->tw_report_data->setItem(row, 4, new QTableWidgetItem(QString::number(sum, 'f', 2)));

                for(int i=2; i<=4; ++i) ui->tw_report_data->item(row, i)->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
                totalValue += sum;
            }
        }
        ui->lb_total_info->setText((m_type == IncomingDoc ? "ИТОГО ПО НАКЛАДНОЙ: " : "ИТОГО СПИСАНО: ") + QString::number(totalValue, 'f', 2));
        break;
    }

    case InventoryDoc: {
        query.prepare("SELECT c.name, m.name, u.name, it.expected_quantity, it.actual_quantity, "
                      "it.quantity, it.price, (it.quantity * it.price), it.reason "
                      "FROM inventory_transactions it "
                      "JOIN materials m ON it.material_id = m.id "
                      "JOIN categories c ON m.category_id = c.id "
                      "JOIN units u ON m.unit_id = u.id "
                      "WHERE it.document_id = :id");
        query.bindValue(":id", m_targetId);

        if (query.exec()) {
            while (query.next()) {
                int row = ui->tw_report_data->rowCount();
                ui->tw_report_data->insertRow(row);

                double diff = query.value(5).toDouble();
                double sumDiff = query.value(7).toDouble();
                QString reason = query.value(8).toString();

                ui->tw_report_data->setItem(row, 0, new QTableWidgetItem(query.value(0).toString()));
                ui->tw_report_data->setItem(row, 1, new QTableWidgetItem(query.value(1).toString()));
                ui->tw_report_data->setItem(row, 2, new QTableWidgetItem(query.value(2).toString()));
                ui->tw_report_data->setItem(row, 3, new QTableWidgetItem(QString::number(query.value(3).toDouble(), 'f', 3)));
                ui->tw_report_data->setItem(row, 4, new QTableWidgetItem(QString::number(query.value(4).toDouble(), 'f', 3)));
                ui->tw_report_data->setItem(row, 5, new QTableWidgetItem(QString::number(diff, 'f', 3)));
                ui->tw_report_data->setItem(row, 6, new QTableWidgetItem(QString::number(query.value(6).toDouble(), 'f', 2)));
                ui->tw_report_data->setItem(row, 7, new QTableWidgetItem(QString::number(sumDiff, 'f', 2)));
                ui->tw_report_data->setItem(row, 8, new QTableWidgetItem(reason.isEmpty() ? "-" : reason));

                if (diff < -0.0001) {
                    ui->tw_report_data->item(row, 5)->setForeground(Qt::red);
                    ui->tw_report_data->item(row, 7)->setForeground(Qt::red);
                } else if (diff > 0.0001) {
                    ui->tw_report_data->item(row, 5)->setForeground(QColor(0, 150, 0));
                    ui->tw_report_data->item(row, 7)->setForeground(QColor(0, 150, 0));
                }

                for(int i=3; i<=7; ++i) ui->tw_report_data->item(row, i)->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
                totalValue += sumDiff;
            }
        }
        ui->lb_total_info->setText("ИТОГОВЫЙ РЕЗУЛЬТАТ (РАЗНИЦА): " + QString::number(totalValue, 'f', 2));
        break;
    }

    case MaterialBatches: {
        query.prepare("SELECT b.incoming_date, s.name, b.current_quantity, b.purchase_price "
                      "FROM batches b LEFT JOIN suppliers s ON b.supplier_id = s.id "
                      "WHERE b.material_id = :id AND b.current_quantity > 0.0001 ORDER BY b.incoming_date ASC");
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
        query.prepare("SELECT d.doc_type, d.doc_date, d.id, it.quantity, u.login FROM inventory_transactions it "
                      "JOIN documents d ON it.document_id = d.id JOIN users u ON d.user_id = u.id "
                      "WHERE it.material_id = :id ORDER BY d.doc_date ASC");
        query.bindValue(":id", m_targetId);
        if (query.exec()) {
            while (query.next()) {
                int row = ui->tw_report_data->rowCount();
                ui->tw_report_data->insertRow(row);
                QString type = query.value(0).toString();
                double qty = query.value(3).toDouble();
                QString displayType;
                if (type == "incoming") { displayType = "Приход"; runningBalance += qty; }
                else if (type == "outgoing") { displayType = "Расход"; runningBalance -= qAbs(qty); qty = -qAbs(qty); }
                else { displayType = "Инвент."; runningBalance += qty; }
                ui->tw_report_data->setItem(row, 0, new QTableWidgetItem(displayType));
                ui->tw_report_data->setItem(row, 1, new QTableWidgetItem(query.value(1).toDateTime().toString("dd.MM.yy HH:mm")));
                ui->tw_report_data->setItem(row, 2, new QTableWidgetItem(QString("Док. №%1").arg(query.value(2).toInt())));
                QTableWidgetItem *iQty = new QTableWidgetItem(QString::number(qty, 'f', 3));
                QTableWidgetItem *iBal = new QTableWidgetItem(QString::number(runningBalance, 'f', 3));
                iQty->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
                iBal->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
                if (qty < -0.0001) iQty->setForeground(Qt::red);
                else if (qty > 0.0001) iQty->setForeground(QColor(0, 120, 0));
                ui->tw_report_data->setItem(row, 3, iQty);
                ui->tw_report_data->setItem(row, 4, iBal);
                ui->tw_report_data->setItem(row, 5, new QTableWidgetItem(query.value(4).toString()));
            }
        }
        ui->lb_total_info->setText(QString("ТЕКУЩИЙ ОСТАТОК: %1").arg(QString::number(runningBalance, 'f', 3)));
        break;
    }
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
    QString defaultName = ui->lb_report_title->text().replace(" ", "_") + "_" + QDateTime::currentDateTime().toString("dd_MM_yyyy");
    QString fileName = QFileDialog::getSaveFileName(this, "Сохранить как PDF", defaultName, "PDF файлы (*.pdf)");

    if (fileName.isEmpty()) return;
    if (QFileInfo(fileName).suffix().isEmpty()) fileName += ".pdf";

    QString html = "<html><head><style>"
                   "body { margin: 0; padding: 0; font-family: sans-serif; }"
                   "table { border-collapse: collapse; width: 100%; }"
                   "th, td { border: 1px solid black; padding: 8px; text-align: left; font-size: 10pt; }"
                   "th { background-color: #f2f2f2; }"
                   ".header { text-align: center; font-size: 18pt; font-weight: bold; margin: 0; padding: 0; }"
                   ".info-table { border: none; margin: 0; }"
                   ".info-table td { border: none; padding: 2px; }"
                   ".total { text-align: left; font-size: 13pt; font-weight: bold; margin-top: 15px; }"
                   ".description { margin: 0; font-size: 11pt; }"
                   "</style></head><body>";

    html += "<div class='header'>" + ui->lb_report_title->text() + "</div>";

    html += "<br>";

    html += "<table class='info-table'>";
    if (ui->lb_key_1->isVisible())
        html += QString("<tr><td><b>%1</b> %2</td></tr>").arg(ui->lb_key_1->text(), ui->lb_val_1->text());
    if (ui->lb_key_2->isVisible())
        html += QString("<tr><td><b>%1</b> %2</td></tr>").arg(ui->lb_key_2->text(), ui->lb_val_2->text());
    if (ui->lb_key_3->isVisible())
        html += QString("<tr><td><b>%1</b> %2</td></tr>").arg(ui->lb_key_3->text(), ui->lb_val_3->text());
    html += "</table>";

    if (ui->lb_description->isVisible()) {
        html += "<div class='description'><b>" + ui->lb_description->text() + "</b></div>";
        html += "<br>";
    }

    html += "<table><thead><tr>";
    for (int i = 0; i < ui->tw_report_data->columnCount(); ++i) {
        html += "<th>" + ui->tw_report_data->horizontalHeaderItem(i)->text() + "</th>";
    }
    html += "</tr></thead><tbody>";

    for (int row = 0; row < ui->tw_report_data->rowCount(); ++row) {
        html += "<tr>";
        for (int col = 0; col < ui->tw_report_data->columnCount(); ++col) {
            QString text = ui->tw_report_data->item(row, col) ? ui->tw_report_data->item(row, col)->text() : "";
            html += "<td>" + text + "</td>";
        }
        html += "</tr>";
    }
    html += "</tbody></table>";
    html += "<br>";

    html += "<div class='total'>" + ui->lb_total_info->text() + "</div>";

    html += "</body></html>";

    QTextDocument doc;
    doc.setHtml(html);

    QPrinter printer(QPrinter::PrinterResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setPageSize(QPageSize(QPageSize::A4));
    printer.setOutputFileName(fileName);

    printer.setPageMargins(QMarginsF(0, 0, 0, 0));

    doc.print(&printer);

    QMessageBox::information(this, "Успех", "Отчёт успешно сохранён в PDF!");
}

void reportwidget::exportCsv()
{
    QString defaultName = ui->lb_report_title->text().replace(" ", "_") + "_" + QDateTime::currentDateTime().toString("dd_MM_yyyy");
    QString fileName = QFileDialog::getSaveFileName(this, "Сохранить для Excel (CSV)", defaultName, "CSV файлы (*.csv)");

    if (fileName.isEmpty()) return;
    if (QFileInfo(fileName).suffix().isEmpty()) fileName += ".csv";

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "Ошибка", "Не удалось создать файл для записи.");
        return;
    }

    QTextStream out(&file);

    QString sep = ";";

    out.setGenerateByteOrderMark(true);

    out << ui->lb_report_title->text() << "\n\n";

    if (ui->lb_key_1->isVisible())
        out << ui->lb_key_1->text() << sep << ui->lb_val_1->text() << "\n";
    if (ui->lb_key_2->isVisible())
        out << ui->lb_key_2->text() << sep << ui->lb_val_2->text() << "\n";
    if (ui->lb_key_3->isVisible())
        out << ui->lb_key_3->text() << sep << ui->lb_val_3->text() << "\n";

    if (ui->lb_description->isVisible()) {
        QString desc = ui->lb_description->text().replace("\n", " ");
        out << desc << "\n";
    }
    out << "\n";

    for (int i = 0; i < ui->tw_report_data->columnCount(); ++i) {
        out << ui->tw_report_data->horizontalHeaderItem(i)->text() << (i == ui->tw_report_data->columnCount() - 1 ? "" : sep);
    }
    out << "\n";

    for (int row = 0; row < ui->tw_report_data->rowCount(); ++row) {
        for (int col = 0; col < ui->tw_report_data->columnCount(); ++col) {
            QString cellText = ui->tw_report_data->item(row, col) ? ui->tw_report_data->item(row, col)->text() : "";

            out << "\"" << cellText << "\"" << (col == ui->tw_report_data->columnCount() - 1 ? "" : sep);
        }
        out << "\n";
    }

    out << "\n" << ui->lb_total_info->text() << "\n";

    file.close();

    QMessageBox::information(this, "Успех", "Данные успешно экспортированы в CSV.\nВы можете открыть этот файл напрямую через Excel.");
}

void reportwidget::exportTxt()
{
    QString defaultName = ui->lb_report_title->text().replace(" ", "_") + "_" + QDateTime::currentDateTime().toString("dd_MM_yyyy");
    QString fileName = QFileDialog::getSaveFileName(this, "Сохранить как текст", defaultName, "Текстовые файлы (*.txt)");

    if (fileName.isEmpty()) return;
    if (QFileInfo(fileName).suffix().isEmpty()) fileName += ".txt";

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "Ошибка", "Не удалось создать файл.");
        return;
    }

    QTextStream out(&file);
    out.setEncoding(QStringConverter::Utf8);

    out << "============================================================\n";
    out << ui->lb_report_title->text().toUpper() << "\n";
    out << "============================================================\n\n";

    if (ui->lb_key_1->isVisible())
        out << ui->lb_key_1->text() << " " << ui->lb_val_1->text() << "\n";
    if (ui->lb_key_2->isVisible())
        out << ui->lb_key_2->text() << " " << ui->lb_val_2->text() << "\n";
    if (ui->lb_key_3->isVisible())
        out << ui->lb_key_3->text() << " " << ui->lb_val_3->text() << "\n";

    if (ui->lb_description->isVisible()) {
        out << "\n" << ui->lb_description->text() << "\n";
    }
    out << "\n";

    QList<int> columnWidths;
    for (int col = 0; col < ui->tw_report_data->columnCount(); ++col) {
        int maxW = ui->tw_report_data->horizontalHeaderItem(col)->text().length();

        for (int row = 0; row < ui->tw_report_data->rowCount(); ++row) {
            QString cellText = ui->tw_report_data->item(row, col) ? ui->tw_report_data->item(row, col)->text() : "";
            if (cellText.length() > maxW) maxW = cellText.length();
        }
        columnWidths.append(maxW + 3);
    }

    for (int col = 0; col < ui->tw_report_data->columnCount(); ++col) {
        QString headerText = ui->tw_report_data->horizontalHeaderItem(col)->text();
        out << headerText.leftJustified(columnWidths[col], ' ');
    }
    out << "\n";

    for (int w : columnWidths) out << QString("-").repeated(w - 1) << " ";
    out << "\n";

    for (int row = 0; row < ui->tw_report_data->rowCount(); ++row) {
        for (int col = 0; col < ui->tw_report_data->columnCount(); ++col) {
            QString cellText = ui->tw_report_data->item(row, col) ? ui->tw_report_data->item(row, col)->text() : "";
            out << cellText.leftJustified(columnWidths[col], ' ');
        }
        out << "\n";
    }

    out << "\n" << QString("-").repeated(40) << "\n";
    out << ui->lb_total_info->text() << "\n";
    out << QString("-").repeated(40) << "\n";

    file.close();

    QMessageBox::information(this, "Успех", "Отчёт сохранён в текстовый файл.");
}
