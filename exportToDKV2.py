#!/usr/bin/python
# coding: utf8

# -*- coding: utf-8 -*-

#
# Das Script exportiert die Daten der Datenbank aus DkVerwaltungQt nach DKV2.
# 
# Aufruf: ./exportToDKV2.py <DkVerwaltungQt-db3-file> <DKV2-db3-file>
#

import os
import sys
import traceback
import sqlite3
# print sqlite3.version
# print sys.argv
#print len(sys. argv)
if len(sys. argv) < 4:
    print "Aufruf: ./exportToDKV2.py <DkVerwaltungQt-db3-file> <DKV2-db3-file> <Year2>"
    exit(1)

DkVerwaltungQt_db3_file=sys.argv[1]
DKV2_db3_file=sys.argv[2]
if not os.path.isfile(DkVerwaltungQt_db3_file):
    print DkVerwaltungQt_db3_file + "existiert nicht."
    exit(2)
if not os.path.isfile(DKV2_db3_file):
    print DKV2_db3_file + "existiert nicht."
    exit(3)

Year2 = sys.argv[3]

try:
    conn = sqlite3.connect('')
    stmt = conn.cursor()
    stmt.execute('ATTACH DATABASE "{0}" AS db_from'.format(DkVerwaltungQt_db3_file)) 
    stmt.execute('ATTACH DATABASE "{0}" AS db_to'.format(DKV2_db3_file))
    stmt.execute('DELETE FROM db_to.Buchungen')
    stmt.execute('DELETE FROM db_to.Vertraege')
    stmt.execute('DELETE FROM db_to.Kreditoren')
    stmt.execute('DELETE FROM db_to.Zinssaetze WHERE Zinssatz > 1.99')
    conn.commit()
    # stmt.execute('DELETE FROM db_from.DKZinssaetze WHERE db_from.DKZinssaetze.Zinssatz LIKE "%Zinssatz%"')
    # conn.commit()
    # Zinssaetze
    insert_stmt = 'INSERT INTO db_to.Zinssaetze (Zinssatz, Bemerkung) '
    insert_stmt += 'SELECT db_from.DKZinssaetze.Zinssatz, "Unser Investor" FROM db_from.DKZinssaetze WHERE NOT EXISTS '
    insert_stmt += '('
    insert_stmt += 'SELECT db_to.Zinssaetze.Zinssatz FROM db_to.Zinssaetze WHERE db_to.Zinssaetze.Zinssatz = db_from.DKZinssaetze.Zinssatz '
    insert_stmt += ')'
    stmt.execute(insert_stmt);
    conn.commit()
    # Kreditoren
    insert_stmt = 'INSERT INTO db_to.Kreditoren (id, Vorname, Nachname, Strasse, Plz, Stadt, EMail) '
    insert_stmt += 'SELECT Personid, Vorname, Name, Straße, PLZ, Ort, Email FROM db_from.DKPersonen'
    stmt.execute(insert_stmt);
    conn.commit()
    # Vertraege
    insert_stmt = 'INSERT INTO db_to.Vertraege (id, KreditorId, Kennung, Betrag, Wert, ZSatz, thesaurierend, Vertragsdatum, aktiv, LaufzeitEnde, LetzteZinsberechnung, Kfrist) '
    insert_stmt += 'SELECT BuchungId, PersonId, DKNummer, AnfangsBetrag, Betrag, db_to.Zinssaetze.id, 1, '
    insert_stmt += '"20" || substr(Anfangsdatum,7,2) || "-" || substr(Anfangsdatum,4,2) || "-" || substr(Anfangsdatum,1,2), 1, '
    insert_stmt += 'CASE WHEN (vorgemerkt != "") THEN ("20" || substr(vorgemerkt,7,2) || "-" || substr(vorgemerkt,4,2) || "-" || substr(vorgemerkt,1,2)) ELSE "3000-12-31" END, NULL, '
    insert_stmt += 'CASE WHEN (vorgemerkt != "") THEN -1 ELSE 6 END '
    insert_stmt += 'FROM db_from.DKBuchungen, db_to.Zinssaetze '
    insert_stmt += 'WHERE db_to.Zinssaetze.Zinssatz = db_from.DKBuchungen.Zinssatz AND Anfangsdatum <> "" AND DkNummer <> "Stammkapital" '
    # insert_stmt += 'AND Rueckzahlung = ""'
    stmt.execute(insert_stmt);
    # Buchungen aufsummieren
    update_stmt = 'UPDATE db_to.Vertraege SET Wert='
    update_stmt += '('
    update_stmt += 'SELECT SUM(Betrag) FROM db_from.DKBuchungen WHERE db_to.Vertraege.Kennung = db_from.DKBuchungen.DkNummer '
    update_stmt += 'AND db_from.DKBuchungen.DkNummer <> "Stammkapital"'
    update_stmt += 'GROUP BY db_from.DKBuchungen.DkNummer'
    update_stmt += ')'
    stmt.execute(update_stmt);
    # Rueckzahlung berücksichtigen
    update_stmt = 'UPDATE db_to.Vertraege SET Wert='
    update_stmt += '('
    update_stmt += 'SELECT (Betrag * -1) FROM db_from.DKBuchungen WHERE db_to.Vertraege.Kennung = db_from.DKBuchungen.DkNummer '
    update_stmt += 'AND (Anfangsdatum IS NULL OR Anfangsdatum = "") '
    update_stmt += 'AND Betrag < 0 '
    update_stmt += 'AND (substr(Rueckzahlung ,7,2) = "' + Year2 + '")'
    update_stmt += 'AND DkNummer <> "Stammkapital"'
    update_stmt += ')'
    # update_stmt += 'round((Betrag + (-1*Wert)),2) '
    # # update_stmt += '(Betrag + (-1.0*CAST( Wert AS FLOAT)))
    # # update_stmt += '-1.0*Wert '
    # # update_stmt += 'WHERE Wert < 0'
    update_stmt += 'WHERE EXISTS '
    update_stmt += '('
    update_stmt += 'SELECT DkNummer FROM db_from.DKBuchungen WHERE db_to.Vertraege.Kennung = db_from.DKBuchungen.DkNummer '
    update_stmt += 'AND db_from.DKBuchungen.Rueckzahlung <> "" '    
    update_stmt += ') '
    stmt.execute(update_stmt);
    update_stmt = 'UPDATE db_to.Vertraege SET Wert=0 '
    update_stmt += 'WHERE EXISTS '
    update_stmt += '('
    update_stmt += 'SELECT DkNummer FROM db_from.DKBuchungen WHERE db_to.Vertraege.Kennung = db_from.DKBuchungen.DkNummer '
    update_stmt += 'AND db_from.DKBuchungen.Rueckzahlung <> "" AND substr(db_from.DKBuchungen.Rueckzahlung,7,2) < "' + Year2 + '" '    
    update_stmt += ') '
    stmt.execute(update_stmt);    

    # LetzteZinsberechnung auf db_from.DKBuchungen.Datum ('2019-01-01'), wenn db_from.DKBuchungen.Datum = '01.01.19'
    update_stmt = 'UPDATE db_to.Vertraege SET LetzteZinsberechnung='
    update_stmt += '('
    update_stmt += 'SELECT ("20" || substr(Datum,7,2) || "-" || substr(Datum,4,2) || "-" || substr(Datum,1,2)) '
    update_stmt += 'FROM db_from.DKBuchungen WHERE db_to.Vertraege.Kennung = db_from.DKBuchungen.DkNummer '
    update_stmt += 'AND db_from.DKBuchungen.Datum = "01.01.' + Year2 + '"'
    update_stmt += ') '
    # update_stmt += 'WHERE db_to.Vertraege.Kennung IN (SELECT db_from.DKBuchungen.DkNummer FROM db_from.DKBuchungen WHERE db_from.DKBuchungen.Datum = "01.01.19")'
    stmt.execute(update_stmt);
    # db_to.Vertraege.Kennung.LetzteZinsberechnung auf db_from.DKBuchungen.Rueckzahlung ('2019-01-01'), wenn db_from.DKBuchungen.Rueckzahlung <> ''
    update_stmt = 'UPDATE db_to.Vertraege '
    update_stmt += 'SET LetzteZinsberechnung='
    update_stmt += '('
    update_stmt += 'SELECT "20" || substr(Rueckzahlung,7,2) || "-" || substr(Rueckzahlung,4,2) || "-" || substr(Rueckzahlung,1,2) '
    update_stmt += 'FROM db_from.DKBuchungen WHERE db_to.Vertraege.Kennung = db_from.DKBuchungen.DkNummer '
    update_stmt += 'AND db_from.DKBuchungen.Rueckzahlung <> "" '
    # update_stmt += 'AND db_from.DKBuchungen.Rueckzahlung IS NOT NULL'
    update_stmt += ')'
    # update_stmt += 'WHERE db_to.Vertraege.Kennung IN '
    # # AND db_from.DKBuchungen.Rueckzahlung IS NOT NULL
    # update_stmt += '(SELECT db_from.DKBuchungen.DkNummer FROM db_from.DKBuchungen WHERE db_from.DKBuchungen.Rueckzahlung <> "")'
    stmt.execute(update_stmt);
    # db_to.Vertraege.Kennung.LaufzeitEnde auf db_from.DKBuchungen.Rueckzahlung ('2019-01-01'), wenn db_from.DKBuchungen.Rueckzahlung <> ''
    update_stmt = 'UPDATE db_to.Vertraege '
    update_stmt += 'SET Aktiv=0 AND LaufzeitEnde='
    update_stmt += '('
    update_stmt += 'SELECT "20" || substr(Rueckzahlung,7,2) || "-" || substr(Rueckzahlung,4,2) || "-" || substr(Rueckzahlung,1,2) '
    update_stmt += 'FROM db_from.DKBuchungen '
    update_stmt += 'WHERE db_to.Vertraege.Kennung = db_from.DKBuchungen.DkNummer '
    update_stmt += 'AND db_from.DKBuchungen.Rueckzahlung <> "" '
    update_stmt += 'GROUP BY db_from.DKBuchungen.DkNummer'    
    update_stmt += ') '
    update_stmt += 'WHERE db_to.Vertraege.Kennung IN '
    update_stmt += '(SELECT db_from.DKBuchungen.DkNummer FROM db_from.DKBuchungen WHERE db_from.DKBuchungen.Rueckzahlung <> "")'
    # print update_stmt
    stmt.execute(update_stmt);
    conn.commit()
except:
    print "Unexpected error:", sys.exc_info()[0]
    print sys.exc_info()
    print traceback.print_exc()

exit(0)
