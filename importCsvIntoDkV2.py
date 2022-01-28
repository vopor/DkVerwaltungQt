#!/usr/bin/python
# coding: utf8

# -*- coding: utf-8 -*-

#
# Das Script importiert die CSV-Datei nach DKV2.
# 
# Aufruf: ./importCsvIntoDkV2.py <DkVerwaltung_csv_file> <DKV2-db3-file>
#

import os
import sys
import shutil
import traceback
import csv
from collections import defaultdict
import sqlite3
import subprocess
# import pandas
import copy

dropUnusedCsvTables = False
dropUnusedDkVerwaltungQtTables = False

def versiontuple(v):
    return tuple(map(int, (v.split("."))))

# https://stackoverflow.com/questions/7831371/is-there-a-way-to-get-a-list-of-column-names-in-sqlite
def get_col_names(cursor, tablename):
    """Get column names of a table, given its name and a cursor
       (or connection) to the database.
    """
    reader=cursor.execute("SELECT * FROM {}".format(tablename))
    return [x[0] for x in reader.description] 

def print_anzahl_in_table(stmt, tableName):
    select_stmt = "SELECT COUNT(*) FROM " + tableName
    stmt.execute(select_stmt)
    anzahl = stmt.fetchone()[0]
    print "Anzahl " + tableName + ": ", anzahl

# print "sqlite3.version=" + sqlite3.version
# print "sqlite3.sqlite_version=" + sqlite3.sqlite_version
# print "argv=" + str(sys.argv)
# print "argc=" + str(len(sys. argv))

if versiontuple(sqlite3.sqlite_version) < versiontuple("3.26.0"):
    print "Die sqlite3 version muss >= 3.26.0 sein, "
    print "da COLUMN RENAME verwendet wird."
    sys.exit(1)

if len(sys. argv) < 3:
    print "Aufruf: ./" + os.path.basename(__file__) + " <DkVerwaltung_csv_file> <DKV2-db3-file>"
    sys.exit(1)

DkVerwaltung_csv_file=sys.argv[1]
DKV2_db3_file=sys.argv[2]

if not os.path.isfile(DkVerwaltung_csv_file):
    print DkVerwaltung_csv_file + " existiert nicht."
    sys.exit(2)

if True:
    if not os.path.isfile(DKV2_db3_file):
        print DKV2_db3_file + "existiert nicht."
        sys.exit(3)

else:
    # if os.path.isfile(DKV2_db3_file):
    #     os.remove(DKV2_db3_file)
    if os.path.isfile(DKV2_db3_file):
        print DKV2_db3_file + " existiert."
        msg = 'Soll sie neu erzeugt werden (ja=neu erzeugen/nein=neu füllen/abbrechen)?'
        str = raw_input("%s (j/n*/a) " % msg).lower()
        if ((str == 'j')):
            os.remove(DKV2_db3_file)
        elif (str == 'n') or (str == ''):
            pass
        else:
            sys.exit(3)

    if not os.path.isfile(DKV2_db3_file):
        Empty_DKV2_db3_file = os.path.dirname(os.path.realpath(__file__)) + os.sep + "Empty-DKV2-Datenbank.dkdb"    
        print Empty_DKV2_db3_file + " => " + DKV2_db3_file
        shutil.copyfile(Empty_DKV2_db3_file, DKV2_db3_file)

try:

    conn = sqlite3.connect(DKV2_db3_file)
    stmt = conn.cursor()

    stmt.execute('DROP TABLE IF EXISTS DKVerwaltungORG;')
    stmt.execute('DROP TABLE IF EXISTS DKVerwaltung;')

    stmt.execute('DROP TABLE IF EXISTS DkPersonen;')
    stmt.execute('DROP TABLE IF EXISTS DkBuchungen;')
    stmt.execute('DROP TABLE IF EXISTS DKZinssaetze;')

    stmt.execute("DELETE FROM Buchungen");
    stmt.execute("DELETE FROM Vertraege");
    stmt.execute("DELETE FROM ExBuchungen");
    stmt.execute("DELETE FROM ExVertraege");
    stmt.execute("DELETE FROM Kreditoren");
    conn.commit()

    print "csv => DKV2"
    #
    # sqlite3 DkVerwaltungQt.db3 < CreateDkVerwaltungQt.sql
    #
    print "csv -> DkVerwaltungQt"
    print "csv importieren"
    if False:
        # https://stackoverflow.com/questions/2887878/importing-a-csv-file-into-a-sqlite3-database-table-using-python
        params = ['sqlite3',  DKV2_db3_file.replace('\\','\\\\'),
                         '-cmd',
                         '.mode csv',
                         '.import ' + "'" + DkVerwaltung_csv_file.replace('\\','\\\\')  + "'"  + ' DKVerwaltungOrg']
        # print params
        # cmdstr = 'sqlite3 ' + DKV2_db3_file + ' -cmd ' + ' .mode csv ' + ' .import ' + DkVerwaltung_csv_file.replace('\\','\\\\')  + ' DKVerwaltungOrg'
        # print cmdstr                         
        p = subprocess.Popen(params)
        (output, err) = p.communicate() 
        p_status = p.wait()
        if p_status != 0:
            print "output=" + output
            print "err=" + err
            print "p_status=" + p_status

        stmt.execute('ALTER TABLE DKVerwaltungOrg RENAME COLUMN "DK-Nr." to "DKNr";')
        stmt.execute('ALTER TABLE DKVerwaltungOrg RENAME COLUMN "Rückzahlung" to "Rueckzahlung";')
    else:
        with open(DkVerwaltung_csv_file, 'r') as f:
            reader = csv.DictReader(f, delimiter=",", quotechar="\"")
            create_table_statement = "CREATE TABLE DKVerwaltungOrg (" 
            ifieldnames = ["DkNr" if fieldname == "DK-Nr." else fieldname for fieldname in reader.fieldnames]
            ifieldnames = ["Rueckzahlung" if fieldname == "Rückzahlung" else fieldname for fieldname in ifieldnames]
            ifieldnames = ["'" + fieldname + "'" for fieldname in ifieldnames]
            create_table_statement += ','.join(ifieldnames) 
            create_table_statement += ");"
            print create_table_statement
            stmt.execute(create_table_statement)
            first_row = True
            for row in reader:
                insert_statement = "INSERT INTO DKVerwaltungOrg ("  + ','.join(ifieldnames) + ") "
                insert_statement += "VALUES "
                insert_statement += "("
                for index, fieldname in enumerate(reader.fieldnames):
                    insert_statement += "'" + row[fieldname] + "'"
                    if index < len(row)-1:
                        insert_statement += ","

                insert_statement += ");"
                # print insert_statement
                stmt.execute(insert_statement)
                conn.commit()
        conn.commit()

    print_anzahl_in_table(stmt, "DKVerwaltungOrg")

    select_exists_statement = "SELECT EXISTS (SELECT * FROM sqlite_master WHERE tbl_name = 'DKVerwaltungOrg' AND sql LIKE '%DK Nummer%');"
    stmt.execute(select_exists_statement)
    column_dk_nummer_exists = stmt.fetchone()[0]
    print "column_dk_nummer_exists: ", column_dk_nummer_exists
    if column_dk_nummer_exists:
        stmt.execute('ALTER TABLE DKVerwaltungOrg RENAME COLUMN "DK Nummer" to "DKNummer";')
    else:
        stmt.execute('ALTER TABLE DKVerwaltungOrg ADD COLUMN "DKNummer";')
    
    select_exists_statement = "SELECT COUNT(*) FROM DKVerwaltungOrg WHERE CAST(DkNr AS INTEGER) = 0;"
    stmt.execute(select_exists_statement)
    count_dknr_alnum = stmt.fetchone()[0]
    print "count_dknr_alnum: ", count_dknr_alnum

    if count_dknr_alnum > 0: # DkNr alphanumerisch
        print "DkNummer setzen: "
        stmt.execute('UPDATE DKVerwaltungOrg SET DkNummer = DkNr;')
        print "Anzahl: ", stmt.rowcount
        print "DkNr setzen: "
        stmt.execute('UPDATE DKVerwaltungOrg SET DkNr = ROWID;')
        print "Anzahl: ", stmt.rowcount
        conn.commit();

    print "Datensätze ohne DkNummer löschen: "
    stmt.execute('DELETE FROM DKVerwaltungOrg WHERE DkNummer = "";')
    print "Anzahl: ", stmt.rowcount
    conn.commit();

    print "Datensätze ohne Betrag löschen: "
    stmt.execute('DELETE FROM DKVerwaltungOrg WHERE Betrag = "" OR Betrag IS NULL;')
    print "Anzahl: ", stmt.rowcount
    conn.commit();

    print_anzahl_in_table(stmt, "DKVerwaltungOrg")

    stmt.execute('CREATE TABLE DKVerwaltung (Datum TEXT, DKNr TEXT, Vorname TEXT, Name TEXT, Anrede TEXT, DKNummer TEXT, Straße TEXT, PLZ TEXT, Ort TEXT, Email TEXT, Rueckzahlung TEXT, vorgemerkt TEXT, Betrag TEXT, Zinssatz TEXT, Bemerkung TEXT);')

    colnamesFrom = get_col_names(stmt, "DKVerwaltungOrg")
    colnamesTo = get_col_names(stmt, "DKVerwaltung")
    # print colnamesFrom
    # print colnamesTo
    colnames = []
    for colname in colnamesTo:
        if colname in colnamesFrom:
            colnames.append(colname)
        else:
            print colname 
            colnames.append('DkNr')
    colnamesstring = ','.join(colnames)
    # print colnamesstring

    if True: # DkNr numerisch
        insert_int_statement = 'INSERT INTO DKVerwaltung SELECT ' + colnamesstring + ' FROM DKVerwaltungOrg WHERE CAST(DkNr AS INTEGER) <> 0 ORDER BY CAST(DkNr AS INTEGER);'
    else: # DkNr alphanumerisch
        insert_int_statement = 'INSERT INTO DKVerwaltung SELECT ' + colnamesstring + ' FROM DKVerwaltungOrg WHERE DkNr <> "" ORDER BY DkNr;'

    # print insert_int_statement
    stmt.execute(insert_int_statement)
    print "Anzahl: ", stmt.rowcount
    print "Personen hinzufügen"
    stmt.execute('CREATE TABLE DKPersonen (PersonId INTEGER PRIMARY KEY AUTOINCREMENT, Vorname TEXT, Name TEXT, Anrede TEXT, Straße TEXT, PLZ TEXT, Ort TEXT, Email TEXT);')
    stmt.execute('INSERT INTO DKPersonen (Vorname, Name, Anrede, Straße, PLZ, Ort, Email) SELECT Vorname, Name, Anrede, Straße, PLZ, Ort, Email FROM DKVerwaltung;') #  WHERE CAST(DkNr AS INTEGER) <> 0
    print "Anzahl: ", stmt.rowcount
    print "Buchungen hinzufügen"
    stmt.execute('CREATE TABLE DKBuchungen (BuchungId INTEGER PRIMARY KEY AUTOINCREMENT, PersonId INTEGER, Datum TEXT, DKNr TEXT, DKNummer TEXT, Rueckzahlung TEXT, vorgemerkt TEXT, Betrag REAL, Zinssatz REAL, Bemerkung TEXT, Anfangsdatum TEXT, AnfangsBetrag REAL, FOREIGN KEY(PersonId) REFERENCES DKPerson(PersonId));')
    stmt.execute('INSERT INTO DKBuchungen (PersonId, Datum, DKNr, DKNummer, Rueckzahlung, vorgemerkt, Betrag, Zinssatz, Bemerkung) SELECT ROWID, Datum, DKNr, DKNummer, Rueckzahlung, vorgemerkt, Betrag, Zinssatz, Bemerkung FROM DKVerwaltung;') # WHERE CAST(DkNr AS INTEGER) <> 0
    print "Anzahl: ", stmt.rowcount
    stmt.execute("UPDATE DkBuchungen SET Betrag = replace(replace(replace(Betrag, ' €', ''), '.', ''), ',', '.');")
    stmt.execute("UPDATE DkBuchungen SET Zinssatz = replace(replace(Zinssatz, '%', ''), ',', '.');")
    stmt.execute('CREATE TABLE DKZinssaetze (Zinssatz REAL PRIMARY KEY, Beschreibung TEXT);')
    stmt.execute('INSERT INTO DKZinssaetze (Zinssatz) SELECT Zinssatz FROM DkBuchungen GROUP BY Zinssatz ORDER BY Zinssatz;')
    conn.commit()
    #
    # sqlite3 DkVerwaltungQt.db3 < UpdateDkVerwaltungQt.sql
    #
    print "DkVerwaltungQt aktualisieren..."
    # Bei Personen mit gleichem Vornamen und Namen nur den zuletzt erfassten Personen-Datensatz weiterverwenden
    # stmt.execute('UPDATE DkBuchungen SET PersonId = (select mpid from (Select t.PersonId AS pid, xx.MaxPersonId as mpid, t.Vorname , t.Name from DkPersonen t inner join ( Select max(PersonId) AS MaxPersonId, Vorname, Name from DkPersonen group by Vorname, Name) xx on t.Vorname = xx.Vorname AND t.Name = xx.Name ORDER BY PersonID) where DkBuchungen.PersonId = pid);')
    update_statement = ''
    update_statement += 'UPDATE DkBuchungen '
    update_statement += 'SET PersonId = '
    update_statement += '('
    update_statement += 'SELECT mpid FROM '    
    update_statement += '('
    update_statement += 'SELECT t.PersonId AS pid, xx.MaxPersonId AS mpid, t.Vorname , t.Name '
    update_statement += 'FROM DkPersonen t INNER JOIN '
    update_statement += '('
    update_statement += 'SELECT MAX(PersonId) AS MaxPersonId, Vorname, Name '
    update_statement += 'FROM DkPersonen GROUP BY Vorname, Name) xx '
    update_statement += 'ON t.Vorname = xx.Vorname AND t.Name = xx.Name ORDER BY PersonID'
    update_statement += ') '
    update_statement += 'WHERE DkBuchungen.PersonId = pid'
    update_statement += ');'
    stmt.execute(update_statement)
    print "Anzahl DkBuchungen DkPersonen zugordnet: ", stmt.rowcount
    stmt.execute('DELETE FROM DkPersonen WHERE NOT EXISTS (SELECT * FROM DkBuchungen WHERE DkBuchungen.PersonId = DkPersonen.PersonId);')
    print "Anzahl Zuordnung DkBuchungen DkPersonen gelöscht: ", stmt.rowcount
    # stmt.execute('ALTER TABLE DkBuchungen ADD COLUMN Anfangsdatum TEXT;')
    # stmt.execute('ALTER TABLE DkBuchungen ADD COLUMN AnfangsBetrag REAL;')
    stmt.execute("UPDATE DkBuchungen SET Anfangsdatum = substr(Bemerkung,1,8) WHERE instr(Bemerkung, 'neu angelegt') <> 0;")
    print "Anzahl DkBuchungen Anfangsdatum gesetzt: ", stmt.rowcount
    stmt.execute("UPDATE DkBuchungen SET Anfangsbetrag = CAST(replace(replace(substr(Bemerkung, instr(Bemerkung, 'Betrag: ') + 8), '.', ''), ',', '.') AS FLOAT) WHERE instr(Bemerkung, 'neu angelegt') <> 0;")
    print "Anzahl DkBuchungen Anfangsbetrag gesetzt: ", stmt.rowcount
    # Wenn Anfangsdatum und Anfangsbetrag nicht den Bemerkungen entnommen werden konnten
    stmt.execute("UPDATE DkBuchungen SET Anfangsdatum = Datum, Anfangsbetrag = Betrag WHERE Anfangsdatum IS NULL OR Anfangsbetrag  IS NULL;")
    print "Anzahl DkBuchungen Anfangsdatum auf Datum und Anfangsbetrag auf Betrag gesetzt: ", stmt.rowcount

    # print "Buchungen vor dem aktuellen Jahr löschen: "
    # stmt.execute('DELETE FROM DkBuchungen WHERE substr(Datum,7,2) < (SELECT MAX(substr(Datum,7,2)) FROM DkBuchungen);')
    print "Buchungen ohne Betrag (vor dem aktuellen Jahr) löschen: "
    stmt.execute('DELETE FROM DkBuchungen WHERE Betrag = 0 OR Betrag IS NULL')
    print "Anzahl: ", stmt.rowcount

    print "Stammkapital löschen: "
    stmt.execute('DELETE FROM DkBuchungen WHERE DkNummer = "Stammkapital";')
    print "Anzahl: ", stmt.rowcount

    conn.commit()

    if False:
        update_statement = ''
        update_statement += 'UPDATE DkBuchungen '
        update_statement += 'SET Datum = Anfangsdatum, Betrag = Anfangsbetrag ' 
        update_statement += 'WHERE BuchungId IN '
        update_statement += '('
        update_statement += 'SELECT MIN(c.BuchungId) FROM DkBuchungen c '
        update_statement += 'GROUP BY c.DkNummer '
        update_statement += ')'
        stmt.execute(update_statement)
        print "Anzahl DkBuchungen Anfangsdatum und Anfangsbetrag zugordnet: ", stmt.rowcount
        conn.commit()

    #
    # ./importDkVerwaltungQtIntoDKV2.py
    #
    print "DkVerwaltungQt -> DKV2"
    due_direkt_nach_jahresabschluss = False
    if not due_direkt_nach_jahresabschluss:

        print "Kreditoren hinzufügen"
        insert_stmt = 'INSERT INTO Kreditoren (id, Vorname, Nachname, Strasse, Plz, Stadt, EMail, Anmerkung, IBAN, Bic) '
        insert_stmt += 'SELECT Personid, Vorname, Name, Straße, PLZ, Ort, Email, "", "", "" FROM DKPersonen'
        stmt.execute(insert_stmt);
        print "Anzahl: ", stmt.rowcount
        conn.commit()

        print "Vertraege hinzufügen"
        insert_stmt = 'INSERT INTO Vertraege (id, KreditorId, Kennung, ZSatz, Betrag, thesaurierend, Vertragsdatum, LaufzeitEnde, Kfrist) '
        insert_stmt += 'SELECT BuchungId, PersonId, DKNummer, DKBuchungen.Zinssatz*100, AnfangsBetrag*100, 1, '
        insert_stmt += '"20" || substr(Anfangsdatum,7,2) || "-" || substr(Anfangsdatum,4,2) || "-" || substr(Anfangsdatum,1,2), '
        # insert_stmt += 'CASE WHEN (vorgemerkt != "") THEN ("20" || substr(vorgemerkt,7,2) || "-" || substr(vorgemerkt,4,2) || "-" || substr(vorgemerkt,1,2)) ELSE "9999-12-31" END, '
        insert_stmt += 'CASE WHEN (vorgemerkt != "") THEN ("20" || substr(vorgemerkt,7,2) || "-" || substr(vorgemerkt,4,2) || "-" || substr(vorgemerkt,1,2)) '
        insert_stmt += 'ELSE '
        insert_stmt += 'CASE WHEN (Rueckzahlung != "") THEN ("20" || substr(Rueckzahlung,7,2) || "-" || substr(Rueckzahlung,4,2) || "-" || substr(Rueckzahlung,1,2)) '
        insert_stmt += 'ELSE "9999-12-31" END END, '
        insert_stmt += 'CASE WHEN (vorgemerkt != "") THEN -1 ELSE 6 END '
        insert_stmt += 'FROM DKBuchungen '
        insert_stmt += 'WHERE Anfangsdatum <> "" '
        # insert_stmt += 'AND Rueckzahlung = ""'
        insert_stmt += 'GROUP BY DkNummer '
        stmt.execute(insert_stmt);
        print "Anzahl: ", stmt.rowcount

        print "Vertraege zinslos setzen"
        update_stmt = 'UPDATE Vertraege SET thesaurierend = 3 WHERE ZSatz = 0;'
        stmt.execute(update_stmt);
        print "Anzahl: ", stmt.rowcount

        conn.commit()

        print "Buchungen hinzufügen"
        insert_stmt = 'INSERT INTO Buchungen (id, VertragsId, Datum, Buchungsart, Betrag) '
        insert_stmt += 'SELECT a.BuchungId, a.BuchungId, '
        insert_stmt += '("20" || substr(a.Datum,7,2) || "-" || substr(a.Datum,4,2) || "-" || substr(a.Datum,1,2)), '
        insert_stmt += 'CASE WHEN (a.Betrag >= 0) THEN 1 ELSE 2 END, a.Betrag*100 '
        insert_stmt += 'FROM DKBuchungen a '
        stmt.execute(insert_stmt);
        print "Anzahl: ", stmt.rowcount
        conn.commit()

        print "DKV2 aktualisieren..."

        print "VertragsId setzen"
        update_stmt = 'UPDATE Buchungen SET VertragsId = '
        update_stmt += '('
        update_stmt += 'SELECT c.id '
        update_stmt += 'FROM DKBuchungen b, Vertraege c '
        update_stmt += 'WHERE Buchungen.id = b.BuchungId '
        update_stmt += 'AND b.DkNummer = c.Kennung '
        update_stmt += ')'

        stmt.execute(update_stmt);
    
        print "Anzahl: ", stmt.rowcount
        # vertraege = stmt.fetchall()
        # print "vertraege", len(vertraege)
        # for row in vertraege:
        #     print row
        conn.commit()
    
    print "Vergleiche DkVerwaltungQt <-> DKV2"

    # DkPersonen und Kreditoren
    print "Anzahl DkPersonen:"
    select_stmt = 'SELECT COUNT(*) FROM DkPersonen'
    stmt.execute(select_stmt)
    print stmt.fetchone()[0]
    print "Anzahl Kreditoren:"
    select_stmt = 'SELECT COUNT(*) FROM Kreditoren'
    stmt.execute(select_stmt)
    print stmt.fetchone()[0]
    # Summierte DkBuchungen und Vertraege
    print "Anzahl DKBuchungen (summiert):"
    select_stmt = 'SELECT COUNT(DISTINCT DkNummer) FROM DKBuchungen '
    # select_stmt += 'WHERE Anfangsdatum <> "" AND DkNummer <> "Stammkapital" '
    stmt.execute(select_stmt)
    print stmt.fetchone()[0]
    print "Anzahl Vertraege:"
    select_stmt = 'SELECT COUNT(*) FROM Vertraege'
    stmt.execute(select_stmt)
    print stmt.fetchone()[0]
    # Summierte DkBuchungen und Vertraege
    print "Anzahl DKBuchungen:"
    select_stmt = 'SELECT COUNT(*) FROM DKBuchungen '
    # select_stmt += 'WHERE Anfangsdatum <> "" AND DkNummer <> "Stammkapital" '
    stmt.execute(select_stmt)
    print stmt.fetchone()[0]
    print "Anzahl Buchungen:"
    select_stmt = 'SELECT COUNT(*) FROM Buchungen'
    stmt.execute(select_stmt)
    print stmt.fetchone()[0]

    print "Vertraege, die nicht übernommen wurden:"
    select_stmt = 'SELECT * FROM DKBuchungen '
    select_stmt += 'WHERE Anfangsdatum <> "" AND DkNummer <> "Stammkapital" '
    select_stmt += 'AND NOT EXISTS'
    select_stmt += '('
    select_stmt += 'SELECT * FROM Vertraege WHERE DKBuchungen.DkNummer = Vertraege.Kennung '
    # select_stmt += 'UNION '
    # select_stmt += 'SELECT * FROM ExVertraege WHERE DKBuchungen.DkNummer = ExVertraege.Kennung '
    select_stmt += ')'
    stmt.execute(select_stmt)
    buchungen = stmt.fetchall()
    for row in buchungen:
        print row
    print "Anzahl: ", len(buchungen)

    if True:
        print "VertragsId prüfen"
        select_stmt = 'SELECT c.id FROM Buchungen, DKBuchungen b, Vertraege c '
        select_stmt += 'WHERE Buchungen.id = b.BuchungId '
        select_stmt += 'AND b.DkNummer = c.Kennung '
        select_stmt += 'AND c.id IS NULL '
        stmt.execute(select_stmt);
        buchungen = stmt.fetchall()
        if len(buchungen) > 0:
            print "buchungen", len(buchungen)
            for row in buchungen:
                print row
        print "Anzahl: ", len(buchungen)

    print "Vertraege mit abweichendem aufsummierten Betrag:"
    
    select_stmt = '';
    select_stmt += 'SELECT KennungA, SummeA '
    select_stmt += 'FROM '
    select_stmt += '( '
    select_stmt += 'SELECT  Vertraege.Kennung AS KennungA, ROUND(SUM(Buchungen.Betrag)/100.,2) AS SummeA '
    select_stmt += 'FROM Vertraege, Buchungen '
    select_stmt += 'WHERE Vertraege.Id = Buchungen.VertragsId '
    select_stmt += 'GROUP BY Vertraege.Id '
    select_stmt += 'ORDER BY Vertraege.Id '
    select_stmt += ') '
    select_stmt += 'WHERE EXISTS '
    select_stmt += '( '
    select_stmt += 'SELECT KennungB, SummeB '
    select_stmt += 'FROM '
    select_stmt += '( '
    select_stmt += 'SELECT DKBuchungen.DkNummer AS KennungB, ROUND(SUM(Betrag),2) AS SummeB '
    select_stmt += 'FROM DKBuchungen '
    select_stmt += 'WHERE KennungA = KennungB '
    select_stmt += 'GROUP BY KennungB '
    select_stmt += 'HAVING SummeB <> SummeA'
    select_stmt += ') '
    select_stmt += ') '
    stmt.execute(select_stmt)
    vertraege = stmt.fetchall()
    for row in vertraege:
        print row
    print "Anzahl: ", len(vertraege)

    select_stmt = '';
    select_stmt += 'SELECT KennungB, SummeB '
    select_stmt += 'FROM '
    select_stmt += '( '
    select_stmt += 'SELECT DKBuchungen.DkNummer AS KennungB, ROUND(SUM(Betrag),2) AS SummeB '
    select_stmt += 'FROM DKBuchungen '
    select_stmt += 'GROUP BY KennungB '
    select_stmt += 'ORDER BY KennungB '
    select_stmt += ') '
    select_stmt += 'WHERE EXISTS '
    select_stmt += '( '
    select_stmt += 'SELECT KennungA, SummeA '
    select_stmt += 'FROM '
    select_stmt += '( '
    select_stmt += 'SELECT  Vertraege.Kennung AS KennungA, ROUND(SUM(Buchungen.Betrag)/100.,2) AS SummeA '
    select_stmt += 'FROM Vertraege, Buchungen '
    select_stmt += 'WHERE Vertraege.Id = Buchungen.VertragsId '
    select_stmt += 'AND KennungA = KennungB '
    select_stmt += 'GROUP BY Vertraege.Id '
    select_stmt += 'HAVING SummeA <> SummeB'
    select_stmt += ') '
    select_stmt += ') '
    stmt.execute(select_stmt)
    vertraege = stmt.fetchall()
    for row in vertraege:
        print row
    print "Anzahl: ", len(vertraege)

    print "Vertraege mit abweichenden aufsummierten Zinsen:"

    # DKV2: vStat_aktiverVertraege_thesa

    # CREATE VIEW vStat_aktiverVertraege_thesa AS SELECT *, ROUND(100* Jahreszins/Wert,6) as gewMittel 
    # FROM (SELECT count(*) as Anzahl, SUM(Wert) as Wert, SUM(ROUND(Zinssatz *Wert /100,2)) AS Jahreszins,
    # ROUND(AVG(Zinssatz),4) as mittlereRate FROM vVertraege_aktiv WHERE thesa)

    # CREATE VIEW vVertraege_aktiv AS SELECT id ,Kreditorin,Vertragskennung,Zinssatz,Wert,Aktivierungsdatum,Kuendigungsfrist
    # ,Vertragsende,thesa,KreditorId FROM vVertraege_aktiv_detail

    # CREATE VIEW vVertraege_aktiv_detail AS SELECT Vertraege.id AS id, Vertraege.Kennung AS Vertragskennung
    # , Vertraege.ZSatz /100. AS Zinssatz, SUM(Buchungen.Betrag) /100. AS Wert, MIN(Buchungen.Datum)  AS Aktivierungsdatum
    # , Vertraege.Kfrist AS Kuendigungsfrist, Vertraege.LaufzeitEnde  AS Vertragsende, Vertraege.thesaurierend AS thesa
    # , Kreditoren.Nachname || ', ' || Kreditoren.Vorname AS Kreditorin, Kreditoren.id AS KreditorId,Kreditoren.Nachname AS Nachname
    # , Kreditoren.Vorname AS Vorname,Kreditoren.Strasse AS Strasse
    # , Kreditoren.Plz AS Plz, Kreditoren.Stadt AS Stadt, Kreditoren.Email AS Email, Kreditoren.IBAN AS Iban, Kreditoren.BIC AS Bic
    # FROM Vertraege INNER JOIN Buchungen  ON Buchungen.VertragsId = Vertraege.id 
    # INNER JOIN Kreditoren ON Kreditoren.id = Vertraege.KreditorId GROUP BY Vertraege.id

    select_stmt = ''
    select_stmt += 'SELECT KennungA, WertA, ZinssatzA, ZinsenA '
    select_stmt += 'FROM '
    select_stmt += '( '
    select_stmt += 'SELECT Vertraege.Kennung AS KennungA, '
    # select_stmt += 'Vertraege.ZSatz AS ZSatz, '
    select_stmt += 'Vertraege.ZSatz /100. AS ZinssatzA, '
    select_stmt += '(SUM(Buchungen.Betrag) /100.) AS WertA, '
    select_stmt += 'ROUND( ((( Vertraege.ZSatz /100.) * (SUM(Buchungen.Betrag) /100.)) /100), 2) AS ZinsenA '
    # select_stmt += 'SUM((( ROUND((Vertraege.ZSatz /100.),2) * ((Buchungen.Betrag) /100. )) / 100.),2) AS ZinsenA '
    # select_stmt += 'ROUND((( ROUND((Vertraege.ZSatz /100.),2) * (SUM(Buchungen.Betrag) /100. )) / 100.),2) AS ZinsenA '
    # select_stmt += 'SUM((( ROUND((Vertraege.ZSatz /100.),2) * ((Buchungen.Betrag) /100. )) / 100.),2) AS ZinsenA '
    select_stmt += 'FROM Vertraege '
    select_stmt += 'INNER JOIN Buchungen ON Buchungen.VertragsId = Vertraege.id '
    select_stmt += 'INNER JOIN Kreditoren ON Kreditoren.id = Vertraege.KreditorId GROUP BY Vertraege.id'
    select_stmt += ') '
    select_stmt += 'WHERE EXISTS '
    select_stmt += '( '
    select_stmt += 'SELECT KennungB, WertB, ZinssatzB, ZinsenB '
    select_stmt += 'FROM '
    select_stmt += '( '
    select_stmt += 'SELECT DKBuchungen.DkNummer AS KennungB, Zinssatz AS ZinssatzB, SUM(Betrag) AS WertB, ROUND(SUM( (Betrag * Zinssatz) / 100.0 ), 2) AS ZinsenB '
    select_stmt += 'FROM DKBuchungen '
    select_stmt += 'WHERE KennungA = KennungB '
    select_stmt += 'GROUP BY KennungB '
    select_stmt += 'HAVING (ROUND (SUM( (Betrag * Zinssatz) / 100.0 ), 2)) <> ZinsenA'
    select_stmt += ') '
    select_stmt += ') '
    stmt.execute(select_stmt)
    vertraege = stmt.fetchall()
    for row in vertraege:
        print row
    print "Anzahl: ", len(vertraege)

    if True:

        print "abgelaufene Vertraege inkl. Buchungen in VertraegeEx bzw. BuchungenEx"

        insert_stmt = 'INSERT INTO ExVertraege SELECT Vertraege.* '
        insert_stmt += 'FROM Vertraege, DkBuchungen '
        insert_stmt += 'WHERE Vertraege.Id = DkBuchungen.Buchungid '
        insert_stmt += 'AND Rueckzahlung <> ""'
        stmt.execute(insert_stmt);
        print "Anzahl ExVertraege: ", stmt.rowcount
        conn.commit()

        update_stmt = 'UPDATE ExVertraege SET LaufzeitEnde = '
        update_stmt += '('
        update_stmt += 'SELECT ("20" || substr(Rueckzahlung,7,2) || "-" || substr(Rueckzahlung,4,2) || "-" || substr(Rueckzahlung,1,2)) '
        update_stmt += 'FROM DKBuchungen b '
        update_stmt += 'WHERE b.DkNummer = ExVertraege.Kennung '
        update_stmt += ')'
        # print update_stmt
        stmt.execute(update_stmt);
        print "Anzahl ExVertraege aktualisiert: ", stmt.rowcount
        conn.commit()

        insert_stmt = 'INSERT INTO ExBuchungen SELECT Buchungen.* '
        insert_stmt += 'FROM ExVertraege, Buchungen '
        insert_stmt += 'WHERE ExVertraege.Id = Buchungen.Vertragsid '
        stmt.execute(insert_stmt);
        print "Anzahl ExBuchungen: ", stmt.rowcount
        conn.commit()

        # DELETE FROM Buchungen
        stmt.execute('DELETE FROM Buchungen WHERE EXISTS (SELECT * FROM ExBuchungen WHERE Buchungen.Id = ExBuchungen.Id);')
        print "Anzahl Buchungen gelöscht: ", stmt.rowcount
        conn.commit()
        stmt.execute('DELETE FROM Vertraege WHERE EXISTS (SELECT * FROM ExVertraege WHERE ExVertraege.Id = Vertraege.Id);')
        print "Anzahl Vertraege gelöscht: ", stmt.rowcount
        conn.commit()
        
        print "Vergleiche DkVerwaltungQt <-> DKV2"

        print "Anzahl Vertraege:"
        select_stmt = 'SELECT COUNT(*) FROM Vertraege'
        stmt.execute(select_stmt)
        print stmt.fetchone()[0]
        print "Anzahl ExVertraege:"
        select_stmt = 'SELECT COUNT(*) FROM ExVertraege'
        stmt.execute(select_stmt)
        print stmt.fetchone()[0]
        print "Anzahl Buchungen:"
        select_stmt = 'SELECT COUNT(*) FROM Buchungen'
        stmt.execute(select_stmt)
        print stmt.fetchone()[0]
        print "Anzahl ExBuchungen:"
        select_stmt = 'SELECT COUNT(*) FROM ExBuchungen'
        stmt.execute(select_stmt)
        print stmt.fetchone()[0]

    print "TODO: Update TimeStamps\n"
    if dropUnusedCsvTables:
        stmt.execute('DROP TABLE DKVerwaltungORG;')
        stmt.execute('DROP TABLE DKVerwaltung;')

    if dropUnusedDkVerwaltungQtTables:
        stmt.execute('DROP TABLE DkPersonen;')
        stmt.execute('DROP TABLE DkBuchungen;')
        stmt.execute('DROP TABLE DKZinssaetze;')


except SystemExit:
    pass
except:
    print >> sys.stderr, "Unexpected error:", sys.exc_info()[0]
    print >> sys.stderr, sys.exc_info()
    print >> sys.stderr, traceback.print_exc()
    sys.exit(99)

sys.exit(0)
