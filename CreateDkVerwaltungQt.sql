/* sqlite3 DkVerwaltungQt.db3 < CreateDkVerwaltungQt.sql */
PRAGMA writable_schema = 1;
delete from sqlite_master where type in ('table', 'index', 'trigger');
PRAGMA writable_schema = 0;
VACUUM;
PRAGMA INTEGRITY_CHECK;
-- CREATE TABLE DKVerwaltungOrg (Datum TEXT, DKNr TEXT, Vorname TEXT, Name TEXT, Anrede TEXT, DKNummer TEXT, Straße TEXT, PLZ TEXT, Ort TEXT, Email TEXT, Rueckzahlung TEXT, vorgemerkt TEXT, Betrag TEXT, Zinssatz TEXT, Bemerkung TEXT);
-- .separator ","
-- .header on
.mode csv
.import DK-Verwaltung_F13_Turley_GmbH.csv DKVerwaltungOrg
ALTER TABLE DKVerwaltungOrg RENAME COLUMN "Dk-Nr." to "DKNr";
ALTER TABLE DKVerwaltungOrg RENAME COLUMN "Dk Nummer" to "DKNummer";
ALTER TABLE DKVerwaltungOrg RENAME COLUMN "Rückzahlung" to "Rueckzahlung";
CREATE TABLE DKVerwaltung (Datum TEXT, DKNr TEXT, Vorname TEXT, Name TEXT, Anrede TEXT, DKNummer TEXT, Straße TEXT, PLZ TEXT, Ort TEXT, Email TEXT, Rueckzahlung TEXT, vorgemerkt TEXT, Betrag TEXT, Zinssatz TEXT, Bemerkung TEXT);
INSERT INTO DKVerwaltung SELECT * FROM DKVerwaltungOrg WHERE CAST(DkNr AS INTEGER) <> 0 ORDER BY CAST(DkNr AS INTEGER);
-- SELECT "Anzahl: " || cast(COUNT(*) as text) FROM DkVerwaltung;
-- SELECT changes();
SELECT "Anzahl: " || cast(changes() as text);
DROP TABLE DKVerwaltungORG;
CREATE TABLE DKPersonen (PersonId INTEGER PRIMARY KEY AUTOINCREMENT, Vorname TEXT, Name TEXT, Anrede TEXT, Straße TEXT, PLZ TEXT, Ort TEXT, Email TEXT);
-- CREATE TABLE DKBuchungen (BuchungId INTEGER PRIMARY KEY AUTOINCREMENT, PersonId INTEGER, Datum TEXT, DKNr TEXT, DKNummer TEXT, Rueckzahlung TEXT, vorgemerkt TEXT, Betrag REAL, Zinssatz REAL, Bemerkung TEXT, FOREIGN KEY(PersonId) REFERENCES DKPerson(PersonId));
CREATE TABLE DKBuchungen (BuchungId INTEGER PRIMARY KEY AUTOINCREMENT, PersonId INTEGER, Datum TEXT, DKNr TEXT, DKNummer TEXT, Rueckzahlung TEXT, vorgemerkt TEXT, Betrag REAL, Zinssatz REAL, Bemerkung TEXT, Anfangsdatum TEXT, AnfangsBetrag REAL, FOREIGN KEY(PersonId) REFERENCES DKPerson(PersonId));
INSERT INTO DKPersonen (Vorname, Name, Anrede, Straße, PLZ, Ort, Email)
SELECT                  Vorname, Name, Anrede, Straße, PLZ, Ort, Email FROM DKVerwaltung
WHERE CAST(DkNr AS INTEGER) <> 0;
INSERT INTO DKBuchungen (PersonId, Datum, DKNr, DKNummer, Rueckzahlung, vorgemerkt, Betrag, Zinssatz, Bemerkung)
SELECT                      ROWID, Datum, DKNr, DKNummer, Rueckzahlung, vorgemerkt, Betrag, Zinssatz, Bemerkung FROM DKVerwaltung
WHERE CAST(DkNr AS INTEGER) <> 0;
DROP table DKVerwaltung;
UPDATE DkBuchungen SET Betrag = replace(replace(replace(Betrag, ' €', ''), '.', ''), ',', '.');
/* UPDATE DkBuchungen SET Betrag = replace(replace(Betrag, ' €', ''), '.', ''); */
UPDATE DkBuchungen SET Zinssatz = replace(replace(Zinssatz, '%', ''), ',', '.');
/* UPDATE DkBuchungen SET Zinssatz = replace(Zinssatz, '%', ''); */
/* UPDATE DkBuchungen SET Zinssatz = replace(Zinssatz, '.', ','); */
CREATE TABLE DKZinssaetze (Zinssatz REAL PRIMARY KEY, Beschreibung TEXT);
INSERT INTO DKZinssaetze (Zinssatz)
SELECT Zinssatz FROM DkBuchungen GROUP BY Zinssatz ORDER BY Zinssatz;

/* sqlite3 DkVerwaltungQt2020.db3 < UpdateDkVerwaltungQt.sql */
UPDATE DkBuchungen SET PersonId = (select mpid from (Select t.PersonId AS pid, xx.MaxPersonId as mpid, t.Vorname , t.Name from DkPersonen t inner join ( Select max(PersonId) AS MaxPersonId, Vorname, Name from DkPersonen group by Vorname, Name) xx on t.Vorname = xx.Vorname AND t.Name = xx.Name ORDER BY PersonID) where DkBuchungen.PersonId = pid);
SELECT "Anzahl DkBuchungen DkPersonen zugordnet: " || cast(changes() as text);
DELETE FROM DkPersonen WHERE NOT EXISTS (SELECT * FROM DkBuchungen WHERE DkBuchungen.PersonId = DkPersonen.PersonId);
SELECT "Anzahl DkPersonen gelöscht: " || cast(changes() as text);
-- ALTER TABLE DkBuchungen ADD COLUMN Anfangsdatum TEXT;
-- ALTER TABLE DkBuchungen ADD COLUMN AnfangsBetrag REAL;
UPDATE DkBuchungen SET Anfangsdatum = substr(Bemerkung,1,8) WHERE instr(Bemerkung, 'neu angelegt') <> 0;
SELECT "Anzahl DkBuchungen Anfangsdatum gesetzt: " || cast(changes() as text);
UPDATE DkBuchungen SET Anfangsbetrag = CAST(replace(replace(substr(Bemerkung, instr(Bemerkung, 'Betrag: ') + 8), '.', ''), ',', '.') AS FLOAT) WHERE instr(Bemerkung, 'neu angelegt') <> 0;
SELECT "Anzahl DkBuchungen Anfangsbetrag gesetzt: " || cast(changes() as text);
