/* sqlite3 DkVerwaltungQt.db3 < CreateDkVerwaltungQt.sql */
CREATE TABLE DKVerwaltungOrg (Datum TEXT, DKNr TEXT, Vorname TEXT, Name TEXT, Anrede TEXT, DKNummer TEXT, Straße TEXT, PLZ TEXT, Ort TEXT, Email TEXT, Rueckzahlung TEXT, vorgemerkt TEXT, Betrag TEXT, Zinssatz TEXT, Bemerkung TEXT);
.separator ","
.header on
.import DK-Verwaltung_F13_Turley_GmbH.csv DKVerwaltungOrg
CREATE TABLE DKVerwaltung (Datum TEXT, DKNr TEXT, Vorname TEXT, Name TEXT, Anrede TEXT, DKNummer TEXT, Straße TEXT, PLZ TEXT, Ort TEXT, Email TEXT, Rueckzahlung TEXT, vorgemerkt TEXT, Betrag TEXT, Zinssatz TEXT, Bemerkung TEXT);
INSERT INTO DKVerwaltung SELECT * FROM DKVerwaltungOrg ORDER BY CAST(DkNr AS INTEGER);
DROP table DKVerwaltungORG;
CREATE TABLE DKPersonen (PersonId INTEGER PRIMARY KEY AUTOINCREMENT, Vorname TEXT, Name TEXT, Anrede TEXT, Straße TEXT, PLZ TEXT, Ort TEXT, Email TEXT);
CREATE TABLE DKBuchungen (BuchungId INTEGER PRIMARY KEY AUTOINCREMENT, PersonId INTEGER, Datum TEXT, DKNr TEXT, DKNummer TEXT, Rueckzahlung TEXT, vorgemerkt TEXT, Betrag REAL, Zinssatz REAL, Bemerkung TEXT, FOREIGN KEY(PersonId) REFERENCES DKPerson(PersonId));
INSERT INTO DKPersonen (Vorname, Name, Anrede, Straße, PLZ, Ort, Email)
SELECT                  Vorname, Name, Anrede, Straße, PLZ, Ort, Email FROM DKVerwaltung;
INSERT INTO DKBuchungen (PersonId, Datum, DKNr, DKNummer, Rueckzahlung, vorgemerkt, Betrag, Zinssatz, Bemerkung)
SELECT                      ROWID, Datum, DKNr, DKNummer, Rueckzahlung, vorgemerkt, Betrag, Zinssatz, Bemerkung FROM DKVerwaltung;
DROP table DKVerwaltung;
UPDATE DkBuchungen SET Betrag = replace(replace(replace(Betrag, ' €', ''), '.', ''), ',', '.');
/* UPDATE DkBuchungen SET Betrag = replace(replace(Betrag, ' €', ''), '.', ''); */
UPDATE DkBuchungen SET Zinssatz = replace(replace(Zinssatz, '%', ''), ',', '.');
/* UPDATE DkBuchungen SET Zinssatz = replace(Zinssatz, '%', ''); */
/* UPDATE DkBuchungen SET Zinssatz = replace(Zinssatz, '.', ','); */
CREATE TABLE DKZinssaetze (Zinssatz REAL PRIMARY KEY, Beschreibung TEXT);
INSERT INTO DKZinssaetze (Zinssatz)
SELECT Zinssatz FROM DkBuchungen GROUP BY Zinssatz ORDER BY Zinssatz;
