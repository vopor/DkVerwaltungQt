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
    void importCsvInternal(const QString & csvFilename);
public:
    explicit MainWindow(QWidget *parent = nullptr);

signals:

private slots:
    void openDatabase();
    void anonymizeDatabase();
    void importCsv();
    void importOds();
    void exportCsv();
    void importDKV2();
    void showAnsparrechner();
    void showStatistik();
    void showPdfFile();

};

#endif // MAINWINDOW_H
