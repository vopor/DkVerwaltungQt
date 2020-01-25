#include "mainwindow.h"

#include "mainform.h"
#include "ansparrechner.h"

#include <QAction>
#include <QMenu>
#include <QMenuBar>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
   mainForm = new MainForm(this);
   setCentralWidget(mainForm);
   createMenu();
}

void MainWindow::createMenu()
{
    QMenu *dateiMenu = new QMenu("Datei");
    dateiMenu->addAction("Ansparrechner", this, &MainWindow::showAnsparrechner);
    dateiMenu->addSeparator();
    dateiMenu->addAction("Beenden", this, &MainWindow::close);
    menuBar()->addMenu(dateiMenu);
}

void MainWindow::showAnsparrechner()
{
   Ansparrechner *ansparrechner = new Ansparrechner(this);
   ansparrechner->exec();
}
