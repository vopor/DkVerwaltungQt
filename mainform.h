#ifndef MAINFORM_H
#define MAINFORM_H

#include <QWidget>

QT_BEGIN_NAMESPACE
class QDialogButtonBox;
class QLabel;
class QLineEdit;
class QCheckBox;
class QModelIndex;
class QPushButton;
class QSplitter;
class QSqlTableModel;
class QSqlRelationalTableModel;
class QTableView;
class BuchungenSortFilterProxyModel;
QT_END_NAMESPACE

class MainForm : public QWidget
{
    Q_OBJECT

public:
    MainForm(QWidget *parent);

public slots:
    void updateViews();

private slots:

    void updatePersonenView();
    void updateBuchungenView();
    void updateBuchungenSummen();
    void updateSummen();

    void toggleAnzeigenPersonen();

    void addPerson();
    void editPerson();
    void deletePerson();

    void addBuchung();
    void editBuchung();
    void deleteBuchung();

    void suchenPerson();
    void filterPerson();
    void suchenEditingFinished();

    void filterBuchungen();

    void generateJahresDkBestaetigungen();
    void generateJahresDkBestaetigungenTerminal();
    void generateJahresDkZinsBescheinigungen();
    void generateJahresDkZinsBescheinigungenTerminal();

    bool checkPrerequisitesExists();
private:

    void createAnzeigenPersonenPanel();
    void createPersonenPanel();
    void createBuchungenPanel();
    void createSummenPanel();
    void createBescheinigungenPanel();

    int getPersonId();
    int getBuchungId();

    int getJahr();
    QString getJahresDkBestaetigungenPath();
    QString getJahresDkZinsBescheinigungenPath();

private:
    QSqlTableModel *personenModel;
    QSqlRelationalTableModel *buchungenModel;
    BuchungenSortFilterProxyModel *buchungenSortModel;

    QLabel *anzeigenPersonenLabel;
    QPushButton *anzeigenPersonenButton;

    QLabel *personIdLabel;
    QLabel *vornameLabel;
    QLabel *nameLabel;

    QLineEdit *personIdEdit;
    QLineEdit *vornameEdit;
    QLineEdit *nameEdit;

    QPushButton *suchenPersonButton;
    QPushButton *filterPersonButton;

    QWidget *anzeigenPersonenPanel;
    QWidget *personenPanel;
    QWidget *buchungenPanel;
    QWidget *summenPanel;
    QWidget *bescheinigungenPanel;

    // QLabel *personenLabel;
    QLabel *buchungenLabel;
    QLabel *summenLabel;

    QTableView *personenView;
    QTableView *buchungenView;

    QSplitter *splitter;

    // QLabel *vorgemerktLabel;
    QCheckBox *vorgemerktCheckBox;
    // QPushButton *filterBuchungenButton;

    QPushButton *addPersonButton;
    QPushButton *editPersonButton;
    QPushButton *deletePersonButton;
    QDialogButtonBox *personButtonBox;

    QLabel *datumBuchungenDkZinsenLabel;
    QLineEdit *datumBuchungenDkZinsenEdit;
    QLabel *summeBuchungenDkZinsenLabel;
    QLineEdit *summeBuchungenDkZinsenEdit;
    QLabel *summeBuchungenDkLabel;
    QLineEdit *summeBuchungenDkEdit;

    QPushButton *addBuchungButton;
    QPushButton *editBuchungButton;
    QPushButton *deleteBuchungButton;
    QDialogButtonBox *buchungButtonBox;

    QLabel *summeDkZinsenLabel;
    QLineEdit *summeDkZinsenEdit;
    QLabel *summeDkLabel;
    QLineEdit *summeDkEdit;
    QLabel *summeDkGeberInnenLabel;
    QLineEdit *summeDkGeberInnenEdit;
    QLabel *summeDkVertraegeLabel;
    QLineEdit *summeDkVertraegeEdit;

    QLabel *jahrDkBescheinigungenLabel;
    QLineEdit *jahrDkBescheinigungenEdit;

    QCheckBox *nurCheckBox;
    QLineEdit *nurEdit;
    QCheckBox *sendenCheckBox;
    QPushButton *continueButton;
    QPushButton *cancelButton;
    int continueButtonClicked = 0;
    int cancelButtonClicked = 0;

    QPushButton *generateJahresDkBestaetigungenButton;
    QPushButton *generateJahresDkBestaetigungenTerminalButton;
    QPushButton *generateJahresDkZinsBescheinigungenButton;
    QPushButton *generateJahresDkZinsBescheinigungenTerminalButton;

    QPushButton *quitButton;
    QDialogButtonBox *buttonBox;
};

#endif
