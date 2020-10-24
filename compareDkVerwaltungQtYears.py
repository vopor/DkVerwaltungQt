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
if len(sys. argv) < 5:
    print "Aufruf: ./compareDKkVerwaltungYears.py <DkVerwaltungQt-db3-file-vorjahr> <DkVerwaltungQt-db3-file> <Year2> <modus>"
    print "modus = 1: Ausbezahlte Zinsen "
    print "modus = 2: Nicht Ausbezahlte Zinsen Laufzeit kleiner 1 Jahr"
    print "modus = 3: Nicht Ausbezahlte Zinsen Laufzeit größer 1 und kleiner 5"
    print "modus = 4: Nicht Ausbezahlte Zinsen Laufzeit größer 5 Jahre"
    print "modus = 5: Alle Nicht Ausbezahlte Zinsen"
    print "modus = 6: Dk's als Mietsicherheit"
    exit(1)

DkVerwaltungQt_db3_file_vorjahr=sys.argv[1]
DkVerwaltungQt_db3_file=sys.argv[2]
if not os.path.isfile(DkVerwaltungQt_db3_file_vorjahr):
    print DkVerwaltungQt_db3_file_vorjahr + "existiert nicht."
    exit(2)
if not os.path.isfile(DkVerwaltungQt_db3_file):
    print DkVerwaltungQt_db3_file + "existiert nicht."
    exit(3)

Year2 = sys.argv[3]
modus=sys.argv[4]

dks_als_mietsicherheit = "\"F13T-2015-050\""
dks_als_mietsicherheit += ",\"F13T-2015-05\""
dks_als_mietsicherheit += ",\"F13T-2015-052\""
dks_als_mietsicherheit += ",\"F13T-2015-055\""
dks_als_mietsicherheit += ",\"F13T-2015-056\""
dks_als_mietsicherheit += ",\"F13T-2015-057\""
dks_als_mietsicherheit += ",\"F13T-2015-058\""
dks_als_mietsicherheit += ",\"F13T-2015-059\""
dks_als_mietsicherheit += ",\"F13T-2015-060\""
dks_als_mietsicherheit += ",\"F13T-2015-062\""
dks_als_mietsicherheit += ",\"F13T-2015-063\""
dks_als_mietsicherheit += ",\"F13T-2015-065\""
dks_als_mietsicherheit += ",\"F13T-2015-066\""
dks_als_mietsicherheit += ",\"F13T-2015-067\""
dks_als_mietsicherheit += ",\"F13T-2015-068\""
dks_als_mietsicherheit += ",\"F13T-2015-069\""
dks_als_mietsicherheit += ",\"F13T-2015-071\""
dks_als_mietsicherheit += ",\"F13T-2015-072\""
dks_als_mietsicherheit += ",\"F13T-2015-074\""
dks_als_mietsicherheit += ",\"F13T-2015-076\""
dks_als_mietsicherheit += ",\"F13T-2015-077\""
dks_als_mietsicherheit += ",\"F13T-2016-010\""
dks_als_mietsicherheit += ",\"F13T-2016-018\""
dks_als_mietsicherheit += ",\"F13T-2016-021\""
dks_als_mietsicherheit += ",\"F13T-2018-030\""
dks_als_mietsicherheit += ",\"F13T-2018-031\""
dks_als_mietsicherheit += ",\"F13T-2019-009\""
dks_als_mietsicherheit += ",\"F13T-2019-012\""

try:
    conn = sqlite3.connect('')
    stmt = conn.cursor()
    stmt.execute('ATTACH DATABASE "{0}" AS db_vorjahr'.format(DkVerwaltungQt_db3_file_vorjahr)) 
    stmt.execute('ATTACH DATABASE "{0}" AS db'.format(DkVerwaltungQt_db3_file))
    conn.commit()

    if modus=="1":
        print "Ausbezahlte Zinsen 20" + Year2 + ": "
        print "DkNummer;Betrag;Betrag mit Zinsen;Zinsen"
        select_stmt = "SELECT DkNummer, SUM(Betrag), MAX(Betrag), MIN(Betrag), Count(*) FROM db_vorjahr.DkBuchungen "
        select_stmt += " WHERE ((substr(Rueckzahlung ,7,2) || substr(Rueckzahlung ,4,2)|| substr(Rueckzahlung ,1,2)) > '" + Year2 + "0101') "
        select_stmt += " GROUP BY DkNummer ORDER BY DkNummer;"
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
                print DkNummer + ";" + str(Betrag).replace(".",",")  + ";" + str(Betrag2).replace(".",",")  + ";" + str(zinsen).replace(".",",") 
        print "Summe der Zinsen: " + str(sumzinsen).replace(".",",") #  + " " + str(len(buchungen))

    elif (modus == "2") or (modus == "3") or (modus == "4") or (modus == "5"):
        print "Nicht Ausbezahlte Zinsen 20" + Year2 + ": "
        print "DkNummer;Betrag;Betrag mit Zinsen;Zinsen;vorgemerkt;Kündigungsdatum"
        select_stmt = "SELECT DkNummer, SUM(Betrag), vorgemerkt, Rueckzahlung FROM db_vorjahr.DkBuchungen "
        select_stmt += "WHERE Rueckzahlung = '' "
        select_stmt += " AND DkNummer NOT IN (" +  dks_als_mietsicherheit + ") "
        select_stmt += "GROUP BY DkNummer ORDER BY DkNummer "
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
            Rueckzahlung = row[3]
            select_stmt2 = "SELECT DkNummer, Betrag FROM db.DkBuchungen WHERE DkNummer ='" + DkNummer + "'";
            stmt.execute(select_stmt2)
            result = stmt.fetchone()
            if(stmt.rowcount == 0):
                print DkNummer + " nicht gefunden."
            Betrag2 = 0
            if result[1] != None:
                Betrag2 = result[1]
            zinsen = Betrag2 - Betrag
            if(zinsen != 0) and (Betrag2 != 0):
                sumzinsen = sumzinsen + zinsen
                laufzeit = 0
                if (vorgemerkt != ''):
                    start_datum = datetime.strptime("1.1.20", '%d.%m.%y')
                    vorgemerkt_datum =  datetime.strptime(vorgemerkt, '%d.%m.%y')
                    laufzeit = abs((vorgemerkt_datum - start_datum).days / 365)
                if ((modus == "2") and (laufzeit == 0)) or ((modus == "3") and (laufzeit > 1 and laufzeit < 5)) or  ((modus == "4") and (laufzeit > 5)) or (modus == "5"):
                    print DkNummer + ";" + str(Betrag).replace(".",",") + ";" + str(Betrag2).replace(".",",") + ";" + str(zinsen).replace(".",",") + ";" + str(vorgemerkt) + ";" + str(Rueckzahlung)
        print "Summe der Zinsen: " + str(sumzinsen)

    elif modus=="6":
        print "Dks als Mietsicherheit 20" + Year2 + ": "
        # print "DkNummer;Vorname;Name;AnfangsBetrag;Betrag mit Zinsen;Zinssatz;vorgemerkt;Rueckzahlung"
        print "DkNummer;AnfangsBetrag;Betrag mit Zinsen;Zinsen;Zinssatz;Rueckzahlung"
        # select_stmt = "SELECT Name, Vorname, DkNummer, AnfangsBetrag, SUM(Betrag), MAX(Betrag), vorgemerkt, Rueckzahlung, Zinssatz FROM db_vorjahr.DkBuchungen, db_vorjahr.DkPersonen "
        select_stmt = "SELECT Name, Vorname, DkNummer, AnfangsBetrag, Betrag, vorgemerkt, Rueckzahlung, Zinssatz FROM db_vorjahr.DkBuchungen, db_vorjahr.DkPersonen "
        select_stmt += "WHERE db_vorjahr.DkPersonen.PersonId = db_vorjahr.DkBuchungen.PersonId "
        select_stmt += " AND Zinssatz = 0.5 "
        # select_stmt += " AND NOT ( round(AnfangsBetrag/100.0, 0) = Anfangsbetrag / 100.0 ) "
        select_stmt += " AND DkNummer IN (" +  dks_als_mietsicherheit + ") "
        # select_stmt += " GROUP BY DkNummer ORDER BY DkNummer;"
        # print select_stmt
        stmt.execute(select_stmt)
        buchungen = stmt.fetchall()
        print len(buchungen)
        for row in buchungen:
            Vorname = row[0]
            Name = row[1]
            DkNummer = row[2]
            AnfangsBetrag = row[3]
            Betrag = row[4]
            vorgemerkt = row[5]
            Rueckzahlung = row[6]
            Zinssatz = row[7]
            select_stmt2 = "SELECT DkNummer, Betrag FROM db.DkBuchungen WHERE DkNummer ='" + DkNummer + "'";
            stmt.execute(select_stmt2)
            result = stmt.fetchone()
            if(stmt.rowcount == 0):
                print DkNummer + " nicht gefunden."
            Betrag2 = result[1]
            zinsen = Betrag2 - Betrag
            # zinsen = (Betrag * Zinssatz) / 100.0
            # print Vorname + ";" + Name + ";" + DkNummer + ";" + str(AnfangsBetrag) + ";" + str(Betrag).replace(".",",") + ";" + str(zinsen).replace(".",",") + ";" + str(Zinssatz).replace(".",",") + ";" + str(vorgemerkt) + ";" + str(Rueckzahlung)
            print DkNummer + ";" + str(AnfangsBetrag) + ";" + str(Betrag).replace(".",",") + ";" + str(zinsen).replace(".",",") + ";" + str(Zinssatz).replace(".",",") + ";" + str(Rueckzahlung)

except:
    print "Unexpected error:", sys.exc_info()[0]
    print sys.exc_info()
