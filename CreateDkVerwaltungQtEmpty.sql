/* sqlite3 DkVerwaltungQt.db3 < CreateDkVerwaltungQtEmpty.sql */
CREATE TABLE DKPersonen (PersonId INTEGER PRIMARY KEY AUTOINCREMENT, Vorname TEXT, Name TEXT, Anrede TEXT, Straße TEXT, PLZ TEXT, Ort TEXT, Email TEXT);
CREATE TABLE DKBuchungen (BuchungId INTEGER PRIMARY KEY AUTOINCREMENT, PersonId INTEGER, Datum TEXT, DKNr TEXT, DKNummer TEXT, Rueckzahlung TEXT, vorgemerkt TEXT, Betrag REAL, Zinssatz REAL, Bemerkung TEXT, FOREIGN KEY(PersonId) REFERENCES DKPerson(PersonId));
CREATE TABLE DKZinssaetze (Zinssatz REAL PRIMARY KEY, Beschreibung TEXT);
/* Zinssätze hinzufügen*/
INSERT INTO DKZinssaetze VALUES (0.0, '');
INSERT INTO DKZinssaetze VALUES (0.5, '');
INSERT INTO DKZinssaetze VALUES (1.0, '');
INSERT INTO DKZinssaetze VALUES (1.5, '');

