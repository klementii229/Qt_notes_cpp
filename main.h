#pragma once
#include <QApplication>
class MainWindow;
class MainApp : public QApplication {
    Q_OBJECT
public:
    MainApp(int argc, char *argv[]);
    ~MainApp();
private:
    MainWindow * mainwindow;
};
