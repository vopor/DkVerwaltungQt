#ifndef DBFKTS_H
#define DBFKTS_H

#include <qglobal.h>
#include <QVariant>

QT_BEGIN_NAMESPACE
class QString;
class QSettings;
QT_END_NAMESPACE



//--------------------------------------------------------------
// Allgemeine Funktionen
//--------------------------------------------------------------

QString readFromFile(const QString &fileName);
void writeToFile(const QString &fileName, const QString &str);
void writeHtmlToPdf(const QString& Filename, const QString& html);

QString escapeFileName(const QString &fileName);

//--------------------------------------------------------------
// DkVerwaltungQt-Funktionen
//--------------------------------------------------------------
bool CreateDatabase(const QString filename);
bool askForDatabase();
bool isValidDb(const QString file);
void BackupDatabase();
bool findDatabase_wUI();
bool asureDatabase_wUI();

QString getOpenOfficePath();
int getYearFromIni();

QString getYearOfCalculation_wUI();
const int AnzTageJahr = 360;
int getAnzTage(const QDate &dateFrom, const QDate &dateTo);

double Runden2(double Betrag);
double computeDkZinsen(double Betrag, double Zinssatz, int anzTage);

//--------------------------------------------------------------
// SQL-Funktionen
//--------------------------------------------------------------

QVariant getVariantValue(const QString &statement);
int getIntValue(const QString &statement);
double getDoubleValue(const QString &statement);
QString getStringValue(const QString &statement);
int getMaxId(const QString &tableName, const QString &fieldName);

#endif // DBFKTS_H

