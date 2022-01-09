#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class MainForm;
class MainWindow : public QMainWindow
{
    Q_OBJECT
private:
    MainForm *mainForm;
private:
    void createMenu();
    void importCsvInternal(const QString &csvFilename);
    void exportCsvInternal(const QString &scriptFilename);
    void exportCsvInternal(const QString &scriptFilename, const QString &dbPath, const QString &csvFilename);
    void updateTablesInternal(const QString &scriptFilename);
    bool reopenDatabase(const QString &dbPath);
    int  executeCommand(const QString &commandLine);
public:
    explicit MainWindow(QWidget *parent = nullptr);

signals:

private slots:
    void openDatabase();
    void anonymizeDatabase();
    void importCsvIntoDkVerwaltungQt();
    void importOdsIntoDkVerwaltungQt();
    void importDkVerwaltungQtIntoDkVerwaltungQt();
    void importDKV2IntoDkVerwaltungQt();
    void exportDkVerwaltungQtToCsv();
    void exportDKV2ToCsv();
    void DKV2ToDkVerwaltungQt();
    void DkVerwaltungQtToDKV2();
    void DkVerwaltungQtTablesToDKV2Views();
    void DKV2ViewsToDkVerwaltungQtTables();
    void compareDkVerwaltungQtWithDKV2();
    void compareDKV2WithDkVerwaltungQtAsCsv();
    void showAnsparrechner();
    void showStatistik();
    void showPdfFile();
    void about();

};

#endif // MAINWINDOW_H
