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


int main(int argc, char *argv[])
{
    QApplication::setApplicationName("DkVerwaltungQt");
    QApplication::setOrganizationName("MHS");

    setlocale(LC_ALL, "C");
    QLocale::setDefault(QLocale::c());

    QApplication app(argc, argv);

    if( !asureDatabase_wUI())
        return ERROR_FILE_NOT_FOUND;

    MainForm form;
    form.resize(1024, 768);
    form.show();

    return app.exec();
}
