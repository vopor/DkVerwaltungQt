#!/usr/bin/python
# coding: utf8

# -*- coding: utf-8 -*-

#
# Das Script importiert die Daten der Datenbank aus DKV2 nach DkVerwaltungQt.
# 
# Aufruf: ./importDkVerwaltungQtIntoDkVerwaltungQt.py <DkVerwaltungQt-db3-file-from> <DkVerwaltungQt-db3-file-to>
#

import os
import sys
import traceback
import sqlite3
# print sqlite3.version
# print sys.argv
#print len(sys. argv)
if len(sys. argv) < 3:
    print >> sys.stderr, "Aufruf: ./" + os.path.basename(__file__) + " <DkVerwaltungQt-db3-file-from> <DkVerwaltungQt-db3-file-to> "
    sys.exit(1)

DkVerwaltungQt_db3_file_from=sys.argv[1]
if not os.path.isfile(DkVerwaltungQt_db3_file_from):
    print >> sys.stderr, DkVerwaltungQt_db3_file_from + " existiert nicht."
    sys.exit(2)

DkVerwaltungQt_db3_file_to=sys.argv[2]
if not os.path.isfile(DkVerwaltungQt_db3_file_to):
    print >> sys.stderr, DkVerwaltungQt_db3_file_to + " existiert nicht."
    sys.exit(3)
    if DkVerwaltungQt_db3_file_to.lower() == DkVerwaltungQt_db3_file_from.lower():
        print >> sys.stderr, "Es darf sich nicht um dieselbe Datenbank handeln"
        sys.exit(4)

try:
    conn = None
    stmt = None
    conn = sqlite3.connect('')
    stmt = conn.cursor()
    stmt.execute('ATTACH DATABASE "{0}" AS db_from'.format(DkVerwaltungQt_db3_file_from))
    stmt.execute('ATTACH DATABASE "{0}" AS db_to'.format(DkVerwaltungQt_db3_file_to))
    db_from = "db_from."
    db_to = "db_to."
    
    stmt.execute("CREATE TABLE IF NOT EXISTS " + db_to + "DKPersonen (PersonId INTEGER PRIMARY KEY AUTOINCREMENT, Vorname TEXT, Name TEXT, Anrede TEXT, Straße TEXT, PLZ TEXT, Ort TEXT, Email TEXT)")
    stmt.execute("CREATE TABLE IF NOT EXISTS " + db_to + "DKBuchungen (BuchungId INTEGER PRIMARY KEY AUTOINCREMENT, PersonId INTEGER, Datum TEXT, DKNr TEXT, DKNummer TEXT, Rueckzahlung TEXT, vorgemerkt TEXT, Betrag REAL, Zinssatz REAL, Bemerkung TEXT, Anfangsdatum TEXT, AnfangsBetrag REAL, FOREIGN KEY(PersonId) REFERENCES DKPerson(PersonId))")
    stmt.execute("CREATE TABLE IF NOT EXISTS " + db_to + "DKZinssaetze (Zinssatz REAL PRIMARY KEY, Beschreibung TEXT)")
    conn.commit()

    stmt.execute("DELETE FROM " + db_to + "DkBuchungen")
    stmt.execute("DELETE FROM " + db_to + "DkPersonen")
    stmt.execute("DELETE FROM " + db_to + "DkZinssaetze")
    conn.commit()

    print "DkPersonen hinzufügen"
    insert_stmt = "INSERT INTO " + db_to + "DkPersonen "
    insert_stmt += "SELECT * FROM " + db_from + "DkPersonen"
    # print insert_stmt
    stmt.execute(insert_stmt);
    print "Anzahl: ", stmt.rowcount
    conn.commit()

    print "DkBuchungen hinzufügen"
    insert_stmt = "INSERT INTO " + db_to + "DkBuchungen "
    insert_stmt += "SELECT * FROM " + db_from + "DkBuchungen "
    stmt.execute(insert_stmt);
    print "Anzahl: ", stmt.rowcount
    conn.commit()

    print "DKZinssaetze hinzufügen"
    insert_stmt = "INSERT INTO " + db_to + "DKZinssaetze "
    insert_stmt += "SELECT * FROM " + db_from + "DKZinssaetze "
    stmt.execute(insert_stmt);
    print "Anzahl: ", stmt.rowcount
    conn.commit()

    sys.exit(0)

except SystemExit:
    pass
except:
    print >> sys.stderr, "Unexpected error:", sys.exc_info()[0]
    print >> sys.stderr, sys.exc_info()
    print >> sys.stderr, traceback.print_exc()
    sys.exit(99)

sys.exit(0)
