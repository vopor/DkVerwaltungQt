#include "ansparrechner.h"
#include "dbfkts.h"

#include <math.h>

#include <QDoubleValidator>

double computeZinsenProMonat(double ZinsenProJahr)
{
    double value = (1 + ZinsenProJahr/100);
    double rootvalue = (1.0/12.0);
    double ZinsenProMonat = 100 * (pow(value, rootvalue) - 1);
    return ZinsenProMonat;
}

double computeMonatszinsfaktor(double ZinsenProMonat)
{
    double Monatszinsfaktor = 1 + (ZinsenProMonat / 100);
    return Monatszinsfaktor;
}

double computeEndkapital(double Sparrate, bool Einzahlungsart, double Zinssatz)
{
   double ZinsenProMonat = computeZinsenProMonat(Zinssatz);
   double Monatszinsfaktor = computeMonatszinsfaktor(ZinsenProMonat);
   double Endkapital = Sparrate * ( (pow(Monatszinsfaktor, 12) - 1) / (Monatszinsfaktor - 1));
   if(Einzahlungsart)
   {
       Endkapital *= Monatszinsfaktor;
   }
   return Endkapital;
}

// https://www.zinsen-berechnen.de/sparrechner.php
Ansparrechner::Ansparrechner(QWidget *parent) : QDialog(parent)
{
   gridLayout = new QGridLayout(this);

   // Anfangskapital: 0
   lSparrate = new QLabel("Sparrate");
   eSparrate = new QLineEdit(this);
   eSparrate->setValidator(new QDoubleValidator(eSparrate));
   // Sparintervall: monatlich (monatlich/viertejährig/halbjährig/jährig)
   // Einzahlungsart: nachschüssig=false/vorschüssig=true
   lZinssatz = new QLabel("Zinssatz");
   eZinssatz = new QLineEdit(this);
   eZinssatz->setValidator(new QDoubleValidator(eZinssatz));
   // Zinsperiode: jährig (monatlich/viertejährig/halbjährig/jährig)
   // Anpsarzeit: 1 Jahr
   lEndkapital = new QLabel("Endkapital", this);
   eEndkapital = new QLineEdit(this);
   eEndkapital->setReadOnly(true);
   computeButton = new QPushButton("Berechnen", this);
   connect(computeButton, &QPushButton::clicked, this, &Ansparrechner::compute);

   int row = 0;
   gridLayout->addWidget(lSparrate, row, 0);
   gridLayout->addWidget(eSparrate, row++, 1);
   gridLayout->addWidget(lZinssatz, row, 0);
   gridLayout->addWidget(eZinssatz, row++, 1);
   gridLayout->addWidget(lEndkapital, row, 0);
   gridLayout->addWidget(eEndkapital, row++, 1);
   gridLayout->addWidget(computeButton, row++, 1);

}

void Ansparrechner::compute()
{
    // double Anfangskapital = 0;
    double Sparrate = eSparrate->text().replace(",", ".").toDouble();
    // Sparintervall: monatlich (monatlich/viertejährig/halbjährig/jährig)
    bool Einzahlungsart = true; // nachschüssig=false/vorschüssig=true
    double Zinssatz = eZinssatz->text().replace(",", ".").toDouble();;
    // Zinsperiode: jährig (monatlich/viertejährig/halbjährig/jährig)
    // int Anpsarzeit = 1; // (1 Jahr)
    double Endkapital = 0;

    Endkapital = computeEndkapital(Sparrate, Einzahlungsart, Zinssatz);

    QString buf;
    buf.sprintf("%lf", Endkapital);
    eEndkapital->setText(buf);

}
