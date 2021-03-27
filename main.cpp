#include <QtWidgets>
#include <QtSql>
#include <cstdlib>
#include <locale.h>

#include "mainwindow.h"
#include "mainform.h"

#include "dbfkts.h"

int main(int argc, char *argv[], char *env[])
{
#ifdef QT_DEBUG    
    dumpEnv(env);
#endif    
    QApplication::setApplicationName("DkVerwaltungQt");
    QApplication::setOrganizationName("MHS");

    setlocale(LC_ALL, "C");
    QLocale::setDefault(QLocale::c());

    QApplication app(argc, argv);

    qDebug() << "settings: " << getSettings().fileName();
    QString dbPath = getStandardPath() + QDir::separator() + "DkVerwaltungQt.db3";
    dbPath = getStringFromIni("DBPath", dbPath);
    qDebug() << "dbPath from ini: " << dbPath;
    if (!createConnection(dbPath))
    {
        bool existingData = QFile::exists(dbPath);
        if (!existingData)
        {
            int mbr = QMessageBox::warning(0, QStringLiteral("Datenbank Fehler"),
                             dbPath + QStringLiteral(" existiert nicht! Soll die Datenbank angelegt werden?"), QMessageBox::Yes | QMessageBox::No);
            if(mbr != QMessageBox::Yes)
            {
                return 1;

            }
            dbPath = getFilePathFromIni("DBPath", getStandardPath(), "DkVerwaltungQt.db3");
            if (!QFile::exists(dbPath))
            {
                QMessageBox::warning(0, QStringLiteral("Datenbank Fehler"),
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
    mw.show();
    return app.exec();
}
