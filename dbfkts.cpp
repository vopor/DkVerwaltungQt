#include "dbfkts.h"

#include <QVariant>
#include <QSqlQuery>
#include <QSqlError>

#include <QtWidgets>

#include <QHostInfo> // requires QT += network

#include <QPrinter>

#include <QWebEngineView>

#include "html2pdfconverter.h"

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

void writeHtmlTextToHtmlFile(const QString &fileName, const QString &str)
{
    writeToFile(fileName, str);
}

void writeHtmlTextToPdfFile(const QString &htmlText, const QString &str)
{
    QMessageBox::warning(0, "Fehler beim Speichern!", "writeHtmlTextToPdfFile() ist nicht implementiert!");
}

void convertHtmlFileToPdfFile(const QString &fileNameHtml, const QString &fileNamePdf)
{
    Html2PdfConverter converter(fileNameHtml, fileNamePdf);
    converter.run();
}

void convertHtmlFileToPdfFileSimple(const QString &fileNameHtml, const QString &fileNamePdf)
{
    QFile htmlfile(fileNameHtml);
    if(htmlfile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QString htmlContent;
        QTextStream in(&htmlfile);
        htmlContent = in.readAll();

        QTextDocument *document = new QTextDocument();
        document->setHtml(htmlContent);

        QPrinter printer(QPrinter::HighResolution);
        printer.setOutputFormat(QPrinter::PdfFormat);
        printer.setOutputFileName(fileNamePdf);

        document->print(&printer);
        delete document;
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
    QString standardPath = QDir::toNativeSeparators(QCoreApplication::applicationDirPath()) + QDir::separator();
#ifdef Q_OS_MAC
    standardPath = standardPath.replace("DkVerwaltungQt.app/Contents/MacOS", "");
#endif
    return standardPath;
}

QString getResouresPath()
{
    QString resourcesPath = QDir::toNativeSeparators(QCoreApplication::applicationDirPath()) + QDir::separator();
#ifdef Q_OS_MAC
    resourcesPath = resourcesPath.replace("DkVerwaltungQt.app/Contents/MacOS", "DkVerwaltungQt.app/Contents/Resources");
#endif
    return resourcesPath;
}

QString getSettingsFile(){
    QString iniPath = getStandardPath();
    iniPath += /* QDir::separator() + */ QStringLiteral("DkVerwaltungQt.ini");
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
    // qDebug() << "userAndHostName" << userAndHostName;
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

QString getStringFromIni(const QString &key, const QString &defaultValue)
{
    QString value = defaultValue;
    QString userAndHostName = getUserAndHostName();
    getSettings().beginGroup(userAndHostName);
    if(getSettings().contains(key))
    {
        value = getSettings().value(key).toString();
    }
    getSettings().endGroup();
    return value;
}

void setStringToIni(const QString &key, const QString &value)
{
    QString userAndHostName = getUserAndHostName();
    getSettings().beginGroup(userAndHostName);
    // if(getSettings().contains(key))
    {
        getSettings().setValue(key, value);
    }
    getSettings().endGroup();
}

QString getThunderbirdPath()
{
    QString thunderbirdPath;
#if defined (Q_OS_MAC)
    thunderbirdPath = getStringFromIni("ThunderbirdPath", "/Applications/Thunderbird.app/Contents/MacOS/thunderbird");
#elif defined(Q_OS_WIN)
    thunderbirdPath = getStringFromIni("ThunderbirdPath", "C:\\Program Files\\Mozilla Thunderbird\\thunderbird.exe");
#elif defined(Q_OS_UNIX)
    thunderbirdPath = getStringFromIni("ThunderbirdPath", "/usr/lib/thunderbird/thunderbird");
#endif
    return thunderbirdPath;
}

QString getTestUserEmailAdress()
{
    QString testUserEmailAdress;
    testUserEmailAdress = getStringFromIni("TestUserEmailAdress", "");
    return testUserEmailAdress;
}

void setTestUserEmailAdress(const QString &testUserEmailAdress)
{
    setStringToIni("TestUserEmailAdress", testUserEmailAdress);
}

int getJahrFromIni()
{
   QString userAndHostName = getUserAndHostName();
   getSettings().beginGroup(userAndHostName);
   int jahr = getSettings().value("Jahr").toInt();
   if(jahr == 0){
      jahr = 20;
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

QString getJahresDkBestaetigungenPath()
{
   QString JahresDkBestaetigungenPath = getStandardPath() /* + QDir::separator() */ + "JahresDkBestaetigungen" + QString::number(2000 + getJahr());
   return JahresDkBestaetigungenPath;
}

QString getJahresDkZinsBescheinigungenPath()
{
   QString JahresDkZinsBescheinigungenPath = getStandardPath() /* + QDir::separator() */  + "JahresDkZinsBescheinigungen" + QString::number(2000 + getJahr());
   return JahresDkZinsBescheinigungenPath;
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

int getAnzTage(const QDate &dateFrom, const QDate &dateTo, bool inclLastDay)
{
   Q_ASSERT(dateFrom.year() == dateTo.year());
   int anzTage = getAnzTageJahr();
   if(dateFrom.isValid() && dateTo.isValid())
   {
      anzTage = (dateTo.month() - dateFrom.month()) * 30;
      anzTage += dateTo.day();
      anzTage -= dateFrom.day();
      anzTage -= ((dateTo.day() == 31) ? 1 : 0);
      if(inclLastDay) anzTage += 1; // da bis 31.12. und nicht 1.1. gerechnet wird
   }
   return anzTage;
}

int getAnzTageNew(const QDate &dateFrom, const QDate &dateTo, bool inclLastDay)
{
   Q_ASSERT(dateFrom.year() == dateTo.year());
   int anzTage = getAnzTageJahr();
   if(dateFrom.isValid() && dateTo.isValid())
   {
      anzTage = (dateTo.month() - dateFrom.month()) * 30;
      anzTage += dateTo.day();
      anzTage -= dateFrom.day();
      anzTage -= ((dateTo.day() == 31) ? 1 : 0);
      if((dateFrom.month() == 2) && (dateTo.month() != 2))
      {
         anzTage += 1;
         if(!QDate::isLeapYear(dateFrom.year())) anzTage++;
      }
      if((dateFrom.month() != 2) && (dateTo.month() == 2))
      {
         anzTage += 1;
         if(!QDate::isLeapYear(dateTo.year())) anzTage++;
      }
      if(inclLastDay) anzTage += 1;
   }
   return anzTage;
}

int getAnzTageZeitraumOld(const QDate &dateFrom, const QDate &dateTo)
{
    int anzTage = getAnzTageJahr();
    if( dateFrom.isValid() && dateTo.isValid() )
    {
       anzTage = 0;
       QDate startDate = dateFrom;
       QDate endDate = dateTo;
       while(startDate < endDate)
       {
          if(startDate.year() < endDate.year())
          {
             anzTage += getAnzTage(startDate, QDate(startDate.year(), 12, 31), true);
             startDate = QDate(startDate.year()+1, 1, 1);
          }else{
             anzTage += getAnzTage(startDate, endDate, false);
             break;
          }
       }
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

double computeDkZinsen(double Betrag, double Zinssatz, const QDate &dateFrom, const QDate &dateTo)
{
   int anzTage = getAnzTage(dateFrom, dateTo, true);
   double Zinsen = computeDkZinsen(Betrag, Zinssatz, anzTage);
   return Zinsen;
}

double computeDkZinsenZeitraumOld(double Betrag, double Zinssatz, const QDate &dateFrom, const QDate &dateTo)
{
    double Zinsen = 0;
    if( dateFrom.isValid() && dateTo.isValid() )
    {
       int anzTage = 0;
       QDate startDate = dateFrom;
       QDate endDate = dateTo;
       while(startDate < endDate)
       {
          if(startDate.year() < endDate.year())
          {
             anzTage = getAnzTageZeitraum(startDate, QDate(startDate.year()+1, 1, 1));
             startDate = QDate(startDate.year()+1, 1, 1);
             Zinsen += computeDkZinsen(Betrag + Zinsen, Zinssatz, anzTage);
          }else{
             anzTage = getAnzTageZeitraum(startDate, endDate);
             Zinsen += computeDkZinsen(Betrag + Zinsen, Zinssatz, anzTage);
             break;
          }
       }
    }
    return Zinsen;
}

bool IsLastDayOfFebruary(const QDate &date)
{
    return date.month() == 2 && date.day() == date.daysInMonth();
}

int Days360(const QDate &StartDate, const QDate &EndDate)
{
    int StartDay = StartDate.day();
    int StartMonth = StartDate.month();
    int StartYear = StartDate.year();
    int EndDay = EndDate.day();
    int EndMonth = EndDate.month();
    int EndYear = EndDate.year();

    if (StartDay == 31 || IsLastDayOfFebruary(StartDate))
    {
        StartDay = 30;
    }

    if (StartDay == 30 && EndDay == 31)
    {
        EndDay = 30;
    }

    return ((EndYear - StartYear) * 360) + ((EndMonth - StartMonth) * 30) + (EndDay - StartDay);
}

bool isLeapYear(int year)
{
    bool b = ((((year % 4) == 0) && ((year % 100) != 0)) || ((year % 400) == 0));
    return b;
}

int dateDiff360(int startDay, int startMonth, int startYear, int endDay, int endMonth, int endYear, bool methodUS)
{
    if (startDay == 31) {
        --startDay;
    } else if (methodUS && (startMonth == 2 && (startDay == 29 || (startDay == 28 && !isLeapYear(startYear))))) {
        startDay = 30;
    }
    if (endDay == 31) {
        if (methodUS && startDay != 30) {
            endDay = 1;
            if (endMonth == 12) {
                ++endYear;
                endMonth = 1;
            } else {
                ++endMonth;
            }
        } else {
            endDay = 30;
        }
    }
    return endDay + endMonth * 30 + endYear * 360 - startDay - startMonth * 30 - startYear * 360;
}

int dateDiff360(const QDate &StartDate, const QDate &EndDate, bool methodUS)
{
   int startDay = StartDate.day();
   int startMonth = StartDate.month();
   int startYear = StartDate.year();
   int endDay = EndDate.day();
   int endMonth = EndDate.month();
   int endYear = EndDate.year();
   int ret = dateDiff360(startDay, startMonth, startYear, endDay, endMonth, endYear, methodUS);
   return ret;
}

int getAnzTageZeitraum(const QDate &StartDate, const QDate &EndDate)
{
   // int ret = Days360(StartDate, EndDate);
   int ret = dateDiff360(StartDate, EndDate, false);
   return ret;
}

int getAnzTageUndJahreZeitraum(const QDate &dateFrom, const QDate &dateTo, int &tageBis, int &ganzeJahre, int &tageVon){
   int ret = 0;
   tageBis = 0;
   ganzeJahre = 0;
   tageVon = 0;
   if( dateFrom.isValid() && dateTo.isValid() && (dateFrom <= dateTo))
   {
       if(dateFrom.year() == dateTo.year())
       {
           tageBis = getAnzTageZeitraum(dateFrom, dateTo);
           return 0;
       }
      if(dateFrom.year() < dateTo.year())
      {
          tageBis = getAnzTageZeitraum(dateFrom, QDate(dateFrom.year(), 12, 31));
      }
      QDate startDate = dateFrom;
      QDate endDate = dateTo;
      startDate = QDate(startDate.year()+1, 1, 1);
      while(startDate <= endDate)
      {
         if(startDate.year() < endDate.year())
         {
            ganzeJahre++;
            startDate = QDate(startDate.year()+1, 1, 1);
         }else{
            tageVon = getAnzTageZeitraum(startDate, dateTo) + 1;
            break;
         }
      }
   }
   return ret;
}

double computeDkZinsenZeitraum(double Betrag, double Zinssatz, const QDate &dateFrom, const QDate &dateTo)
{
    double Zinsen = 0;
    if( dateFrom.isValid() && dateTo.isValid() && (dateFrom <= dateTo))
    {
        int tageBis = 0;
        int ganzeJahre = 0;
        int tageVon = 0;
        getAnzTageUndJahreZeitraum(dateFrom, dateTo, tageBis, ganzeJahre, tageVon);
        Zinsen += computeDkZinsen(Betrag + Zinsen, Zinssatz, tageBis);
        for(int i=0;i<ganzeJahre;i++)
        {
           // Zinsen += ((Betrag + Zinsen) * Zinssatz) / 100.0;
            Zinsen += (Betrag + Zinsen) * Zinssatz / 100.0;
        }
        Zinsen += computeDkZinsen(Betrag + Zinsen, Zinssatz, tageVon);
    }
    return Runden2(Zinsen);
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

