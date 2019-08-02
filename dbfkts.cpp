#include "dbfkts.h"

#include <QVariant>
#include <QSqlQuery>
#include <QSqlError>

#include <QtWidgets>

#include <QHostInfo> // requires QT += network

#include "qdebug.h"

//--------------------------------------------------------------
// Allgemeine Funktionen
//--------------------------------------------------------------

QString readFromFile(const QString &fileName)
{
    QString ret;
    QFile file(fileName);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QTextStream in(&file);
        in.setCodec("UTF-8");
        // while(!in.atEnd())
        // {
        //    QString line = in.readLine();
        // }
        ret = in.readAll();
    }
    return ret;
}

void writeToFile(const QString &fileName, const QString &str)
{
    QFile file(fileName);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QTextStream out(&file);
        out.setCodec("UTF-8");
        out << str;
    }else{
        QMessageBox::warning(0, "Fehler beim Speichern!", fileName + " konnte nicht gespeichert werden!");
    }
}

QString escapeFileName(const QString &fileName)
{
    QString ret = fileName;
    ret = ret.replace(" ", "_");
    ret = ret.replace("ä", "ae");
    ret = ret.replace("ö", "oe");
    ret = ret.replace("ü", "ue");
    ret = ret.replace("ß", "ss");
    ret = ret.replace("/", "+");
    return ret;
}
//--------------------------------------------------------------
// DkVerwaltungQt-Funktionen
//--------------------------------------------------------------

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

QString getOpenOfficePath()
{
    QString oo;
#if defined (Q_OS_MAC)
     // oo = "/Applications/OpenOffice.app/Contents/MacOS/soffice";
    oo = getFilePathFromIni("OOPath", "/Applications/OpenOffice.app/Contents/MacOS/", "soffice");
#elif defined(Q_OS_WIN)
    // oo = "C:\\Program Files\\OpenOffice 4\\program\\soffice.exe";
    oo = getFilePathFromIni("OOPath", "C:\\Program Files\\OpenOffice 4\\program\\", "soffice.exe");
#endif
    return oo;
}

QString getJahresDkBestaetigungenPath()
{
   QString JahresDkBestaetigungenPath = getStandardPath() + QDir::separator() + "JahresDkBestaetigungen" + QString::number(2000 + getJahr());
   return JahresDkBestaetigungenPath;
}

QString getJahresDkZinsBescheinigungenPath()
{
   QString JahresDkZinsBescheinigungenPath = getStandardPath() + QDir::separator() + "JahresDkZinsBescheinigungen" + QString::number(2000 + getJahr());
   return JahresDkZinsBescheinigungenPath;
}

int getJahrFromIni()
{
   QString userAndHostName = getUserAndHostName();
   getSettings().beginGroup(userAndHostName);
   int jahr = getSettings().value("Jahr").toInt();
   if(jahr == 0){
      jahr = 18;
      getSettings().setValue("Jahr", jahr);
   }
   getSettings().endGroup();
   return jahr;
}

int getJahr()
{
   static int jahr = 0;
   if(jahr == 0)
   {
      jahr = getJahrFromIni();
   }
   return jahr;
}

int getAnzTageJahr()
{
   return 360;
}

void datediff()
{
QString source = "20090512";
QDate sourceDate = QDate::fromString(source, "yyyyMMdd");

qint64 daysBetween = QDate::currentDate().toJulianDay() - sourceDate.toJulianDay();
QDate difference = QDate::fromJulianDay(daysBetween);

QDate firstDate = QDate::fromJulianDay(0);

int years = difference.year() - firstDate.year();
int months = difference.month() - firstDate.month();
int days = difference.day() - firstDate.day();

// TODO: check if a number is negative and if so, normalize

qDebug() << QDate::currentDate().toString("yy-MM-dd");
qDebug() << sourceDate.toString("yy-MM-dd");
qDebug() << daysBetween;
qDebug() << QString("%1 years, %2 months and %3 days").arg(years).arg(months).arg(days);
}

// Zinstage = oFuncAcc.callFunction( "DAYS360", Array(dBeginn, dEnde, 1))
// 359=TAGE360(DATUM(2016;1;1); DATUM(2016;12;31);1)
// 4: - European method, 12 months of 30 days each (30E/360)
// If either day1 or day2 is 31, it is changed to 30. Each month is now assumed to have 30 days, and the result calculated.

// DATEDIFF(MONTH, @dt_Start, @dt_Ende) * 30
//    + DAY(@dt_Ende)
//    - DAY(@dt_Start)
//    - CASE WHEN DAY(@dt_Ende) = 31 THEN 1 ELSE 0 END AS [Tage30/360]

int getAnzTage(const QDate &dateFrom, const QDate &dateTo)
{
   Q_ASSERT(dateFrom.year() == dateTo.year());
   int anzTage = getAnzTageJahr();
   if(dateFrom.isValid() && dateTo.isValid())
   {
      // anzTage = (int)dateFrom.daysTo(dateTo)+1;
      anzTage = (dateTo.month() - dateFrom.month()) * 30;
      anzTage += dateTo.day();
      anzTage -= dateFrom.day();
      anzTage -= ((dateTo.day() == 31) ? 1 : 0);
      anzTage += 1; // da bis 31.12. und nicht 1.1. gerechnet wird
      // anzTage = qMin(anzTage, getAnzTageJahr());
      // Q_ASSERT(anzTage <= getAnzTageJahr());
   }
   return anzTage;
}

double Runden2(double Betrag)
{
   int temp = qRound(Betrag * 100);
   double ret = ((double)temp / 100);
   return ret;
}

double computeDkZinsen(double Betrag, double Zinssatz, int anzTage)
{
    double Zinsen = ((Betrag * Zinssatz) / 100.0);
    Zinsen = Runden2(Zinsen * anzTage / getAnzTageJahr());
    return Zinsen;
}

//--------------------------------------------------------------
// SQL-Funktionen
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

QString getStringValue(const QString &statement){
   QString stringValue = "";
   QVariant v = getVariantValue(statement);
   if(v.isValid()){
       stringValue = v.toString();
   }
   return stringValue;
}

int getMaxId(const QString &tableName, const QString &fieldName)
{
    QString statement = QString("SELECT MAX(%1) FROM %2").arg(fieldName).arg(tableName);
    int intValue = getIntValue(statement);
    return intValue;
}
