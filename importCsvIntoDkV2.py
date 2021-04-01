#!/usr/bin/python
# coding: utf8

# -*- coding: utf-8 -*-

#
# Das Script exportiert die Daten der Datenbank aus DkVerwaltungQt nach DKV2.
# 
# Aufruf: ./importCsvIntoDkVerwaltungQt.py <DkVerwaltung_csv_file> <DkVerwaltungQt-db3-file>
#

import os
import sys
import traceback
# import csv
# from collections import defaultdict
import sqlite3
import subprocess
# import pandas

dropUnusedCsvTables = False
dropUnusedDkVerwaltungQtTables = False

# https://stackoverflow.com/questions/7831371/is-there-a-way-to-get-a-list-of-column-names-in-sqlite
def get_col_names(cursor, tablename):
    """Get column names of a table, given its name and a cursor
       (or connection) to the database.
    """
    reader=cursor.execute("SELECT * FROM {}".format(tablename))
    return [x[0] for x in reader.description] 

print "sqlite3.version=" + sqlite3.version
print "argv=" + str(sys.argv)
print "argc=" + str(len(sys. argv))

if len(sys. argv) < 3:
    print "Aufruf: ./" + os.path.basename(__file__) + " <DkVerwaltung_csv_file> <DkVerwaltungQt-db3-file>"
    sys.exit(1)

DkVerwaltung_csv_file=sys.argv[1]
DkVerwaltungQt_db3_file=sys.argv[2]

if not os.path.isfile(DkVerwaltung_csv_file):
    print DkVerwaltung_csv_file + " existiert nicht."
    sys.exit(2)

# if os.path.isfile(DkVerwaltungQt_db3_file):
#     os.remove(DkVerwaltungQt_db3_file)
if os.path.isfile(DkVerwaltungQt_db3_file):
    print DkVerwaltungQt_db3_file + " existiert."
    msg = 'Soll sie neu erzeugt werden?'
    str = raw_input("%s (J/n) " % msg).lower()
    if not ((str == 'j') or (str == '')):
        sys.exit(3)
    os.remove(DkVerwaltungQt_db3_file)

try:
    
    #
    # sqlite3 DkVerwaltungQt.db3 < CreateDkVerwaltungQt.sql
    #

    # https://stackoverflow.com/questions/2887878/importing-a-csv-file-into-a-sqlite3-database-table-using-python
    # from pathlib import Path
    # db_name = Path('my.db').resolve()
    # csv_file = Path('file.csv').resolve()
    params = ['sqlite3',  DkVerwaltungQt_db3_file.replace('\\','\\\\'),
                         '-cmd',
                         '.mode csv',
                         '.import ' + "'" + DkVerwaltung_csv_file.replace('\\','\\\\')  + "'"  + ' DKVerwaltungOrg']
    # print params
    # cmdstr = 'sqlite3 ' + DkVerwaltungQt_db3_file + ' -cmd ' + ' .mode csv ' + ' .import ' + DkVerwaltung_csv_file.replace('\\','\\\\')  + ' DKVerwaltungOrg'
    # print cmdstr                         
    p = subprocess.Popen(params)
    (output, err) = p.communicate() 
    p_status = p.wait()
    if p_status != 0:
        print "output=" + output
        print "err=" + err
        print "p_status=" + p_status

    conn = sqlite3.connect(DkVerwaltungQt_db3_file)
    stmt = conn.cursor()
    # stmt.execute('ATTACH DATABASE "{0}" AS db'.format(DkVerwaltungQt_db3_file))
    stmt.execute('ALTER TABLE DKVerwaltungOrg RENAME COLUMN "Dk-Nr." to "DKNr";')
    stmt.execute('ALTER TABLE DKVerwaltungOrg RENAME COLUMN "Dk Nummer" to "DKNummer";')
    stmt.execute('ALTER TABLE DKVerwaltungOrg RENAME COLUMN "Rückzahlung" to "Rueckzahlung";')
    stmt.execute('CREATE TABLE DKVerwaltung (Datum TEXT, DKNr TEXT, Vorname TEXT, Name TEXT, Anrede TEXT, DKNummer TEXT, Straße TEXT, PLZ TEXT, Ort TEXT, Email TEXT, Rueckzahlung TEXT, vorgemerkt TEXT, Betrag TEXT, Zinssatz TEXT, Bemerkung TEXT);')

    colnamesFrom = get_col_names(stmt, "DKVerwaltungOrg")
    colnamesTo = get_col_names(stmt, "DKVerwaltung")
    print colnamesFrom
    print colnamesTo
    colnames = []
    for colname in colnamesTo:
        if colname in colnamesFrom:
            colnames.append(colname)
        else:
            print colname 
            colnames.append('DkNr')
    colnamesstring = ','.join(colnames)
    # print colnamesstring
    insert_int_statement = 'INSERT INTO DKVerwaltung SELECT ' + colnamesstring + ' FROM DKVerwaltungOrg WHERE CAST(DkNr AS INTEGER) <> 0 ORDER BY CAST(DkNr AS INTEGER);'
    print insert_int_statement
    stmt.execute(insert_int_statement)
    print "Anzahl: ", stmt.rowcount
    if dropUnusedCsvTables: stmt.execute('DROP TABLE DKVerwaltungORG;')
    stmt.execute('CREATE TABLE DKPersonen (PersonId INTEGER PRIMARY KEY AUTOINCREMENT, Vorname TEXT, Name TEXT, Anrede TEXT, Straße TEXT, PLZ TEXT, Ort TEXT, Email TEXT);')
    stmt.execute('INSERT INTO DKPersonen (Vorname, Name, Anrede, Straße, PLZ, Ort, Email) SELECT Vorname, Name, Anrede, Straße, PLZ, Ort, Email FROM DKVerwaltung WHERE CAST(DkNr AS INTEGER) <> 0;')
    stmt.execute('CREATE TABLE DKBuchungen (BuchungId INTEGER PRIMARY KEY AUTOINCREMENT, PersonId INTEGER, Datum TEXT, DKNr TEXT, DKNummer TEXT, Rueckzahlung TEXT, vorgemerkt TEXT, Betrag REAL, Zinssatz REAL, Bemerkung TEXT, Anfangsdatum TEXT, AnfangsBetrag REAL, FOREIGN KEY(PersonId) REFERENCES DKPerson(PersonId));')
    stmt.execute('INSERT INTO DKBuchungen (PersonId, Datum, DKNr, DKNummer, Rueckzahlung, vorgemerkt, Betrag, Zinssatz, Bemerkung) SELECT ROWID, Datum, DKNr, DKNummer, Rueckzahlung, vorgemerkt, Betrag, Zinssatz, Bemerkung FROM DKVerwaltung WHERE CAST(DkNr AS INTEGER) <> 0;')
    if dropUnusedCsvTables: stmt.execute('DROP TABLE DKVerwaltung;')
    stmt.execute("UPDATE DkBuchungen SET Betrag = replace(replace(replace(Betrag, ' €', ''), '.', ''), ',', '.');")
    stmt.execute("UPDATE DkBuchungen SET Zinssatz = replace(replace(Zinssatz, '%', ''), ',', '.');")
    stmt.execute('CREATE TABLE DKZinssaetze (Zinssatz REAL PRIMARY KEY, Beschreibung TEXT);')
    stmt.execute('INSERT INTO DKZinssaetze (Zinssatz) SELECT Zinssatz FROM DkBuchungen GROUP BY Zinssatz ORDER BY Zinssatz;')
    conn.commit()

    #
    # sqlite3 DkVerwaltungQt.db3 < UpdateDkVerwaltungQt.sql
    #

    #
    # Bei Personen mit gleichem Vornamen und Namen nur den zuletzt erfassten Personen-Datensatz weiterverwenden
    #
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
    # stmt.execute('UPDATE DkBuchungen SET PersonId = (select mpid from (Select t.PersonId AS pid, xx.MaxPersonId as mpid, t.Vorname , t.Name from DkPersonen t inner join ( Select max(PersonId) AS MaxPersonId, Vorname, Name from DkPersonen group by Vorname, Name) xx on t.Vorname = xx.Vorname AND t.Name = xx.Name ORDER BY PersonID) where DkBuchungen.PersonId = pid);')
    print "Anzahl DkBuchungen DkPersonen zugordnet: ", stmt.rowcount
    stmt.execute('DELETE FROM DkPersonen WHERE NOT EXISTS (SELECT * FROM DkBuchungen WHERE DkBuchungen.PersonId = DkPersonen.PersonId);')
    print "Anzahl DkPersonen gelöscht: ", stmt.rowcount
    # stmt.execute('ALTER TABLE DkBuchungen ADD COLUMN Anfangsdatum TEXT;')
    # stmt.execute('ALTER TABLE DkBuchungen ADD COLUMN AnfangsBetrag REAL;')
    stmt.execute("UPDATE DkBuchungen SET Anfangsdatum = substr(Bemerkung,1,8) WHERE instr(Bemerkung, 'neu angelegt') <> 0;")
    print "Anzahl DkBuchungen Anfangsdatum gesetzt: ", stmt.rowcount
    stmt.execute("UPDATE DkBuchungen SET Anfangsbetrag = CAST(replace(replace(substr(Bemerkung, instr(Bemerkung, 'Betrag: ') + 8), '.', ''), ',', '.') AS FLOAT) WHERE instr(Bemerkung, 'neu angelegt') <> 0;")
    print "Anzahl DkBuchungen Anfangsbetrag gesetzt: ", stmt.rowcount
    conn.commit()

    #
    # ./importDkVerwaltungQtIntoDKV2.py
    #
    


except SystemExit:
    pass
except:
    print "Unexpected error:", sys.exc_info()[0]
    print sys.exc_info()
    print traceback.print_exc()

sys.exit(0)
