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
#include <QProcess>

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
    dateiMenu->addSeparator();
    dateiMenu->addAction("Import Csv...", this, &MainWindow::importCsv);
    dateiMenu->addAction("Import Ods...", this, &MainWindow::importOds);
    dateiMenu->addAction("Import DKV2...", this, &MainWindow::importDKV2);
    dateiMenu->addSeparator();
    dateiMenu->addAction("Export Csv...", this, &MainWindow::exportCsv);
    dateiMenu->addAction("Export DKV2 Csv...", this, &MainWindow::exportDKV2Csv);
    dateiMenu->addSeparator();
    dateiMenu->addAction("Statistik", this, &MainWindow::showStatistik);
    dateiMenu->addSeparator();
    dateiMenu->addAction("Ansparrechner", this, &MainWindow::showAnsparrechner);
    dateiMenu->addAction("Pdf-Datei anzeigen...", this, &MainWindow::showPdfFile);
    dateiMenu->addSeparator();
    dateiMenu->addAction("Beenden", this, &MainWindow::close);
    menuBar()->addMenu(dateiMenu);
}

void MainWindow::openDatabase()
{
    QString dbPath = getStandardPath() + QDir::separator() + "DkVerwaltungQt.db3";
    QString oldDbPath = getStringFromIni("DBPath", dbPath);
    dbPath = QFileDialog::getOpenFileName(this, QStringLiteral("Datenbank öffnen"), oldDbPath, "sqlite3 (*.db3 *.dkdb)");
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
                                   QStringLiteral("Soll die bestehende Datenbank anonymisiert werden?"), QMessageBox::Yes | QMessageBox::No);
    if(mbr == QMessageBox::Yes)
    {
        ::anonymizeDatabase();
        mainForm->updateViews();
    }
}

void MainWindow::importCsvInternal(const QString & csvFilename)
{
    QString defaultDBPath = getStandardPath() + QDir::separator() + "DkVerwaltungQt.db3";
    QString dbPath = getStringFromIni("DBPath", defaultDBPath);
    closeConnection(dbPath);
    QFile::remove(dbPath);
    QString sourcePath = getResouresPath();
    QString commandLine = sourcePath + QDir::separator() + "importCsvIntoDkVerwaltungQt.py" + " " + csvFilename + " " + dbPath;
    qDebug() << commandLine;
    QString stdOutput;
    QString stdError;
    run_executeCommand(nullptr, commandLine, stdOutput, stdError);
    if (openConnection(dbPath))
    {
        MainForm *oldMainForm = mainForm;
        oldMainForm->hide();
        mainForm = new MainForm(this);
        setCentralWidget(mainForm);
        oldMainForm->deleteLater();
        oldMainForm = nullptr;
    }else{
        QMessageBox::warning(0, QStringLiteral("Fehler"), QStringLiteral("Die Datenbank ") + dbPath + " kann nicht geöffnet werden!");
    }

}

void MainWindow::importCsv()
{
    QString defaultDBPath = getStandardPath() + QDir::separator() + "DkVerwaltungQt.db3";
    QString dbPath = getStringFromIni("DBPath", defaultDBPath);
    QString csvPath = QFileInfo(dbPath).dir().absolutePath();
    QString csvFilename = QFileDialog::getOpenFileName(this, QStringLiteral("Csv-Datei öffnen"), csvPath, "csv (*.csv)");
    if(!csvFilename.isEmpty() && !dbPath.isEmpty())
    {
        int mbr = QMessageBox::warning(0, QStringLiteral("Csv-Datei importieren?"),
                                       QStringLiteral("Soll die bestehende Datenbank aus der Csv-Datei neu erzeugt werden?"), QMessageBox::Yes | QMessageBox::No);
        if(mbr != QMessageBox::Yes)
        {
            return;
        }
        importCsvInternal(csvFilename);
    }
}

// https://stackoverflow.com/questions/56529277/saving-specific-excel-sheet-as-csv
void MainWindow::importOds()
{
    QString defaultDBPath = getStandardPath() + QDir::separator() + "DkVerwaltungQt.db3";
    QString dbPath = getStringFromIni("DBPath", defaultDBPath);
    QString odsPath = QFileInfo(dbPath).dir().absolutePath();
    QString odsFilename = QFileDialog::getOpenFileName(this, QStringLiteral("Ods-Datei öffnen"), odsPath, "ods (*.ods)");
    if(!odsFilename.isEmpty() && !dbPath.isEmpty())
    {
        int mbr = QMessageBox::warning(0, QStringLiteral("Ods-Datei importieren?"),
                                       QStringLiteral("Soll die bestehende Datenbank aus der Ods-Datei neu erzeugt werden?"), QMessageBox::Yes | QMessageBox::No);
        if(mbr != QMessageBox::Yes)
        {
            return;
        }
        QString oo = getOpenOfficePath();
        // QString macro = "vnd.sun.star.script:DkVerwaltungQt.Module1.FelderFuellen?language=Basic&location=application";
        QString macro = "macro:///DkVerwaltungQt.Module1.ConvertSheet(" + odsFilename + ", Journal)";
        QString program = oo;
        QStringList arguments;
        arguments.append("-headless");
        arguments.append(macro);
        QProcess::execute(oo, arguments); // synchron
        QString csvFilename = odsFilename;
        csvFilename = csvFilename.replace(".ods", ".Journal.csv");
        if(QFileInfo(csvFilename).exists())
        {
            importCsvInternal(csvFilename);
        }
        else
        {
            QMessageBox::warning(0, QStringLiteral("Fehler"), QStringLiteral("Die Csv-Datei ") + csvFilename + " kann nicht geöffnet werden!");
        }

    }
}

void MainWindow::exportCsvInternal(const QString &scriptFilename)
{
    QString defaultDBPath = getStandardPath() + QDir::separator() + "DkVerwaltungQt.db3";
    QString dbPath = getStringFromIni("DBPath", defaultDBPath);
    QString csvPath = QFileInfo(dbPath).dir().absolutePath();
    QString csvFilename = dbPath;
    csvFilename = csvFilename + ".csv";
    csvFilename = QFileDialog::getSaveFileName(this, QStringLiteral("Csv-Datei exportieren"), csvFilename, "csv (*.csv)");
    if(csvFilename.length())
    {
        QString sourcePath = getResouresPath();
        QString commandLine = sourcePath + QDir::separator() + scriptFilename + " " + dbPath + " " + csvFilename;
        qDebug() << commandLine;
        QString stdOutput;
        QString stdError;
        run_executeCommand(nullptr, commandLine, stdOutput, stdError);
        if(QFileInfo(csvFilename).exists())
        {
            QString cmd = "open -e " + csvFilename;
            QProcess::startDetached(cmd);
        }else{
            QMessageBox::warning(0, QStringLiteral("Fehler"), QStringLiteral("Die Csv-Datei ") + csvFilename + " kann nicht geöffnet werden!");
        }
    }
}

void MainWindow::exportCsv()
{
    exportCsvInternal("exportDkVerwaltungQtToCsv.py");
}

void MainWindow::exportDKV2Csv()
{
    if(!isDKV2Database())
    {
        QMessageBox::warning(0, QStringLiteral("Fehler"), QStringLiteral("Die Datenbank enthält keine DKV2 Tabellen."));
        return;
    }
    exportCsvInternal("exportDKV2ToCsv.py");
}

void MainWindow::importDKV2()
{
    QString defaultDBPath = getStandardPath() + QDir::separator() + "DkVerwaltungQt.db3";
    QString dbPath = getStringFromIni("DBPath", defaultDBPath);
    QString dkdbPath = QFileInfo(dbPath).dir().absolutePath();
    QString dkdbFilename = QFileDialog::getOpenFileName(this, QStringLiteral("DKV2-Datenbank importieren"), dkdbPath, "dkdb (*.dkdb)");
    if(!dkdbFilename.isEmpty() && !dbPath.isEmpty())
    {
        int mbr = QMessageBox::warning(0, QStringLiteral("DKV2-Datenbank importieren?"),
                                       QStringLiteral("Sollen die aktuellen Daten durch die DKV2-Daten ersetzt werden?"), QMessageBox::Yes | QMessageBox::No);
        if(mbr != QMessageBox::Yes)
        {
            return;
        }
        closeConnection(dbPath);
        QString sourcePath = getResouresPath();
        QString commandLine = sourcePath + QDir::separator() + "importDKV2IntoDkVerwaltungQt.py" + " " + dkdbFilename + " " + dbPath;
        qDebug() << commandLine;
        QString stdOutput;
        QString stdError;
        run_executeCommand(nullptr, commandLine, stdOutput, stdError);
        if (openConnection(dbPath))
        {
            MainForm *oldMainForm = mainForm;
            oldMainForm->hide();
            mainForm = new MainForm(this);
            setCentralWidget(mainForm);
            oldMainForm->deleteLater();
            oldMainForm = nullptr;
        }else{
            QMessageBox::warning(0, QStringLiteral("Fehler"), QStringLiteral("Die Datenbank ") + dbPath + " kann nicht geöffnet werden!");
        }
    }
}

void MainWindow::showAnsparrechner()
{
   Ansparrechner *ansparrechner = new Ansparrechner(this);
   ansparrechner->exec();
}

void MainWindow::showStatistik()
{
    QString statementYear2 = "SELECT MAX(SUBSTR(Datum ,7,2)) FROM DkBuchungen";
    QString strYear = getStringValue(statementYear2);

    QString statement;
    statement = "SELECT ROUND(SUM(Betrag),2) FROM DkBuchungen WHERE DkNummer <> 'Stammkapital' ";
    statement += "AND (Rueckzahlung = '' OR Rueckzahlung IS  NULL);";
    double SummeDkEnde = getDoubleValue(statement);

    statement = "SELECT (ROUND(SUM(Betrag),2) * -1) FROM DkBuchungen WHERE DkNummer <> 'Stammkapital' AND NOT (Rueckzahlung = '' OR Rueckzahlung IS  NULL)  AND Betrag < 0 ";
    statement += "AND SUBSTR(Datum ,7,2) = '" + strYear + "';";
    double SummeDkAbgaenge = getDoubleValue(statement);

    statement = "SELECT SUM( replace(Betrag,',','.') ) FROM DkBuchungen WHERE CAST(replace(Betrag,',','.') AS FLOAT) > 0 ";
    // statement += "AND ((substr(Datum ,7,2) || substr(Datum ,4,2)|| substr(Datum ,1,2)) > '" + strYear + "0101');";
    statement += "AND ((substr(Anfangsdatum ,7,2) || substr(Anfangsdatum ,4,2)|| substr(Anfangsdatum ,1,2)) >= '" + strYear + "0101');";
    double SummeDkZugaenge = getDoubleValue(statement);

    double SummeDkAnfang = SummeDkEnde + SummeDkAbgaenge - SummeDkZugaenge;

    statement = "SELECT SUM( replace(Betrag,',','.') ) FROM DkBuchungen WHERE CAST(replace(Betrag,',','.') AS FLOAT) > 0 ";
    statement += "AND ((substr(vorgemerkt ,7,2) || substr(vorgemerkt ,4,2)|| substr(vorgemerkt ,1,2)) >= '" + strYear + "0101') ";
    statement += "AND ((substr(vorgemerkt ,7,2) || substr(vorgemerkt ,4,2)|| substr(vorgemerkt ,1,2)) <= '" + strYear + "1231'); ";
    double SummeDkVorgemerkt = getDoubleValue(statement);

    QString title = "Statistik 20" + strYear;

    QString statistik;

    statistik += title;
    statistik += "\n\n";

    statistik += "Summe der Direktkredite Anfang:\n";
    statistik += QString::number(SummeDkAnfang, 'f', 2);
    statistik += "\n\n";

    statistik += "Durchschnittlicher Zinssattz:\n";
    statistik += getStringValue("SELECT ROUND(AVG(Zinssatz),2) FROM DkBuchungen WHERE DkNummer <> 'Stammkapital';");
    statistik += "\n\n";

    statistik += "Summe der gekündigten Direktkredite:\n";
    statistik += QString::number(SummeDkAbgaenge, 'f', 2);
    statistik += "\n\n";

    statistik += "Summe der neu eingeworbenen Direktkredite:\n";
    statistik += QString::number(SummeDkZugaenge, 'f', 2);
    statistik += "\n\n";

    statistik += "Summe der Direktkredite Ende:\n";
    statistik += QString::number(SummeDkEnde, 'f', 2);
    statistik += "\n\n";

    statistik += "Summe der vorgemerkten Direktkredite:\n";
    statistik += QString::number(SummeDkVorgemerkt, 'f', 2);
    statistik += "\n\n";

    QMessageBox::information(this, title, statistik);
}

void MainWindow::showPdfFile()
{
    QString fileName = QFileDialog::getOpenFileName(nullptr, QObject::tr("Open Image"), QDir::homePath(), QObject::tr("PDF Files (*.pdf *.*)"));
    if(fileName.isEmpty())
        return;
    ::showPdfFile(fileName);
}
