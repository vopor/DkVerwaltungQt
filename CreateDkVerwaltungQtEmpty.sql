/* sqlite3 DkVerwaltungQt.db3 < CreateDkVerwaltungQtEmpty.sql */

/*
Tabelle:
DkPersonen

Felder:

noch wünschenswerte Felder:
- Bankverbindung

*/
CREATE TABLE DKPersonen (PersonId INTEGER PRIMARY KEY AUTOINCREMENT, Vorname TEXT, Name TEXT, Anrede TEXT, Straße TEXT, PLZ TEXT, Ort TEXT, Email TEXT);

/*
Tabelle:
DKBuchungen

Felder:
BuchungId - Primärschlüssel
PersonId - Referenziert die Tabelle DkPersonen
Datum - kommt aktuell aus OOCP (OpenOffice Calc Programm) und entspricht dem Datum >= dem 1.1. des aktuellen Jahres
DKNr - numerische frei zu vergebende Dk-Nummer
DKNummer - alphanumerische frei zu vergebende Dk-Nummer
Rueckzahlung - enthält das Datum, andem der Dk tatsächlich zurückgezahlt wurde
vorgemerkt -  enthält das Datum, andem geplant ist, den Dk zurück zu zahlen
Betrag - enthält den Betrag zum Datum (Datum >= dem 1.1. des aktuellen Jahres)
Zinssatz - referenziert Zinssatz aus DKZinssaetze
Bemerkung - Bemerkungsfeld (aus OOCP), welches unter anderem das Anfangsdatum und den AnfangsBetrag enthält
Anfangsdatum - Anfangsdatum des Dks (wird durch UpdateDkVerwaltungQt.sql erzeugt und aus Bemerkung gefüllt)
AnfangsBetrag - AnfangsBetrag des Dks (wird durch UpdateDkVerwaltungQt.sql erzeugt und aus Bemerkung gefüllt)

Anmerkungen:

Da das OOCP (OpenOffice Calc Programm) jeweils nur ein Jahr abbildet, gibt es die Felder Datum+Betrag und nicht nur Anfangsdatum+AnfangsBetrag.
Anfangsdatum+AnfangsBetrag werden erst während des Import in die DkVerwaltungQt erzeugt und gefüllte (CreateDkVerwaltungQt.sql+UpdateDkVerwaltungQt.sql)
Anfangsdatum+AnfangsBetrag würden eigentlich für die ganze weitere Berechnung ausreichen.

Zurückzuzahlen ist der Dk leider z.Zt. nur komplett und nicht in Teilzahlungen, da nur ein Feld Rueckzahlung gefüllt werden kann und keine (Splitt-)Buchungen möglich sind.
Oder man macht es wie im OOCP: Es werden Buchungen mit einem negativen Betrag und denselben Dk-Nummern, PersonId, ... erzeugt.

noch wünschenswerte Felder:
- Kündingungsfrist

*/

CREATE TABLE DKBuchungen (BuchungId INTEGER PRIMARY KEY AUTOINCREMENT, PersonId INTEGER, Datum TEXT, DKNr TEXT, DKNummer TEXT, Rueckzahlung TEXT, vorgemerkt TEXT, Betrag REAL, Zinssatz REAL, Bemerkung TEXT, Anfangsdatum TEXT, AnfangsBetrag REAL, FOREIGN KEY(PersonId) REFERENCES DKPerson(PersonId));

/*
Tabelle:
DKZinssaetze

*/
CREATE TABLE DKZinssaetze (Zinssatz REAL PRIMARY KEY, Beschreibung TEXT);

/* Zinssätze hinzufügen*/
INSERT INTO DKZinssaetze VALUES (0.0, '');
INSERT INTO DKZinssaetze VALUES (0.5, '');
INSERT INTO DKZinssaetze VALUES (1.0, '');
INSERT INTO DKZinssaetze VALUES (1.5, '');

