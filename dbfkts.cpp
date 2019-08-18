#include "dbfkts.h"

#include <QVariant>
#include <QSqlQuery>
#include <QSqlError>
#include <QtWidgets>
#include <QHostInfo> // requires QT += network
#include <qdebug.h>
#include <QDate>
#include <QPushButton>
#include <QtPrintSupport>

#include "appconfiguration.h"

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
        QMessageBox::warning(nullptr, "Fehler beim Speichern!", fileName + " konnte nicht gespeichert werden!");
    }
}

void writeHtmlToPdf(const QString& Filename, const QString& html)
{
    QPrinter printer(QPrinter::PrinterResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setPaperSize(QPrinter::A4);
    printer.setOutputFileName(Filename);

    QTextDocument td;
    td.setHtml(html);
    td.print(&printer);
}

QString escapeFileName(const QString &fileName)
{
    QString ret = fileName;
    ret = ret.replace(" ", "_");
    ret = ret.replace("*", "+");
    ret = ret.replace("?", "-");
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

class dbCloser
{   // for use on the stack only
public:
    dbCloser (QSqlDatabase* d){Db=d;}
    dbCloser() = delete;
    ~dbCloser(){Db->close();}
private:
    QSqlDatabase* Db;
};

bool CreateDatabase(const QString filename)
{
    if( QFile(filename).exists())
    {
        QFile::remove(filename +".old");
        if( !QFile::rename(filename, filename +".old"))
            return false;
    }
    bool ret = true;
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(filename);
    ret &= db.open();
    if( !ret) return ret;

    dbCloser c(&db);
    QSqlQuery q;
    ret &= q.exec("CREATE TABLE DKPersonen (PersonId INTEGER PRIMARY KEY AUTOINCREMENT, Vorname TEXT, Name TEXT, Anrede TEXT, Straße TEXT, PLZ TEXT, Ort TEXT, Email TEXT);");
    ret &= q.exec("CREATE TABLE DKBuchungen (BuchungId INTEGER PRIMARY KEY AUTOINCREMENT, PersonId INTEGER, Datum TEXT, DKNr TEXT, DKNummer TEXT, Rueckzahlung TEXT, vorgemerkt TEXT, Betrag REAL, Zinssatz REAL, Bemerkung TEXT, FOREIGN KEY(PersonId) REFERENCES DKPerson(PersonId))");
    ret &= q.exec("CREATE TABLE DKZinssaetze (Zinssatz REAL PRIMARY KEY, Beschreibung TEXT)");
    for (double zins=0; zins < 2.; zins+=0.1)
    {
        QString sql (QString("INSERT INTO DKZinssaetze VALUES (") + QString().setNum(zins) + ", '')");
        ret &= q.exec(sql);
    }
    return ret;
}

bool isValidDb(const QString filename)
{
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(filename);
    if( !db.open())
        return false;

    dbCloser c(&db);

    QSqlQuery sqlp("SELECT * FROM DKPersonen");
    if( !sqlp.exec())
        return false;
    QSqlQuery sqlb("SELECT * FROM DKBuchungen");
    if( !sqlb.exec())
        return false;
    QSqlQuery sqlz("SELECT * FROM DKZinssaetze");
    if( !sqlz.exec())
        return false;

    return true;
}

bool askForDatabase()
{
    do
    {
        QString file = QFileDialog::getOpenFileName(nullptr, "DkVerarbeitungs Datenbank", pAppConfig->getWorkdir(), "*.db3", nullptr);
        if( file.isEmpty())
        {
            qDebug() <<  "User pressed cancel in File selection dlg" << endl;
            return false;
        }
        if( QFile(file).exists() && isValidDb(file))
        {
            pAppConfig->setDb( file);
            return true;
        }
        else {
            qDebug() << "invalid db3 file selected - retry" << endl;
        }
    }while(true);
}

bool overwrite_copy(QString from, QString to)
{
    qDebug() << from << " to " << to << endl;
    if( QFile().exists(to))
        QFile().remove(to);
    return QFile().copy(from, to);
}

void BackupDatabase()
{
    QList<QString> backupnames;
    for(int i = 0; i<10; i++)
    {
        QString nbr;
        QTextStream ts(&nbr); ts.setFieldWidth(2); ts.setPadChar('0');
        ts << i;
        QString backupextension (QString("-"+ nbr + ".db3bak"));
        QString name = pAppConfig->getDb().replace(".db3", backupextension);
        backupnames.append(name);
    }
    QFile().remove(backupnames[9]);
    for(int i = 8; i>=0; i--)
    {
        if( !QFile().exists(backupnames[i]))
            continue;
        if( !overwrite_copy(backupnames[i], backupnames[i+1]))
            qDebug() << "Backup copy failed" << endl;
    }
    if( !overwrite_copy(pAppConfig->getDb(), backupnames[0]))
        qDebug() << "Backup copy failed" << endl;
}

bool createConnection()
{
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(pAppConfig->getDb());
    if (!db.open()) {
        QMessageBox::warning(nullptr, QObject::tr("Database Error"),
                             db.lastError().text());
        return false;
    }
    return true;
}

QString getStandardPath()
{
    QString standardPath = pAppConfig->getWorkdir();
#ifdef Q_OS_MAC
    standardPath = standardPath.replace("DkVerwaltungQt.app/Contents/MacOS", "");
#endif
    return standardPath;
}

QString getOpenOfficePath()
{
    QString oo;
#if defined (Q_OS_MAC)
     // oo = "/Applications/OpenOffice.app/Contents/MacOS/soffice";
    oo = getFilePathFromIni("OOPath", "/Applications/OpenOffice.app/Contents/MacOS/", "soffice");
#elif defined(Q_OS_WIN)
    oo = pAppConfig->getConfigPath_wUI("OOPath", "C:\\Program files\\OpenOffice 4\\Progarm", "soffice.exe");
#endif
    return oo;
}

QString getJahresDkZinsBescheinigungenPath(QString Year)
{
   QString JahresDkZinsBescheinigungenPath = pAppConfig->getWorkdir() + QDir::separator() + "JahresDkZinsBescheinigungen" + Year;
   return JahresDkZinsBescheinigungenPath;
}

QString getYear_wUI()
{
    int year = QDate::currentDate().year();
    QString thisYear(QString::number(year));
    QString lastYear(QString::number(year -1));
    QString two_yBack(QString::number(year-2));

    QMessageBox msgBox;
    QPushButton *two_yBackButton = msgBox.addButton(two_yBack, QMessageBox::ActionRole);
    QPushButton *lastYearButton = msgBox.addButton(lastYear, QMessageBox::ActionRole);
    msgBox.addButton(thisYear, QMessageBox::ActionRole);
    msgBox.setIcon(QMessageBox::Question);
    msgBox.setText("Für welches Jahr soll die Berechnung durchgeführt werden?");

    msgBox.exec();
    if( two_yBackButton == msgBox.clickedButton())
    {
        return two_yBack;
    }
    if( lastYearButton == msgBox.clickedButton())
    {
        return lastYear;
    }
    return thisYear;
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
   double ret = qRound(Betrag * 100) / 100.;
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
