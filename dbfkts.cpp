#include "dbfkts.h"

#include <QVariant>
#include <QSqlQuery>
#include <QSqlError>

#include <QtWidgets>

#include <QHostInfo> // requires QT += network

#include "qdebug.h"

bool createConnection(const QString &dbName)
{
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(dbName);
    if (!db.open()) {
        QMessageBox::warning(0, QObject::tr("Database Error"),
                             db.lastError().text());
        return false;
    }
    return true;
}

QString getStandardPath()
{
//    QString homePath;
//    #if QT_VERSION >= 0x050000
//        homePath = QStandardPaths::writableLocation(QStandardPaths::DataLocation);
//    #else
//        homePath = QDesktopServices::storageLocation(QDesktopServices::DataLocation);
//    #endif
//    return homePath;
    QString standardPath = QDir::toNativeSeparators(QCoreApplication::applicationDirPath());
#ifdef Q_OS_MAC
    standardPath = standardPath.replace("DkVerwaltungQt.app/Contents/MacOS", "");
#endif
    return standardPath;
}

QString getSettingsFile(){
    QString iniPath = getStandardPath();
    iniPath += QDir::separator() + QStringLiteral("DkVerwaltungQt.ini");
    return iniPath;
}

QSettings &getSettings(){
   static QSettings settings(getSettingsFile(), QSettings::IniFormat);
   return settings;
}

// http://stackoverflow.com/questions/26552517/get-system-username-in-qt
QString getUserName(){
    QString userName;
#ifdef Q_OS_WIN
    userName = qgetenv("USERNAME");
#else
    userName = qgetenv("USER");
#endif
    return userName;
}

QString getComputerName(){
    QString computerName;
#ifdef Q_OS_WIN
    computerName = qgetenv("COMPUTERNAME");
#else
    computerName = qgetenv("HOSTNAME");
#endif
    return computerName;
}

QString getUserAndHostName(){
    QString userAndHostName;
    QString userName = getUserName();
    userAndHostName = userName ;
    QString localHostName;
    localHostName = QHostInfo::localHostName(); // requires QT += network
    // localHostName = getComputerName();
    if(localHostName.length()){
        localHostName = localHostName.split('.').at(0);
        userAndHostName += "_" + localHostName;
    }
    qDebug() << "userAndHostName" << userAndHostName;
    return userAndHostName;
}

QString getFilePathFromIni(const QString &iniEntry, const QString &filePath, const QString &fileName)
{
//    QString iniEntryOS = iniEntry;
//#ifdef Q_OS_MAC
//    iniEntryOS += "Mac";
//#elif Q_OS_WIN
//    iniEntryOS += "Win";
//#endif
    QString iniEntrySpecific = iniEntry;
    QString userAndHostName = getUserAndHostName();
    getSettings().beginGroup(userAndHostName);
    QString filePathFromIni = getSettings().value(iniEntrySpecific).toString();
    if(filePathFromIni.isEmpty()){
        filePathFromIni = filePath + QDir::separator() + fileName;
        if(!QFileInfo(filePathFromIni).exists()){
            filePathFromIni = QFileDialog::getOpenFileName(NULL, fileName + QStringLiteral(" öffnen"), filePathFromIni);
            if(filePathFromIni.isEmpty()){
                QMessageBox::warning(0, QStringLiteral("Fehler"), fileName + QStringLiteral(" nicht ausgewählt!"));
            }
        }
        getSettings().setValue(iniEntrySpecific, filePathFromIni);
    }
    getSettings().endGroup();
    return filePathFromIni;
}

//--------------------------------------------------------------

QVariant getVariantValue(const QString &statement)
{
    QSqlQuery query;
    query.prepare(statement);
    query.exec();
    QVariant variantValue;
    if (query.next()){
        variantValue = query.value(0);
    }else{
        qDebug() << "statement: " << statement;
        if (query.lastError().isValid())
            qDebug() << query.lastError();
    }
    return variantValue;
}

int getIntValue(const QString &statement){
    int intValue = -1;
    QVariant v = getVariantValue(statement);
    if(v.isValid()){
        intValue = v.toInt();
    }
    return intValue;
}

double getDoubleValue(const QString &statement){
    double doubleValue = -1;
    QVariant v = getVariantValue(statement);
    if(v.isValid()){
        doubleValue = v.toDouble();
    }
    return doubleValue;
}

int getMaxId(const QString &tableName, const QString &fieldName)
{
    QString statement = QString("SELECT MAX(%1) FROM %2").arg(fieldName).arg(tableName);
    int intValue = getIntValue(statement);
    return intValue;
}

//int getIntValue(const QString &statement){
//    QSqlQuery query;
//    query.prepare(statement);
//    query.exec();
//    int intValue = -1;
//    if (query.next()){
//        intValue = query.value(0).toInt();
//    }else{
//        qDebug() << "statement: " << statement;
//        if (query.lastError().isValid())
//            qDebug() << query.lastError();
//    }
//    return intValue;
//}

//int getMaxId(const QString &tableName, const QString &fieldName)
//{
//    QSqlQuery query;
//    query.prepare(QString("SELECT MAX(%1) FROM %2").arg(fieldName).arg(tableName));
//    query.exec();
//    int maxId = 0;
//    if (query.next()) {
//        maxId = query.value(0).toInt();
//    }
//    return maxId;
//}
