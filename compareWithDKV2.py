#!/usr/bin/python
# coding: utf8

# -*- coding: utf-8 -*-

#
# Das Script vergleicht die Daten der Datenbank von DkVerwaltungQt mit DKV2.
# 
# Aufruf: ./compareWithDKV2.py <DkVerwaltungQt-db3-file> <DKV2-db3-file>
#

import os
import sys
import sqlite3
# print sqlite3.version
# print sys.argv
if len(sys. argv) < 3:
    print "Aufruf: ./compareWithDKV2.py <DkVerwaltungQt-db3-file> <DKV2-db3-file>"
    exit(1)

DkVerwaltungQt_db3_file=sys.argv[1]
DKV2_db3_file=sys.argv[2]
if not os.path.isfile(DkVerwaltungQt_db3_file):
    print DkVerwaltungQt_db3_file + "existiert nicht."
    exit(2)
if not os.path.isfile(DKV2_db3_file):
    print DKV2_db3_file + "existiert nicht."
    exit(3)

try:
    conn = sqlite3.connect('')
    stmt = conn.cursor()
    stmt.execute('ATTACH DATABASE "{0}" AS db_from'.format(DkVerwaltungQt_db3_file)) 
    stmt.execute('ATTACH DATABASE "{0}" AS db_to'.format(DKV2_db3_file))
    conn.commit()
    # DkPersonen und Kreditoren
    print "Anzahl DkPersonen:"
    select_stmt = 'SELECT COUNT(*) FROM db_from.DkPersonen'
    stmt.execute(select_stmt)
    print stmt.fetchone()[0]
    print "Anzahl Kreditoren:"
    select_stmt = 'SELECT COUNT(*) FROM db_to.Kreditoren'
    stmt.execute(select_stmt)
    print stmt.fetchone()[0]
    # Summierte DkBuchungen und Vertraege
    print "Anzahl DKBuchungen (summiert):"
    select_stmt = 'SELECT COUNT(DISTINCT DkNummer) FROM db_from.DKBuchungen WHERE Anfangsdatum <> "" AND DkNummer <> "Stammkapital"'
    stmt.execute(select_stmt)
    print stmt.fetchone()[0]
    print "Anzahl Vertraege:"
    select_stmt = 'SELECT COUNT(*) FROM db_to.Vertraege'
    stmt.execute(select_stmt)
    print stmt.fetchone()[0]
    
    print "Vertraege, die nicht übernommen wurden:"
    select_stmt = 'SELECT * FROM db_from.DKBuchungen '
    select_stmt += 'WHERE NOT EXISTS'
    select_stmt += '('
    select_stmt += 'SELECT * FROM db_to.Vertraege WHERE db_from.DKBuchungen.DkNummer = db_to.Vertraege.Kennung'    
    select_stmt += ')'
    stmt.execute(select_stmt)
    buchungen = stmt.fetchall()
    for row in buchungen:
        print row[4]
   
    print "Vertraege mit abweichendem aufsummierten Betrag:"
    
    select_stmt = 'SELECT COUNT(*) FROM db_to.Vertraege WHERE db_to.Vertraege.Wert <> '
    select_stmt += '('
    select_stmt += 'SELECT SUM(Betrag) FROM db_from.DKBuchungen WHERE db_to.Vertraege.Kennung = db_from.DKBuchungen.DkNummer AND Rueckzahlung = "" GROUP BY db_from.DKBuchungen.DkNummer '
    select_stmt += ')'
    stmt.execute(select_stmt)
    print stmt.fetchone()[0]

    select_stmt = 'SELECT * FROM db_to.Vertraege WHERE db_to.Vertraege.Wert <> '
    select_stmt += '('
    select_stmt += 'SELECT SUM(Betrag) FROM db_from.DKBuchungen WHERE db_to.Vertraege.Kennung = db_from.DKBuchungen.DkNummer AND Rueckzahlung = "" GROUP BY db_from.DKBuchungen.DkNummer '
    select_stmt += ')'
    stmt.execute(select_stmt)
    vertraege = stmt.fetchall()
    for row in vertraege:
        print row

    print "Summe DkBuchungen ohne Rückzahlung:"
    select_stmt = 'SELECT SUM(db_from.DKBuchungen.Betrag) FROM db_from.DKBuchungen WHERE Rueckzahlung = ""'
    stmt.execute(select_stmt)
    print stmt.fetchone()[0]

    print "Summe DkBuchungen mit Rückzahlung:"
    select_stmt = 'SELECT SUM(db_from.DKBuchungen.Betrag) FROM db_from.DKBuchungen WHERE Rueckzahlung <> "" '
    select_stmt += 'AND (Anfangsdatum IS NULL OR Anfangsdatum = "") '
    select_stmt += 'AND Betrag < 0 '
    select_stmt += 'AND (substr(Rueckzahlung ,7,2) = "19")'
    stmt.execute(select_stmt)
    print stmt.fetchone()[0]

    print "Summe Vertraege:"
    select_stmt = 'SELECT SUM(db_to.Vertraege.Wert) FROM db_to.Vertraege'
    stmt.execute(select_stmt)
    print stmt.fetchone()[0]

    print "Summe DkBuchungen Zinsen Rückzahlungen Betrag:"
    select_stmt = 'SELECT SUM(db_from.DKBuchungen.Betrag) FROM db_from.DKBuchungen WHERE Rueckzahlung <> ""'
    stmt.execute(select_stmt)
    print stmt.fetchone()[0]

    print "Summe DkBuchungen Rückzahlungen:"
    select_stmt = 'SELECT * FROM db_from.DKBuchungen a WHERE EXISTS '
    select_stmt += '('
    select_stmt += 'SELECT * FROM db_from.DKBuchungen b WHERE a.DkNummer = b.DkNummer AND Rueckzahlung <> ""'
    select_stmt += ')'
    stmt.execute(select_stmt)
    buchungen = stmt.fetchall()
    # for row in buchungen:
    #     # print row[4]
    #     print row
    
    stmt.execute(select_stmt)
    print stmt.fetchone()[0]

    print "Summe DkBuchungen Betrag:"
    select_stmt = 'SELECT SUM(db_from.DKBuchungen.Betrag) FROM db_from.DKBuchungen'
    stmt.execute(select_stmt)
    print stmt.fetchone()[0]

    print "Summe DkBuchungen AnfangsBetrag:"
    select_stmt = 'SELECT SUM(db_from.DKBuchungen.AnfangsBetrag) FROM db_from.DKBuchungen '
    select_stmt += 'WHERE Anfangsdatum <> "" AND DkNummer <> "Stammkapital" AND Rueckzahlung = ""'
    stmt.execute(select_stmt)
    print stmt.fetchone()[0]

except:
    print "Unexpected error:", sys.exc_info()[0]
    print sys.exc_info()

exit(0)
