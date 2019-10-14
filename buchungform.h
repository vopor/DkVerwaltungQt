#ifndef BUCHUNGFORM_H
#define BUCHUNGFORM_H

#include <QDialog>

#include <QModelIndex>

QT_BEGIN_NAMESPACE
class QComboBox;
class QDataWidgetMapper;
class QDateEdit;
class QDialogButtonBox;
class QLabel;
class QLineEdit;
class QTextEdit;
class QDateEdit;
class QPushButton;
class QSqlTableModel;
class QSqlRelationalTableModel;

QT_END_NAMESPACE

enum {
    DkBuchungen_BuchungId = 0,
    DkBuchungen_PersonId = 1,
    DkBuchungen_Datum = 2,
    DkBuchungen_DKNr = 3,
    DkBuchungen_DKNummer = 4,
    DkBuchungen_Rueckzahlung = 5,
    DkBuchungen_vorgemerkt = 6,
    DkBuchungen_Betrag = 7,
    DkBuchungen_Zinssatz = 8,
    DkBuchungen_Bemerkung = 9
};

class BuchungForm : public QDialog
{
    Q_OBJECT

public:
    // QWidget *parent, QSqlTableModel *tableModel, int PersonId
    // BuchungForm(int BuchungId, QWidget *parent = 0);
    BuchungForm(QWidget *parent, QSqlTableModel *personenModel, const QModelIndex &PersonIndex, QSqlRelationalTableModel *buchungenModel, const QModelIndex &BuchungIndex);

    void done(int result);

private slots:
    void addBuchung();
    void deleteBuchung();
    void dkBestaetigenBuchung();

private:
    void writeDataForDkBestaetigenBuchung();

private:

    QSqlTableModel *m_personenModel;
    QModelIndex m_PersonIndex;
    QSqlRelationalTableModel *m_buchungenModel;
    QModelIndex m_BuchungIndex;
    QDataWidgetMapper *mapper;

    QLabel *buchungIdLabel;
    QLabel *personIdLabel;
    QLabel *datumLabel;
    QLabel *dkNrLabel;
    QLabel *dkNummerLabel;
    QLabel *rueckzahlungLabel;
    QLabel *vorgemerktLabel;
    QLabel *betragLabel;
    QLabel *zinssatzLabel;
    QLabel *bemerkungLabel;

    QLineEdit *buchungIdEdit;
    QLineEdit *personIdEdit;
    QDateEdit *datumEdit;
    QLineEdit *dkNrEdit;
    QLineEdit *dkNummerEdit;
    QLineEdit *rueckzahlungEdit;
    QLineEdit *vorgemerktEdit;
    QLineEdit *betragEdit;
    // QLineEdit *zinssatzEdit;
    QComboBox *zinssatzComboBox;
    QTextEdit *bemerkungEdit;

    QPushButton *firstButton;
    QPushButton *previousButton;
    QPushButton *nextButton;
    QPushButton *lastButton;

    QPushButton *addButton;
    QPushButton *deleteButton;
    QPushButton *dkBestaetigenButton;
    QPushButton *closeButton;

    QDialogButtonBox *buttonBox;
};

#endif
