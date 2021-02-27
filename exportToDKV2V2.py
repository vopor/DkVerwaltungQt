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
if len(sys. argv) < 3:
    print "Aufruf: ./exportToDKV2.py <DkVerwaltungQt-db3-file> <DKV2-db3-file> "
    sys.exit(1)

DkVerwaltungQt_db3_file=sys.argv[1]
DKV2_db3_file=sys.argv[2]
if not os.path.isfile(DkVerwaltungQt_db3_file):
    print DkVerwaltungQt_db3_file + "existiert nicht."
    sys.exit(2)
if not os.path.isfile(DKV2_db3_file):
    print DKV2_db3_file + "existiert nicht."
    sys.exit(3)

try:
    conn = sqlite3.connect('')
    stmt = conn.cursor()
    stmt.execute('ATTACH DATABASE "{0}" AS db_from'.format(DkVerwaltungQt_db3_file)) 
    stmt.execute('ATTACH DATABASE "{0}" AS db_to'.format(DKV2_db3_file))
    stmt.execute('DELETE FROM db_to.Buchungen')
    stmt.execute('DELETE FROM db_to.Vertraege')
    stmt.execute('DELETE FROM db_to.Kreditoren')
    conn.commit()

    due_direkt_nach_jahresabschluss = True
    select_stmt = "SELECT COUNT(*), DkNummer FROM db_from.DkBuchungen WHERE DkNummer <> 'Stammkapital' GROUP BY DkNummer HAVING COUNT(DkNummer) > 1 ORDER BY DkNummer"
    stmt.execute(select_stmt)
    buchungen = stmt.fetchall()
    if len(buchungen) > 0:
        due_direkt_nach_jahresabschluss = False
        print
        print "Es existieren mehrere Buchungen pro Vertrag"
        print
        # print "buchungen", len(buchungen)
        # for row in buchungen:
        #     print row
        # sys.exit(4)
    else:
        print
        print "Datenübernahme direkt nach dem Jahreswechsel"
        print

    # due_direkt_nach_jahresabschluss = False # funktioniert auch
    if due_direkt_nach_jahresabschluss:
        
        print "Kreditoren hinzufügen"
        insert_stmt = 'INSERT INTO db_to.Kreditoren (id, Vorname, Nachname, Strasse, Plz, Stadt, EMail, Anmerkung, IBAN, Bic) '
        insert_stmt += 'SELECT Personid, Vorname, Name, Straße, PLZ, Ort, Email, "", "", "" FROM db_from.DKPersonen'
        stmt.execute(insert_stmt);
        print "Anzahl: ", stmt.rowcount
        conn.commit()
    
        print "Vertraege hinzufügen"
        insert_stmt = 'INSERT INTO db_to.Vertraege (id, KreditorId, Kennung, ZSatz, Betrag, thesaurierend, Vertragsdatum, LaufzeitEnde, Kfrist) '
        insert_stmt += 'SELECT BuchungId, PersonId, DKNummer, db_from.DKBuchungen.Zinssatz*100, AnfangsBetrag*100, 1, '
        insert_stmt += '"20" || substr(Anfangsdatum,7,2) || "-" || substr(Anfangsdatum,4,2) || "-" || substr(Anfangsdatum,1,2), '
        insert_stmt += 'CASE WHEN (vorgemerkt != "") THEN ("20" || substr(vorgemerkt,7,2) || "-" || substr(vorgemerkt,4,2) || "-" || substr(vorgemerkt,1,2)) ELSE "3000-12-31" END, '
        insert_stmt += 'CASE WHEN (vorgemerkt != "") THEN -1 ELSE 6 END '
        insert_stmt += 'FROM db_from.DKBuchungen '
        # insert_stmt += 'WHERE Rueckzahlung = ""'
        stmt.execute(insert_stmt);
        print "Anzahl: ", stmt.rowcount
        conn.commit()

        print "Buchungen hinzufügen"
        insert_stmt = 'INSERT INTO db_to.Buchungen (id, VertragsId, Datum, Buchungsart, Betrag) '
        insert_stmt += 'SELECT a.BuchungId, a.BuchungId, '
        insert_stmt += '("20" || substr(a.Datum,7,2) || "-" || substr(a.Datum,4,2) || "-" || substr(a.Datum,1,2)), '
        insert_stmt += 'CASE WHEN (a.Betrag >= 0) THEN 1 ELSE 2 END, a.Betrag*100 '
        insert_stmt += 'FROM db_from.DKBuchungen a '
        stmt.execute(insert_stmt);
        print "Anzahl: ", stmt.rowcount
        conn.commit()

        sys.exit(0)

    if not due_direkt_nach_jahresabschluss:
        print "Kreditoren hinzufügen"
        insert_stmt = 'INSERT INTO db_to.Kreditoren (id, Vorname, Nachname, Strasse, Plz, Stadt, EMail, Anmerkung, IBAN, Bic) '
        insert_stmt += 'SELECT Personid, Vorname, Name, Straße, PLZ, Ort, Email, "", "", "" FROM db_from.DKPersonen'
        stmt.execute(insert_stmt);
        print "Anzahl: ", stmt.rowcount
        conn.commit()
    
        print "Vertraege hinzufügen"
        insert_stmt = 'INSERT INTO db_to.Vertraege (id, KreditorId, Kennung, ZSatz, Betrag, thesaurierend, Vertragsdatum, LaufzeitEnde, Kfrist) '
        insert_stmt += 'SELECT BuchungId, PersonId, DKNummer, db_from.DKBuchungen.Zinssatz*100, AnfangsBetrag*100, 1, '
        insert_stmt += '"20" || substr(Anfangsdatum,7,2) || "-" || substr(Anfangsdatum,4,2) || "-" || substr(Anfangsdatum,1,2), '
        insert_stmt += 'CASE WHEN (vorgemerkt != "") THEN ("20" || substr(vorgemerkt,7,2) || "-" || substr(vorgemerkt,4,2) || "-" || substr(vorgemerkt,1,2)) ELSE "3000-12-31" END, '
        insert_stmt += 'CASE WHEN (vorgemerkt != "") THEN -1 ELSE 6 END '
        insert_stmt += 'FROM db_from.DKBuchungen '
        insert_stmt += 'WHERE Anfangsdatum <> "" '
        # insert_stmt += 'AND Rueckzahlung = ""'
        insert_stmt += 'GROUP BY DkNummer '
        stmt.execute(insert_stmt);
        print "Anzahl: ", stmt.rowcount
        conn.commit()

        print "Buchungen hinzufügen"
        insert_stmt = 'INSERT INTO db_to.Buchungen (id, VertragsId, Datum, Buchungsart, Betrag) '
        insert_stmt += 'SELECT a.BuchungId, a.BuchungId, '
        insert_stmt += '("20" || substr(a.Datum,7,2) || "-" || substr(a.Datum,4,2) || "-" || substr(a.Datum,1,2)), '
        insert_stmt += 'CASE WHEN (a.Betrag >= 0) THEN 1 ELSE 2 END, a.Betrag*100 '
        insert_stmt += 'FROM db_from.DKBuchungen a '
        stmt.execute(insert_stmt);
        print "Anzahl: ", stmt.rowcount
        conn.commit()
        
        print "VertragsId setzen"
        update_stmt = 'UPDATE db_to.Buchungen SET VertragsId = '
        update_stmt += '('
        update_stmt += 'SELECT c.id FROM db_from.DKBuchungen b, db_to.Vertraege c '
        update_stmt += 'WHERE db_to.Buchungen.id = b.BuchungId '
        update_stmt += 'AND b.DkNummer = c.Kennung '
        update_stmt += ')'

        stmt.execute(update_stmt);
    
        print "Anzahl: ", stmt.rowcount
        # vertraege = stmt.fetchall()
        # print "vertraege", len(vertraege)
        # for row in vertraege:
        #     print row
        conn.commit()

    sys.exit(0)

except SystemExit:
    pass
except:
    print "Unexpected error:", sys.exc_info()[0]
    print sys.exc_info()
    print traceback.print_exc()

sys.exit(0)
