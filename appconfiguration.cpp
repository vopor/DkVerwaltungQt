#include "appconfiguration.h"

#include <qstring.h>
#include <qcoreapplication.h>
#include <qdebug.h>
#include <qtemporaryfile.h>
#include <qfiledialog.h>

AppConfig* AppConfig::Instance;

AppConfig::AppConfig () :
    ExeDir (QDir::toNativeSeparators(QCoreApplication::applicationDirPath())),
    CurrentDir (QDir::currentPath()),
    IniFilePath(locateIniFile()),
    WorkDir(QFileInfo(IniFilePath).path())
{
#ifdef Q_OS_MAC
    ExeDir = ExeDir.replace("DkVerwaltungQt.app/Contents/MacOS", "");
#endif
    initDbName();
    WorkDirFiles << getAnschreibenTemplate();
    WorkDirFiles << getAuszugsTemplate();
    WorkDirFiles << getZinsbescheinigungsTemplate();
}

void AppConfig::setDb(QString f)
{
    DbName = f;
    USE_Ini;
    Ini.beginGroup(IniSection);
    Ini.setValue(IniKeyDbPath, DbName);
    Ini.endGroup();
}

QString AppConfig::getConfigString( const QString &key, const QString &DefVal)
{
    USE_Ini;
    Ini.beginGroup(IniSection);
    QString Value = Ini.value(key, QVariant(DefVal)).toString();
    Ini.endGroup();
    return Value;
}

QString AppConfig::getConfigPath_wUI(const QString &key, const QString path, const QString &filename)
{
    QString startSearchFolder = path;
    if( startSearchFolder.isEmpty())
        startSearchFolder = getWorkdir();
    QString DefVal = startSearchFolder + QDir::separator() + filename;
    USE_Ini;
    Ini.beginGroup(IniSection);
    QString Value = Ini.value(key).toString();
    Ini.endGroup();
    if( Value.isEmpty() || !QFile(Value).exists())
    {   // ask the user
        Value = QFileDialog::getOpenFileName(nullptr, filename + QStringLiteral(" öffnen"), getWorkdir(), DefVal);
        if( !Value.isEmpty())
        {
            Ini.beginGroup(IniSection);
            Ini.setValue(key, Value);
            Ini.endGroup();
        }
    }
    return Value;
}

bool AppConfig::isValidWorkdir()
{
    foreach(QString Filename, WorkDirFiles)
    {
        QString AbsoluteFilename = WorkDir.filePath(Filename);
        if( !QFile(AbsoluteFilename).exists())
        {
            qDebug() << "Configuration file "
                     << Filename << " does not exist " << endl;
            return false;
        }
    }
    return true;
}

QString AppConfig::locateIniFile()
{
    // find ini file in current directory or application directory
    QDir cd(CurrentDir);
    QString fp = cd.filePath(IniFileName);
    QFile IniInCurDir(QDir(CurrentDir).filePath((IniFileName)));
    if( IniInCurDir.exists())
    {
        return IniInCurDir.fileName();
    }
    QFile IniInAppDir(QDir(ExeDir).filePath((IniFileName)));
    if( IniInAppDir.exists())
    {
        return IniInAppDir.fileName();
    }
    // no ini file -> create one in the current Directory
    return IniInCurDir.fileName();
}

void AppConfig::initDbName()
{
    USE_Ini;
    Ini.beginGroup(IniSection);
    QString s = Ini.value(IniKeyDbPath).toString();
    Ini.endGroup();
    if (s.isEmpty())
    {   // use curdir and save to ini
        setDb( WorkDir.filePath(DefaultDbName));
    }
    else {
        // read from ini
        DbName = s;
    }
}
