#include <QtWidgets>
#include <QtSql>
#include <QFile>

#include "buchungform.h"
#include "personform.h" // wg. enum
#include "dbfkts.h"

BuchungForm::BuchungForm(QWidget *parent, QSqlTableModel *personenModel, const QModelIndex &indexPersonId, QSqlRelationalTableModel *buchungenModel, const QModelIndex &indexBuchungId)
    : QDialog(parent)
{
    m_personenModel = personenModel;
    m_PersonIndex = indexPersonId;
    m_buchungenModel = buchungenModel;
    m_BuchungIndex = indexBuchungId;

    buchungIdEdit = new QLineEdit;
    buchungIdEdit->setReadOnly(true);
    buchungIdLabel = new QLabel(tr("BuchungId"));
    buchungIdLabel->setBuddy(buchungIdEdit);

    // personenComboBox = new QComboBox;
    // personenLabel = new QLabel(tr("Person:"));
    // personenLabel->setBuddy(personenComboBox);

    personIdEdit = new QLineEdit;
    personIdEdit->setReadOnly(true);
    personIdLabel = new QLabel(tr("PersonId"));
    personIdLabel->setBuddy(personIdEdit);

    datumEdit = new QLineEdit;
    datumEdit->setInputMask("00.00.00;_");

    datumLabel = new QLabel(tr("Datum"));
    datumLabel->setBuddy(datumEdit);

    dkNrEdit = new QLineEdit;
    dkNrLabel = new QLabel(tr("DkNr"));
    dkNrLabel->setBuddy(dkNrEdit);

    dkNummerEdit = new QLineEdit;
    dkNummerLabel = new QLabel(tr("DkNummer"));
    dkNummerLabel->setBuddy(dkNummerEdit);

    rueckzahlungEdit = new QLineEdit;
    rueckzahlungLabel = new QLabel(tr("Rückzahlung"));
    rueckzahlungLabel->setBuddy(rueckzahlungEdit);

    vorgemerktEdit = new QLineEdit;
    vorgemerktLabel = new QLabel(tr("vorgemerkt"));
    vorgemerktLabel->setBuddy(vorgemerktEdit);

    betragEdit = new QLineEdit;
    // betragEdit->setInputMask("00000.00 €");
    // betragEdit->setValidator( new QDoubleValidator(0, 100000, 2, this) );
    // QRegExp preco("^(\\d{1,3}(\\.\\d{3})*|(\\d+))(\\,\\d{2})?$");
    // QRegExp regCurrency("^(\\d{1,5}})+(\\,)?(\\d{0,2})?$");
    // betragEdit->setValidator(new QRegExpValidator(regCurrency, betragEdit));
    // betragEdit->setInputMask("00000.00 €");
    betragEdit->setValidator(new QDoubleValidator(0.0, 100000.0, 2, betragEdit));


    betragLabel = new QLabel(tr("Betrag"));
    betragLabel->setBuddy(betragEdit);

    // zinssatzEdit = new QLineEdit;
    // //zinssatzEdit->setInputMask("00009.99 %");
    // zinssatzEdit->setValidator(new QDoubleValidator(0.0, 100.0, 2, zinssatzEdit));
    // zinssatzLabel = new QLabel(tr("Zinssatz"));
    // zinssatzLabel->setBuddy(zinssatzEdit);

    zinssatzComboBox = new QComboBox;
    zinssatzLabel = new QLabel(tr("Zinssatz"));
    zinssatzLabel->setBuddy(zinssatzComboBox);

    bemerkungEdit = new QTextEdit;
    bemerkungLabel = new QLabel(tr("Bemerkung"));
    bemerkungLabel->setBuddy(bemerkungEdit);

    firstButton = new QPushButton(tr("<< Erste"));
    previousButton = new QPushButton(tr("< Vorherige"));
    nextButton = new QPushButton(tr("Nächste >"));
    lastButton = new QPushButton(tr("Letzte >>"));

    addButton = new QPushButton(tr("Hinzufügen"));
    if(!m_PersonIndex.isValid()){
       addButton->setEnabled(false);
    }
    deleteButton = new QPushButton(tr("Löschen"));
    dkBestaetigenButton = new QPushButton(tr("Dk bestätigen"));
    dkVerlaengernButton = new QPushButton(tr("Dk verlängern"));
    dkAnfrageVerlaengernButton = new QPushButton(tr("Dk Anfrage verlängern"));
    closeButton = new QPushButton(tr("Schliessen"));

    buttonBox = new QDialogButtonBox;
    buttonBox->addButton(addButton, QDialogButtonBox::ActionRole);
    buttonBox->addButton(deleteButton, QDialogButtonBox::ActionRole);
    buttonBox->addButton(dkBestaetigenButton, QDialogButtonBox::ActionRole);
    buttonBox->addButton(dkVerlaengernButton, QDialogButtonBox::ActionRole);
    buttonBox->addButton(dkAnfrageVerlaengernButton, QDialogButtonBox::ActionRole);
    buttonBox->addButton(closeButton, QDialogButtonBox::AcceptRole);

    // m_buchungenModel = buchungenModel;
    // QSqlRelationalTableModel *tableModel = new QSqlRelationalTableModel(this);
    // tableModel->setTable("DkBuchungen");
    // tableModel->setRelation(DkBuchungen_PersonId, QSqlRelation("DkPersonen", "PersonId", "Name"));
    // // tableModel->setSort(DkBuchungen_Datum, Qt::AscendingOrder);
    // tableModel->select();

    QSqlQueryModel *zinssaetzeModel = new QSqlQueryModel(zinssatzComboBox);
    zinssaetzeModel->setQuery("SELECT zinssatz FROM DKZinssaetze");
    zinssatzComboBox->setModel(zinssaetzeModel);


    mapper = new QDataWidgetMapper(this);
    mapper->setSubmitPolicy(QDataWidgetMapper::AutoSubmit);
    mapper->setModel(m_buchungenModel);
    mapper->setItemDelegate(new QSqlRelationalDelegate(this));

    mapper->addMapping(buchungIdEdit, DkBuchungen_BuchungId);
    mapper->addMapping(personIdEdit, DkBuchungen_PersonId);
    mapper->addMapping(datumEdit, DkBuchungen_Datum);
    mapper->addMapping(dkNrEdit, DkBuchungen_DKNr);
    mapper->addMapping(dkNummerEdit, DkBuchungen_DKNummer);
    mapper->addMapping(rueckzahlungEdit, DkBuchungen_Rueckzahlung);
    mapper->addMapping(vorgemerktEdit, DkBuchungen_vorgemerkt);
    mapper->addMapping(betragEdit, DkBuchungen_Betrag);
    // mapper->addMapping(zinssatzEdit, DkBuchungen_Zinssatz);
    mapper->addMapping(zinssatzComboBox, DkBuchungen_Zinssatz);
    mapper->addMapping(bemerkungEdit, DkBuchungen_Bemerkung, "plainText");

    int BuchungId = -1;
    if (indexBuchungId.isValid()) {
        QSqlRecord record = buchungenModel->record(indexBuchungId.row());
        BuchungId = record.value(DkBuchungen_BuchungId).toInt();
    }

    if (BuchungId != -1) {
        for (int row = 0; row < m_buchungenModel->rowCount(); ++row) {
            QSqlRecord record = m_buchungenModel->record(row);
            if (record.value(DkBuchungen_BuchungId).toInt() == BuchungId) {
                mapper->setCurrentIndex(row);
                break;
            }
        }
    } else {
        mapper->toFirst();
    }

    connect(firstButton, SIGNAL(clicked()), mapper, SLOT(toFirst()));
    connect(previousButton, SIGNAL(clicked()),
            mapper, SLOT(toPrevious()));
    connect(nextButton, SIGNAL(clicked()), mapper, SLOT(toNext()));
    connect(lastButton, SIGNAL(clicked()), mapper, SLOT(toLast()));
    connect(addButton, SIGNAL(clicked()), this, SLOT(addBuchung()));
    connect(deleteButton, SIGNAL(clicked()), this, SLOT(deleteBuchung()));
    connect(dkBestaetigenButton, SIGNAL(clicked()), this, SLOT(dkBestaetigenBuchung()));
    connect(dkVerlaengernButton, SIGNAL(clicked()), this, SLOT(dkVerlaengernBuchung()));
    connect(dkAnfrageVerlaengernButton, SIGNAL(clicked()), this, SLOT(dkAnfrageVerlaengernBuchung()));

    connect(closeButton, SIGNAL(clicked()), this, SLOT(accept()));

    QHBoxLayout *topButtonLayout = new QHBoxLayout;
    topButtonLayout->setContentsMargins(20, 0, 20, 5);
    topButtonLayout->addStretch();
    topButtonLayout->addWidget(firstButton);
    topButtonLayout->addWidget(previousButton);
    topButtonLayout->addWidget(nextButton);
    topButtonLayout->addWidget(lastButton);
    topButtonLayout->addStretch();

    QGridLayout *mainLayout = new QGridLayout;
    int row = 0;
    mainLayout->addLayout(topButtonLayout, row, 0, 1, 3);
    row++;
    mainLayout->addWidget(buchungIdLabel, row, 0);
    mainLayout->addWidget(buchungIdEdit, row, 1, 1, 2);
    row++;
    mainLayout->addWidget(personIdLabel, row, 0);
    mainLayout->addWidget(personIdEdit, row, 1, 1, 2);
    row++;
    mainLayout->addWidget(datumLabel, row, 0);
    mainLayout->addWidget(datumEdit, row, 1, 1, 2);
    row++;
    mainLayout->addWidget(dkNrLabel, row, 0);
    mainLayout->addWidget(dkNrEdit, row, 1);
    row++;
    mainLayout->addWidget(dkNummerLabel, row, 0);
    mainLayout->addWidget(dkNummerEdit, row, 1, 1, 2);
    row++;
    mainLayout->addWidget(rueckzahlungLabel, row, 0);
    mainLayout->addWidget(rueckzahlungEdit, row, 1);
    row++;
    mainLayout->addWidget(rueckzahlungLabel, row, 0);
    mainLayout->addWidget(rueckzahlungEdit, row, 1);
    row++;
    mainLayout->addWidget(vorgemerktLabel, row, 0);
    mainLayout->addWidget(vorgemerktEdit, row, 1);
    row++;
    mainLayout->addWidget(betragLabel, row, 0);
    mainLayout->addWidget(betragEdit, row, 1);
    row++;
    mainLayout->addWidget(zinssatzLabel, row, 0);
    mainLayout->addWidget(zinssatzComboBox, row, 1);
    row++;
    mainLayout->addWidget(bemerkungLabel, row, 0);
    mainLayout->addWidget(bemerkungEdit, row, 1);
    row++;
    mainLayout->addWidget(buttonBox, row, 0, 1, 3);
    // mainLayout->setRowMinimumHeight(row, 10);
    // mainLayout->setRowStretch(row, 1);
    mainLayout->setColumnStretch(1, 1);
    setLayout(mainLayout);

    if (BuchungId == -1) {
        nextButton->setFocus();
    } else {
        datumEdit->setFocus();
    }

    setWindowTitle(tr("Buchungen bearbeiten"));
}

void BuchungForm::done(int result)
{
    mapper->submit();
    QDialog::done(result);
}

void BuchungForm::addBuchung()
{
    int row = mapper->currentIndex();
    mapper->submit();
    m_buchungenModel->insertRow(row);
    mapper->setCurrentIndex(row);
    // m_BuchungIndex = m_buchungenModel->index(row, 0);

    int maxBuchungId = getMaxId("DkBuchungen", "BuchungId") + 1;
    buchungIdEdit->setText(QString::number(maxBuchungId));

    int PersonId = -1;
    if (m_BuchungIndex.isValid()) {
        QSqlRecord record = m_buchungenModel->record(m_BuchungIndex.row());
        PersonId = record.value(DkBuchungen_PersonId).toInt();
    }
    personIdEdit->setText(QString::number(PersonId));

    datumEdit->clear();
    dkNrEdit->clear();
    dkNummerEdit->clear();
    rueckzahlungEdit->clear();
    vorgemerktEdit->clear();
    betragEdit->clear();
    // zinssatzEdit->clear();
    bemerkungEdit->clear();

    datumEdit->setFocus();
}

void BuchungForm::deleteBuchung()
{
    int row = mapper->currentIndex();
    m_buchungenModel->removeRow(row);
    mapper->submit();
    mapper->setCurrentIndex(qMin(row, m_buchungenModel->rowCount() - 1));
    // m_BuchungIndex = m_buchungenModel->index(row, 0);
}

void BuchungForm::writeDataForDkVorlageBuchung(){
    // if(!m_PersonIndex.isValid())
    //    return;
    // QSqlRecord personRecord = m_personenModel->record(m_PersonIndex.row());
    // if(personRecord.isNull())
    //    return;
    // QString fileName = "DkVerwaltungQt.tmp";
    // QString dbPath = getSettings().value(QStringLiteral("DBPath")).toString();
    // QString filePath = dbPath + QDir::separator() + fileName; // todo: add current dir
    // QString dbPath = getSettings().value(QStringLiteral("DBPath")).toString();
    QString dbPath = getFilePathFromIni("DBPath", getStandardPath(), "DkVerwaltungQt.db3");
    QString filePath = dbPath.replace(".db3", ".tmp");
    QFile file(filePath);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)){
        QTextStream out(&file);
        if(m_PersonIndex.isValid()){
            out << m_personenModel->data(m_personenModel->index(m_PersonIndex.row(), DkPersonen_Vorname)).toString() << "\n";
            out << m_personenModel->data(m_personenModel->index(m_PersonIndex.row(), DkPersonen_Name)).toString() << "\n";
            out << m_personenModel->data(m_personenModel->index(m_PersonIndex.row(), DkPersonen_Anrede)).toString() << "\n";
            out << "\n"; // co
            out << m_personenModel->data(m_personenModel->index(m_PersonIndex.row(), DkPersonen_Strasse)).toString() << "\n";
            out << m_personenModel->data(m_personenModel->index(m_PersonIndex.row(), DkPersonen_PLZ)).toString() << "\n";
            out << m_personenModel->data(m_personenModel->index(m_PersonIndex.row(), DkPersonen_Ort)).toString() << "\n";
            out << m_personenModel->data(m_personenModel->index(m_PersonIndex.row(), DkPersonen_Email)).toString() << "\n";
        }
        int row = mapper->currentIndex();
        m_BuchungIndex = m_buchungenModel->index(row, 0); // oder einfach row nehmen...
        if(m_BuchungIndex.isValid()){
            out << m_buchungenModel->data(m_buchungenModel->index(m_BuchungIndex.row(), DkBuchungen_DKNummer)).toString() << "\n";
            out << m_buchungenModel->data(m_buchungenModel->index(m_BuchungIndex.row(), DkBuchungen_Datum)).toString() << "\n";
            out << m_buchungenModel->data(m_buchungenModel->index(m_BuchungIndex.row(), DkBuchungen_Betrag)).toString() << "\n";
            double AnfangsBetrag = m_buchungenModel->data(m_buchungenModel->index(m_BuchungIndex.row(), DkBuchungen_AnfangsBetrag)).toDouble();
            double Betrag = m_buchungenModel->data(m_buchungenModel->index(m_BuchungIndex.row(), DkBuchungen_Betrag)).toDouble();
            double Zinssatz = m_buchungenModel->data(m_buchungenModel->index(m_BuchungIndex.row(), DkBuchungen_Zinssatz)).toDouble();
            QString strAnfangsdatum = m_buchungenModel->data(m_buchungenModel->index(m_BuchungIndex.row(), DkBuchungen_Anfangsdatum)).toString();
            QString strDatum = m_buchungenModel->data(m_buchungenModel->index(m_BuchungIndex.row(), DkBuchungen_Datum)).toString();
            // QDate dateFrom = getDateFromYearString(strDatum);
            QDate dateFrom = getDateFromYearString(strAnfangsdatum);
            // QString strEnddatum = QDate::currentDate().addDays(-1).toString("dd.MM.yy");
            QString strEnddatum = m_buchungenModel->data(m_buchungenModel->index(m_BuchungIndex.row(), DkBuchungen_vorgemerkt)).toString();
            QDate dateTo = getDateFromYearString(strEnddatum).addDays(-1);
            if(strEnddatum.length() == 0)
            {
                strEnddatum = "31.12." + QDate::currentDate().toString("dd.MM.yy").right(2);
                dateTo = getDateFromYearString(strEnddatum);
            }
            // double Zinsen = computeDkZinsenZeitraum(Betrag, Zinssatz, dateFrom, dateTo);
            // Zinsen += (Betrag - AnfangsBetrag);
            double Zinsen = computeDkZinsenZeitraum(AnfangsBetrag, Zinssatz, dateFrom, dateTo);
            out << Zinsen << "\n";
            out << m_buchungenModel->data(m_buchungenModel->index(m_BuchungIndex.row(), DkBuchungen_Zinssatz)).toString() << "\n";
            out << "\n"; // "r" Mann/Frau ???
            out << m_buchungenModel->data(m_buchungenModel->index(m_BuchungIndex.row(), DkBuchungen_vorgemerkt)).toString() << "\n";
            out << m_buchungenModel->data(m_buchungenModel->index(m_BuchungIndex.row(), DkBuchungen_Anfangsdatum)).toString() << "\n";
            out << m_buchungenModel->data(m_buchungenModel->index(m_BuchungIndex.row(), DkBuchungen_AnfangsBetrag)).toString() << "\n";
        }
    }
}

void BuchungForm::dkVorlageBuchung(const QString &vorlageIniEntry, const QString &vorlageName)
{
    writeDataForDkVorlageBuchung();
    // /Applications/OpenOffice.app/Contents/MacOS/soffice '/Users/volker/Documents/13ha/DkVerwaltungQt/Vorlage Anschreiben DK-Eingang.ott' 'vnd.sun.star.script:DkVerwaltungQt.Module1.FelderFuellen?language=Basic&location=application'
    QString oo = getOpenOfficePath();
//#ifdef Q_OS_MAC
//     // oo = "/Applications/OpenOffice.app/Contents/MacOS/soffice";
//    oo = getFilePathFromIni("OOPath", "/Applications/OpenOffice.app/Contents/MacOS/", "soffice");
//#elif Q_OS_WIN
//    // oo = "C:\\Program Files\\OpenOffice 4\\program\\soffice.exe";
//    oo = getFilePathFromIni("OOPath", "C:\\Program Files\\OpenOffice 4\\program\\", "soffice.exe");
//#endif
    if(!oo.length())
        return;
    // QString ott = "/Users/volker/Documents/13ha/DkVerwaltungQt/Vorlage Anschreiben DK-Eingang.ott";
    QString ott = getFilePathFromIni(vorlageIniEntry, getStandardPath(), vorlageName);
    if(!ott.length())
        return;

    QString macro = "vnd.sun.star.script:DkVerwaltungQt.Module1.FelderFuellen?language=Basic&location=application";
    QString program = oo;
    QStringList arguments;
    arguments.append(ott);
    arguments.append(macro);
    QString workingDirectory = QString();
    qint64 pid = 0;
    // QProcess::execute(oo, arguments, workingDirectory, &pid); // synchron
    QProcess::startDetached(oo, arguments, workingDirectory, &pid); // assynchron
    qDebug() << "pid " << pid;
}

void BuchungForm::dkBestaetigenBuchung()
{
    QString vorlageIniEntry = "DkEingangVorlagePath";
    QString vorlageName = "Vorlage Anschreiben DK-Eingang.ott";
    dkVorlageBuchung(vorlageIniEntry, vorlageName);
}

void BuchungForm::dkVerlaengernBuchung()
{
    QString vorlageIniEntry = "DkVerlaengerungVorlagePath";
    QString vorlageName = "Vorlage Anschreiben DK-Verlängerung.ott";
    dkVorlageBuchung(vorlageIniEntry, vorlageName);
}

void BuchungForm::dkAnfrageVerlaengernBuchung()
{
    QString vorlageIniEntry = "DkAnfrageVerlaengerungVorlagePath";
    QString vorlageName = "Vorlage Anschreiben Anfrage DK-Verlängerung.ott";
    dkVorlageBuchung(vorlageIniEntry, vorlageName);
}
