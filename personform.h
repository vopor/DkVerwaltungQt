#ifndef PERSONFORM_H
#define BUCHUNGENFORM_H

#include <QDialog>

QT_BEGIN_NAMESPACE
class QComboBox;
class QDataWidgetMapper;
class QDateEdit;
class QDialogButtonBox;
class QLabel;
class QLineEdit;
class QTextEdit;
class QPushButton;
class QSqlTableModel;
QT_END_NAMESPACE

enum {
    DkPersonen_PersonId = 0,
    DkPersonen_Vorname = 1,
    DkPersonen_Name = 2,
    DkPersonen_Anrede = 3,
    DkPersonen_Strasse = 4,
    DkPersonen_PLZ = 5,
    DkPersonen_Ort = 6,
    DkPersonen_Email = 7
};

class PersonForm : public QDialog
{
    Q_OBJECT

public:
    PersonForm(QWidget *parent, QSqlTableModel *perosnenModel, int PersonId);
    void disableNavigationButtons();
    void disableCreationButton();
    void done(int result);

private slots:
    void addPerson();
    void deletePerson();

private:
    QSqlTableModel *m_personenModel;
    int m_PersonId;
    QDataWidgetMapper *mapper;

    QLabel *personIdLabel;
    QLabel *vornameLabel;
    QLabel *nameLabel;
    QLabel *anredeLabel;
    QLabel *strasseLabel;
    QLabel *plzLabel;
    QLabel *ortLabel;
    QLabel *emailLabel;

    QLineEdit *personIdEdit;
    QLineEdit *vornameEdit;
    QLineEdit *nameEdit;
    QLineEdit *anredeEdit;
    QLineEdit *strasseEdit;
    QLineEdit *plzEdit;
    QLineEdit *ortEdit;
    QLineEdit *emailEdit;

    QPushButton *firstButton;
    QPushButton *previousButton;
    QPushButton *nextButton;
    QPushButton *lastButton;
    QPushButton *addButton;
    QPushButton *closeButton;
    QDialogButtonBox *buttonBox;
};

#endif
