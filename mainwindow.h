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
public:
    explicit MainWindow(QWidget *parent = nullptr);

signals:

private slots:
    void showAnsparrechner();

};

#endif // MAINWINDOW_H
