#!/usr/bin/python
# coding: utf8

# -*- coding: utf-8 -*-

#
# Das Script importiert die Daten der Datenbank aus DKV2 nach DkVerwaltungQt.
# 
# Aufruf: ./importDKV2IntoDkVerwaltungQt.py <DKV2-db3-file> <DkVerwaltungQt-db3-file>
#

import os
import sys
import traceback
import sqlite3
# print sqlite3.version
# print sys.argv
#print len(sys. argv)
if len(sys. argv) < 2:
    print "Aufruf: ./" + os.path.basename(__file__) + " <DKV2-db3-file> <DkVerwaltungQt-db3-file> "
    sys.exit(1)

DKV2_db3_file=sys.argv[1]
if not os.path.isfile(DKV2_db3_file):
    print DKV2_db3_file + "existiert nicht."
    sys.exit(2)

sameDB = False
if len(sys. argv) == 2:
    sameDB = True
else:
    DkVerwaltungQt_db3_file=sys.argv[2]
    if not os.path.isfile(DkVerwaltungQt_db3_file):
        print DkVerwaltungQt_db3_file + "existiert nicht."
        sys.exit(3)
    if DkVerwaltungQt_db3_file.lower() == DKV2_db3_file.lower():
        sameDB = True

try:
    conn = None
    stmt = None
    if sameDB:
        conn = sqlite3.connect(DKV2_db3_file)
        stmt = conn.cursor()
        db_from = ""
        db_to = ""
    else:
        conn = sqlite3.connect('')
        stmt = conn.cursor()
        stmt.execute('ATTACH DATABASE "{0}" AS db_from'.format(DKV2_db3_file))
        stmt.execute('ATTACH DATABASE "{0}" AS db_to'.format(DkVerwaltungQt_db3_file))
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
    insert_stmt += "SELECT Id AS PersonId, Vorname AS Vorname, Nachname AS Name, '' AS Anrede, Strasse AS Straße, Plz AS PLZ, Stadt AS Ort, EMail AS Email FROM " + db_from + "Kreditoren"
    # print insert_stmt
    stmt.execute(insert_stmt);
    print "Anzahl: ", stmt.rowcount
    conn.commit()

    print "DkBuchungen hinzufügen"
    insert_stmt = "INSERT INTO " + db_to + "DkBuchungen "
    insert_stmt += "SELECT b.Id AS BuchungId, v.KreditorId AS PersonId, ";
    insert_stmt += "( substr(b.Datum,9,2) || '.' || substr(b.Datum,6,2) || '.' || substr(b.Datum,3,2))  AS Datum, ";
    insert_stmt += "b.VertragsId AS DKNr, v.Kennung AS DKNummer, ";
    # TODO:
    # wird auch beim Import nach DKV2 nicht berücksichtigt.
    # müsste aus exBuchunen, exVertraege kommen
    # 2013-12-04 -> 26.01.18
    # insert_stmt += "'' AS Rueckzahlung, ";
    insert_stmt += "CASE WHEN (b.Betrag < 0) THEN ( substr(b.Datum,9,2) || '.' || substr(b.Datum,6,2) || '.' || substr(b.Datum,3,2)) ELSE '' END AS Rueckzahlung, ";
    insert_stmt += "CASE WHEN (v.LaufzeitEnde != '9999-12-31') THEN ( substr(v.LaufzeitEnde,9,2) || '.' || substr(v.LaufzeitEnde,6,2) || '.' || substr(v.LaufzeitEnde,3,2)) END AS vorgemerkt, ";
    # insert_stmt += "CASE WHEN (Buchungsart = 2) THEN ( -1 * (b.Betrag / 100.0) ) ELSE (b.Betrag / 100.0) END AS Betrag, ";
    insert_stmt += "(b.Betrag / 100.0) AS Betrag, ";
    insert_stmt += "(v.ZSatz / 100.0) AS Zinssatz, ";
    # TODO:
    # 23.01.14 F13T-2013-004
    # [auto] DK-Nr. 004 neu angelegt. Betrag: 10.000,00 €.
    # 30.06.17 [auto] DK-Nr. 004 gekündigt. Betrag: 10.000,00 €.
    insert_stmt += "'' AS Bemerkung, ";
    # insert_stmt += "c.Anfangsdatum AS Anfangsdatum, ";
    insert_stmt += "( substr(c.Anfangsdatum,9,2) || '.' || substr(c.Anfangsdatum,6,2) || '.' || substr(c.Anfangsdatum,3,2)) AS Anfangsdatum, ";
    insert_stmt += "(v.Betrag / 100.0) AS AnfangsBetrag ";
    # insert_stmt += "FROM Buchungen b, Vertraege v, ";
    insert_stmt += "FROM "
    insert_stmt += "(SELECT * FROM " + db_from + "Buchungen UNION SELECT * FROM " + db_from + "ExBuchungen) b, "
    insert_stmt += "(SELECT * FROM " + db_from + "Vertraege UNION SELECT * FROM " + db_from + "ExVertraege) v, "
    insert_stmt += "(SELECT  MIN(x.Datum) AS Anfangsdatum,  x.VertragsId AS VertragsId FROM (SELECT * FROM " + db_from + "Buchungen UNION SELECT * FROM " + db_from + "ExBuchungen) x GROUP BY x.VertragsId) AS c "
    insert_stmt += "WHERE b.VertragsId = v.Id ";
    insert_stmt += "AND c.VertragsId = v.Id; ";
    stmt.execute(insert_stmt);
    print "Anzahl: ", stmt.rowcount
    conn.commit()

    print "DKZinssaetze hinzufügen"
    insert_stmt = "INSERT INTO " + db_to + "DKZinssaetze "
    insert_stmt += "SELECT ZSatz / 100.0 AS Zinssatz, NULL AS Beschreibung FROM " + db_from + "Vertraege GROUP BY ZSatz ORDER BY ZSatz;"
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
