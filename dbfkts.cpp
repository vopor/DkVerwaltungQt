#include "dbfkts.h"

#include <QVariant>
#include <QSqlQuery>
#include <QSqlError>

#include <QMessageBox>

#include <QtWidgets>

#include <QHostInfo> // requires QT += network

#include <QPrinter>

#include <QWebEngineView>

#include "html2pdfconverter.h"

#include "webenginepdfviewer.h"

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

void showPdfFile(const QString &fileNamePdf)
{
    PdfViewer &w = *(new PdfViewer(fileNamePdf));
    w.setAttribute(Qt::WA_DeleteOnClose);
    w.resize(640, 480);
    w.show();
    // QEventLoop eventLoop;
    // QObject::connect(&w, SIGNAL(finished()), &eventLoop, SLOT(quit()));
    // eventLoop.exec();
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

void dumpEnv()
{
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    QString env_variable;
    QStringList paths_list = env.toStringList();
    foreach( env_variable, paths_list ){
        qDebug() << env_variable;
    }
}

int run_executeCommand(QWidget *button, const QString &commandLine, QString &stdOutput, QString &stdError)
{
    int retCode = -1;
    if(commandLine.isEmpty())
        return -1;
    if(button) button->setEnabled(false);
    qDebug() << commandLine;
    QPointer<QProcess> process = new QProcess;
    QMetaObject::Connection connFinished = QObject::connect(process, static_cast<void (QProcess::*)(int exitCode, QProcess::ExitStatus exitStatus)>(&QProcess::finished), [process, button, commandLine, &stdOutput, &stdError, &retCode] (int exitCode, QProcess::ExitStatus exitStatus){
        if(process->state() == QProcess::NotRunning){
            qDebug() << commandLine << " finished: " << process->processId();
            if(process->exitStatus()  == QProcess::NormalExit){
                qDebug() << commandLine << " exited normal: " << process->processId() << "exitcode: " << process->exitCode();
                stdOutput =process->readAllStandardOutput();
                qDebug() << "stdOutput: ";
                qDebug() << stdOutput;
                stdError = process->readAllStandardError();;
                qDebug() << "stdError: ";
                qDebug() << stdError;
                retCode = process->exitCode();
            }
        }
        process->deleteLater();
        if(button) button->setEnabled(true);

    });
    // QObject::connect(process, static_cast<void (QProcess::*)(QProcess::ProcessError error)>(&QProcess::errorOccurred), [process, button, commandLine] (QProcess::ProcessError error)
    QObject::connect(process, &QProcess::errorOccurred, [process, button, commandLine, &stdOutput, &stdError, &retCode] (QProcess::ProcessError error)
    {
        qDebug() << commandLine << " process error: " << error << " pid: " << process->processId();
        stdOutput =process->readAllStandardOutput();
        qDebug() << "stdOutput: ";
        qDebug() << stdOutput;
        stdError = process->readAllStandardError();;
        qDebug() << "stdError: ";
        qDebug() << stdError;
        retCode = process->exitCode();
        process->deleteLater();
        if(button) button->setEnabled(true);
        // process = nullptr;
    });
    process->start( commandLine );
    if(process) process->waitForStarted();
    if(process) qDebug() << commandLine << " process started: " << process->processId();
    if(process) process->waitForFinished();
    if(process) qDebug() << commandLine << " process finished: " << process->processId();
    // qDebug() << commandLine << " process finished";
    // bringAppToForeground(process->processId());
    return retCode;
#ifdef XXX
    QObject::disconnect(connFinished);
    process->waitForFinished(10000);
    if(process->exitStatus()  == QProcess::NormalExit){
        qDebug() << commandLine << " exited normal: " << process->processId() << "exitcode: " << process->exitCode();
        QByteArray output = process->readAllStandardOutput();
        QByteArrayList baList = output.split('\n');
        foreach(QByteArray ba, baList){
            qDebug() << ba;
        }
    }else{
        qDebug() << commandLine << " error: " << process->error() << "exitcode: " << process->exitCode();
        QByteArray output = process->readAllStandardError();
        qDebug() << output;
    }
    button->setEnabled(true);
#endif
}

//--------------------------------------------------------------
// DkVerwaltungQt-Funktionen
//--------------------------------------------------------------

bool createConnection(const QString &dbName)
{
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    // bool dbExisted = QFileInfo(dbName).exists();
    bool ret = openConnection(dbName);
    return ret;
}

bool isDKV2Database()
{
    QSqlDatabase db = QSqlDatabase::database();
    int c = getIntValue("SELECT COUNT(name) FROM sqlite_master WHERE type='table' AND name='Meta'");
    if(c)
    {
        c &= getIntValue("SELECT COUNT(*) FROM pragma_table_info('Meta') WHERE name='Name'");
        c &= getIntValue("SELECT COUNT(*) FROM pragma_table_info('Meta') WHERE name='Wert'");
        QString Version = getStringValue("SELECT Wert FROM Meta WHERE Name = 'Version'");
        QString dkv2_exe_Version = getStringValue("SELECT Wert FROM Meta WHERE Name = 'dkv2.exe.Version'");
        return true;
    }
    return false;
}

bool isDkVerwaltungQtDatabase()
{
    QSqlDatabase db = QSqlDatabase::database();
    int c = getIntValue("SELECT COUNT(name) FROM sqlite_master WHERE type='table' AND name='DKPersonen'");
    c &= getIntValue("SELECT COUNT(name) FROM sqlite_master WHERE type='table' AND name='DKBuchungen'");
    c &= getIntValue("SELECT COUNT(name) FROM sqlite_master WHERE type='table' AND name='DKZinssaetze'");
    return c;
}

void anonymizeDatabase()
{
    QSqlDatabase db = QSqlDatabase::database();
    QString anonymizeDkPersonen = "UPDATE DkPersonen SET Vorname='Vorname_' || PersonId, Name='Name_' || PersonId, Anrede='Anrede_' || PersonId, Straße='Straße_' || PersonId, PLZ='PLZ_' || PersonId, Ort='Ort_' || PersonId, Email='Email_' || PersonId";
    db.exec(anonymizeDkPersonen);
    displayLastSqlError();
    QString anonymizeDkBuchungen = "UPDATE DkBuchungen SET Bemerkung='Bemerkung_' || BuchungId";
    db.exec(anonymizeDkBuchungen);
    displayLastSqlError();
}

void displayLastSqlError()
{
    QSqlDatabase db = QSqlDatabase::database();
    QSqlError sqlError = db.lastError();
    QString text = sqlError.text();
    if(text.length())
    {
        QMessageBox::warning(0, QObject::tr("Database Error"), db.lastError().text());
    }
}

bool createDkVerwaltungQtViewsFromDKV2()
{
    QSqlDatabase db = QSqlDatabase::database();
    bool b = isDKV2Database();
    if(b)
    {
        int c = 0;
        // DKPersonen-Table
        c = getIntValue("SELECT COUNT(name) FROM sqlite_master WHERE type='table' AND name='DKPersonen'");
        if(c)
        {
            c = getIntValue("SELECT COUNT(name) FROM sqlite_master WHERE type='table' AND name='DKPersonenTable'");
            if(!c)
            {
                db.exec("ALTER TABLE DKPersonen RENAME TO DKPersonenTable;");
            }else{
                db.exec("DROP TABLE DKPersonen;");
            }
            displayLastSqlError();
        }
        // DKBuchungen-Table
        c = getIntValue("SELECT COUNT(name) FROM sqlite_master WHERE type='table' AND name='DKBuchungen'");
        if(c)
        {
            c = getIntValue("SELECT COUNT(name) FROM sqlite_master WHERE type='table' AND name='DKBuchungenTable'");
            if(!c)
            {
                db.exec("ALTER TABLE DKBuchungen RENAME TO DKBuchungenTable;");
            }else{
                db.exec("DROP TABLE DKBuchungen;");
            }
            displayLastSqlError();
        }
        // DKZinssaetze-Table
        c = getIntValue("SELECT COUNT(name) FROM sqlite_master WHERE type='table' AND name='DKZinssaetze'");
        if(c)
        {
            c = getIntValue("SELECT COUNT(name) FROM sqlite_master WHERE type='table' AND name='DKZinssaetzeTable'");
            if(!c)
            {
                db.exec("ALTER TABLE DKZinssaetze RENAME TO DKZinssaetzeTable;");
            }else{
                db.exec("DROP TABLE DKZinssaetze;");
            }
            displayLastSqlError();
        }
        // DKPersonen-View
        c = getIntValue("SELECT COUNT(name) FROM sqlite_master WHERE type='view' AND name='DKPersonen'");
        if(c)
        {
            db.exec("DROP VIEW DKPersonen;");
            displayLastSqlError();
        }
        db.exec("CREATE VIEW DKPersonen AS SELECT Id AS PersonId, Vorname AS Vorname, Nachname AS Name, Strasse AS Straße, Plz AS PLZ, Stadt AS Ort, EMail AS Email FROM Kreditoren;");
        displayLastSqlError();
        // DKBuchungen-View
        c = getIntValue("SELECT COUNT(name) FROM sqlite_master WHERE type='view' AND name='DKBuchungen'");
        if(c)
        {
            db.exec("DROP VIEW DKBuchungen;");
            displayLastSqlError();
        }
        QString statement = "CREATE VIEW DKBuchungen AS ";
        statement += "SELECT b.Id AS BuchungId, v.KreditorId AS PersonId, ";
        statement += "( substr(b.Datum,9,2) || '.' || substr(b.Datum,6,2) || '.' || substr(b.Datum,3,2))  AS Datum, ";
        statement += "b.VertragsId AS DKNr, v.Kennung AS DKNummer, ";
        // TODO:
        // wird auch beim Import nach DKV2 nicht berücksichtigt.
        // müsste aus exBuchunen, exVertraege kommen
        // 2013-12-04 -> 26.01.18
        // statement += "'' AS Rueckzahlung, ";
        statement += "CASE WHEN (b.Betrag < 0) THEN b.Datum ELSE '' END AS Rueckzahlung, ";
        statement += "CASE WHEN (v.LaufzeitEnde != '3000-12-31') THEN ( substr(v.LaufzeitEnde,9,2) || '.' || substr(v.LaufzeitEnde,6,2) || '.' || substr(v.LaufzeitEnde,3,2)) END AS vorgemerkt, ";
        // statement += "CASE WHEN (Buchungsart = 2) THEN ( -1 * (b.Betrag / 100.0) ) ELSE (b.Betrag / 100.0) END AS Betrag, ";
        statement += "(b.Betrag / 100.0) AS Betrag, ";
        statement += "(v.ZSatz / 100.0) AS Zinssatz, ";
        // TODO:
        // 23.01.14 F13T-2013-004
        // [auto] DK-Nr. 004 neu angelegt. Betrag: 10.000,00 €.
        // 30.06.17 [auto] DK-Nr. 004 gekündigt. Betrag: 10.000,00 €.
        statement += "'' AS Bemerkung, ";
        // statement += "c.Anfangsdatum AS Anfangsdatum, ";
        statement += "( substr(c.Anfangsdatum,9,2) || '.' || substr(c.Anfangsdatum,6,2) || '.' || substr(c.Anfangsdatum,3,2)) AS Anfangsdatum, ";
        statement += "(v.Betrag / 100.0) AS AnfangsBetrag ";
        // statement += "FROM Buchungen b, Vertraege v, ";
        statement += "FROM (SELECT * FROM Buchungen UNION SELECT * FROM ExBuchungen) b, (SELECT * FROM Vertraege UNION SELECT * FROM ExVertraege) v, ";
        statement += "(SELECT  MIN(x.Datum) AS Anfangsdatum,  x.VertragsId AS VertragsId FROM Buchungen x GROUP BY x.VertragsId) AS c ";
        statement += "WHERE b.VertragsId = v.Id ";
        statement += "AND c.VertragsId = v.Id; ";
        db.exec(statement);
        displayLastSqlError();

        // DKZinssaetze
        c = getIntValue("SELECT COUNT(name) FROM sqlite_master WHERE type='view' AND name='DKZinssaetze'");
        if(c)
        {
            db.exec("DROP VIEW DKZinssaetze;");
            displayLastSqlError();
        }
        db.exec("CREATE VIEW DKZinssaetze AS SELECT ZSatz / 100.0 AS Zinssatz, NULL AS Beschreibung FROM Vertraege GROUP BY ZSatz ORDER BY ZSatz;");
        displayLastSqlError();
        // TODO
        // reconnect readonly
    }
    return b;
}

bool dropDkVerwaltungQtViewsFromDKV2()
{
    QSqlDatabase db = QSqlDatabase::database();
    bool b = isDKV2Database();
    if(b)
    {
        int c = 0;
        // DKPersonen-Table
        c = getIntValue("SELECT COUNT(name) FROM sqlite_master WHERE type='view' AND name='DKPersonen'");
        if(c)
        {
            c = getIntValue("SELECT COUNT(name) FROM sqlite_master WHERE type='table' AND name='DKPersonenTable'");
            if(c)
            {
                db.exec("DROP VIEW DKPersonen;");
                displayLastSqlError();
                db.exec("ALTER TABLE DKPersonenTable RENAME TO DKPersonen;");
                displayLastSqlError();
            }
        }
        // DKBuchungen-Table
        c = getIntValue("SELECT COUNT(name) FROM sqlite_master WHERE type='view' AND name='DKBuchungen'");
        if(c)
        {
            c = getIntValue("SELECT COUNT(name) FROM sqlite_master WHERE type='table' AND name='DKBuchungenTable'");
            if(c)
            {
                db.exec("DROP VIEW DKBuchungen;");
                displayLastSqlError();
                db.exec("ALTER TABLE DKBuchungenTable RENAME TO DKBuchungen;");
                displayLastSqlError();
            }
        }
        // DKZinssaetze-Table
        c = getIntValue("SELECT COUNT(name) FROM sqlite_master WHERE type='view' AND name='DKZinssaetze'");
        if(c)
        {
            c = getIntValue("SELECT COUNT(name) FROM sqlite_master WHERE type='table' AND name='DKZinssaetzeTable'");
            if(c)
            {
                db.exec("DROP VIEW DKZinssaetze;");
                displayLastSqlError();
                db.exec("ALTER TABLE DKZinssaetzeTable RENAME TO DKZinssaetze;");
                displayLastSqlError();
            }
        }
    }
    return b;
}

bool createDkVerwaltungQtTables()
{
    QSqlDatabase db = QSqlDatabase::database();
    db.exec("CREATE TABLE IF NOT EXISTS DKPersonen (PersonId INTEGER PRIMARY KEY AUTOINCREMENT, Vorname TEXT, Name TEXT, Anrede TEXT, Straße TEXT, PLZ TEXT, Ort TEXT, Email TEXT);");
    displayLastSqlError();
    db.exec("CREATE TABLE IF NOT EXISTS DKBuchungen (BuchungId INTEGER PRIMARY KEY AUTOINCREMENT, PersonId INTEGER, Datum TEXT, DKNr TEXT, DKNummer TEXT, Rueckzahlung TEXT, vorgemerkt TEXT, Betrag REAL, Zinssatz REAL, Bemerkung TEXT, Anfangsdatum TEXT, AnfangsBetrag REAL, FOREIGN KEY(PersonId) REFERENCES DKPerson(PersonId));");
    displayLastSqlError();
    db.exec("CREATE TABLE IF NOT EXISTS DKZinssaetze (Zinssatz REAL PRIMARY KEY, Beschreibung TEXT);");
    displayLastSqlError();
    return true;
}

bool fillDkVerwaltungQtTablesFromDKV2()
{
    qDebug() << "Noch nicht implementiert.";
    return false;
}

bool openConnection(const QString &dbName)
{
    QSqlDatabase db = QSqlDatabase::database();
    db.setDatabaseName(dbName);
    if (!db.open()) {
        QMessageBox::warning(0, QObject::tr("Database Error"),
                             db.lastError().text());
        return false;
    }else{
        // createDkVerwaltungQtViewsFromDKV2();
        createDkVerwaltungQtTables();
        fillDkVerwaltungQtTablesFromDKV2();
    }
    return true;
}

bool closeConnection(const QString &dbName)
{
    QSqlDatabase db = QSqlDatabase::database();
    db.close();
    return true;
}

QString getExecutablePath()
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

QString getResouresPath()
{
    QString resourcesPath = QDir::toNativeSeparators(QCoreApplication::applicationDirPath());
#ifdef Q_OS_MAC
    resourcesPath = resourcesPath.replace("DkVerwaltungQt.app/Contents/MacOS", "DkVerwaltungQt.app/Contents/Resources");
#endif
    return resourcesPath;
}

QString getStandardPath()
{
    QString standardPath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    standardPath = standardPath + QDir::separator() + "DkVerwaltungQt";
    if(!QFileInfo(standardPath).exists())
    {
        QDir().mkpath(standardPath);
    }
    if(!QFileInfo(standardPath).exists())
    {
        QMessageBox::warning(nullptr, "Fehler", QStringLiteral("Das Verzeichnis existiert nicht bzw. konnte nicht angelegt werden:\n") + standardPath + "\n", QMessageBox::Ok);
    }
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

QString getCompareToolPath()
{
    QString compareToolPath;
#if defined (Q_OS_MAC)
    compareToolPath = getFilePathFromIni("CompareToolPath", "/Applications/Beyond Compare.app/Contents/MacOS/", "bcomp");
#elif defined(Q_OS_WIN)
    compareToolPath = getFilePathFromIni("CompareToolPath", "", "");
#endif
    return compareToolPath;
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
   QString JahresDkBestaetigungenPath = getStandardPath() + QDir::separator() + "JahresDkBestaetigungen" + QString::number(2000 + getJahr());
   return JahresDkBestaetigungenPath;
}

QString getJahresDkZinsBescheinigungenPath()
{
   QString JahresDkZinsBescheinigungenPath = getStandardPath() + QDir::separator()  + "JahresDkZinsBescheinigungen" + QString::number(2000 + getJahr());
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

QDate getDateFromYearString(const QString &yearString)
{
    QDate ret;
    QStringList list = yearString.split('.');
    if(list.count() == 3)
    {
        QString year4String;
        if(list.at(2).length() == 2)
        {
            year4String = list.at(0) + '.' + list.at(1) + '.' + "20" + list.at(2);
        }else if(list.at(2).length() == 4){
            year4String = yearString;
        }
        ret = QDate::fromString(year4String, "dd.MM.yyyy");
    }
    return ret;
}

int getAnzTage(const QDate &dateFrom, const QDate &dateTo, bool inclLastDay)
{
   // Q_ASSERT(dateFrom.year() == dateTo.year());
   if(dateFrom.year() != dateTo.year())
   {
       qDebug() << "getAnzTage: (dateFrom.year() != dateTo.year()):";
       qDebug() << dateFrom << " " << dateTo;
   }
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
   // Q_ASSERT(dateFrom.year() == dateTo.year());
   if(dateFrom.year() != dateTo.year())
   {
       qDebug() << "getAnzTageNew: (dateFrom.year() != dateTo.year()):";
       qDebug() << dateFrom << " " << dateTo;
   }
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
    double doubleValue = 0;
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

