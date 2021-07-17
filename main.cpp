#include <QtWidgets>
#include <QtSql>
#include <cstdlib>
#include <locale.h>

#include "mainwindow.h"
#include "mainform.h"

#include "dbfkts.h"

#ifndef Q_OS_WIN

#include <unistd.h>
#include <sys/time.h>

QString getCurrentTimeString()
{
    char buf[64] = "";
    char fmt[64];
    struct timeval tv;
    struct tm *tm;

    gettimeofday (&tv, NULL);
    tm = localtime (&tv.tv_sec);
    strftime (fmt, sizeof (fmt), "%H:%M:%S:%%06u", tm);
    snprintf (buf, sizeof (buf), fmt, tv.tv_usec);
    return buf;
}

#else

QString getCurrentTimeString()
{
    return QString();
}

#endif

static QtMessageHandler oldMessageHandler = nullptr;
void myMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    oldMessageHandler(type, context, msg);

    QString timestring = getCurrentTimeString();
    QString msg_timestring = timestring + " " + msg;

    QString outputFilename = getStandardPath() + QDir::separator() + "DkVerwaltungQt.log" ;
    QFile outputFile( outputFilename );
    outputFile.open( QIODevice::WriteOnly | QIODevice::Append);
    QTextStream outputStream( &outputFile );
    outputStream << msg_timestring << Qt::endl;
    outputFile.close();

    return;

    QByteArray localMsg = msg.toLocal8Bit();
    const char *file = context.file ? context.file : "";
    const char *function = context.function ? context.function : "";
    switch (type) {
    case QtDebugMsg:
        fprintf(stderr, "Debug: %s (%s:%u, %s)\n", localMsg.constData(), file, context.line, function);
        break;
    case QtInfoMsg:
        fprintf(stderr, "Info: %s (%s:%u, %s)\n", localMsg.constData(), file, context.line, function);
        break;
    case QtWarningMsg:
        fprintf(stderr, "Warning: %s (%s:%u, %s)\n", localMsg.constData(), file, context.line, function);
        break;
    case QtCriticalMsg:
        fprintf(stderr, "Critical: %s (%s:%u, %s)\n", localMsg.constData(), file, context.line, function);
        break;
    case QtFatalMsg:
        fprintf(stderr, "Fatal: %s (%s:%u, %s)\n", localMsg.constData(), file, context.line, function);
        break;
    }
}

void installTranslateFile(const QString& translationFile)
{
    QTranslator* pTrans = new QTranslator();
    if( pTrans->load(translationFile))
        if( QCoreApplication::installTranslator(pTrans)) {
            qInfo() << "Successfully installed language file " << translationFile;
            return;
        }
    qCritical() << "failed to load translations " << translationFile;
}

void setGermanUi()
{
    QString translationFolder = QApplication::applicationDirPath() + "/../translations/%1";
    installTranslateFile(translationFolder.arg("qtbase_de.qm"));
}

#ifdef XXX
void setGermanUi()
{

    QString translationsPath(QLibraryInfo::location(QLibraryInfo::TranslationsPath));
    QLocale locale = QLocale::system();

    QTranslator qtTranslator;
    if (qtTranslator.load(locale, "qt", "_", translationsPath))
    {
        QCoreApplication::installTranslator(&qtTranslator);
    }
    QTranslator qtBaseTranslator;
    if (qtBaseTranslator.load(locale, "qtbase", "_", translationsPath))
    {
        QCoreApplication::installTranslator(&qtBaseTranslator);
    }
}
#endif

int main(int argc, char *argv[])
{
    oldMessageHandler = qInstallMessageHandler(myMessageOutput);
#ifdef QT_DEBUG    
    dumpEnv();
#endif    
    QApplication::setApplicationName("DkVerwaltungQt");
    QApplication::setOrganizationName("MHS");

    setlocale(LC_ALL, "C");
    QLocale::setDefault(QLocale::c());    

    QApplication app(argc, argv);
    setGermanUi();

    qDebug() << "settings: " << getSettings().fileName();
    QString dbPath = getStandardPath() + QDir::separator() + "DkVerwaltungQt.db3";
    dbPath = getStringFromIni("DBPath", dbPath);
    qDebug() << "dbPath from ini: " << dbPath;
    if (!createConnection(dbPath))
    {
        bool existingData = QFile::exists(dbPath);
        if (!existingData)
        {
            int mbr = QMessageBox::warning(nullptr, QStringLiteral("Datenbank Fehler"),
                             dbPath + QStringLiteral(" existiert nicht! Soll die Datenbank angelegt werden?"), QMessageBox::Yes | QMessageBox::No);
            if(mbr != QMessageBox::Yes)
            {
                return 1;

            }
            dbPath = getFilePathFromIni("DBPath", getStandardPath(), "DkVerwaltungQt.db3");
            if (!QFile::exists(dbPath))
            {
                QMessageBox::warning(nullptr, QStringLiteral("Datenbank Fehler"),
                                     dbPath + QStringLiteral(" existiert nicht!l"), QMessageBox::Ok);
                return 2;
            }
            if (!createConnection(dbPath))
            {
                return 3;
            }
        }
    }
    qDebug() << "connection: " << dbPath;
    MainWindow mw;
    mw.setWindowTitle("DkVerwaltungQt - [" + dbPath + "]");
    mw.show();
    return app.exec();
}
