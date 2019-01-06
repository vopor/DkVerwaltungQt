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
QString escapeFileName(const QString &fileName);

//--------------------------------------------------------------
// DkVerwaltungQt-Funktionen
//--------------------------------------------------------------

bool createConnection(const QString &dbName);

QString getStandardPath();
QString getSettingsFile();
QSettings &getSettings();
QString getFilePathFromIni(const QString &iniEntry, const QString &filePath, const QString &fileName);
QString getOpenOfficePath();
QString getJahresDkBestaetigungenPath();
QString getJahresDkZinsBescheinigungenPath();
int getJahrFromIni();

int getJahr();
int getAnzTageJahr();
int getAnzTage(const QDate &dateFrom, const QDate &dateTo);

double Runden2(double Betrag);
double computeDkZinsen(double Betrag, double Zinssatz, int anzTage);

//--------------------------------------------------------------
// SQL-Funktionen
//--------------------------------------------------------------

QVariant getVariantValue(const QString &statement);
int getIntValue(const QString &statement);
double getDoubleValue(const QString &statement);
int getMaxId(const QString &tableName, const QString &fieldName);

#endif // DBFKTS_H

