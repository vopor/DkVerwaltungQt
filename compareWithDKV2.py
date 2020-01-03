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
    select_stmt = 'SELECT COUNT(DISTINCT DkNummer) FROM db_from.DKBuchungen WHERE Anfangsdatum <> "" AND DkNummer <> "Stammkapital" AND Rueckzahlung = ""'
    stmt.execute(select_stmt)
    print stmt.fetchone()[0]
    print "Anzahl Vertraege:"
    select_stmt = 'SELECT COUNT(*) FROM db_to.Vertraege'
    stmt.execute(select_stmt)
    print stmt.fetchone()[0]
    
    print "Vertraege, die nicht übernommen wurden:"
    select_stmt = 'SELECT * FROM db_from.DKBuchungen WHERE Rueckzahlung = "" '
    select_stmt += 'AND NOT EXISTS'
    select_stmt += '('
    select_stmt += 'SELECT * FROM db_to.Vertraege WHERE db_from.DKBuchungen.DkNummer = db_to.Vertraege.Kennung'    
    select_stmt += ')'
    stmt.execute(select_stmt)
    buchungen = stmt.fetchall()
    for row in buchungen:
        print row[4]
   
    select_stmt = 'SELECT COUNT(*) FROM db_to.Vertraege WHERE db_to.Vertraege.Wert <> '
    select_stmt += '('
    select_stmt += 'SELECT SUM(Betrag) FROM db_from.DKBuchungen WHERE db_to.Vertraege.Kennung = db_from.DKBuchungen.DkNummer GROUP BY db_from.DKBuchungen.DkNummer '
    select_stmt += ')'
    stmt.execute(select_stmt)
    print stmt.fetchone()[0]

    select_stmt = 'SELECT * FROM db_to.Vertraege WHERE db_to.Vertraege.Wert <> '
    select_stmt += '('
    select_stmt += 'SELECT SUM(Betrag) FROM db_from.DKBuchungen WHERE db_to.Vertraege.Kennung = db_from.DKBuchungen.DkNummer GROUP BY db_from.DKBuchungen.DkNummer '
    select_stmt += ')'
    stmt.execute(select_stmt)
    vertraege = stmt.fetchall()
    for row in vertraege:
        print row

    print "Summe DkBuchungen:"
    select_stmt = 'SELECT SUM(db_from.DKBuchungen.Betrag) FROM db_from.DKBuchungen'
    stmt.execute(select_stmt)
    print stmt.fetchone()[0]

    print "Summe Vertraege:"
    select_stmt = 'SELECT SUM(db_to.Vertraege.Wert) FROM db_to.Vertraege'
    stmt.execute(select_stmt)
    print stmt.fetchone()[0]


    '''
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
    insert_stmt += 'WHERE db_to.Zinssaetze.Zinssatz = db_from.DKBuchungen.Zinssatz AND Anfangsdatum <> "" AND DkNummer <> "Stammkapital" AND Rueckzahlung = ""'
    stmt.execute(insert_stmt);
    # Buchungen aufsummieren
    update_stmt = 'UPDATE db_to.Vertraege SET Wert='
    update_stmt += '('
    update_stmt += 'SELECT SUM(Betrag) FROM db_from.DKBuchungen GROUP BY db_from.DKBuchungen.DkNummer WHERE db_to.Vertraege.Kennung = db_from.DKBuchungen.DkNummer '
    update_stmt += 'AND db_from.DKBuchungen.DkNummer <> "Stammkapital"'
    update_stmt += ')' 
    conn.commit()
    '''
except:
    print "Unexpected error:", sys.exc_info()[0]
    print sys.exc_info()

exit(0)
