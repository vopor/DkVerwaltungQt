#include "mainwindow.h"

#include "mainform.h"
#include "ansparrechner.h"

#include "dbfkts.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QAction>
#include <QMenu>
#include <QMenuBar>
#include <QDir>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
   setWindowTitle("DkVerwaltungQt");
   mainForm = new MainForm(this);
   setCentralWidget(mainForm);
   createMenu();
}

void MainWindow::createMenu()
{
    QMenu *dateiMenu = new QMenu("Datei");
    dateiMenu->addAction("Datenbank öffnen...", this, &MainWindow::openDatabase);
    dateiMenu->addAction("Datenbank anonymisieren", this, &MainWindow::anonymizeDatabase);
    dateiMenu->addAction("Ansparrechner", this, &MainWindow::showAnsparrechner);
    dateiMenu->addSeparator();
    dateiMenu->addAction("Beenden", this, &MainWindow::close);
    menuBar()->addMenu(dateiMenu);
}

void MainWindow::openDatabase()
{
    QString dbPath = getStandardPath() + QDir::separator() + "DkVerwaltungQt.db3";
    QString oldDbPath = getStringFromIni("DBPath", dbPath);
    dbPath = QFileDialog::getOpenFileName(this, QStringLiteral("Datenbank öffnen"), oldDbPath);
    if(!dbPath.isEmpty())
    {
        closeConnection(dbPath);
        if (openConnection(dbPath))
        {
            setStringToIni("DBPath", dbPath);
            setWindowTitle("DkVerwaltungQt - [" + dbPath + "]");
            // mainForm->updateViews();
            MainForm *oldMainForm = mainForm;
            oldMainForm->hide();
            mainForm = new MainForm(this);
            // mainForm->setWindowTitle("DkVerwaltungQt");
            setCentralWidget(mainForm);
            oldMainForm->deleteLater();
            oldMainForm = nullptr;
        }else{
            if (!openConnection(oldDbPath)){
                QMessageBox::warning(0, QStringLiteral("Fehler"), QStringLiteral("Weder ") + dbPath + " noch " + oldDbPath + " können geöffnet werden!");
            }
        }
    }
    qDebug() << "dbPath from ini: " << dbPath;
}

void MainWindow::anonymizeDatabase()
{
    int mbr = QMessageBox::warning(0, QStringLiteral("Datenbank anonymisieren?"),
                                   QStringLiteral("Soll die Datenbank anonymisiert werden?"), QMessageBox::Yes | QMessageBox::No);
    if(mbr == QMessageBox::Yes)
    {
        ::anonymizeDatabase();
        mainForm->updateViews();
    }
}

void MainWindow::showAnsparrechner()
{
   Ansparrechner *ansparrechner = new Ansparrechner(this);
   ansparrechner->exec();
}
