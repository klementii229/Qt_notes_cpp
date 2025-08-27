#include "main.h"
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    MainApp a(argc, argv);
    return a.exec();
}

MainApp::MainApp(int argc, char *argv[]) : QApplication(argc, argv) {
    mainwindow = new class MainWindow;
    mainwindow->show();
}

MainApp::~MainApp() {
    delete mainwindow;
}
