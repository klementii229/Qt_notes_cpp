#pragma once
#include <QMainWindow>
#include <qsqldatabase.h>
#include <QCalendarWidget>
QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_actionShowChart_triggered();
    void on_actionSelectDay_triggered();
    void on_actionAddMetric_triggered();
    void on_actionSaveNote_triggered();
    void on_actionDeleteNote_triggered();
    void on_addMetricBtn_clicked();
    void on_delMetricBtn_clicked();
private:
    Ui::MainWindow *ui;
    QSqlDatabase  db;
    QDate selected_date;
    void show_notes();
    void save_notes();
};
