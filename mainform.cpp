#include <QtWidgets>
#include <QtSql>
#include <QSqlRelationalTableModel>
#include <QSortFilterProxyModel>

#include "mainform.h"
#include "personform.h"
#include "buchungform.h"
#include "appconfiguration.h"

#include "dbfkts.h"

#include <QStyledItemDelegate>

class NumberFormatDelegate : public QStyledItemDelegate
{
public:
    explicit NumberFormatDelegate(QObject *parent = nullptr);
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

MainForm::MainForm()
{
    createAnzeigenPersonenPanel();
    createPersonenPanel();
    createBuchungenPanel();
    createSummenPanel();

    splitter = new QSplitter(Qt::Vertical);
    splitter->setFrameStyle(QFrame::StyledPanel);
    splitter->addWidget(personenPanel);
    splitter->addWidget(buchungenPanel);
    //splitter->addWidget(summenPanel);

    QHBoxLayout *hlayout = new QHBoxLayout;
    // hlayout->setSizeConstraint(QLayout::SetFixedSize);
    generateJahresDkBestaetigungenButton = new QPushButton(tr("Jahres Dk-Bestätigungen generieren"));
    hlayout->addWidget(generateJahresDkBestaetigungenButton);
    connect(generateJahresDkBestaetigungenButton, SIGNAL(clicked()), this, SLOT(generateJahresDkBestaetigungen()));
    generateJahresDkZinsBescheinigungenButton = new QPushButton(tr("Jahres Dk-ZinsBescheinigungen generieren"));
    hlayout->addWidget(generateJahresDkZinsBescheinigungenButton);
    connect(generateJahresDkZinsBescheinigungenButton, SIGNAL(clicked()), this, SLOT(generateJahresDkZinsBescheinigungen()));

    quitButton = new QPushButton(tr("Beenden"));
    buttonBox = new QDialogButtonBox;
    buttonBox->addButton(quitButton, QDialogButtonBox::ActionRole);
    connect(quitButton, SIGNAL(clicked()), this, SLOT(close()));

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(anzeigenPersonenPanel);
    mainLayout->addWidget(splitter);
    mainLayout->addWidget(summenPanel);
    mainLayout->addLayout(hlayout);
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

void MainForm::generateJahresDkBestaetigungen()
{
    generateJahresDkBestaetigungenButton->setEnabled(false);
    // Unterordner JahresDkBestaetigungen in getStandardPath()
    // QString JahresDkBestaetigungenPath = getStandardPath() + QDir::separator() + "JahresDkBestaetigungen";
    QString fileName = pAppConfig->getWorkdir() + QDir::separator() + pAppConfig->getAuszugsTemplate();
    QString strDatum = QDate::currentDate().toString("dd.MM.yyyy");
    QString strJahr4 = getYear_wUI();
    QString strJahr = strJahr4.right(2);
    QString strSylvester = "31.12." + strJahr4;

    //    QString JahresDkBestaetigungenPath = getJahresDkBestaetigungenPath(strJahr4);
    QString JahresDkBestaetigungenPath = pAppConfig->getOutputDirDkBestaetigungen(strJahr4);
    QDir().mkpath(JahresDkBestaetigungenPath);

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
                    // if(Betrag < 0){
                    //     continue;
                    // }
                    QString strEnddatum;
                    // QString statement = "SELECT Rueckzahlung From DkBuchungen WHERE DkNummer = '" + strDkNummer + "'";
                    // statement += " AND substr(Rueckzahlung ,7,2) == '" + strJahr + "'";
                    // strEnddatum = getStringValue(statement);
                    bool bRueckzahlung = false;
                    if(strRueckzahlung.endsWith(strJahr))
                    {
                        bRueckzahlung = true;
                        strEnddatum = strRueckzahlung;
                    }
                    // strEnddatum = "";
                    if(strEnddatum.length() == 0)
                    {
                        strEnddatum = "31.12." + strJahr;
                    }else{
                        ; // __asm nop
                    }
                    //                    else
                    //                    {
                    //                        QDate d1 = QDate::fromString(strEnddatum, "dd.MM.yy");
                    //                        QDate d2 = QDate::fromString(strSylvester, "dd.MM.yyyy");
                    //                        if(d1.isValid()){
                    //                            if( d1 > d2 )
                    //                            {
                    //                                strEnddatum = "31.12." + strJahr;
                    //                            }
                    //                            // else
                    //                            {
                    //                                // gekündigte Dks
                    //                                // strEnddatum = QDate::fromString(strEnddatum, "dd.MM.yy").toString("dd.MM.yyyy");
                    //                                // strEnddatum = strEnddatum.left(6) + "20" + strEnddatum.right(2);
                    //                            }
                    //                        }
                    //                    }

                    // Tagesgenaue Berechnung
                    QDate dateFrom = QDate::fromString(strAnfangsdatum, "dd.MM.yy");
                    QDate dateTo = QDate::fromString(strEnddatum, "dd.MM.yy");
                    if(bRueckzahlung){
                        dateTo = dateTo.addDays(-1);
                    }

                    // int anzTage = getAnzTageJahr();
                    // if(dateFrom.isValid() && dateTo.isValid()){
                    //     anzTage = qMin((int)dateFrom.daysTo(dateTo), anzTage);
                    // }
                    int anzTage = getAnzTage(dateFrom, dateTo);
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

            writeToFile(personFileNameHtml, str);
            // TODO: In PDF umwandeln oder per Script

            // TODO: txt-Dateien für Anschreiben in Mail generieren oder per Script
            // QString personFileNameTxt = personFilePath + ".txt";

            // TODO: Evtl. hier einzeln senden ( sendDKJAKtos.py) oder per Script
        }
        // TODO: Evtl. hier  alle senden ( sendDKJAKtos.py) oder per Script
    }
    generateJahresDkBestaetigungenButton->setEnabled(true);
}

QString MainForm::createpersonalHtmlFilenameFromId(const long PersonRow)
{
    QString PersonalHtmlFileName = personenModel->data(personenModel->index(PersonRow, DkPersonen_PersonId)).toString();
    PersonalHtmlFileName += "_" + personenModel->data(personenModel->index(PersonRow, DkPersonen_Email)).toString();
    PersonalHtmlFileName += "_" + personenModel->data(personenModel->index(PersonRow, DkPersonen_Vorname)).toString();
    PersonalHtmlFileName += "_" + personenModel->data(personenModel->index(PersonRow, DkPersonen_Name)).toString();
    PersonalHtmlFileName = escapeFileName(PersonalHtmlFileName) + ".html";
    return PersonalHtmlFileName;
}

void MainForm::InsertAddressData(QString& OutputHtml, const long PersonRow, const QString& strJahr_4d)
{
    OutputHtml = OutputHtml.replace("&lt;Vorname&gt;", personenModel->data(personenModel->index(PersonRow, DkPersonen_Vorname)).toString());
    OutputHtml = OutputHtml.replace("&lt;Name&gt;", personenModel->data(personenModel->index(PersonRow, DkPersonen_Name)).toString());

    OutputHtml = OutputHtml.replace("&lt;Strasse&gt;", personenModel->data(personenModel->index(PersonRow, DkPersonen_Strasse)).toString());
    OutputHtml = OutputHtml.replace("&lt;PLZ&gt;", personenModel->data(personenModel->index(PersonRow, DkPersonen_PLZ)).toString());
    OutputHtml = OutputHtml.replace("&lt;Ort&gt;", personenModel->data(personenModel->index(PersonRow, DkPersonen_Ort)).toString());

    OutputHtml = OutputHtml.replace("&lt;Datum&gt;", QDate::currentDate().toString("dd.MM.yyyy"));
    OutputHtml = OutputHtml.replace("&lt;Jahr&gt;", strJahr_4d);
}

void MainForm::AddTableHeader(QString& OutputHtml)
{
    OutputHtml += ("<table width=\"100%\" border=\"1\">\n");

    OutputHtml += ("<thead>\n<tr>\n"\
                "<th align=\"left\">Direktkredit</th>\n"\
                "<th align=\"right\">Jahreszinsbetrag</th>\n"\
                "<th align=\"left\">Zinssatz</th>\n"\
                "</tr>\n</thead>\n");
}

void MainForm::BeginTableBody(QString& OutputHtml)
{
    OutputHtml += "<tbody>\n";
}

void MainForm::EndTableBody(QString& OutputHtml)
{
    OutputHtml += "</tbody>\n";
}

void MainForm::AddTableRow(QString& OutputHtml, const long index, const QString& year)
{
    QString strSylvester = "31.12." + year;

    buchungenView->selectRow(index);
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
        QString strEnddatum = buchungenSortModel->data(buchungenSortModel->index(BuchungIndex.row(), DkBuchungen_vorgemerkt)).toString();

        if(strEnddatum.length() == 0)
        {
            strEnddatum = "31.12." + year;
        }
        else
        {
            if(QDate::fromString(strEnddatum, "dd.MM.yyyy") >  QDate::fromString(strSylvester, "dd.MM.yyyy"))
            {
                strEnddatum = "31.12." + year;
            }
            else
            {
                // strEnddatum = QDate::fromString(strEnddatum, "dd.MM.yy").toString("dd.MM.yyyy");
            }
        }

        // Tagesgenaue Berechnung
        QDate dateFrom = QDate::fromString(strAnfangsdatum, "dd.MM.yyyy");
        QDate dateTo = QDate::fromString(strEnddatum, "dd.MM.yyyy");

        int anzTage = getAnzTage(dateFrom, dateTo);
        double Zinsen = computeDkZinsen(Betrag, Zinssatz, anzTage);
        double Endbetrag = Betrag + Zinsen;

        // strAnfangsdatum = QDate::fromString(strAnfangsdatum, "dd.MM.yy").toString("dd.MM.yyyy");
        QString strAnfangsbetrag = QString("%L1").arg(Betrag ,12,'f',2,' ') + " &euro;";
        QString strZinssatz = QString("%L1").arg(Zinssatz ,12,'f',2,' ') + " &#37;";
        QString strZinsen = QString("%L1").arg(Zinsen ,12,'f',2,' ') + " &euro;";
        QString strEndbetrag = QString("%L1").arg(Endbetrag ,12,'f',2,' ') + " &euro;";

        strZeile = strZeile.replace("&lt;DkNummer&gt;", strDkNummer);
        strZeile = strZeile.replace("&lt;Jahreszinsbetrag&gt;", strZinsen);
        strZeile = strZeile.replace("&lt;Zinssatz&gt;", strZinssatz);

        OutputHtml += strZeile;
    }
}

void MainForm::CalculateSums(const long index, double& SummeBetrag, double& SummeZinsen)
{
    buchungenView->selectRow(index);
    QModelIndex BuchungIndex = buchungenView->currentIndex();
    if(BuchungIndex.isValid())
    {
        double Zinssatz = buchungenSortModel->data(buchungenSortModel->index(BuchungIndex.row(), DkBuchungen_Zinssatz)).toDouble();
        double Betrag = buchungenSortModel->data(buchungenSortModel->index(BuchungIndex.row(), DkBuchungen_Betrag)).toDouble();
        // Tagesgenaue Berechnung
        QString strAnfangsdatum = buchungenSortModel->data(buchungenSortModel->index(BuchungIndex.row(), DkBuchungen_Datum)).toString();
        QDate dateFrom = QDate::fromString(strAnfangsdatum, "dd.MM.yy");
        QString strEnddatum = buchungenSortModel->data(buchungenSortModel->index(BuchungIndex.row(), DkBuchungen_vorgemerkt)).toString();
        QDate dateTo = QDate::fromString(strEnddatum, "dd.MM.yy");

        int anzTage = getAnzTage(dateFrom, dateTo);
        double Zinsen = computeDkZinsen(Betrag, Zinssatz, anzTage);

        SummeBetrag += Betrag;
        SummeZinsen += Zinsen;
    }
}

void MainForm::EndTable(QString& OutputHtml)
{
    OutputHtml += "</table>\n";
}

void AddTableFooter( QString& OutputHtml, const double , const double Zins)
{
//  QString strSummeBetrag = QString("%L1").arg(Betrag, 12, 'f', 2, ' ') + " &euro;";        // nicht fürs Finanzamt
    QString strSummeZinsen = QString("%L1").arg(Zins, 12, 'f', 2, ' ') + " &euro;";
//  QString strSummeEndbetrag = QString("%L1").arg(SummeEndbetrag ,12,'f',2,' ') + " &euro;";// nicht fürs Finanzamt
    OutputHtml += "<tfoot>\n<tr>\n<th align=\"left\">Summen</th>\n<th align=\"right\">";
    OutputHtml += strSummeZinsen;
    OutputHtml += "</th>\n<th></th>\n</tr>\n</tfoot>\n";

//    strFooter = strFooter.replace("&lt;SummeBetrag&gt;", strSummeBetrag);                  // nicht fürs Finanzamt
//    strFooter = strFooter.replace("&lt;SummeEndbetrag&gt;", strSummeEndbetrag);            // nicht fürs Finanzamt
}

void MainForm::generateJahresDkZinsBescheinigungen()
{
    generateJahresDkZinsBescheinigungenButton->setEnabled(false);

    QString TemplateHtml = readFromFile(pAppConfig->getZinsbescheinigungsTemplate());

    QString strJahr_4d = getYear_wUI();
    QString strJahr_2d = strJahr_4d.right(2);

    // TODO: Initialisierung und Aufräumen (z.B. selection merken und wieder setzen)
    // QModelIndex curIndex = personenView->currentIndex();
    buchungenView->sortByColumn(DkBuchungen_DKNummer, Qt::AscendingOrder);

    for(int i=0;i<personenModel->rowCount();i++)
    {
        personenView->selectRow(i);
        QModelIndex PersonIndex = personenView->currentIndex();
        if(PersonIndex.isValid())
        {
            // 1 Datei pro Person mit allen Buchungen
            // Dateinamen Muster: <PersonId>_<EMail>_<Vorname>_<Name>.txt
            QString PersonalHtmlFileName = pAppConfig->getOutputDirZinsbescheinigungen(strJahr_4d) + QDir::separator() + createpersonalHtmlFilenameFromId(PersonIndex.row());
            QString PersonalPdfFileName = PersonalHtmlFileName.replace(".html", ".pdf");
            // TODO: PersonId raus, wegen Parsen der EMail in sendDKJAKtos.py

            QString OutputHtml (TemplateHtml);

            InsertAddressData(OutputHtml, PersonIndex.row(), strJahr_4d);
            QString TableHtml;
            AddTableHeader(TableHtml);

            BeginTableBody(TableHtml);
            double SummeBetrag( 0.);
            double SummeZinsen( 0.);
            for(int j=0;j<buchungenSortModel->rowCount();j++)
            {
                AddTableRow(TableHtml, j, strJahr_4d);
                CalculateSums(j, SummeBetrag, SummeZinsen);
            }
            EndTableBody(TableHtml);
            AddTableFooter(TableHtml, SummeBetrag, SummeZinsen);

            EndTable(TableHtml);
            OutputHtml = OutputHtml.replace("&lt;Auflistung&gt;", TableHtml);

            writeToFile(PersonalHtmlFileName, OutputHtml);
            writeHtmlToPdf(PersonalPdfFileName, OutputHtml);

            // TODO: txt-Dateien für Anschreiben in Mail generieren oder per Script
            // QString personFileNameTxt = personFilePath + ".txt";

            // TODO: Evtl. hier einzeln senden ( sendDKJAKtos.py) oder per Script
        }
        // TODO: Evtl. hier  alle senden ( sendDKJAKtos.py) oder per Script
    } // EO for ...

    generateJahresDkZinsBescheinigungenButton->setEnabled(true);
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

    QString statementDk = "SELECT SUM(Betrag) FROM DkBuchungen";
    // QString statementDk = "SELECT SUM(replace(Betrag,',','.')) FROM DkBuchungen";
    int PersonId = getPersonId();
    if((PersonId != -1) && !anzeigenPersonenButton->isChecked()){
        statementDk += " WHERE PersonId=";
        statementDk += QString::number(PersonId);
    }
    double summeDk = getDoubleValue(statementDk);
    QString summeDkText = QString::number(summeDk, 'f', 2);
    summeBuchungenDkEdit->setText(summeDkText);

    QString statementDkZinsen = "SELECT SUM( (Betrag * Zinssatz) / 100.0 ) FROM DkBuchungen";
    // QString statementDkZinsen = "SELECT SUM( replace(Betrag,',','.') * replace(Zinssatz,',','.') / 100.0 ) FROM DkBuchungen";
    if((PersonId != -1) && !anzeigenPersonenButton->isChecked()){
        statementDkZinsen += " WHERE PersonId=";
        statementDkZinsen += QString::number(PersonId);
    }
    double summeDkZinsen = getDoubleValue(statementDkZinsen);
    QDate dateTo = QDate::fromString(datumBuchungenDkZinsenEdit->text(), "dd.MM.yy");
    QDate dateFrom = QDate(dateTo.year(), 1, 1);

    int anzTage = getAnzTage(dateFrom, dateTo);
    summeDkZinsen = Runden2(summeDkZinsen * anzTage / getAnzTageJahr());
    QString summeDkZinsenText = QString::number(summeDkZinsen, 'f', 2);
    summeBuchungenDkZinsenEdit->setText(summeDkZinsenText);
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
    bool b = personenModel->insertRow(row);
    Q_UNUSED(b);

    QModelIndex index = personenModel->index(row, DkPersonen_PersonId);

    personenView->edit(index);
    int maxPersonId = getMaxId("DkPersonen", "PersonId") + 1;
    personenModel->setData(index, maxPersonId);

    // Dialog anzeigen
    PersonForm form(this, personenModel, maxPersonId);
    form.disableNavigationButtons();
    form.exec();
    updatePersonenView();
    personenView->setCurrentIndex(index);
}

void MainForm::editPerson(){
    int PersonId = getPersonId();
    PersonForm form(this, personenModel, PersonId);
    form.disableCreationButton();
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

    QString statementDkZinsen = "SELECT SUM( (Betrag * Zinssatz) / 100.0 ) FROM DkBuchungen";
    // QString statementDkZinsen = "SELECT SUM( replace(Betrag,',','.') * replace(Zinssatz,',','.') / 100.0 ) FROM DkBuchungen";
    double summeDkZinsen = getDoubleValue(statementDkZinsen);
    QString summeDkZinsenText = QString::number(summeDkZinsen, 'f', 2);
    summeDkZinsenEdit->setText(summeDkZinsenText);
}

void MainForm::createSummenPanel()
{
    summenPanel = new QWidget;

    summenLabel = new QLabel(tr("Summen"));

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

    QHBoxLayout *hlayout = new QHBoxLayout;
    hlayout->setSizeConstraint(QLayout::SetFixedSize);
    hlayout->addWidget(summenLabel);
    hlayout->addWidget(summeDkZinsenLabel);
    hlayout->addWidget(summeDkZinsenEdit);
    hlayout->addWidget(summeDkLabel);
    hlayout->addWidget(summeDkEdit);

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
            QSqlRecord record = (static_cast<QSqlTableModel*>(buchungenSortModel->sourceModel()))->record(index.row());
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
    summeBuchungenDkZinsenEdit->setReadOnly(true);
    summeBuchungenDkLabel = new QLabel("Summe Direktkredite");
    summeBuchungenDkEdit = new QLineEdit;
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
