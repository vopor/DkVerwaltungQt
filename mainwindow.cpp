#include "mainwindow.h"

#include "mainform.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
   setCentralWidget(new MainForm(this));
}
