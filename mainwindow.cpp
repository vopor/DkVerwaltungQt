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
#include <QStandardPaths>

#include <QApplication>
#include <QSqlQuery>

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
    dateiMenu->addAction("Datenbank anlegen...", this, &MainWindow::newDatabase);
    dateiMenu->addAction("Datenbank öffnen...", this, &MainWindow::openDatabase);
    dateiMenu->addAction("Datenbank anonymisieren", this, &MainWindow::anonymizeDatabase);
    dateiMenu->addSeparator();
    dateiMenu->addAction("Import Ods into DkVerwaltungQt...", this, &MainWindow::importOdsIntoDkVerwaltungQt);
    dateiMenu->addAction("Import Csv into DkVerwaltungQt...", this, &MainWindow::importCsvIntoDkVerwaltungQt);
    dateiMenu->addAction("Import DkVerwaltungQt into DkVerwaltungQt...", this, &MainWindow::importDkVerwaltungQtIntoDkVerwaltungQt);
    dateiMenu->addAction("Import DKV2 into DkVerwaltungQt...", this, &MainWindow::importDKV2IntoDkVerwaltungQt);
    // dateiMenu->addSeparator();
    // dateiMenu->addAction("Import Ods into DKV2...", this, &MainWindow::importDkVerwaltungQtIntoDkVerwaltungQt);
    // dateiMenu->addAction("Import Csv into DKV2...", this, &MainWindow::importDkVerwaltungQtIntoDkVerwaltungQt);
    // dateiMenu->addAction("Import DkVerwaltungQt into DKV2...", this, &MainWindow::importDkVerwaltungQtIntoDkVerwaltungQt);
    // dateiMenu->addAction("Import DKV2 into DKV2...", this, &MainWindow::importDkVerwaltungQtIntoDkVerwaltungQt);
    dateiMenu->addSeparator();
    dateiMenu->addAction("Export DkVerwaltungQt to Csv...", this, &MainWindow::exportDkVerwaltungQtToCsv);
    dateiMenu->addAction("Export DKV2 to Csv...", this, &MainWindow::exportDKV2ToCsv);
    dateiMenu->addSeparator();
    dateiMenu->addAction("DkVerwaltungQt -> DKV2", this, &MainWindow::DkVerwaltungQtToDKV2);
    dateiMenu->addAction("DKV2 -> DkVerwaltungQt", this, &MainWindow::DKV2ToDkVerwaltungQt);
    dateiMenu->addSeparator();
    dateiMenu->addAction("DkVerwaltungQt-Tables -> DKV2-Views", this, &MainWindow::DkVerwaltungQtTablesToDKV2Views);
    dateiMenu->addAction("DKV2-Views -> DkVerwaltungQt-Tables", this, &MainWindow::DKV2ViewsToDkVerwaltungQtTables);
    dateiMenu->addSeparator();
    dateiMenu->addAction("Compare DKV2 DkVerwaltungQt with DKV2", this, &MainWindow::compareDkVerwaltungQtWithDKV2);
    dateiMenu->addAction("Compare DKV2 with DkVerwaltungQt as Csv", this, &MainWindow::compareDKV2WithDkVerwaltungQtAsCsv);
    dateiMenu->addSeparator();
    dateiMenu->addAction("Statistik", this, &MainWindow::showStatistik);
    dateiMenu->addSeparator();
    dateiMenu->addAction("Ansparrechner", this, &MainWindow::showAnsparrechner);
    dateiMenu->addAction("Pdf-Datei anzeigen...", this, &MainWindow::showPdfFile);
    dateiMenu->addSeparator();
    dateiMenu->addAction("Beenden", this, &MainWindow::close);
    dateiMenu->addAction("Über...", this, &MainWindow::about);
    menuBar()->addMenu(dateiMenu);
}

void MainWindow::about()
{
    QMessageBox::information(this, "DkVerwaltungQt", "DkVerwaltungQt\n\nVersion 1.0");
}

bool MainWindow::reopenDatabase(const QString &dbPath)
{
    bool ret = false;
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
            ret = true;
        }else{
            QMessageBox::warning(this, QStringLiteral("Fehler"), QStringLiteral("Die Datenbank ") + dbPath + " kann nicht geöffnet werden!");
        }
    }
    return ret;
}

void MainWindow::newDatabase()
{
    QString dbPath = getStandardPath() + QDir::separator() + "DkVerwaltungQt.db3";
    QString oldDbPath = getStringFromIni("DBPath", dbPath);
    dbPath = QFileDialog::getSaveFileName(this, QStringLiteral("Datenbank anlegen"), oldDbPath, "sqlite3 (*.db3 *.dkdb)");
    if(!dbPath.isEmpty())
    {
        if (!reopenDatabase(dbPath))
        {
            if (!openConnection(oldDbPath))
            {
                QMessageBox::warning(this, QStringLiteral("Fehler"), QStringLiteral("Weder ") + dbPath + " noch " + oldDbPath + " können geöffnet werden!");
            }
        }
    }
    qDebug() << "dbPath from ini: " << dbPath;
}

void MainWindow::openDatabase()
{
    QString dbPath = getStandardPath() + QDir::separator() + "DkVerwaltungQt.db3";
    QString oldDbPath = getStringFromIni("DBPath", dbPath);
    dbPath = QFileDialog::getOpenFileName(this, QStringLiteral("Datenbank öffnen"), oldDbPath, "sqlite3 (*.db3 *.dkdb)");
    if(!dbPath.isEmpty())
    {
        if (!reopenDatabase(dbPath))
        {
            if (!openConnection(oldDbPath))
            {
                QMessageBox::warning(this, QStringLiteral("Fehler"), QStringLiteral("Weder ") + dbPath + " noch " + oldDbPath + " können geöffnet werden!");
            }
        }
    }
    qDebug() << "dbPath from ini: " << dbPath;
}

void MainWindow::anonymizeDatabase()
{
    int mbr = QMessageBox::warning(this, QStringLiteral("Datenbank anonymisieren?"),
                                   QStringLiteral("Soll die bestehende Datenbank anonymisiert werden?"), QMessageBox::Yes | QMessageBox::No);
    if(mbr == QMessageBox::Yes)
    {
        ::anonymizeDatabase();
        mainForm->updateViews();
    }
}

int MainWindow::executeCommand(const QString &commandLine)
{
    qDebug() << commandLine;
    QString stdOutput;
    QString stdError;
    int retCode = run_executeCommand(nullptr, commandLine, stdOutput, stdError);
    if(retCode != 0)
    {
        QString msg = QString("Fehler ") + "(" + QString::number(retCode) + ")" + ":\n\n";
        msg += stdError;
        msg += "\n\n";
        msg += "command:\n\n";
        msg += commandLine;
        QMessageBox::warning(this, QStringLiteral("Fehler"), msg);
    }
    else
    {
        QString msg = "Hinweis:\n\n";
        msg += stdOutput;
        QMessageBox::information(this, QStringLiteral("Hinweis"), msg);
    }
    return retCode;
}

void MainWindow::importCsvInternal(const QString & csvFilename)
{
    QString defaultDBPath = getStandardPath() + QDir::separator() + "DkVerwaltungQt.db3";
    QString dbPath = getStringFromIni("DBPath", defaultDBPath);
    closeConnection(dbPath);
    // QFile::remove(dbPath);
    QString sourcePath = getResouresPath();
    QString commandLine = sourcePath + QDir::separator() + "importCsvIntoDkVerwaltungQt.py" + " " + "\"" + csvFilename + "\"" + " " + "\"" + dbPath + "\"";
    int retCode = executeCommand(commandLine);
    if(retCode == 0)
    {
        reopenDatabase(dbPath);
    }
}

void MainWindow::importCsvIntoDkVerwaltungQt()
{
    QString defaultDBPath = getStandardPath() + QDir::separator() + "DkVerwaltungQt.db3";
    QString dbPath = getStringFromIni("DBPath", defaultDBPath);
    QString csvPath = QFileInfo(dbPath).dir().absolutePath();
    QString csvFilename = QFileDialog::getOpenFileName(this, QStringLiteral("Csv-Datei öffnen"), csvPath, "csv (*.csv)");
    if(!csvFilename.isEmpty() && !dbPath.isEmpty())
    {
        int mbr = QMessageBox::warning(this, QStringLiteral("Csv-Datei importieren?"), QStringLiteral("Sollen die aktuellen DkVerwaltungQt-Daten durch die Csv-Daten ersetzt werden?"), QMessageBox::Yes | QMessageBox::No);
        if(mbr != QMessageBox::Yes)
        {
            return;
        }
        importCsvInternal(csvFilename);
    }
}

// https://stackoverflow.com/questions/56529277/saving-specific-excel-sheet-as-csv
void MainWindow::importOdsIntoDkVerwaltungQt()
{
    QString defaultDBPath = getStandardPath() + QDir::separator() + "DkVerwaltungQt.db3";
    QString dbPath = getStringFromIni("DBPath", defaultDBPath);
    QString odsPath = QFileInfo(dbPath).dir().absolutePath();
    QString odsFilename = QFileDialog::getOpenFileName(this, QStringLiteral("Ods-Datei öffnen"), odsPath, "ods (*.ods)");
    if(!odsFilename.isEmpty() && !dbPath.isEmpty())
    {

        int mbr = QMessageBox::warning(this, QStringLiteral("Ods-Datei importieren?"), QStringLiteral("Sollen die aktuellen DkVerwaltungQt-Daten durch die ODS-Daten ersetzt werden?"), QMessageBox::Yes | QMessageBox::No);
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
            QMessageBox::warning(this, QStringLiteral("Fehler"), QStringLiteral("Die Csv-Datei ") + csvFilename + " kann nicht geöffnet werden!");
        }

    }
}

void MainWindow::importDkVerwaltungQtIntoDkVerwaltungQt()
{
    QString defaultDBPath = getStandardPath() + QDir::separator() + "DkVerwaltungQt.db3";
    QString dbPath = getStringFromIni("DBPath", defaultDBPath);
    QString db3Path = QFileInfo(dbPath).dir().absolutePath();
    QString db3Filename = QFileDialog::getOpenFileName(this, QStringLiteral("DkVerwaltungQt-Datenbank importieren"), db3Path, "db3 (*.db3)");
    if(!db3Filename.isEmpty() && !dbPath.isEmpty())
    {
        int mbr = QMessageBox::warning(this, QStringLiteral("DkVerwaltungQt-Datenbank importieren?"), QStringLiteral("Sollen die aktuellen DkVerwaltungQt-Daten durch die DkVerwaltungQt-Daten ersetzt werden?"), QMessageBox::Yes | QMessageBox::No);
        if(mbr != QMessageBox::Yes)
        {
            return;
        }
        closeConnection(dbPath);
        QString sourcePath = getResouresPath();
        QString commandLine = sourcePath + QDir::separator() + "importDkVerwaltungQtIntoDkVerwaltungQt.py" + " " + db3Filename + " " + dbPath;
        int retCode = executeCommand(commandLine);
        if(retCode == 0)
        {
            reopenDatabase(dbPath);
        }
    }

}

void MainWindow::importDKV2IntoDkVerwaltungQt()
{
    QString defaultDBPath = getStandardPath() + QDir::separator() + "DkVerwaltungQt.db3";
    QString dbPath = getStringFromIni("DBPath", defaultDBPath);
    QString dkdbPath = QFileInfo(dbPath).dir().absolutePath();
    QString dkdbFilename = QFileDialog::getOpenFileName(this, QStringLiteral("DKV2-Datenbank importieren"), dkdbPath, "dkdb (*.dkdb)");
    if(!dkdbFilename.isEmpty() && !dbPath.isEmpty())
    {
        int mbr = QMessageBox::warning(this, QStringLiteral("DKV2-Datenbank importieren?"), QStringLiteral("Sollen die aktuellen DkVerwaltungQt-Daten durch die DKV2-Daten ersetzt werden?"), QMessageBox::Yes | QMessageBox::No);
        if(mbr != QMessageBox::Yes)
        {
            return;
        }
        closeConnection(dbPath);
        QString sourcePath = getResouresPath();
        QString commandLine = sourcePath + QDir::separator() + "importDKV2IntoDkVerwaltungQt.py" + " " + dkdbFilename + " " + dbPath;
        int retCode = executeCommand(commandLine);
        if(retCode == 0)
        {
            reopenDatabase(dbPath);
        }
    }
}

void MainWindow::updateTablesInternal(const QString &scriptFilename)
{
    QString defaultDBPath = getStandardPath() + QDir::separator() + "DkVerwaltungQt.db3";
    QString dbPath = getStringFromIni("DBPath", defaultDBPath);
    closeConnection(dbPath);
    QString sourcePath = getResouresPath();
    QString commandLine = sourcePath + QDir::separator() + scriptFilename + " " + "\"" + dbPath + "\"" + " " + "\"" + dbPath + "\"";
    int retCode = executeCommand(commandLine);
    if(retCode == 0)
    {
        reopenDatabase(dbPath);
    }
}

void MainWindow::DKV2ToDkVerwaltungQt()
{
    if(!isDKV2Database())
    {
        QMessageBox::warning(this, QStringLiteral("Fehler"), QStringLiteral("Die Datenbank enthält keine DKV2 Tabellen."));
        return;
    }
    updateTablesInternal("importDKV2IntoDkVerwaltungQt.py");

}

void MainWindow::DkVerwaltungQtToDKV2()
{
    if(!isDKV2Database())
    {
        QMessageBox::warning(this, QStringLiteral("Fehler"), QStringLiteral("Die Datenbank enthält keine DKV2 Tabellen."));
        return;
    }
    updateTablesInternal("importDkVerwaltungQtIntoDKV2.py");
}

void MainWindow::DkVerwaltungQtTablesToDKV2Views()
{
    if(!isDKV2Database())
    {
        QMessageBox::warning(this, QStringLiteral("Fehler"), QStringLiteral("Die Datenbank enthält keine DKV2 Tabellen."));
        return;
    }
    QString defaultDBPath = getStandardPath() + QDir::separator() + "DkVerwaltungQt.db3";
    QString dbPath = getStringFromIni("DBPath", defaultDBPath);
    closeConnection(dbPath);
    openConnection(dbPath);
    QSqlQuery query;
    query.prepare("PRAGMA FOREIGN_KEYS = OFF");
    query.exec();
    displayLastSqlError();
    // // mainForm->closeViews();
    // MainForm *oldMainForm = mainForm;
    // oldMainForm->hide();
    // delete oldMainForm;
    // // oldMainForm->deleteLater();
    // oldMainForm = nullptr;
    // // QCoreApplication::removePostedEvents(nullptr, 0);
    // qApp->sendPostedEvents(0, QEvent::DeferredDelete);
    createDkVerwaltungQtViewsFromDKV2();
    // mainForm = new MainForm(this);
    // setCentralWidget(mainForm);
    // // mainForm->updateViews();
    reopenDatabase(dbPath);
}

void MainWindow::DKV2ViewsToDkVerwaltungQtTables()
{
    if(!isDKV2Database())
    {
        QMessageBox::warning(this, QStringLiteral("Fehler"), QStringLiteral("Die Datenbank enthält keine DKV2 Tabellen."));
        return;
    }
    QString defaultDBPath = getStandardPath() + QDir::separator() + "DkVerwaltungQt.db3";
    QString dbPath = getStringFromIni("DBPath", defaultDBPath);
    closeConnection(dbPath);
    openConnection(dbPath);
    dropDkVerwaltungQtViewsFromDKV2();
    QSqlQuery query;
    query.prepare("PRAGMA FOREIGN_KEYS = ON");
    query.exec();
    displayLastSqlError();
    reopenDatabase(dbPath);
}

void MainWindow::exportCsvInternal(const QString &scriptFilename, const QString &dbPath, const QString &csvFilename)
{
    QString sourcePath = getResouresPath();
    QString commandLine = sourcePath + QDir::separator() + scriptFilename + " " + "\"" + dbPath + "\"" + " " + "\"" + csvFilename + "\"";
    int retCode = executeCommand(commandLine);
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
        exportCsvInternal(scriptFilename, dbPath, csvFilename);
        if(QFileInfo(csvFilename).exists())
        {
            QString cmd = "open -e " + csvFilename;
            QProcess::startDetached(cmd);
        }else{
            QMessageBox::warning(this, QStringLiteral("Fehler"), QStringLiteral("Die Csv-Datei ") + csvFilename + " kann nicht geöffnet werden!");
        }
    }
}

void MainWindow::exportDkVerwaltungQtToCsv()
{
    exportCsvInternal("exportDkVerwaltungQtToCsv.py");
}

void MainWindow::exportDKV2ToCsv()
{
    if(!isDKV2Database())
    {
        QMessageBox::warning(this, QStringLiteral("Fehler"), QStringLiteral("Die Datenbank enthält keine DKV2 Tabellen."));
        return;
    }
    exportCsvInternal("exportDKV2ToCsv.py");
}

void MainWindow::compareDkVerwaltungQtWithDKV2()
{
    QString defaultDBPath = getStandardPath() + QDir::separator() + "DkVerwaltungQt.db3";
    QString dbPath = getStringFromIni("DBPath", defaultDBPath);
    QString sourcePath = getResouresPath();
    QString commandLine = sourcePath + QDir::separator() + "compareDkVerwaltungQtWithDKV2.py" + " "  + "\"" + dbPath  + "\"" + " "  + "\"" + dbPath  + "\"";
    int retCode = executeCommand(commandLine);
}

void MainWindow::compareDKV2WithDkVerwaltungQtAsCsv()
{
    QString defaultDBPath = getStandardPath() + QDir::separator() + "DkVerwaltungQt.db3";
    QString dbPath = getStringFromIni("DBPath", defaultDBPath);
    QString csvPath = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
    QString csvFilenameDKV2 = csvPath + QDir::separator() + "DKV2.csv";
    QString csvFilenameDkVerwaltungQt = csvPath + QDir::separator() + "DkVerwaltungQt.csv";

    exportCsvInternal("exportDKV2ToCsv.py", dbPath, csvFilenameDKV2);
    if(!QFileInfo(csvFilenameDKV2).exists())
    {
        QMessageBox::warning(this, QStringLiteral("Fehler"), QStringLiteral("Die Csv-Datei ") + csvFilenameDKV2 + " konnte nicht erzeugt werden!");
        return;
    }
    exportCsvInternal("exportDkVerwaltungQtToCsv.py", dbPath, csvFilenameDkVerwaltungQt);
    if(!QFileInfo(csvFilenameDkVerwaltungQt).exists())
    {
        QMessageBox::warning(this, QStringLiteral("Fehler"), QStringLiteral("Die Csv-Datei ") + csvFilenameDkVerwaltungQt + " konnte nicht erzeugt werden!");
        return;
    }
    QString program = getCompareToolPath();
    QStringList arguments;
    arguments << csvFilenameDKV2;
    arguments << csvFilenameDkVerwaltungQt;
    QProcess::startDetached(program, arguments);
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
