#ifndef ANSPARRECHNER_H
#define ANSPARRECHNER_H

#include <QDialog>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>

class Ansparrechner : public QDialog
{
public:
    Ansparrechner(QWidget *parent = nullptr);

private slots:
    void compute();

private:
    QGridLayout *gridLayout;

    // Anfangskapital: 0
    QLabel *lSparrate;
    QLineEdit *eSparrate;
    // Sparintervall: monatlich (monatlich/viertejährig/halbjährig/jährig)
    // Einzahlungsart: nachschüssig=false/vorschüssig=true
    QLabel *lZinssatz;
    QLineEdit *eZinssatz;
    // Zinsperiode: jährig (monatlich/viertejährig/halbjährig/jährig)
    // Anpsarzeit: 1 Jahr
    QLabel *lEndkapital;
    QLineEdit *eEndkapital;

    QPushButton *computeButton;
};

#endif // ZINSRECHNER_H
