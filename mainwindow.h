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
public:
    explicit MainWindow(QWidget *parent = nullptr);

signals:

private slots:
    void openDatabase();
    void anonymizeDatabase();
    void importCsv();
    void importOds();
    void importDKV2();
    void exportCsv();
    void exportDKV2Csv();
    void showAnsparrechner();
    void showStatistik();
    void showPdfFile();

};

#endif // MAINWINDOW_H
