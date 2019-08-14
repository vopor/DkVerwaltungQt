#include <QtWidgets>
#include <QtSql>

#include "personform.h"

#include "dbfkts.h"

PersonForm::PersonForm(QWidget *parent, QSqlTableModel *personenModel, int PersonId )
    : QDialog(parent)
{

    m_personenModel = personenModel;
    m_PersonId = PersonId;

    personIdEdit = new QLineEdit;
    personIdEdit->setReadOnly(true);
    personIdLabel = new QLabel(tr("PersonId"));
    personIdLabel->setBuddy(personIdEdit);

    vornameEdit = new QLineEdit;
    vornameLabel = new QLabel(tr("Vorname"));
    vornameLabel->setBuddy(vornameEdit);

    nameEdit = new QLineEdit;
    nameLabel = new QLabel(tr("Name"));
    nameLabel->setBuddy(nameEdit);

    anredeEdit = new QLineEdit;
    anredeLabel = new QLabel(tr("Anrede"));
    anredeLabel->setBuddy(anredeEdit);

    strasseEdit = new QLineEdit;
    strasseLabel = new QLabel(tr("Strasse"));
    strasseLabel->setBuddy(strasseEdit);

    plzEdit = new QLineEdit;
    plzEdit->setInputMask("99990");
    plzLabel = new QLabel(tr("PLZ"));
    plzLabel->setBuddy(plzEdit);

    ortEdit = new QLineEdit;
    ortLabel = new QLabel(tr("Ort"));
    ortLabel->setBuddy(ortEdit);

    emailEdit = new QLineEdit;
    emailLabel = new QLabel(tr("Email"));
    emailLabel->setBuddy(emailEdit);

    firstButton = new QPushButton(tr("<< Erste"));
    previousButton = new QPushButton(tr("< Vorherige"));
    nextButton = new QPushButton(tr("Nächste >"));
    lastButton = new QPushButton(tr("Letzte >>"));

    addButton = new QPushButton(tr("Hinzufügen"));
    closeButton = new QPushButton(tr("Schliessen"));

    buttonBox = new QDialogButtonBox;
    buttonBox->addButton(addButton, QDialogButtonBox::ActionRole);
    buttonBox->addButton(closeButton, QDialogButtonBox::AcceptRole);

    mapper = new QDataWidgetMapper(this);
    mapper->setSubmitPolicy(QDataWidgetMapper::AutoSubmit);
    mapper->setModel(m_personenModel);
    mapper->setItemDelegate(new QSqlRelationalDelegate(this));

    mapper->addMapping(personIdEdit, DkPersonen_PersonId);
    mapper->addMapping(vornameEdit, DkPersonen_Vorname);
    mapper->addMapping(nameEdit, DkPersonen_Name);
    mapper->addMapping(anredeEdit, DkPersonen_Anrede);
    mapper->addMapping(strasseEdit, DkPersonen_Strasse);
    mapper->addMapping(plzEdit, DkPersonen_PLZ);
    mapper->addMapping(ortEdit, DkPersonen_Ort);
    mapper->addMapping(emailEdit, DkPersonen_Email);

    if (PersonId != -1) {
        for (int row = 0; row < m_personenModel->rowCount(); ++row) {
            QSqlRecord record = m_personenModel->record(row);
            if (record.value(DkPersonen_PersonId).toInt() == PersonId) {
                mapper->setCurrentIndex(row);
                break;
             }
        }

    } else {
        mapper->toFirst();
    }

    connect(firstButton, SIGNAL(clicked()), mapper, SLOT(toFirst()));
    connect(previousButton, SIGNAL(clicked()), mapper, SLOT(toPrevious()));
    connect(nextButton, SIGNAL(clicked()), mapper, SLOT(toNext()));
    connect(lastButton, SIGNAL(clicked()), mapper, SLOT(toLast()));
    connect(addButton, SIGNAL(clicked()), this, SLOT(addPerson()));
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
    mainLayout->addWidget(personIdLabel, row, 0);
    mainLayout->addWidget(personIdEdit, row, 1, 1, 2);
    row++;
    mainLayout->addWidget(vornameLabel, row, 0);
    mainLayout->addWidget(vornameEdit, row, 1, 1, 2);
    row++;
    mainLayout->addWidget(nameLabel, row, 0);
    mainLayout->addWidget(nameEdit, row, 1, 1, 2);
    row++;
    mainLayout->addWidget(anredeLabel, row, 0);
    mainLayout->addWidget(anredeEdit, row, 1);
    row++;
    mainLayout->addWidget(strasseLabel, row, 0);
    mainLayout->addWidget(strasseEdit, row, 1, 1, 2);
    row++;
    mainLayout->addWidget(plzLabel, row, 0);
    mainLayout->addWidget(plzEdit, row, 1);
    row++;
    mainLayout->addWidget(ortLabel, row, 0);
    mainLayout->addWidget(ortEdit, row, 1);
    row++;
    mainLayout->addWidget(emailLabel, row, 0);
    mainLayout->addWidget(emailEdit, row, 1);
    row++;
    mainLayout->addWidget(buttonBox, row, 0, 1, 3);
    //mainLayout->setRowMinimumHeight(row, 10);
    //mainLayout->setRowStretch(row, 1);
    mainLayout->setColumnStretch(1, 1);
    setLayout(mainLayout);

    if (PersonId == -1) {
        nextButton->setFocus();
    } else {
        vornameEdit->setFocus();
    }

    setWindowTitle(tr("Person bearbeiten"));
}

void PersonForm::disableNavigationButtons()
{
    firstButton->hide();
    previousButton->hide();
    nextButton->hide();
    lastButton->hide();
}

void PersonForm::disableCreationButton()
{
    addButton->hide();
}

void PersonForm::done(int result)
{
    mapper->submit();
    QDialog::done(result);
}

void PersonForm::addPerson()
{
    int row = mapper->currentIndex();
    mapper->submit();
    m_personenModel->insertRow(row);
    mapper->setCurrentIndex(row);

    int maxPersonId = getMaxId("DkPersonen", "PersonId") + 1;
    personIdEdit->setText(QString::number(maxPersonId));

    vornameEdit->clear();
    nameEdit->clear();
    anredeEdit->clear();
    strasseEdit->clear();
    plzEdit->clear();
    ortEdit->clear();
    emailEdit->clear();

    vornameEdit->setFocus();
}
