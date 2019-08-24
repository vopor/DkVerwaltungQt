#ifndef APPCONFIGURATION_H
#define APPCONFIGURATION_H

#include <QString>
#include <QDir>
#include <QMap>
#include <qsettings.h>
/*
 * The Ini file named DkVerwaltungsQt.ini is read from the current
 * directory or else from the exe directory
 * If both are not there, one is created in the current directory
 *
 * DB is read from ini file (any file name allowed)
 * or used from current directory or exe directory with default file name
 *
 * All other files are read from the folder in which the ini file was located
 * or where the DB is located, if no ini was found
*/
#define USE_Ini QSettings Ini(IniFilePath, QSettings::IniFormat)

class AppConfig
{
public:
    AppConfig();

    bool isValidWorkdir();
    QString getDb(){ return DbName;}
    QString getAnschreibenTemplate(){return AnschreibenTemplate;}
    QString getZinsbescheinigungsTemplate(){return getWorkdir()+QDir::separator()+ZinsbeschTemplate;}
    QString getOutputDirZinsbescheinigungen(QString Jahr){
        return getOutputDir("JahresDkZinsBescheinigungen", Jahr);
    }
    QString getOutputDirDkBestaetigungen(QString Jahr){
        return getOutputDir("JahresDkBestaetigungen", Jahr);
    }
    QString getAuszugsTemplate(){return JahresAuszugsTemplate;}
    void setDb(QString f);
    void clearDb();
    QString getWorkdir(){ return WorkDir.absolutePath();}
    QString getConfigString( const QString &key, const QString &defaultvalue=QString(""));
    QString getConfigPath_wUI(const QString &key, const QString path, const QString &filename);
public: // singlton stuff
    static AppConfig* get()
    {
        if( !Instance) Instance = new AppConfig;
        return Instance;
    }

private:
    // helper (initialize first!)
    const QString DefaultDbName ="DkVerwaltungQt.db3";
    const QString IniFileName = "DkVerwaltungsQt.ini";
    const QString AnschreibenTemplate = "Vorlage_Anschreiben_DK-Eingang.ott";
    const QString ZinsbeschTemplate ="Zinsbescheinigung.html";
    const QString JahresAuszugsTemplate = "Jahreskontoauszug.html";
    const QString IniKeyDbPath = "DBPath";
    const QString IniSection = "AllUsers";
    QString ExeDir;
    QString CurrentDir;
    QString locateIniFile();
    void initDbName (void);
    //
    QString IniFilePath;
    QDir WorkDir;
    QList<QString> WorkDirFiles;
    QString DbName;
    QString getOutputDir(QString Dirname, QString Jahr){
        QString tmp = getWorkdir()+QDir::separator()+ Dirname +"-" + Jahr;
        if( !QDir(tmp).exists())
            if( !QDir().mkpath(tmp)) throw("Failed to create output directory");
        return tmp;
    }

private: // singleton stuff
    static AppConfig* Instance;
};

#define pAppConfig AppConfig::get()

#endif // APPCONFIGURATION_H
