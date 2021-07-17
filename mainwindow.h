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
    void importDKV2IntoDkVerwaltungQt();
    void DKV2ToDkVerwaltungQt();
    void DkVerwaltungQtToDKV2();
    void exportDkVerwaltungQtToCsv();
    void exportDKV2ToCsv();
    void showAnsparrechner();
    void showStatistik();
    void showPdfFile();
    void about();

};

#endif // MAINWINDOW_H
