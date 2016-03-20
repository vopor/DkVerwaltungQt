#ifndef DBFKTS_H
#define DBFKTS_H

#include <qglobal.h>
#include <QVariant>

QT_BEGIN_NAMESPACE
class QString;
class QSettings;
QT_END_NAMESPACE

bool createConnection(const QString &dbName);

QString getStandardPath();
QString getSettingsFile();
QSettings &getSettings();
QString getFilePathFromIni(const QString &iniEntry, const QString &filePath, const QString &fileName);

QVariant getVariantValue(const QString &statement);
int getIntValue(const QString &statement);
double getDoubleValue(const QString &statement);
int getMaxId(const QString &tableName, const QString &fieldName);

#endif // DBFKTS_H

