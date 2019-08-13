#include <cstdlib>
#include <locale.h>

#include <QtWidgets>
#include <QtSql>
#include <QStringLiteral>
#include <QFile>
#include <qdebug.h>

#include "appconfiguration.h"
#include "dbfkts.h"
#include "mainform.h"

bool resolveDbIssue()
{
    QString MsgText("Die Db3 Datei \n");
    MsgText += pAppConfig->getDb();
    MsgText += "\n ist defekt oder existiert nicht. \nWähle JA um eine neue Datei anzulegen, NEIN um eine Datei auszuwählen oder ABBRECHEN um das Programm zu beenden";
    QMessageBox msgb;
    msgb.setText(MsgText);
    msgb.setInformativeText("Neue Datei Anlegen?");
    msgb.setStandardButtons(QMessageBox::Yes|QMessageBox::No|QMessageBox::Cancel);

    switch( msgb.exec())
    {
    case QMessageBox::Yes:
        return CreateDatabase(pAppConfig->getDb());
    case QMessageBox::No:
        return askForDatabase();
    case QMessageBox::Cancel:
    default:
        return false;
    }
}

int main(int argc, char *argv[])
{
    QApplication::setApplicationName("DkVerwaltungQt");
    QApplication::setOrganizationName("MHS");

    setlocale(LC_ALL, "C");
    QLocale::setDefault(QLocale::c());

    QApplication app(argc, argv);

    if( !QFile::exists(pAppConfig->getDb()))
    {
        if( !resolveDbIssue())
            return ERROR_FILE_NOT_FOUND;
    }
    else if( !isValidDb(pAppConfig->getDb()))
    {
        if( !resolveDbIssue())
            return ERROR_FILE_NOT_FOUND;
    }

    if( !pAppConfig->isValidWorkdir())
    {
        QMessageBox("DkVerwaltung konnte nicht initialisiert werden",
                    "Zur Laufzeit verwendete Dateien konnten nicht gefunden werden. Die Anwendung wird beendet",
                    QMessageBox::Critical,
                    QMessageBox::Ok,QMessageBox::NoButton,QMessageBox::NoButton).exec();
        return ERROR_BAD_CONFIGURATION;
    }
    if (!createConnection())
        return 1;

    MainForm form;
    form.resize(1024, 768);
    form.show();
    return app.exec();
}
