#include <QtWidgets>
#include <QtSql>
#include <QSqlRelationalTableModel>
#include <QSortFilterProxyModel>

#include "mainform.h"
#include "personform.h"
#include "buchungform.h"

#include "dbfkts.h"

#include <QStyledItemDelegate>

class NumberFormatDelegate : public QStyledItemDelegate
{
public:
   explicit NumberFormatDelegate(QObject *parent = 0);
   virtual QString displayText(const QVariant &value, const QLocale &locale) const;
};

NumberFormatDelegate::NumberFormatDelegate(QObject *parent) : QStyledItemDelegate(parent)
{
}

QString NumberFormatDelegate::displayText(const QVariant &value, const QLocale &locale) const
{
    QString formattedNum = locale.toString(value.toDouble(), 'f', 2);
    return formattedNum;
}

class BuchungenSortFilterProxyModel : public QSortFilterProxyModel{
public:
    BuchungenSortFilterProxyModel(QObject * parent);
protected:
    virtual bool lessThan(const QModelIndex & source_left, const QModelIndex & source_right) const;
};

BuchungenSortFilterProxyModel::BuchungenSortFilterProxyModel(QObject * parent) : QSortFilterProxyModel(parent)
{
}

bool BuchungenSortFilterProxyModel::lessThan(const QModelIndex & source_left, const QModelIndex & source_right) const
{
    int sortCol = -1;
    if(source_left.column() == source_right.column())
    {
        sortCol = source_left.column();
    }
    if((sortCol == DkBuchungen_Datum) || (sortCol == DkBuchungen_vorgemerkt) ){
        QString stringLeft = sourceModel()->data(source_left).toString();
        QString stringRight = sourceModel()->data(source_right).toString();
        if(stringLeft.length() && stringRight.length()){
            QDate dateLeft = QDate::fromString(stringLeft, "dd.MM.yy");
            QDate dateRight = QDate::fromString(stringRight, "dd.MM.yy");
            if(!dateLeft.isValid()) return true;
            if(!dateRight.isValid()) return false;
            return dateLeft < dateRight;
        }
    }
    return QSortFilterProxyModel::lessThan(source_left, source_right);
}

MainForm::MainForm(QWidget *parent) : QWidget(parent)
{
    setWindowTitle("DkVerwaltungQt");

    createAnzeigenPersonenPanel();
    createPersonenPanel();
    createBuchungenPanel();
    createSummenPanel();
    createBescheinigungenPanel();

    splitter = new QSplitter(Qt::Vertical);
    splitter->setFrameStyle(QFrame::StyledPanel);
    splitter->addWidget(personenPanel);
    splitter->addWidget(buchungenPanel);
    //splitter->addWidget(summenPanel);

    quitButton = new QPushButton(tr("Beenden"));
    buttonBox = new QDialogButtonBox;
    buttonBox->addButton(quitButton, QDialogButtonBox::ActionRole);
    connect(quitButton, SIGNAL(clicked()), parent, SLOT(close()));

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(anzeigenPersonenPanel);
    mainLayout->addWidget(splitter);
    mainLayout->addWidget(summenPanel);
    // mainLayout->addLayout(hlayout);
    mainLayout->addWidget(bescheinigungenPanel);
    mainLayout->addWidget(buttonBox);
    setLayout(mainLayout);

    setWindowTitle(tr("Dk-Verwaltung"));
    // setGeometry(100, 100, 800, 600);
    // setMinimumSize(800, 600);
    // personenView->sortByColumn(DkPersonen_PersonId);
    // buchungenView->sortByColumn(DkBuchungen_BuchungId);

    updatePersonenView();
    // personenView->setCurrentIndex(personenModel->index(0, 0));
    // updateBuchungenView();
    updateSummen();

}

void MainForm::createBescheinigungenPanel()
{
   bescheinigungenPanel = new QWidget;

   QHBoxLayout *hlayout = new QHBoxLayout;

   // hlayout->setSizeConstraint(QLayout::SetFixedSize);
   jahrDkBescheinigungenLabel = new QLabel(tr("Jahr"));;
   // jahrDkBescheinigungenLabel->setFixedWidth(60);

   jahrDkBescheinigungenLabel->setSizePolicy(QSizePolicy::Fixed, jahrDkBescheinigungenLabel->sizePolicy().verticalPolicy());
   hlayout->addWidget(jahrDkBescheinigungenLabel);
   jahrDkBescheinigungenEdit = new QLineEdit;
   hlayout->addWidget(jahrDkBescheinigungenEdit);
   // QString strYear = QString::number(QDate::currentDate().year()-1);
   QString statementYear4 = "SELECT '20' || MAX(SUBSTR(Datum ,7,2)) FROM DkBuchungen";
   QString strYear = getStringValue(statementYear4);
   jahrDkBescheinigungenEdit->setText(strYear);
   jahrDkBescheinigungenEdit->setFixedWidth(60);
   jahrDkBescheinigungenLabel->setBuddy(jahrDkBescheinigungenEdit);

   nurCheckBox = new QCheckBox("nur für");
   connect(nurCheckBox, &QCheckBox::clicked, [this](){
       if(nurCheckBox->isChecked())
       {
           QString testUserEmailAdress = getTestUserEmailAdress();
           nurEdit->setText(testUserEmailAdress);
           nurEdit->setEnabled(true);
           nurEdit->setFocus();
       }else{
           nurEdit->setEnabled(false);
       }
   });
   nurEdit = new QLineEdit;
   nurEdit->setMinimumWidth(200);
   nurEdit->setEnabled(false);
   sendenCheckBox = new QCheckBox("sofort senden");
   continueButton = new QPushButton("weiter");
   continueButton->setEnabled(false);
   cancelButton = new QPushButton("abbrechen");
   cancelButton->setEnabled(false);

   hlayout->addWidget(nurCheckBox);
   hlayout->addWidget(nurEdit);
   hlayout->addWidget(sendenCheckBox);
   hlayout->addWidget(continueButton);
   hlayout->addWidget(cancelButton);

   generateJahresDkBestaetigungenButton = new QPushButton(tr("Jahres Dk-Bestätigungen generieren"));
   hlayout->addWidget(generateJahresDkBestaetigungenButton);
   connect(generateJahresDkBestaetigungenButton, SIGNAL(clicked()), this, SLOT(generateJahresDkBestaetigungen()));

   generateJahresDkBestaetigungenTerminalButton = new QPushButton(tr("T"));
   generateJahresDkBestaetigungenTerminalButton->setFixedWidth(50);
   hlayout->addWidget(generateJahresDkBestaetigungenTerminalButton);
   connect(generateJahresDkBestaetigungenTerminalButton, SIGNAL(clicked()), this, SLOT(generateJahresDkBestaetigungenTerminal()));

   generateJahresDkZinsBescheinigungenButton = new QPushButton(tr("Jahres Dk-ZinsBescheinigungen generieren"));
   hlayout->addWidget(generateJahresDkZinsBescheinigungenButton);
   connect(generateJahresDkZinsBescheinigungenButton, SIGNAL(clicked()), this, SLOT(generateJahresDkZinsBescheinigungen()));

   generateJahresDkZinsBescheinigungenTerminalButton = new QPushButton(tr("T"));
   generateJahresDkZinsBescheinigungenTerminalButton->setFixedWidth(50);
   hlayout->addWidget(generateJahresDkZinsBescheinigungenTerminalButton);
   connect(generateJahresDkZinsBescheinigungenTerminalButton, SIGNAL(clicked()), this, SLOT(generateJahresDkZinsBescheinigungenTerminal()));

   bescheinigungenPanel->setLayout(hlayout);
}

int MainForm::getJahr()
{
   QString jahrStr = jahrDkBescheinigungenEdit->text();
   if(jahrStr.length() == 4)
   {
       jahrStr = jahrStr.right(2);
   }
   int jahr = jahrStr.toInt();
   return jahr;
}

QString MainForm::getJahresDkBestaetigungenPath()
{
   QString JahresDkBestaetigungenPath = getStandardPath() + QDir::separator() + "JahresDkBestaetigungen" + QString::number(2000 + getJahr());
   return JahresDkBestaetigungenPath;
}

QString MainForm::getJahresDkZinsBescheinigungenPath()
{
   QString JahresDkZinsBescheinigungenPath = getStandardPath() + QDir::separator() + "JahresDkZinsBescheinigungen" + QString::number(2000 + getJahr());
   return JahresDkZinsBescheinigungenPath;
}

bool MainForm::checkPrerequisitesExists()
{
    bool b = false;
    QStringList missingFiles;
#ifdef QT_DEBUG_XXX
    const char *p = __FILE__;
    QString sourcePath = p;
    sourcePath = QFileInfo(p).canonicalPath();
    QString homePath = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
    sourcePath = homePath + "/Documents/GitHub/DkVerwaltungQt";
    if(!QFileInfo(sourcePath).exists())
    {
        sourcePath = getStandardPath() + QDir::separator()  + ".." + QDir::separator() + "DkVerwaltungQt";
    }
#else
    QString sourcePath = getResouresPath();
#endif
    QString JahresDkBestaetigungenPath = getJahresDkBestaetigungenPath();
    b = QDir().mkpath(JahresDkBestaetigungenPath);
    QString JahresDkZinsBescheinigungenPath = getJahresDkZinsBescheinigungenPath();
    b = QDir().mkpath(JahresDkZinsBescheinigungenPath);
    QStringList dirs;
    dirs << JahresDkBestaetigungenPath << JahresDkZinsBescheinigungenPath;
    for (int i = 0; i < dirs.size(); ++i){
        QStringList files;
        files << "Jahreskontoauszug.html" << "Zinsbescheinigung.html" << "F13TurleyGmbH2.gif" << "mail-content.txt";
#ifdef Q_OS_MAC
        files << "url2pdf" << "html2pdf.sh"  << "sendDKJAKtos.py" << "printCommandDescription.sh";
#endif
        for (int j = 0; j < files.size(); ++j){
            QString file = files.at(j);
            QString destFile = dirs.at(i) + QDir::separator() + file;
            QString sourceFile = sourcePath + QDir::separator() + file;
            if(QFileInfo(sourceFile).exists())
            {
                if(!QFileInfo(destFile).exists())
                {
                   QFile::copy(sourceFile, destFile);
                }
                else
                {
                   if(QFileInfo(sourceFile).lastModified() > QFileInfo(destFile).lastModified() )
                   {
                        QFile::remove(destFile);
                        QFile::copy(sourceFile, destFile);
                   }
                }
            }

            if(!QFileInfo(destFile).exists())
            {
               missingFiles << file;
            }
        }
    }
    if(missingFiles.length()){
        if(QMessageBox::No == QMessageBox::warning(this, tr("Fehlende Dateien"), tr("Fehlende Dateien in:\n") + sourcePath + tr("\n") + missingFiles.join("\n") + tr("\nTrotzdem forfahren?\n"), QMessageBox::Yes | QMessageBox::No))
        {
            return false;
        }
    }
    return true;
}

void bringAppToForeground(qint64 pid)
{
    QString cmd;
    cmd = "/usr/bin/python -c ";
    cmd += "'";
    cmd += "import Cocoa; ";
    cmd += "x = Cocoa.NSRunningApplication.runningApplicationWithProcessIdentifier_(" + QString::number(pid) + "); ";
    cmd += "x.activateWithOptions_(Cocoa.NSApplicationActivateIgnoringOtherApps); ";
    cmd += "'";
    QProcess::startDetached(cmd, QStringList(), QString(), &pid);

}

void run_executeCommand(QPushButton *button, const QString &commandLine)
{
    if(commandLine.isEmpty())
        return;
    if(button) button->setEnabled(false);
    qDebug() << commandLine;
    QProcess *process = new QProcess;
    QMetaObject::Connection connFinished = QObject::connect(process, static_cast<void (QProcess::*)(int exitCode, QProcess::ExitStatus exitStatus)>(&QProcess::finished), [process, button, commandLine] (int exitCode, QProcess::ExitStatus exitStatus){
        if(process->state() == QProcess::NotRunning){
           qDebug() << commandLine << " finished: " << process->processId();
           if(process->exitStatus()  == QProcess::NormalExit){
              qDebug() << commandLine << " exited normal: " << process->processId() << "exitcode: " << process->exitCode();
              QByteArray output = process->readAllStandardOutput();
              qDebug() << output;
           }
        }
        delete process;
        // process = nullptr;
        if(button) button->setEnabled(true);

    });
    // QObject::connect(process, static_cast<void (QProcess::*)(QProcess::ProcessError error)>(&QProcess::errorOccurred), [process, button, commandLine] (QProcess::ProcessError error)
    QObject::connect(process, &QProcess::errorOccurred, [process, button, commandLine] (QProcess::ProcessError error)
    {
        qDebug() << commandLine << " process error: " << error << " pid: " << process->processId();
        QByteArray output = process->readAllStandardError();
        qDebug() << output;
        delete process;
        if(button) button->setEnabled(true);
        // process = nullptr;
    });

    process->start( commandLine );
    process->waitForStarted();
    qDebug() << commandLine << " process started: " << process->processId();
    // bringAppToForeground(process->processId());
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

int openTerminal(const QString &path, const QString &command=QString())
{
    QString scriptCmd = "tell application \"Terminal\" to do script ";
    scriptCmd += "\"";
    scriptCmd += "cd ";
    scriptCmd += path;
    if(command.length())
    {
       scriptCmd += " && " + command;
    }
    scriptCmd += "\"";
    return QProcess::startDetached("osascript",  QStringList() << "-e" << scriptCmd);
}

void MainForm::generateJahresDkBestaetigungen()
{
    if(!checkPrerequisitesExists()){
        return;
    }

    generateJahresDkBestaetigungenButton->setEnabled(false);
    QString JahresDkBestaetigungenPath = getJahresDkBestaetigungenPath();
    QDir().mkpath(JahresDkBestaetigungenPath);
    QString fileName = JahresDkBestaetigungenPath + QDir::separator() + "Jahreskontoauszug.html";
    QString strDatum = QDate::currentDate().toString("dd.MM.yyyy");
    QString strJahr = QString::number(getJahr());
    QString strJahr4 = QString::number(2000 + getJahr());
    QString strSylvester = "31.12." + strJahr4;

    // TODO: Initialisierung und Aufräumen (z.B. selection merken und wieder setzen)
    // QModelIndex curIndex = personenView->currentIndex();
    buchungenView->sortByColumn(DkBuchungen_DKNummer, Qt::AscendingOrder);

    for(int i=0;i<personenModel->rowCount();i++)
    {
        personenView->selectRow(i);
        QModelIndex PersonIndex = personenView->currentIndex();
        //QModelIndex PersonIndex = personenModel->index(i, 0);
        if(PersonIndex.isValid())
        {
            // Datei pro Person mit allen Buchungen
            // <PersonId>_<EMail>_<Vorname>_<Name>.txt
            QString personFilePath = JahresDkBestaetigungenPath + QDir::separator();
            QString personFileName;

            // TODO: PersonId raus, wegen Parsen der EMail in sendDKJAKtos.py
            personFileName += personenModel->data(personenModel->index(PersonIndex.row(), DkPersonen_PersonId)).toString();
            personFileName += "_" + personenModel->data(personenModel->index(PersonIndex.row(), DkPersonen_Email)).toString();
            personFileName += "_" + personenModel->data(personenModel->index(PersonIndex.row(), DkPersonen_Vorname)).toString();
            personFileName += "_" + personenModel->data(personenModel->index(PersonIndex.row(), DkPersonen_Name)).toString();
            personFileName = escapeFileName(personFileName);
            personFilePath += personFileName;

            QString personFileNameHtml = personFilePath + ".html";
            QString str = readFromFile(fileName);
            str = str.replace("&lt;Vorname&gt;", personenModel->data(personenModel->index(PersonIndex.row(), DkPersonen_Vorname)).toString());
            str = str.replace("&lt;Name&gt;", personenModel->data(personenModel->index(PersonIndex.row(), DkPersonen_Name)).toString());

            str = str.replace("&lt;Strasse&gt;", personenModel->data(personenModel->index(PersonIndex.row(), DkPersonen_Strasse)).toString());
            str = str.replace("&lt;PLZ&gt;", personenModel->data(personenModel->index(PersonIndex.row(), DkPersonen_PLZ)).toString());
            str = str.replace("&lt;Ort&gt;", personenModel->data(personenModel->index(PersonIndex.row(), DkPersonen_Ort)).toString());
            str = str.replace("&lt;Datum&gt;", strDatum);
            str = str.replace("&lt;Jahr&gt;", strJahr4);

            QString strAuflistung;
            strAuflistung += "<table width=\"100%\" border=\"1\">\n";
            // strAuflistung += "<caption>Direktkredite</caption>\n";

            QString strHeader;
            strHeader += "<thead>\n";
            strHeader += "<tr>\n";
            strHeader += "<th align=\"left\">Direktkredit</th>\n";
            strHeader += "<th align=\"right\">Stand</th>\n";
            strHeader += "<th align=\"left\">am</th>\n";
            strHeader += "<th align=\"left\">Zinssatz</th>\n";
            strHeader += "<th align=\"right\">Zinsen</th>\n";
            strHeader += "<th align=\"right\">Stand</th>\n";
            strHeader += "<th align=\"left\">zum</th>\n";
            strHeader += "</tr>\n";
            strHeader += "</thead>\n";
            strAuflistung += strHeader;

            double SummeBetrag = 0.0;
            double SummeZinsen = 0.0;
            double SummeEndbetrag = 0.0;
            strAuflistung += "<tbody>\n";
            for(int j=0;j<buchungenSortModel->rowCount();j++)
            {
                buchungenView->selectRow(j);
                QModelIndex BuchungIndex = buchungenView->currentIndex();
                if(BuchungIndex.isValid())
                {
                    QString strZeile;
                    // // strZeile += "<tr>\n";
                    //  // strZeile += "<td colspan=\"4\">Direktkredit: &lt;DkNummer&gt;</td>\n";
                    //  // strZeile += "</tr>\n";
                    // strZeile += "<tr>\n";
                    // strZeile += "<td>Direktkredit</td>\n";
                    // strZeile += "<td>Stand zum &lt;Anfangsdatum&gt;</td>\n";
                    // strZeile += "<td>Zinssatz</td>\n";
                    // strZeile += "<td>Zinsen</td>\n";
                    // strZeile += "<td>Stand zum &lt;Enddatum&gt;</td>\n";
                    // strZeile += "</tr>\n";
                    strZeile += "<tr>\n";
                    strZeile += "<td align=\"left\">&lt;DkNummer&gt;</td>\n";
                    strZeile += "<td align=\"right\">&lt;Anfangsbetrag&gt;</td>\n";
                    strZeile += "<td align=\"left\">&lt;Anfangsdatum&gt;</td>\n";
                    strZeile += "<td align=\"left\">&lt;Zinssatz&gt;</td>\n";
                    strZeile += "<td align=\"right\">&lt;Zinsen&gt;</td>\n";
                    strZeile += "<td align=\"right\">&lt;Endbetrag&gt;</td>\n";
                    strZeile += "<td align=\"left\">&lt;Enddatum&gt;</td>\n";
                    strZeile += "</tr>\n";

                    QString strDkNummer = buchungenSortModel->data(buchungenSortModel->index(BuchungIndex.row(), DkBuchungen_DKNummer)).toString();
                    QString strAnfangsdatum = buchungenSortModel->data(buchungenSortModel->index(BuchungIndex.row(), DkBuchungen_Datum)).toString();
                    double Zinssatz = buchungenSortModel->data(buchungenSortModel->index(BuchungIndex.row(), DkBuchungen_Zinssatz)).toDouble();
                    double Betrag = buchungenSortModel->data(buchungenSortModel->index(BuchungIndex.row(), DkBuchungen_Betrag)).toDouble();
                    // QString strEnddatum = buchungenSortModel->data(buchungenSortModel->index(BuchungIndex.row(), DkBuchungen_vorgemerkt)).toString();
                    QString strRueckzahlung = buchungenSortModel->data(buchungenSortModel->index(BuchungIndex.row(), DkBuchungen_Rueckzahlung)).toString();
                    if(strRueckzahlung.length() && !strRueckzahlung.endsWith(strJahr)){
                        continue;
                    }
                    QString strEnddatum;
                    bool bRueckzahlung = false;
                    if(strRueckzahlung.endsWith(strJahr))
                    {
                       bRueckzahlung = true;
                       strEnddatum = strRueckzahlung;
                    }
                    if(strEnddatum.length() == 0)
                    {
                        strEnddatum = "31.12." + strJahr;
                    }
                    // Tagesgenaue Berechnung
                    QDate dateFrom = QDate::fromString(strAnfangsdatum, "dd.MM.yy");
                    QDate dateTo = QDate::fromString(strEnddatum, "dd.MM.yy");
                    bool inclLastDay = true;
                    if(bRueckzahlung){
                       // dateTo = dateTo.addDays(-1);
                        inclLastDay = false;
                    }

                    // int anzTage = getAnzTageJahr();
                    // if(dateFrom.isValid() && dateTo.isValid()){
                    //     anzTage = qMin((int)dateFrom.daysTo(dateTo), anzTage);
                    // }
                    int anzTage = getAnzTage(dateFrom, dateTo, inclLastDay);
                    // double Zinsen = ((Betrag * Zinssatz) / 100.0);
                    // Zinsen = Runden(Zinsen * anzTage / getAnzTageJahr(), 2);
                    double Zinsen = computeDkZinsen(Betrag, Zinssatz, anzTage);
                    double Endbetrag = Betrag + Zinsen;

                    SummeBetrag += Betrag;
                    SummeZinsen += Zinsen;
                    SummeEndbetrag += Endbetrag;

                    // strAnfangsdatum = QDate::fromString(strAnfangsdatum, "dd.MM.yy").toString("dd.MM.yyyy");
                    QString strAnfangsbetrag = QString("%L1").arg(Betrag ,12,'f',2,' ') + " &euro;";
                    QString strZinssatz = QString("%L1").arg(Zinssatz ,12,'f',2,' ') + " &#37;";
                    QString strZinsen = QString("%L1").arg(Zinsen ,12,'f',2,' ') + " &euro;";
                    QString strEndbetrag = QString("%L1").arg(Endbetrag ,12,'f',2,' ') + " &euro;";

                    strZeile = strZeile.replace("&lt;DkNummer&gt;", strDkNummer);
                    strZeile = strZeile.replace("&lt;Anfangsdatum&gt;", strAnfangsdatum);
                    strZeile = strZeile.replace("&lt;Anfangsbetrag&gt;", strAnfangsbetrag);
                    strZeile = strZeile.replace("&lt;Zinssatz&gt;", strZinssatz);
                    strZeile = strZeile.replace("&lt;Zinsen&gt;", strZinsen);
                    strZeile = strZeile.replace("&lt;Endbetrag&gt;", strEndbetrag);
                    strZeile = strZeile.replace("&lt;Enddatum&gt;", strEnddatum);

                    strAuflistung += strZeile;
                }
            }
            strAuflistung += "</tbody>\n";

            QString strSummeBetrag = QString("%L1").arg(SummeBetrag ,12,'f',2,' ') + " &euro;";
            QString strSummeZinsen = QString("%L1").arg(SummeZinsen ,12,'f',2,' ') + " &euro;";
            QString strSummeEndbetrag = QString("%L1").arg(SummeEndbetrag ,12,'f',2,' ') + " &euro;";

            QString strFooter;
            strFooter += "<tfoot>\n";
            strFooter += "<tr>\n";
            strFooter += "<th align=\"left\">Summen</th>\n";
            strFooter += "<th align=\"right\">&lt;SummeBetrag&gt;</th>\n";
            strFooter += "<th></th>\n";
            strFooter += "<th></th>\n";
            strFooter += "<th align=\"right\">&lt;SummeZinsen&gt;</th>\n";
            strFooter += "<th align=\"right\">&lt;SummeEndbetrag&gt;</th>\n";
            strFooter += "<th></th>\n";
            strFooter += "</tr>\n";
            strFooter += "</tfoot>\n";

            strFooter = strFooter.replace("&lt;SummeBetrag&gt;", strSummeBetrag);
            strFooter = strFooter.replace("&lt;SummeZinsen&gt;", strSummeZinsen);
            strFooter = strFooter.replace("&lt;SummeEndbetrag&gt;", strSummeEndbetrag);

            strAuflistung += strFooter;
            strAuflistung += "</table>\n";
            str = str.replace("&lt;Auflistung&gt;", strAuflistung);

            QString personEmail = personenModel->data(personenModel->index(PersonIndex.row(), DkPersonen_Email)).toString();

            if(nurCheckBox->isChecked())
            {
                QString nurEditText = nurEdit->text();
                QString testUserEmailAdress = getTestUserEmailAdress();
                if(nurEditText.compare(testUserEmailAdress, Qt::CaseInsensitive))
                {
                    setTestUserEmailAdress(nurEditText);
                }
                if( (nurEditText.length() == 0) || (nurEditText.compare(personEmail, Qt::CaseInsensitive) != 0) )
                {
                    continue;
                }
            }

            if(str.length() == 0)
                continue;
            writeHtmlTextToHtmlFile(personFileNameHtml, str);

            QString fileNamePdf = personFileNameHtml;
            fileNamePdf = fileNamePdf.replace(".html", ".pdf");
            convertHtmlFileToPdfFile(personFileNameHtml, fileNamePdf);

            if(!sendenCheckBox->isChecked())
            {
                if(nurCheckBox->isChecked())
                {
                    QUrl url = QStringLiteral("file://") + fileNamePdf;
                    QDesktopServices::openUrl(url);
                }
                continue;
            }

            // https://forum.qt.io/topic/85050/sending-email-attachment-via-default-email-client/4
            // QDesktopServices::openUrl(QUrl("mailto:?subject=test&body=test&attach=C:/myfolder/file.pdf") );
            // attachment='file:///c:/test.txt'
            // https://stackoverflow.com/questions/45588952/how-can-i-compose-and-send-email-in-thunderbird-from-commandline
            QString mailto = personEmail;
            QString mailfrom = "13hafreiheit@gmx.de";
            QString subject = "Kontoauszug 2020 DK F13 Turley GmbH";
            QString fileNameMailContent = getJahresDkBestaetigungenPath() + QDir::separator() + "mail-content.txt";
            QString body = readFromFile(fileNameMailContent);
            body = body.replace("&lt;Vorname&gt;", personenModel->data(personenModel->index(PersonIndex.row(), DkPersonen_Vorname)).toString());
            body = body.replace("&lt;Name&gt;", personenModel->data(personenModel->index(PersonIndex.row(), DkPersonen_Name)).toString());

            QString attach = fileNamePdf;
            // kein Attachment möglich
            // QString attach = "file:///" + fileNamePdf;
            // QString url = QString("mailto:%1?subject=%2&body=%3&attach=%4").arg(mailto).arg(subject).arg(body).arg(attach);
            // QDesktopServices::openUrl( QUrl(url) );

            QString thunderbirdApp = getThunderbirdPath();
            QStringList thunderbirdParams; // -compose
            QString thunderbirdCompose = QString("to='%1',subject='%2',body='%3',attachment='%4'").arg(mailto).arg(subject).arg(body).arg(attach);
            QProcess::startDetached(thunderbirdApp, QStringList() << "-compose" << thunderbirdCompose);

            if(nurCheckBox->isChecked())
            {
                continue;
            }

            QEventLoop eventLoop;
            continueButton->setEnabled(true);
            cancelButton->setEnabled(true);
            QObject::connect(continueButton, &QPushButton::clicked, &eventLoop, &QEventLoop::quit);
            QObject::connect(cancelButton, &QPushButton::clicked, &eventLoop, &QEventLoop::quit);
            connect(continueButton, &QPushButton::clicked, [this](){
                continueButtonClicked = 1;
            });
            connect(cancelButton, &QPushButton::clicked, [this](){
                cancelButtonClicked = 1;
            });
            continueButtonClicked = 0;
            cancelButtonClicked = 0;
            eventLoop.exec();
            continueButton->setEnabled(false);
            cancelButton->setEnabled(false);
            if(cancelButtonClicked)
                break;
            if(continueButtonClicked)
                continue;
         }
    }
    generateJahresDkBestaetigungenButton->setEnabled(true);
}

void MainForm::generateJahresDkBestaetigungenTerminal()
{
    if(!checkPrerequisitesExists()){
        return;
    }
    QString JahresDkBestaetigungenPath = getJahresDkBestaetigungenPath();
    QString cmd = "./printCommandDescription.sh";
    openTerminal(JahresDkBestaetigungenPath, cmd);
}

void MainForm::generateJahresDkZinsBescheinigungen()
{
    if(!checkPrerequisitesExists()){
        return;
    }
   generateJahresDkZinsBescheinigungenButton->setEnabled(false);
   QString JahresDkZinsBescheinigungenPath = getJahresDkZinsBescheinigungenPath();
   QDir().mkpath(JahresDkZinsBescheinigungenPath);
   QString fileName = JahresDkZinsBescheinigungenPath + QDir::separator() + "Zinsbescheinigung.html";
   QString strDatum = QDate::currentDate().toString("dd.MM.yyyy");
   QString strJahr = QString::number(getJahr());
   QString strJahr4 = QString::number(2000 + getJahr());
   QString strSylvester = "31.12." + strJahr4;

   // TODO: Initialisierung und Aufräumen (z.B. selection merken und wieder setzen)
   // QModelIndex curIndex = personenView->currentIndex();
   buchungenView->sortByColumn(DkBuchungen_DKNummer, Qt::AscendingOrder);

   for(int i=0;i<personenModel->rowCount();i++)
   {
       personenView->selectRow(i);
       QModelIndex PersonIndex = personenView->currentIndex();
       //QModelIndex PersonIndex = personenModel->index(i, 0);
       if(PersonIndex.isValid())
       {
           // Datei pro Person mit allen Buchungen
           // <PersonId>_<EMail>_<Vorname>_<Name>.txt
           QString personFilePath = JahresDkZinsBescheinigungenPath + QDir::separator();
           QString personFileName;

           // TODO: PersonId raus, wegen Parsen der EMail in sendDKJAKtos.py
           personFileName += personenModel->data(personenModel->index(PersonIndex.row(), DkPersonen_PersonId)).toString();
           personFileName += "_" + personenModel->data(personenModel->index(PersonIndex.row(), DkPersonen_Email)).toString();
           personFileName += "_" + personenModel->data(personenModel->index(PersonIndex.row(), DkPersonen_Vorname)).toString();
           personFileName += "_" + personenModel->data(personenModel->index(PersonIndex.row(), DkPersonen_Name)).toString();
           personFileName = escapeFileName(personFileName);
           personFilePath += personFileName;

           QString personFileNameHtml = personFilePath + ".html";
           QString str = readFromFile(fileName);
           str = str.replace("&lt;Vorname&gt;", personenModel->data(personenModel->index(PersonIndex.row(), DkPersonen_Vorname)).toString());
           str = str.replace("&lt;Name&gt;", personenModel->data(personenModel->index(PersonIndex.row(), DkPersonen_Name)).toString());

           str = str.replace("&lt;Strasse&gt;", personenModel->data(personenModel->index(PersonIndex.row(), DkPersonen_Strasse)).toString());
           str = str.replace("&lt;PLZ&gt;", personenModel->data(personenModel->index(PersonIndex.row(), DkPersonen_PLZ)).toString());
           str = str.replace("&lt;Ort&gt;", personenModel->data(personenModel->index(PersonIndex.row(), DkPersonen_Ort)).toString());
           str = str.replace("&lt;Datum&gt;", strDatum);
           str = str.replace("&lt;Jahr&gt;", strJahr4);

           QString strAuflistung;
           strAuflistung += "<table width=\"100%\" border=\"1\">\n";
           // strAuflistung += "<caption>Direktkredite</caption>\n";

           QString strHeader;
           strHeader += "<thead>\n";
           strHeader += "<tr>\n";
           strHeader += "<th align=\"left\">Direktkredit</th>\n";
           strHeader += "<th align=\"right\">Jahreszinsbetrag</th>\n";
           strHeader += "<th align=\"left\">Zinssatz</th>\n";
           strHeader += "</tr>\n";
           strHeader += "</thead>\n";
           strAuflistung += strHeader;

           double SummeBetrag = 0.0;
           double SummeZinsen = 0.0;
           double SummeEndbetrag = 0.0;
           strAuflistung += "<tbody>\n";
           for(int j=0;j<buchungenSortModel->rowCount();j++)
           {
               buchungenView->selectRow(j);
               QModelIndex BuchungIndex = buchungenView->currentIndex();
               if(BuchungIndex.isValid())
               {
                   QString strZeile;
                   strZeile += "<tr>\n";
                   strZeile += "<td align=\"left\">&lt;DkNummer&gt;</td>\n";
                   strZeile += "<td align=\"right\">&lt;Jahreszinsbetrag&gt;</td>\n";
                   strZeile += "<td align=\"left\">&lt;Zinssatz&gt;</td>\n";
                   strZeile += "</tr>\n";

                   QString strDkNummer = buchungenSortModel->data(buchungenSortModel->index(BuchungIndex.row(), DkBuchungen_DKNummer)).toString();
                   QString strAnfangsdatum = buchungenSortModel->data(buchungenSortModel->index(BuchungIndex.row(), DkBuchungen_Datum)).toString();
                   double Zinssatz = buchungenSortModel->data(buchungenSortModel->index(BuchungIndex.row(), DkBuchungen_Zinssatz)).toDouble();
                   double Betrag = buchungenSortModel->data(buchungenSortModel->index(BuchungIndex.row(), DkBuchungen_Betrag)).toDouble();
                   // QString strEnddatum = buchungenSortModel->data(buchungenSortModel->index(BuchungIndex.row(), DkBuchungen_vorgemerkt)).toString();
                   QString strRueckzahlung = buchungenSortModel->data(buchungenSortModel->index(BuchungIndex.row(), DkBuchungen_Rueckzahlung)).toString();
                   if(strRueckzahlung.length() && !strRueckzahlung.endsWith(strJahr)){
                       continue;
                   }
                   QString strEnddatum;
                   bool bRueckzahlung = false;
                   if(strRueckzahlung.endsWith(strJahr))
                   {
                      bRueckzahlung = true;
                      strEnddatum = strRueckzahlung;
                   }
                   if(strEnddatum.length() == 0)
                   {
                       strEnddatum = "31.12." + strJahr;
                   }
                   // Tagesgenaue Berechnung
                   QDate dateFrom = QDate::fromString(strAnfangsdatum, "dd.MM.yy");
                   QDate dateTo = QDate::fromString(strEnddatum, "dd.MM.yy");
                   bool inclLastDay = true;
                   if(bRueckzahlung){
                       inclLastDay = false;
                   }
                   int anzTage = getAnzTage(dateFrom, dateTo, inclLastDay);
                   double Zinsen = computeDkZinsen(Betrag, Zinssatz, anzTage);
                   double Endbetrag = Betrag + Zinsen;

                   SummeBetrag += Betrag;
                   SummeZinsen += Zinsen;
                   SummeEndbetrag += Endbetrag;

                   // strAnfangsdatum = QDate::fromString(strAnfangsdatum, "dd.MM.yy").toString("dd.MM.yyyy");
                   QString strAnfangsbetrag = QString("%L1").arg(Betrag ,12,'f',2,' ') + " &euro;";
                   QString strZinssatz = QString("%L1").arg(Zinssatz ,12,'f',2,' ') + " &#37;";
                   QString strZinsen = QString("%L1").arg(Zinsen ,12,'f',2,' ') + " &euro;";
                   QString strEndbetrag = QString("%L1").arg(Endbetrag ,12,'f',2,' ') + " &euro;";

                   strZeile = strZeile.replace("&lt;DkNummer&gt;", strDkNummer);
                   strZeile = strZeile.replace("&lt;Jahreszinsbetrag&gt;", strZinsen);
                   strZeile = strZeile.replace("&lt;Zinssatz&gt;", strZinssatz);

                   strAuflistung += strZeile;
               }
           }
           strAuflistung += "</tbody>\n";

           QString strSummeBetrag = QString("%L1").arg(SummeBetrag ,12,'f',2,' ') + " &euro;";
           QString strSummeZinsen = QString("%L1").arg(SummeZinsen ,12,'f',2,' ') + " &euro;";
           QString strSummeEndbetrag = QString("%L1").arg(SummeEndbetrag ,12,'f',2,' ') + " &euro;";

           QString strFooter;
           strFooter += "<tfoot>\n";
           strFooter += "<tr>\n";
           strFooter += "<th align=\"left\">Summen</th>\n";
           strFooter += "<th align=\"right\">&lt;SummeZinsen&gt;</th>\n";
           strFooter += "<th></th>\n";
           strFooter += "</tr>\n";
           strFooter += "</tfoot>\n";

           strFooter = strFooter.replace("&lt;SummeBetrag&gt;", strSummeBetrag);
           strFooter = strFooter.replace("&lt;SummeZinsen&gt;", strSummeZinsen);
           strFooter = strFooter.replace("&lt;SummeEndbetrag&gt;", strSummeEndbetrag);

           strAuflistung += strFooter;
           strAuflistung += "</table>\n";
           str = str.replace("&lt;Auflistung&gt;", strAuflistung);

           writeToFile(personFileNameHtml, str);
           // TODO: In PDF umwandeln oder per Script

           // TODO: txt-Dateien für Anschreiben in Mail generieren oder per Script
           // QString personFileNameTxt = personFilePath + ".txt";

           // TODO: Evtl. hier einzeln senden ( sendDKJAKtos.py) oder per Script
        }
        // TODO: Evtl. hier  alle senden ( sendDKJAKtos.py) oder per Script
   }
   generateJahresDkZinsBescheinigungenButton->setEnabled(true);
}

void MainForm::generateJahresDkZinsBescheinigungenTerminal()
{
    if(!checkPrerequisitesExists()){
        return;
    }
    QString JahresDkZinsBescheinigungenPath = getJahresDkZinsBescheinigungenPath();
    QString cmd = "./printCommandDescription.sh";
    openTerminal(JahresDkZinsBescheinigungenPath,cmd);
}

void MainForm::filterBuchungen()
{
    // QMessageBox::warning(this, tr("Buchungen filtern"), tr("Nicht implementiert!"), QMessageBox::Ok); return;
    updateBuchungenView();
}

void MainForm::updateBuchungenView()
{
    // bool bPersonenVisible = personenPanel->isVisible();
    bool bAnzeigenPersonenButton = anzeigenPersonenButton->isChecked();
    bool bPersonenVisible = !bAnzeigenPersonenButton;
    if(!bAnzeigenPersonenButton){
        QModelIndex index = personenView->currentIndex();
        if (index.isValid()) {
            QSqlRecord record = personenModel->record(index.row());
            int PersonId = record.value("PersonId").toInt();
            buchungenModel->setFilter(QString("PersonId = %1").arg(PersonId));
            buchungenLabel->setText(tr("Buchungen von %1 %2").arg(record.value("Vorname").toString()).arg(record.value("Name").toString()));
        } else {
            buchungenModel->setFilter("PersonId = -1");
            buchungenLabel->setText(tr("Buchungen"));
        }
    }else{
        if(vorgemerktCheckBox->isChecked()){
            buchungenModel->setFilter("vorgemerkt <> ''");
        }else{
            buchungenModel->setFilter("");
        }
        buchungenLabel->setText(tr("Buchungen"));
    }
    // QModelIndex index = personenView->currentIndex();
    // if (index.isValid()) {
    //     QSqlRecord record = personenModel->record(index.row());
    //     int PersonId = record.value("PersonId").toInt();
    //     buchungenLabel->setText(tr("Buchungen von %1 %2").arg(record.value("Vorname").toString()).arg(record.value("Name").toString()));
    // }else{
    //     buchungenLabel->setText(tr("Buchungen"));
    // }
    QModelIndex index = buchungenView->currentIndex();
    buchungenModel->select();
    buchungenView->horizontalHeader()->setVisible(buchungenModel->rowCount() > 0);
    // vorherigen Index selektieren
    int newRow = qMin(index.row(), buchungenSortModel->rowCount() - 1);
    QModelIndex newIndex = buchungenSortModel->index(newRow, 0);
    // wenn neuen Index selektieren, dann
    // QModelIndex newIndex = buchungenSortModel->mapFromSource();
    buchungenView->setCurrentIndex(newIndex);
    addBuchungButton->setEnabled(bPersonenVisible);
    // evtl. im Dialog disablen, dann PeronIs´d -1 übergeben...
    // editBuchungButton->setEnabled(bPersonenVisible);

    updateBuchungenSummen();
    updateSummen();
}

void MainForm::updateBuchungenSummen()
{
    QString zinsenText = "Summe Zinsen zum " + datumBuchungenDkZinsenEdit->text();
    summeBuchungenDkZinsenLabel->setText(zinsenText);

    // DKV2: vStat_aktiverVertraege_thesa

    // CREATE VIEW vStat_aktiverVertraege_thesa AS SELECT *, ROUND(100* Jahreszins/Wert,6) as gewMittel FROM (SELECT count(*) as Anzahl, SUM(Wert) as Wert, SUM(ROUND(Zinssatz *Wert /100,2)) AS Jahreszins,ROUND(AVG(Zinssatz),4) as mittlereRate FROM vVertraege_aktiv WHERE thesa)

    // CREATE VIEW vVertraege_aktiv AS SELECT id ,Kreditorin,Vertragskennung,Zinssatz,Wert,Aktivierungsdatum,Kuendigungsfrist,Vertragsende,thesa,KreditorId FROM vVertraege_aktiv_detail

    // CREATE VIEW vVertraege_aktiv_detail AS SELECT Vertraege.id AS id, Vertraege.Kennung AS Vertragskennung, Vertraege.ZSatz /100. AS Zinssatz, SUM(Buchungen.Betrag) /100. AS Wert, MIN(Buchungen.Datum)  AS Aktivierungsdatum, Vertraege.Kfrist AS Kuendigungsfrist, Vertraege.LaufzeitEnde  AS Vertragsende, Vertraege.thesaurierend AS thesa
    // , Kreditoren.Nachname || ', ' || Kreditoren.Vorname AS Kreditorin, Kreditoren.id AS KreditorId,Kreditoren.Nachname AS Nachname,Kreditoren.Vorname AS Vorname,Kreditoren.Strasse AS Strasse
    // ,Kreditoren.Plz AS Plz, Kreditoren.Stadt AS Stadt, Kreditoren.Email AS Email, Kreditoren.IBAN AS Iban, Kreditoren.BIC AS Bic
    // FROM Vertraege INNER JOIN Buchungen  ON Buchungen.VertragsId = Vertraege.id INNER JOIN Kreditoren ON Kreditoren.id = Vertraege.KreditorId GROUP BY Vertraege.id

    QString statementDk = "SELECT SUM(Betrag) FROM DkBuchungen";
    int PersonId = getPersonId();
    if((PersonId != -1) && !anzeigenPersonenButton->isChecked()){
        statementDk += " WHERE PersonId=";
        statementDk += QString::number(PersonId);
    }
    double summeDk = getDoubleValue(statementDk);
    QString summeDkText = QString::number(summeDk, 'f', 2);
    summeBuchungenDkEdit->setText(summeDkText);

    // QString statementDkZinsen = "SELECT ROUND (SUM( (Betrag * Zinssatz) / 100.0 ), 2) FROM DkBuchungen";
    QString statementDkZinsen = "SELECT ROUND ( (Betrag * Zinssatz) / 100.0 ) FROM DkBuchungen";
    if((PersonId != -1) && !anzeigenPersonenButton->isChecked()){
        statementDkZinsen += " WHERE PersonId=";
        statementDkZinsen += QString::number(PersonId);
    }
    double summeDkZinsen = getDoubleValue(statementDkZinsen);
    QDate dateTo = QDate::fromString(datumBuchungenDkZinsenEdit->text(), "dd.MM.yy");
    QDate dateFrom = QDate(dateTo.year(), 1, 1);
    // int anzTage = getAnzTageJahr();
    // if(dateFrom.isValid() && dateTo.isValid()){
    //     anzTage = qMin((int)dateFrom.daysTo(dateTo), getAnzTageJahr());
    // }
    int anzTage = getAnzTage(dateFrom, dateTo, true);
    summeDkZinsen = Runden2(summeDkZinsen * anzTage / getAnzTageJahr());
    QString summeDkZinsenText = QString::number(summeDkZinsen, 'f', 2);
    summeBuchungenDkZinsenEdit->setText(summeDkZinsenText);
}

void MainForm::updateViews()
{
    updatePersonenView();
    updateSummen();
}

void MainForm::updatePersonenView()
{
    QModelIndex index = personenView->currentIndex();

    personenModel->select();
    personenView->horizontalHeader()->setVisible(personenModel->rowCount() > 0);

    int row = 0;
    if(index.isValid()){
        row = index.row();
    }
    int newRow = qMin(row, personenModel->rowCount() - 1);
    QModelIndex newIndex = personenModel->index(newRow, 0);
    personenView->setCurrentIndex(newIndex);

    updateBuchungenView();

}

int MainForm::getPersonId()
{
   int PersonId = -1;
   QModelIndex index = personenView->currentIndex();
   if (index.isValid()) {
       QSqlRecord record = personenModel->record(index.row());
       PersonId = record.value(DkPersonen_PersonId).toInt();
   }
   return PersonId;
}

void MainForm::addPerson()
{
    int row = personenModel->rowCount();
    // personenModel->beginInsertRows(index, row, row);
    // personenModel->beginInsertRows(QModelIndex(), row, row);
    bool b = personenModel->insertRow(row);
    Q_UNUSED(b);
    // personenModel->endInsertRows();

    QModelIndex index = personenModel->index(row, DkPersonen_PersonId);
    // noch zu früh
    // personenView->setCurrentIndex(index);

    personenView->edit(index);
    int maxPersonId = getMaxId("DkPersonen", "PersonId") + 1;
    personenModel->setData(index, maxPersonId);
    // personenModel->endInsertRow(index, row);

    // Dialog anzeigen
    int PersonId = maxPersonId;
    PersonForm form(this, personenModel, PersonId);
    form.exec();
    updatePersonenView();
    personenView->setCurrentIndex(index);
}

void MainForm::editPerson(){
    int PersonId = getPersonId();
    PersonForm form(this, personenModel, PersonId);
    form.exec();
    updatePersonenView();
}

void MainForm::deletePerson()
{
    QModelIndex index = personenView->currentIndex();
    if (!index.isValid())
        return;

    QSqlDatabase::database().transaction();
    QSqlRecord record = personenModel->record(index.row());
    int PersonId = record.value(DkPersonen_PersonId).toInt();
    int anzBuchungen = 0;

    QSqlQuery query(QString("SELECT COUNT(*) FROM DkBuchungen "
                            "WHERE PersonId = %1").arg(PersonId));
    if (query.next())
        anzBuchungen = query.value(0).toInt();
    if (anzBuchungen > 0) {
        int r = QMessageBox::warning(this, tr("Person löschen"),
                    tr("%1 und alle Buchunngen löschen?")
                    .arg(record.value(DkPersonen_Name).toString()),
                    QMessageBox::Yes | QMessageBox::No);
        if (r == QMessageBox::No) {
            QSqlDatabase::database().rollback();
            return;
        }

        query.exec(QString("DELETE FROM DkBuchungen "
                           "WHERE PersonId = %1").arg(PersonId));
    }

    personenModel->removeRow(index.row());
    personenModel->submitAll();
    QSqlDatabase::database().commit();

    updatePersonenView();

    personenView->setFocus();
}

void MainForm::suchenEditingFinished()
{
    if(filterPersonButton->isChecked()){
        filterPerson();
    }else{
        suchenPerson();
    }
}

void MainForm::suchenPerson()
{
    int searchVal = -1; // QVariant searchVal;
    if(personIdEdit->text().length()){
        searchVal = personIdEdit->text().toInt();
    }else{
        QString statement;
        statement += "SELECT PersonId FROM DkPersonen";
        statement += " WHERE ";
        statement += "Vorname LIKE '%" + vornameEdit->text() + "%'";
        statement += " AND ";
        statement += "Name LIKE '%" + nameEdit->text() + "%'";
        int PersonId = getPersonId();
        QString condition;
        condition = " AND PersonID > ";
        condition += QString::number(PersonId);
        searchVal = getIntValue(statement + condition);
        if(searchVal == -1){
            condition = " AND PersonID > 0";
            searchVal = getIntValue(statement + condition);
        }

    }
    if(searchVal == -1) // if(!searchVal.isValid())
        return;

    // QModelIndex curindex = personenView->currentIndex();
    int searchCol = DkPersonen_PersonId; // DkPersonen_Vorname = 1, DkPersonen_Name = 2,
    int startIndexRow = 0; // index.row()+1;
    startIndexRow = startIndexRow < personenModel->rowCount() ? startIndexRow : 0;
    QModelIndex startIndex = personenModel->index(startIndexRow, searchCol);
    // QModelIndexList QAbstractItemModel::match(const QModelIndex & start, int role, const QVariant & value, int hits = 1, Qt::MatchFlags flags = Qt::MatchFlags( Qt::MatchStartsWith | Qt::MatchWrap )) const
    QModelIndexList modelList = personenModel->match(startIndex, Qt::DisplayRole, searchVal);
    if(modelList.count()){
        personenView->setCurrentIndex(modelList.at(0));
    }

}

void MainForm::filterPerson()
{
    //QString filter = personenModel->filter();
    //if(filter.length() == 0){
    if(filterPersonButton->isChecked()){
        QString filter = "";
        if(personIdEdit->text().length()){
            filter = "PersonId = " + QString::number(personIdEdit->text().toInt());
        }else{
            filter += "Vorname LIKE '%" + vornameEdit->text() + "%'";
            filter += " AND ";
            filter += "Name LIKE '%" + nameEdit->text() + "%'";
        }
        personenModel->setFilter(filter);
    }else{
        personenModel->setFilter("");
    }
    updatePersonenView();
}

void MainForm::addBuchung()
{
    // buchenModel->beginInsertRow(index, row);

    int row = buchungenModel->rowCount();
    bool b = buchungenModel->insertRow(row);
    Q_UNUSED(b);
    QModelIndex indexBuchungId = buchungenModel->index(row, DkBuchungen_BuchungId);
    buchungenView->setCurrentIndex(indexBuchungId);
    buchungenView->edit(indexBuchungId);
    int BuchungId = getMaxId("DkBuchungen", "BuchungId") + 1;
    buchungenModel->setData(indexBuchungId, BuchungId);

    QModelIndex indexBuchungPersonId = buchungenModel->index(row, DkBuchungen_PersonId);
    buchungenView->edit(indexBuchungPersonId);
    int PersonId = getPersonId();
    buchungenModel->setData(indexBuchungPersonId, PersonId);

    // buchungenModel->endInsertRow(index, row);

    QModelIndex indexPersonId = personenView->currentIndex();
    BuchungForm form(this, personenModel, indexPersonId, buchungenModel, indexBuchungId);
    form.exec();
    updateBuchungenView();
}

int MainForm::getBuchungId()
{
   int BuchungId = -1;
   QModelIndex index = buchungenView->currentIndex();
   if (index.isValid()) {
       QSqlRecord record = buchungenModel->record(index.row());
       BuchungId = record.value(DkBuchungen_BuchungId).toInt();
   }
   return BuchungId;
}

void MainForm::editBuchung()
{
    // int PersonId = getPersonId();
    // int BuchungId = getBuchungId();
    // QModelIndex indexBuchungId = buchungenView->currentIndex();
    // QModelIndex indexPersonId = personenView->currentIndex();
    QModelIndex indexBuchungId = buchungenSortModel->index(buchungenView->currentIndex().row(), DkBuchungen_BuchungId);
    indexBuchungId = buchungenSortModel->mapToSource(indexBuchungId);
    QModelIndex indexPersonId;
    bool bAnzeigenPersonenButton = anzeigenPersonenButton->isChecked();
    bool bPersonenVisible = !bAnzeigenPersonenButton;
    if(bPersonenVisible){
        indexPersonId = personenModel->index(personenView->currentIndex().row(), DkPersonen_PersonId);
    }
    // BuchungForm form(BuchungId, this);
    BuchungForm form(this, personenModel, indexPersonId, buchungenModel, indexBuchungId);
    form.exec();
    updateBuchungenView();
    // buchungenView->setFocus();
}

void MainForm::deleteBuchung(){
    if(QMessageBox::question(this, tr("Buchung löschen"), tr("Soll die aktuelle Buchung gelöscht werden?")) == QMessageBox::No)
        return;

    QModelIndex index = buchungenView->currentIndex();
    if (!index.isValid())
        return;

    // QSqlDatabase::database().transaction();
    buchungenModel->removeRow(index.row());
    buchungenModel->submitAll();
    // QSqlDatabase::database().commit();

    updateBuchungenView();

    // jetzt in updateBuchungenView();
    // int newRow = qMin(index.row(), buchungenModel->rowCount() - 1);
    // QModelIndex newIndex = buchungenModel->index(newRow, 0);
    // buchungenView->setCurrentIndex(newIndex);

    buchungenView->setFocus();

}

void MainForm::updateSummen()
{

    QString statementDk = "SELECT SUM(Betrag) FROM DkBuchungen";
    // QString statementDk = "SELECT SUM(replace(Betrag,',','.')) FROM DkBuchungen";
    double summeDk = getDoubleValue(statementDk);
    QString summeDkText = QString::number(summeDk, 'f', 2);
    summeDkEdit->setText(summeDkText);

    // DKV2: vStat_aktiverVertraege_thesa
    // CREATE VIEW vStat_aktiverVertraege_thesa AS SELECT *, ROUND(100* Jahreszins/Wert,6) as gewMittel FROM (SELECT count(*) as Anzahl, SUM(Wert) as Wert, SUM(ROUND(Zinssatz *Wert /100,2)) AS Jahreszins,ROUND(AVG(Zinssatz),4) as mittlereRate FROM vVertraege_aktiv WHERE thesa)
    // CREATE VIEW vVertraege_aktiv AS SELECT id ,Kreditorin,Vertragskennung,Zinssatz,Wert,Aktivierungsdatum,Kuendigungsfrist,Vertragsende,thesa,KreditorId FROM vVertraege_aktiv_detail
    // CREATE VIEW vVertraege_aktiv_detail AS SELECT Vertraege.id AS id, Vertraege.Kennung AS Vertragskennung, Vertraege.ZSatz /100. AS Zinssatz, SUM(Buchungen.Betrag) /100. AS Wert, MIN(Buchungen.Datum)  AS Aktivierungsdatum, Vertraege.Kfrist AS Kuendigungsfrist, Vertraege.LaufzeitEnde  AS Vertragsende, Vertraege.thesaurierend AS thesa
    // , Kreditoren.Nachname || ', ' || Kreditoren.Vorname AS Kreditorin, Kreditoren.id AS KreditorId,Kreditoren.Nachname AS Nachname,Kreditoren.Vorname AS Vorname,Kreditoren.Strasse AS Strasse
    // ,Kreditoren.Plz AS Plz, Kreditoren.Stadt AS Stadt, Kreditoren.Email AS Email, Kreditoren.IBAN AS Iban, Kreditoren.BIC AS Bic
    // FROM Vertraege INNER JOIN Buchungen  ON Buchungen.VertragsId = Vertraege.id INNER JOIN Kreditoren ON Kreditoren.id = Vertraege.KreditorId GROUP BY Vertraege.id
    // QString statementDkZinsen = "SELECT SUM( (Betrag * Zinssatz) / 100.0 ) FROM DkBuchungen";
    QString statementDkZinsen = "SELECT SUM( ROUND((Betrag * Zinssatz / 100.0),2) ) FROM DkBuchungen";
    double summeDkZinsen = getDoubleValue(statementDkZinsen);
    QString summeDkZinsenText = QString::number(summeDkZinsen, 'f', 2);
    summeDkZinsenEdit->setText(summeDkZinsenText);

    // DKV2: vAnzahl_allerKreditoren
    // CREATE VIEW vAnzahl_allerKreditoren AS
    // SELECT COUNT(*) AS Anzahl FROM (SELECT DISTINCT KreditorId FROM Vertraege)
    QString statementDkGeberInnen = "SELECT COUNT(*) FROM DkPersonen";
    QString summeDkGeberInnenText =  getStringValue(statementDkGeberInnen);
    summeDkGeberInnenEdit->setText(summeDkGeberInnenText);

    QString statementDkVertraege = "SELECT COUNT(*) FROM (SELECT DISTINCT DkNummer FROM DkBuchungen);";
    QString summeDkVertraegeText =  getStringValue(statementDkVertraege);
    summeDkVertraegeEdit->setText(summeDkVertraegeText);

}

void MainForm::createSummenPanel()
{
   summenPanel = new QWidget;

   // summenLabel = new QLabel(tr("Summen"));
   summenLabel = nullptr;

   summeDkZinsenEdit = new QLineEdit;
   summeDkZinsenEdit->setReadOnly(true);
   summeDkZinsenEdit->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
   summeDkZinsenLabel = new QLabel(tr("Zinsen zum Jahresende"));
   summeDkZinsenLabel->setBuddy(summeDkZinsenEdit);

   summeDkEdit = new QLineEdit;
   summeDkEdit->setReadOnly(true);
   summeDkEdit->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
   summeDkLabel = new QLabel(tr("Direktkredite"));
   summeDkLabel->setBuddy(summeDkEdit);

   summeDkGeberInnenLabel = new QLabel(tr("Anzahl DkGeber*Innen"));
   summeDkGeberInnenEdit = new QLineEdit;
   summeDkGeberInnenEdit->setReadOnly(true);
   summeDkGeberInnenEdit->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
   summeDkGeberInnenLabel->setBuddy(summeDkGeberInnenEdit);

   summeDkVertraegeLabel = new QLabel(tr("Anzahl Verträge"));
   summeDkVertraegeEdit = new QLineEdit;
   summeDkVertraegeEdit->setReadOnly(true);
   summeDkVertraegeEdit->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
   summeDkVertraegeLabel->setBuddy(summeDkVertraegeEdit);

   QHBoxLayout *hlayout = new QHBoxLayout;
   hlayout->setSizeConstraint(QLayout::SetFixedSize);
   // hlayout->addWidget(summenLabel);
   hlayout->addWidget(summeDkZinsenLabel);
   hlayout->addWidget(summeDkZinsenEdit);
   hlayout->addWidget(summeDkLabel);
   hlayout->addWidget(summeDkEdit);
   hlayout->addWidget(summeDkGeberInnenLabel);
   hlayout->addWidget(summeDkGeberInnenEdit);
   hlayout->addWidget(summeDkVertraegeLabel);
   hlayout->addWidget(summeDkVertraegeEdit);


   summenPanel->setLayout(hlayout);

   updateSummen();
}

void MainForm::toggleAnzeigenPersonen(){
    // QMessageBox::warning(this, tr("Personen-Tabelle ein-/ausbelnden"), tr("Nicht implementiert!"), QMessageBox::Ok); return;
    if(anzeigenPersonenButton->isChecked()){
        personenPanel->hide();
        // personenPanel->setEnabled(false);
        // addPersonButton->setEnabled(false);
        anzeigenPersonenButton->setText("Personen-Tabelle einblenden");
        // buchungenLabel->hide();
        vorgemerktCheckBox->show();
        addBuchungButton->setEnabled(false);
    }else{
        personenPanel->show();
        // personenPanel->setEnabled(true);
        // addPersonButton->setEnabled(true);
        anzeigenPersonenButton->setText("Personen-Tabelle ausblenden");
        // buchungenLabel->show();
        vorgemerktCheckBox->hide();
        addBuchungButton->setEnabled(true);
        int PersonId = -1;
        QModelIndex index = buchungenView->currentIndex();
        if (index.isValid()) {
            index = buchungenSortModel->mapToSource(index);
            QSqlRecord record = ((QSqlTableModel *)buchungenSortModel->sourceModel())->record(index.row());
            PersonId = record.value(DkBuchungen_PersonId).toInt();
            // QModelIndexList QAbstractItemModel::match(const QModelIndex & start, int role, const QVariant & value, int hits = 1, Qt::MatchFlags flags = Qt::MatchFlags( Qt::MatchStartsWith | Qt::MatchWrap )) const
            QModelIndex startIndex = personenModel->index(0, DkPersonen_PersonId);
            QModelIndexList modelList = personenModel->match(startIndex, Qt::DisplayRole, PersonId);
            if(modelList.count()){
                personenView->setCurrentIndex(modelList.at(0));
            }
        }
    }
    updateBuchungenView();
    updateBuchungenSummen();
}

void MainForm::createAnzeigenPersonenPanel(){
    anzeigenPersonenPanel = new QWidget;
    anzeigenPersonenLabel = new QLabel(tr("Alle Buchungen anzeigen und Personen-Tabelle ein- oder ausblenden"));
    anzeigenPersonenButton = new QPushButton(tr("Personen-Tabelle ausblenden"));
    anzeigenPersonenButton->setCheckable(true);
    connect(anzeigenPersonenButton,SIGNAL(clicked(bool)), this, SLOT(toggleAnzeigenPersonen()));

    QHBoxLayout *hlayout = new QHBoxLayout;
    hlayout->setSizeConstraint(QLayout::SetFixedSize);
    hlayout->addWidget(anzeigenPersonenLabel);
    hlayout->addWidget(anzeigenPersonenButton);
    anzeigenPersonenPanel->setLayout(hlayout);
}

void MainForm::createPersonenPanel()
{
    personenPanel = new QWidget;

    personenModel = new QSqlTableModel(this);
    personenModel->setTable("DkPersonen");
    // personenModel->setSort(DkPersonen_Name, Qt::AscendingOrder);
    // personenModel->setHeaderData(DkPersonen_Name, Qt::Horizontal, tr("Name"));
    personenModel->setEditStrategy(QSqlTableModel::OnRowChange);
    personenModel->select();

    // suchenPersonLabel = new QLabel(tr("Suchen"));

    personIdEdit = new QLineEdit;
    personIdLabel = new QLabel(tr("PersonId"));
    personIdLabel->setBuddy(personIdEdit);

    vornameEdit = new QLineEdit;
    vornameLabel = new QLabel(tr("Vorname"));
    vornameLabel->setBuddy(vornameEdit);

    nameEdit = new QLineEdit;
    nameLabel = new QLabel(tr("Name"));
    nameLabel->setBuddy(nameEdit);

    // raus wegen Seiteneffekt d.h. Focuswechsel nach Edit-Dialog
    // connect(personIdEdit, SIGNAL(editingFinished()), this, SLOT(suchenEditingFinished()));
    // connect(vornameEdit, SIGNAL(editingFinished()), this, SLOT(suchenEditingFinished()));
    // connect(nameEdit, SIGNAL(editingFinished()), this, SLOT(suchenEditingFinished()));

    suchenPersonButton = new QPushButton(tr("Person suchen"));
    filterPersonButton = new QPushButton(tr("Person filtern"));
    filterPersonButton->setCheckable(true);

    QHBoxLayout *hlayout = new QHBoxLayout;
    hlayout->addWidget(personIdLabel);
    hlayout->addWidget(personIdEdit);
    hlayout->addWidget(vornameLabel);
    hlayout->addWidget(vornameEdit);
    hlayout->addWidget(nameLabel);
    hlayout->addWidget(nameEdit);
    hlayout->addWidget(suchenPersonButton);
    hlayout->addWidget(filterPersonButton);

    connect(suchenPersonButton, SIGNAL(clicked()), this, SLOT(suchenPerson()));
    connect(filterPersonButton, SIGNAL(clicked()), this, SLOT(filterPerson()));

    personenView = new QTableView;
    personenView->setModel(personenModel);
    personenView->setItemDelegate(new QSqlRelationalDelegate(this));
    personenView->setSelectionMode(QAbstractItemView::SingleSelection);
    personenView->setSelectionBehavior(QAbstractItemView::SelectRows);
    // personenView->setEditTriggers(QAbstractItemView::AllEditTriggers);
    personenView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    // personenView->setColumnHidden(DkPersonen_PersonId, true);
    // personenView->setSortingEnabled(true);
    // personenView->sortByColumn(DkPersonen_PersonId);
    personenView->resizeColumnsToContents();
    personenView->horizontalHeader()->setStretchLastSection(true);

    // personenLabel = new QLabel(tr("Personen"));
    // personenLabel->setBuddy(personenView);

    connect(personenView->selectionModel(),
            SIGNAL(currentRowChanged(const QModelIndex &,
                                     const QModelIndex &)),
            this, SLOT(updateBuchungenView()));

    addPersonButton = new QPushButton(tr("Person hinzufügen"));
    editPersonButton = new QPushButton(tr("Person bearbeiten"));
    deletePersonButton = new QPushButton(tr("Person löschen"));

    connect(addPersonButton, SIGNAL(clicked()), this, SLOT(addPerson()));
    connect(editPersonButton, SIGNAL(clicked()), this, SLOT(editPerson()));
    connect(deletePersonButton, SIGNAL(clicked()), this, SLOT(deletePerson()));

    personButtonBox = new QDialogButtonBox;
    personButtonBox->addButton(addPersonButton, QDialogButtonBox::ActionRole);
    personButtonBox->addButton(editPersonButton, QDialogButtonBox::ActionRole);
    personButtonBox->addButton(deletePersonButton, QDialogButtonBox::ActionRole);

    QVBoxLayout *layout = new QVBoxLayout;
    // layout->addWidget(personenLabel);
    // layout->addWidget(suchenPersonLabel);
    layout->addLayout(hlayout);
    layout->addWidget(personenView);
    layout->addWidget(personButtonBox);
    personenPanel->setLayout(layout);

    personenView->setSortingEnabled(true);
    // personenModel->setSort(DkPersonen_Name, Qt::AscendingOrder);
    personenView->sortByColumn(DkPersonen_PersonId, Qt::AscendingOrder);
}

void MainForm::createBuchungenPanel()
{
    buchungenPanel = new QWidget;

    vorgemerktCheckBox = new QCheckBox("nur vorgemerkte Buchungen anzeigen");
    connect(vorgemerktCheckBox, SIGNAL(clicked()), this, SLOT(filterBuchungen()));
    vorgemerktCheckBox->hide();

    buchungenModel = new QSqlRelationalTableModel(this);
    buchungenModel->setTable("DkBuchungen");
    //buchungenModel->setRelation(DkBuchungen_PersonId,
    //        QSqlRelation("DkPersonen", "PersonId", "Name"));
    // buchungenModel->setSort(DkBuchungen_Datum, Qt::AscendingOrder);
    // buchungenModel->setHeaderData(Employee_Name, Qt::Horizontal, tr("Name"));
    buchungenView = new QTableView;
    // buchungenView->setModel(buchungenModel);
    buchungenSortModel = new BuchungenSortFilterProxyModel(this);
    buchungenSortModel->setSourceModel(buchungenModel);
    buchungenView->setModel(buchungenSortModel);
    buchungenView->setSelectionMode(QAbstractItemView::SingleSelection);
    buchungenView->setSelectionBehavior(QAbstractItemView::SelectRows);
    buchungenView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    buchungenView->horizontalHeader()->setStretchLastSection(true);

    buchungenView->setItemDelegateForColumn(DkBuchungen_Betrag, new NumberFormatDelegate(this));
    buchungenView->setItemDelegateForColumn(DkBuchungen_Zinssatz, new NumberFormatDelegate(this));

    // buchungenView->setColumnHidden(DkBuchungen_BuchungId, true);
    // buchungenView->setColumnHidden(DkBuchungen_PersonId, true);

    // dkNrEdit = new QLineEdit;
    // dkNrLabel = new QLabel(tr("DkNr"));
    // dkNrLabel->setBuddy(dkNrEdit);

    // QHBoxLayout *hlayout = new QHBoxLayout;
    // hlayout->addWidget(dkNrLabel);
    // hlayout->addWidget(dkNrEdit);

    buchungenLabel = new QLabel(tr("Buchungen"));
    buchungenLabel->setBuddy(buchungenView);

    datumBuchungenDkZinsenLabel = new QLabel("Datum Zinsen");
    datumBuchungenDkZinsenEdit = new QLineEdit;
    datumBuchungenDkZinsenEdit->setInputMask("00.00.00;_");
    connect(datumBuchungenDkZinsenEdit, SIGNAL(editingFinished()), this, SLOT(updateBuchungenSummen()));
    datumBuchungenDkZinsenEdit->setText(QDate::currentDate().toString("dd.MM.yy"));
    summeBuchungenDkZinsenLabel = new QLabel("Summe Zinsen");
    summeBuchungenDkZinsenEdit = new QLineEdit;
    summeBuchungenDkZinsenEdit->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    summeBuchungenDkZinsenEdit->setReadOnly(true);
    summeBuchungenDkLabel = new QLabel("Summe Direktkredite");
    summeBuchungenDkEdit = new QLineEdit;
    summeBuchungenDkEdit->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    summeBuchungenDkEdit->setReadOnly(true);

    QHBoxLayout *hlayout = new QHBoxLayout;
    hlayout->addWidget(datumBuchungenDkZinsenLabel);
    hlayout->addWidget(datumBuchungenDkZinsenEdit);
    hlayout->addWidget(summeBuchungenDkZinsenLabel);
    hlayout->addWidget(summeBuchungenDkZinsenEdit);
    hlayout->addWidget(summeBuchungenDkLabel);
    hlayout->addWidget(summeBuchungenDkEdit);

    addBuchungButton = new QPushButton(tr("Buchung hinzufügen"));
    editBuchungButton = new QPushButton(tr("Buchung bearbeiten"));
    deleteBuchungButton = new QPushButton(tr("Buchung löschen"));

    connect(addBuchungButton, SIGNAL(clicked()), this, SLOT(addBuchung()));
    connect(editBuchungButton, SIGNAL(clicked()), this, SLOT(editBuchung()));
    connect(deleteBuchungButton, SIGNAL(clicked()), this, SLOT(deleteBuchung()));

    buchungButtonBox = new QDialogButtonBox;
    buchungButtonBox->addButton(addBuchungButton, QDialogButtonBox::ActionRole);
    buchungButtonBox->addButton(editBuchungButton, QDialogButtonBox::ActionRole);
    buchungButtonBox->addButton(deleteBuchungButton, QDialogButtonBox::ActionRole);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(buchungenLabel);
    layout->addWidget(vorgemerktCheckBox);
    layout->addWidget(buchungenView);
    layout->addLayout(hlayout);
    layout->addWidget(buchungButtonBox);
    buchungenPanel->setLayout(layout);

    // buchungenModel->setSort(DkBuchungen_Datum, Qt::AscendingOrder);
    buchungenView->setSortingEnabled(true);
    //buchungenView->sortByColumn(DkBuchungen_Datum, Qt::AscendingOrder);
    buchungenView->sortByColumn(DkBuchungen_BuchungId, Qt::AscendingOrder);
}
