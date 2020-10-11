#!/usr/bin/python
# coding: utf8

# -*- coding: utf-8 -*-

#
# Das Script vergleicht die Daten der Datenbank von DkVerwaltungQt mit der Datenbank des Vorjahres
# und gibt die Zinsen für den Steuerberater aus.
# 
# Aufruf: ./compareDKkVerwaltungYears.py <DkVerwaltungQt-db3-file-vorjahr> <DkVerwaltungQt-db3-file>
#

import os
import sys
import sqlite3
from datetime import datetime
from datetime import date

# print sqlite3.version
# print sys.argv
if len(sys. argv) < 3:
    print "Aufruf: ./compareDKkVerwaltungYears.py <DkVerwaltungQt-db3-file-vorjahr> <DkVerwaltungQt-db3-file>"
    exit(1)

DkVerwaltungQt_db3_file_vorjahr=sys.argv[1]
DkVerwaltungQt_db3_file=sys.argv[2]
if not os.path.isfile(DkVerwaltungQt_db3_file_vorjahr):
    print DkVerwaltungQt_db3_file_vorjahr + "existiert nicht."
    exit(2)
if not os.path.isfile(DkVerwaltungQt_db3_file):
    print DkVerwaltungQt_db3_file + "existiert nicht."
    exit(3)

try:
    conn = sqlite3.connect('')
    stmt = conn.cursor()
    stmt.execute('ATTACH DATABASE "{0}" AS db_vorjahr'.format(DkVerwaltungQt_db3_file_vorjahr)) 
    stmt.execute('ATTACH DATABASE "{0}" AS db'.format(DkVerwaltungQt_db3_file))
    conn.commit()

    print "Ausbezahlte Zinsen 2019: "

    print "DkNummer;Betrag;Betrag mit Zinsen;Zinsen"
    select_stmt = "SELECT DkNummer, SUM(Betrag), MAX(Betrag), MIN(Betrag), Count(*) FROM db_vorjahr.DkBuchungen WHERE ((substr(Rueckzahlung ,7,2) || substr(Rueckzahlung ,4,2)|| substr(Rueckzahlung ,1,2)) > '190101') GROUP BY DkNummer ORDER BY DkNummer;"
    stmt.execute(select_stmt)
    buchungen = stmt.fetchall()
    sumzinsen = 0
    for row in buchungen:
        zinsen = 0
        DkNummer = row[0]
        SumBetrag = row[1]
        Betrag = row[2]
        if SumBetrag < 0:
            zinsen = SumBetrag * -1
            Betrag2 = Betrag + zinsen
            sumzinsen = sumzinsen + zinsen
            print DkNummer + ";" + str(Betrag) + ";" + str(Betrag2) + ";" + str(zinsen)
    print "Summe der Zinsen: " + str(sumzinsen)

    print "Nicht Ausbezahlte Zinsen 2019: "

    print "DkNummer;Betrag;Betrag mit Zinsen;Zinsen;Kündigungsdatum"
    select_stmt = "SELECT DkNummer, SUM(Betrag), vorgemerkt FROM db_vorjahr.DkBuchungen WHERE Rueckzahlung = '' GROUP BY DkNummer ORDER BY DkNummer;"
    stmt.execute(select_stmt)
    buchungen = stmt.fetchall()
    sumzinsen = 0
    for row in buchungen:
        zinsen = 0
        DkNummer = row[0]
        if DkNummer == 'Stammkapital':
            continue
        Betrag = row[1]
        vorgemerkt = row[2]
        select_stmt2 = "SELECT DkNummer, Betrag FROM db.DkBuchungen WHERE DkNummer ='" + DkNummer + "'";
        stmt.execute(select_stmt2)
        result = stmt.fetchone()
        if(stmt.rowcount == 0):
            print DkNummer + " nicht gefunden."
        Betrag2 = result[1]
        zinsen = Betrag2 - Betrag
        if(zinsen != 0):
            sumzinsen = sumzinsen + zinsen
            laufzeit = 0
            if (vorgemerkt != ''):
                start_datum = datetime.strptime("1.1.20", '%d.%m.%y')
                vorgemerkt_datum =  datetime.strptime(vorgemerkt, '%d.%m.%y')
                laufzeit = abs((vorgemerkt_datum - start_datum).days / 365)
            # if (laufzeit == 0):
            # if (laufzeit > 1 and laufzeit < 5):                
            # if (laufzeit > 5):
            if True:
                print DkNummer + ";" + str(Betrag).replace(".",",") + ";" + str(Betrag2).replace(".",",") + ";" + str(zinsen).replace(".",",") + ";" + str(vorgemerkt)
    print "Summe der Zinsen: " + str(sumzinsen)

except:
    print "Unexpected error:", sys.exc_info()[0]
    print sys.exc_info()
