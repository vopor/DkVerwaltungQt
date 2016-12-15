/* sqlite3 DkVerwaltungQt.db3 < UpdateDkVerwaltungQt.sql */
update DkBuchungen set PersonId = (select mpid from (Select t.PersonId AS pid, xx.MaxPersonId as mpid, t.Vorname , t.Name from DkPersonen t inner join ( Select max(PersonId) AS MaxPersonId, Vorname, Name from DkPersonen group by Vorname, Name) xx on t.Vorname = xx.Vorname AND t.Name = xx.Name ORDER BY PersonID) where DkBuchungen.PersonId = pid);
DELETE FROM DkPersonen WHERE NOT EXISTS (SELECT * FROM DkBuchungen WHERE DkBuchungen.PersonId = DkPersonen.PersonId);
ALTER TABLE DkBuchungen ADD COLUMN Anfangsdatum TEXT;
ALTER TABLE DkBuchungen ADD COLUMN AnfangsBetrag REAL;
UPDATE DkBuchungen SET Anfangsdatum = substr(Bemerkung,1,8) WHERE instr(Bemerkung, 'neu angelegt') <> 0;
UPDATE DkBuchungen SET Anfangsbetrag = CAST(replace(replace(substr(Bemerkung, instr(Bemerkung, 'Betrag: ') + 8), '.', ''), ',', '.') AS FLOAT) WHERE instr(Bemerkung, 'neu angelegt') <> 0;


