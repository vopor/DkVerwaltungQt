#include <QtWidgets>
#include <QtSql>
#include <QFile>

#include "appconfiguration.h"
#include "dbfkts.h"
#include "personform.h" // wg. enum
#include "buchungform.h"

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

    personIdEdit = new QLineEdit;
    personIdEdit->setReadOnly(true);
    personIdLabel = new QLabel(tr("PersonId"));
    personIdLabel->setBuddy(personIdEdit);

    datumEdit = new QDateEdit;

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
    betragEdit->setValidator(new QDoubleValidator(0.0, 100000.0, 2, betragEdit));


    betragLabel = new QLabel(tr("Betrag"));
    betragLabel->setBuddy(betragEdit);

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
    closeButton = new QPushButton(tr("Schliessen"));

    buttonBox = new QDialogButtonBox;
    buttonBox->addButton(addButton, QDialogButtonBox::ActionRole);
    buttonBox->addButton(deleteButton, QDialogButtonBox::ActionRole);
    buttonBox->addButton(dkBestaetigenButton, QDialogButtonBox::ActionRole);
    buttonBox->addButton(closeButton, QDialogButtonBox::AcceptRole);

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
    bemerkungEdit->clear();

    datumEdit->setFocus();
}

void BuchungForm::deleteBuchung()
{
    int row = mapper->currentIndex();
    m_buchungenModel->removeRow(row);
    mapper->submit();
    mapper->setCurrentIndex(qMin(row, m_buchungenModel->rowCount() - 1));
}

void BuchungForm::writeDataForDkBestaetigenBuchung(){
    QString dbPath = pAppConfig->getDb();
    QString filePath = dbPath.replace(".db3", ".tmp");
    QFile file(filePath);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)){
        QTextStream out(&file);
        if(m_PersonIndex.isValid()){
            out << m_personenModel->data(m_personenModel->index(m_PersonIndex.row(), DkPersonen_Vorname)).toString() << "\n";
            out << m_personenModel->data(m_personenModel->index(m_PersonIndex.row(), DkPersonen_Name)).toString() << "\n";
            out << m_personenModel->data(m_personenModel->index(m_PersonIndex.row(), DkPersonen_Anrede)).toString() << "\n";
            out  << "\n"; // co
            out << m_personenModel->data(m_personenModel->index(m_PersonIndex.row(), DkPersonen_Strasse)).toString() << "\n";
            out << m_personenModel->data(m_personenModel->index(m_PersonIndex.row(), DkPersonen_PLZ)).toString() << "\n";
            out << m_personenModel->data(m_personenModel->index(m_PersonIndex.row(), DkPersonen_Ort)).toString() << "\n";
        }
        int row = mapper->currentIndex();
        m_BuchungIndex = m_buchungenModel->index(row, 0); // oder einfach row nehmen...
        if(m_BuchungIndex.isValid()){
            out << m_buchungenModel->data(m_buchungenModel->index(m_BuchungIndex.row(), DkBuchungen_DKNummer)).toString() << "\n";
            out << m_buchungenModel->data(m_buchungenModel->index(m_BuchungIndex.row(), DkBuchungen_Datum)).toString() << "\n";
            out << m_buchungenModel->data(m_buchungenModel->index(m_BuchungIndex.row(), DkBuchungen_Betrag)).toString() << "\n";
            out << m_buchungenModel->data(m_buchungenModel->index(m_BuchungIndex.row(), DkBuchungen_Zinssatz)).toString() << "\n";
        }
    }
}

void BuchungForm::dkBestaetigenBuchung()
{
    writeDataForDkBestaetigenBuchung();
    // /Applications/OpenOffice.app/Contents/MacOS/soffice '/Users/volker/Documents/13ha/DkVerwaltungQt/Vorlage Anschreiben DK-Eingang.ott' 'vnd.sun.star.script:DkVerwaltungQt.Module1.FelderFuellen?language=Basic&location=application'
    QString oo = getOpenOfficePath();
    if(!oo.length())
        return;
    QString ott =pAppConfig->getWorkdir() + QDir::separator() + pAppConfig->getAnschreibenTemplate();

    if(!ott.length())
        return;

    QString macro = "vnd.sun.star.script:DkVerwaltungQt.Module1.FelderFuellen?language=Basic&location=application";
    QString program = oo;
    QStringList arguments;
    arguments.append(ott);
    arguments.append(macro);
    QString workingDirectory = QString();
    qint64 pid = 0;
    QProcess::startDetached(oo, arguments, workingDirectory, &pid); // assynchron
    qDebug() << "pid " << pid;
}
