#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "pch.h"

using namespace Qt;
//буду писать комментарии разработчику, что будет смотреть это
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(QCoreApplication::applicationDirPath() + "/../../database.db");

    if (!db.open()) {
        QMessageBox::critical(this, "Ошибка открытия бд", db.lastError().text());
        qApp->closeAllWindows();
    } else {
        qDebug() << "База открыта!";
    }// тут сделал окошко, что б в случа его ошибки не в терминал вылазили, а условному пользователю
    this->setWindowIcon(QIcon(":/icons/icon.png"));
    selected_date.setDate(2025, 8, 26); //захардкожено для стартовой заметки
    show_notes();
}

MainWindow::~MainWindow()
{
    delete ui;
    db.close();
}


void MainWindow::on_actionShowChart_triggered()
{

    QLineSeries *series = new QLineSeries();
    series->append(0, 6);
    series->append(2, 4);
    series->append(3, 8);
    series->append(7, 4);
    series->append(10, 5);


    QChart *chart = new QChart();
    chart->legend()->hide();
    chart->addSeries(series);
    chart->createDefaultAxes();
    chart->setTitle("Пример линейного графика");


    QChartView *chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);


    QDialog dlg(this);
    dlg.setWindowTitle("График");
    QVBoxLayout *lay = new QVBoxLayout(&dlg);
    lay->addWidget(chartView);
    dlg.resize(500, 300);
    dlg.exec();
}
void MainWindow::on_actionSelectDay_triggered()
{
    QDialog dialog(this);
    dialog.setWindowTitle("Выбор даты");

    QCalendarWidget *calendar = new QCalendarWidget(&dialog);

    QPushButton *btnOk = new QPushButton("Принять", &dialog);
    QPushButton *btnCancel = new QPushButton("Отменить", &dialog);

    QHBoxLayout *hLayout = new QHBoxLayout;
    hLayout->addStretch();
    hLayout->addWidget(btnOk);
    hLayout->addWidget(btnCancel);

    QVBoxLayout *layout = new QVBoxLayout(&dialog);
    layout->addWidget(calendar);
    layout->addLayout(hLayout);

    QObject::connect(btnOk, &QPushButton::clicked, &dialog, &QDialog::accept);
    QObject::connect(btnCancel, &QPushButton::clicked, &dialog, &QDialog::reject);

    if (dialog.exec() == QDialog::Accepted) {
        selected_date = calendar->selectedDate();
        qDebug() << "Выбрана дата:" << selected_date.toString("dd.MM.yyyy");
        show_notes();
    }
}
void MainWindow::on_actionAddMetric_triggered() {
    QDialog dialog(this);
    dialog.setWindowTitle("Добавить тип метрики");

    QLabel *labelName = new QLabel("Название метрики:", &dialog);
    QLineEdit *editName = new QLineEdit(&dialog);

    QPushButton *btnOk = new QPushButton("Принять", &dialog);
    QPushButton *btnCancel = new QPushButton("Отменить", &dialog);

    QHBoxLayout *btnLayout = new QHBoxLayout;
    btnLayout->addStretch();
    btnLayout->addWidget(btnOk);
    btnLayout->addWidget(btnCancel);

    QFormLayout *formLayout = new QFormLayout;
    formLayout->addRow(labelName, editName);

    QVBoxLayout *mainLayout = new QVBoxLayout(&dialog);
    mainLayout->addLayout(formLayout);
    mainLayout->addLayout(btnLayout);

    QObject::connect(btnOk, &QPushButton::clicked, &dialog, &QDialog::accept);
    QObject::connect(btnCancel, &QPushButton::clicked, &dialog, &QDialog::reject);

    if (dialog.exec() == QDialog::Accepted) {
        QString name = editName->text().trimmed();
        if (name.isEmpty()) {
            QMessageBox::warning(this, "Ошибка", "Название метрики не может быть пустым!");
            return;
        }

        QSqlQuery insert;
        insert.prepare("INSERT INTO metric_types(name) VALUES(:name)");
        insert.bindValue(":name", name);

        if (!insert.exec()) {
            QMessageBox::critical(this, "Ошибка добавления", insert.lastError().text());
        } else {
            QMessageBox::information(this, "Успех", "Метрика добавлена!");
        }
    }
}
void MainWindow::on_actionDeleteNote_triggered() {
    const QString d = selected_date.toString(Qt::ISODate);
    QSqlQuery q;
    q.prepare("SELECT id FROM days WHERE date = :date");
    q.bindValue(":date", d);

    int dayId = -1;
    if (q.exec() && q.next()) {
        dayId = q.value(0).toInt();
    }
    if (dayId == -1) {
        qDebug() << "Нет такой даты:" << d;
        return;
    }

    QSqlQuery del;
    del.prepare("DELETE FROM notes WHERE day_id = :day");
    del.bindValue(":day", dayId);

    if (!del.exec()) {
        qDebug() << "Ошибка удаления заметки:" << del.lastError().text();
    } else {
        qDebug() << "Заметка удалена для даты" << d;
        ui->title->clear();
        ui->textEdit->clear();
    }
}
void MainWindow::on_actionSaveNote_triggered()
{
    save_notes();
}

void MainWindow::show_notes()
{
    {
        QSqlQuery q;
        q.prepare(R"(
            SELECT n.title, n.text
            FROM days d
            LEFT JOIN notes n ON n.day_id = d.id
            WHERE d.date = :date
        )");
        q.bindValue(":date", selected_date);

        QString title, body;
        if (q.exec() && q.next()) {
            title = q.value(0).toString();
            body  = q.value(1).toString();
        }
        ui->title->setText(title.isEmpty() ? "Title" : title);
        ui->textEdit->setPlainText(body);
    }
    //МЕТРИКИ
    {
        ui->tableWidget->clear();
        ui->tableWidget->setColumnCount(2);
        ui->tableWidget->setHorizontalHeaderLabels({"Метрика","Оценка"});
        ui->tableWidget->setRowCount(0);

        QSqlQuery q;
        q.prepare(R"(
            SELECT mt.name AS metric, m.value
            FROM metrics m
            JOIN metric_types mt ON mt.id = m.type_id
            JOIN days d          ON d.id  = m.day_id
            WHERE d.date = :date
            ORDER BY mt.name
        )");
        q.bindValue(":date", selected_date);

        if (!q.exec()) {
            qDebug() << "SQL error (metrics):" << q.lastError().text();
            return;
        }

        int row = 0;
        while (q.next()) {
            ui->tableWidget->insertRow(row);
            ui->tableWidget->setItem(row, 0, new QTableWidgetItem(q.value(0).toString()));
            ui->tableWidget->setItem(row, 1, new QTableWidgetItem(q.value(1).toString()));
            row++;
        }
        ui->tableWidget->verticalHeader()->setVisible(false);
        ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    }
    ui->tableWidget->verticalHeader()->setVisible(false);
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    //ВАЖНО
    // разрешаю редактирование только второй колонки (оценки)
    ui->tableWidget->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::SelectedClicked);
}


void MainWindow::save_notes() {
    const QString d = selected_date.toString(Qt::ISODate);

    QSqlQuery q;
    q.prepare("INSERT OR IGNORE INTO days(date) VALUES(:date)");
    q.bindValue(":date", d);
    q.exec();
    q.prepare("SELECT id FROM days WHERE date = :date");
    q.bindValue(":date", d);
    int dayId = -1;
    if (q.exec() && q.next())
        dayId = q.value(0).toInt();

    if (dayId == -1) {
        qDebug() << "No day id";
        return;
    }
    QSqlQuery up;
    up.prepare(R"(
        INSERT INTO notes(day_id, title, text)
        VALUES (:day, :title, :text)
        ON CONFLICT(day_id) DO UPDATE SET
            title = excluded.title,
            text  = excluded.text
    )");
    up.bindValue(":day",   dayId);
    up.bindValue(":title", ui->title->text());
    up.bindValue(":text",  ui->textEdit->toPlainText());
    if (!up.exec())
        qDebug() << "SQL error (save note):" << up.lastError().text();

    //сохраняем метрики
    for (int row = 0; row < ui->tableWidget->rowCount(); ++row) {
        auto *item = ui->tableWidget->item(row, 0);
        if (!item) continue; // защита от пустой строки

        QString metricName = item->text();
        if (metricName.isEmpty()) continue;

        QWidget *w = ui->tableWidget->cellWidget(row, 1);
        QSpinBox *spin = qobject_cast<QSpinBox*>(w);
        int value = (spin ? spin->value() : 5);

        // проверка диапазона (1..10)
        if (value < 1) value = 1;
        if (value > 10) value = 10;

        QSqlQuery tq;
        tq.prepare("SELECT id FROM metric_types WHERE name = :name");
        tq.bindValue(":name", metricName);
        int typeId = -1;
        if (tq.exec() && tq.next())
            typeId = tq.value(0).toInt();

        if (typeId == -1) {
            qDebug() << "No metric type for" << metricName;
            continue;
        }

        QSqlQuery mq;
        mq.prepare(R"(
            INSERT INTO metrics(day_id, type_id, value)
            VALUES(:day, :type, :value)
            ON CONFLICT(day_id, type_id) DO UPDATE SET
                value = excluded.value
        )");
        mq.bindValue(":day", dayId);
        mq.bindValue(":type", typeId);
        mq.bindValue(":value", value);

        if (!mq.exec())
            qDebug() << "SQL error (save metric):" << mq.lastError().text();
    }
}



void MainWindow::on_addMetricBtn_clicked()
{
    QStringList types;
    QSqlQuery q("SELECT name FROM metric_types ORDER BY name");
    while (q.next())
        types << q.value(0).toString();

    if (types.isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "В базе нет типов метрик!");
        return;
    }

    bool ok = false;
    QString choice = QInputDialog::getItem(
        this, "Выбор метрики", "Метрика:", types, 0, false, &ok);

    if (!ok || choice.isEmpty())
        return;

    for (int row = 0; row < ui->tableWidget->rowCount(); ++row) {
        if (ui->tableWidget->item(row, 0)->text() == choice) {
            QMessageBox::information(this, "Внимание", "Эта метрика уже есть!");
            return;
        }
    }

    int row = ui->tableWidget->rowCount();
    ui->tableWidget->insertRow(row);

    ui->tableWidget->setItem(row, 0, new QTableWidgetItem(choice));

    QSpinBox *spin = new QSpinBox();
    spin->setRange(1, 10);
    spin->setValue(5); // стартовое значение
    ui->tableWidget->setCellWidget(row, 1, spin);
}

void MainWindow::on_delMetricBtn_clicked()
{
    int row = ui->tableWidget->currentRow();
    if (row < 0) {
        QMessageBox::information(this, "Удаление", "Выберите метрику для удаления!");
        return;
    }

    QString name = ui->tableWidget->item(row, 0)->text();
    ui->tableWidget->removeRow(row);
    const QString d = selected_date.toString(Qt::ISODate);

    QSqlQuery q;
    q.prepare("SELECT id FROM days WHERE date = :date");
    q.bindValue(":date", d);

    int dayId = -1;
    if (q.exec() && q.next())
        dayId = q.value(0).toInt();

    if (dayId == -1) {
        qDebug() << "Не найден день для даты:" << d;
        return;
    }
    QSqlQuery tq;
    tq.prepare("SELECT id FROM metric_types WHERE name = :name");
    tq.bindValue(":name", name);

    int typeId = -1;
    if (tq.exec() && tq.next())
        typeId = tq.value(0).toInt();

    if (typeId == -1) {
        qDebug() << "Не найден тип метрики для" << name;
        return;
    }
    QSqlQuery del;
    del.prepare("DELETE FROM metrics WHERE day_id = :day AND type_id = :type");
    del.bindValue(":day", dayId);
    del.bindValue(":type", typeId);

    if (!del.exec())
        qDebug() << "SQL error (delete metric):" << del.lastError().text();
    else
        qDebug() << "Метрика удалена:" << name;
}
