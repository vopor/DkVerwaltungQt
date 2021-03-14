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
    QString dbPath = getFilePathFromIni("DBPath", getStandardPath(), "DkVerwaltungQt.db3");
    qDebug() << "dbPath: " << dbPath;
    if(!dbPath.length())
        return 1;
    bool existingData = QFile::exists(dbPath);
    if (!existingData){
        QMessageBox::warning(0, QStringLiteral("Datenbank Fehler"),
                         dbPath + QStringLiteral(" existiert nicht!"));
        return 2;
    }
    if (!createConnection(dbPath))
        return 1;

    MainWindow mw;
    mw.show();
    return app.exec();
}
