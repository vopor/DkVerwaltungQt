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
    print "modus = 3: Nicht Ausbezahlte Zinsen Laufzeit größer gleich 1 und kleiner 5"
    print "modus = 4: Nicht Ausbezahlte Zinsen Laufzeit größer gleich 5 Jahre"
    print "modus = 5: Alle Nicht Ausbezahlte Zinsen"
    print "modus = 6: Alle ohne Zinsen"
    print "modus = 7: Dk's als Mietsicherheit"

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
Year4 = "20" + Year2
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
dks_als_mietsicherheit += ",\"F13T-2019-024\""

try:
    conn = sqlite3.connect('')
    stmt = conn.cursor()
    stmt.execute('ATTACH DATABASE "{0}" AS db_vorjahr'.format(DkVerwaltungQt_db3_file_vorjahr)) 
    stmt.execute('ATTACH DATABASE "{0}" AS db'.format(DkVerwaltungQt_db3_file))
    conn.commit()

    if modus=="1":
        print "\"" + modus  + "." + "Ausbezahlte Zinsen" + " " + Year4 + "\""  + ".csv" 
        print "DkNummer    ;    Betrag;    Betrag mit Zinsen;    Zinsen;vorgemerkt    ;Kündigungsdatum    ;counter"
        
        select_stmt = "SELECT DkNummer, SUM(round(Betrag,2)), MAX(round(Betrag,2)), MIN(round(Betrag,2)), Count(*), vorgemerkt, Rueckzahlung FROM db_vorjahr.DkBuchungen "
        select_stmt += " WHERE "
        select_stmt += " ((substr(Rueckzahlung ,7,2) || substr(Rueckzahlung ,4,2)|| substr(Rueckzahlung ,1,2)) > '" + Year2 + "0101') "
        # select_stmt += "AND Zinssatz <> 0 " 
        # select_stmt += "Rueckzahlung <> '' "
        # select_stmt += "AND DkNummer NOT IN (" +  dks_als_mietsicherheit + ") "
        select_stmt += " GROUP BY DkNummer "
        # select_stmt += " HAVING SUM(Betrag) < 0 "
        select_stmt += " ORDER BY DkNummer"

        # SELECT COUNT(*), SUM(ZinsBetrag), SUM(BetragOhneZinsen), SUM(BetragMitZinsen) FROM 
        # (SELECT DkNummer, SUM(Betrag) AS ZinsBetrag, MAX(Betrag) AS BetragOhneZinsen, MIN(Betrag) AS BetragMitZinsen, Count(*), vorgemerkt, Rueckzahlung FROM DkBuchungen  
        # WHERE ((substr(Rueckzahlung ,7,2) || substr(Rueckzahlung ,4,2)|| substr(Rueckzahlung ,1,2)) > '190101')  GROUP BY DkNummer ORDER BY DkNummer);

        print select_stmt
        stmt.execute(select_stmt)
        
        buchungen = stmt.fetchall()
        sumzinsen = 0
        sumbetrag = 0
        sumbetrag2 = 0
        count = 0
        for row in buchungen:
            zinsen = 0
            DkNummer = row[0]
            SumBetrag = row[1]
            Betrag = row[2]
            MinBetrag = row[3]
            counter  = row[4]
            vorgemerkt = row[5]
            Rueckzahlung = row[6]
            if SumBetrag < 0:
                zinsen = SumBetrag * -1
                Betrag2 = Betrag + zinsen
                sumzinsen = sumzinsen + zinsen
                sumbetrag = sumbetrag + Betrag
                sumbetrag2 = sumbetrag2 + Betrag2
                count = count + 1
                print DkNummer + ";" + str(Betrag).replace(".",",")  + ";" + str(Betrag2).replace(".",",")  + ";" + str(zinsen).replace(".",",")  + ";" + str(vorgemerkt) + ";" + str(Rueckzahlung) + ";" + str(counter)
        print modus +".Anzahl : " + str(count) + ";" + str(sumbetrag).replace(".",",") + ";" + str(sumbetrag2).replace(".",",") + ";" + str(sumzinsen).replace(".",",")
        # print select_stmt

    elif (modus == "1") or (modus == "2") or (modus == "3") or (modus == "4") or (modus == "5") or (modus == "6") or (modus == "7"):
        if (modus == "1"):
            print "\"" + modus + "." + "Ausbezahlte Zinsen" + " " + Year4 + "\"" + ".csv" 
        elif (modus == "2"):
            print "\"" + modus + "." + "Nicht Ausbezahlte Zinsen Laufzeit kleiner 1 Jahr" + " " + Year4 + "\""  + ".csv" 
        elif (modus == "3"):
            print "\"" + modus + "." + "Nicht Ausbezahlte Zinsen Laufzeit größer gleich 1 und kleiner 5" + " " + Year4 + "\""  + ".csv" 
        elif (modus == "4"):
            print "\"" + modus + "." + "Nicht Ausbezahlte Zinsen Laufzeit größer gleich 5 Jahre" + " " + Year4 + "\""  + ".csv" 
        elif (modus == "5"):
            print "\"" + modus + "." + "Alle Nicht Ausbezahlte Zinsen" + " " + Year4 + "\""  + ".csv" 
        elif (modus == "6"):
            print "\"" + modus + "." + "Alle ohne Zinsen" + " " + Year4 + "\""  + ".csv" 
        elif (modus == "7"):
            print "\"" + modus + "." + "Dks als Mietsicherheit" + " " + Year4 + "\""  + ".csv" 

        # print "DkNummer;Betrag;Betrag mit Zinsen;Zinsen;vorgemerkt;Kündigungsdatum"
        print "DkNummer    ;    Betrag;    Betrag mit Zinsen;    Zinsen;vorgemerkt    ;Kündigungsdatum    "
        select_stmt = "SELECT DkNummer, SUM(round(Betrag,2)), vorgemerkt, Rueckzahlung FROM db_vorjahr.DkBuchungen "
        select_stmt += "WHERE "
        
        if (modus == "1"):
            select_stmt += "((substr(Rueckzahlung ,7,2) || substr(Rueckzahlung ,4,2)|| substr(Rueckzahlung ,1,2)) >= '" + Year2 + "0101') "
        else:
            select_stmt += "(";
            select_stmt += "Rueckzahlung = '' "
            select_stmt += "OR ((substr(Rueckzahlung ,7,2) || substr(Rueckzahlung ,4,2)|| substr(Rueckzahlung ,1,2)) < '" + Year2 + "0101') "
            select_stmt += ")";

        select_stmt += "AND DkNummer <> 'Stammkapital' "
        
        if (modus == "6"):
            # select_stmt += "AND Zinssatz = 0 " 
            select_stmt += "AND (Zinssatz = 0 OR ((substr(Rueckzahlung ,7,2) || substr(Rueckzahlung ,4,2)|| substr(Rueckzahlung ,1,2)) < '" + Year2 + "0101')) "
        else:
            select_stmt += "AND Zinssatz <> 0 "
        
        if ((modus != "1") and (modus != "5") and (modus != "6")):
            if(modus == "7"):
                select_stmt += "AND DkNummer IN (" +  dks_als_mietsicherheit + ") "
            else:  
                select_stmt += "AND DkNummer NOT IN (" +  dks_als_mietsicherheit + ") "

        select_stmt += " GROUP BY DkNummer "
        # select_stmt += " HAVING SUM(Betrag) > 0 "
        select_stmt += " ORDER BY DkNummer"

        stmt.execute(select_stmt)
        buchungen = stmt.fetchall()
        sumzinsen = 0
        sumbetrag = 0
        sumbetrag2 = 0
        count = 0
        for row in buchungen:
            zinsen = 0
            DkNummer = row[0]
            if DkNummer == 'Stammkapital':
                continue
            Betrag = row[1]
            vorgemerkt = row[2]
            Rueckzahlung = row[3]
            # select_stmt2 = "SELECT DkNummer, round(Betrag,2) FROM db.DkBuchungen WHERE DkNummer ='" + DkNummer + "'";
            select_stmt2 = "SELECT DkNummer, SUM(round(Betrag,2)) FROM db.DkBuchungen WHERE DkNummer ='" + DkNummer + "'";
            # select_stmt2 += " AND Rueckzahlung = ''  "
            stmt.execute(select_stmt2)
            result = stmt.fetchone()
            if(stmt.rowcount == 0):
                print DkNummer + " nicht gefunden."
            Betrag2 = 0
            if result[1] != None:
                Betrag2 = result[1]
            if (modus == "1xxx"):
                if Betrag < 0:
                    zinsen = Betrag * -1
                    Betrag2 = Betrag + zinsen
            else:
                zinsen = Betrag2 - Betrag
            if ((modus == "1") and (zinsen != 0)) or ((modus == "6") and (zinsen == 0)) or ((modus == "5") and (zinsen != 0)) or ((modus != "6") and ((zinsen != 0) and (Betrag2 != 0))): 
                laufzeit = 0
                if (vorgemerkt != ''):
                    start_datum = datetime.strptime("1.1.20", '%d.%m.%y')
                    vorgemerkt_datum =  datetime.strptime(vorgemerkt, '%d.%m.%y')
                    laufzeit = abs((vorgemerkt_datum - start_datum).days / 365)
                if (modus == "1") or ((modus == "2") and (laufzeit < 1)) or ((modus == "3") and (laufzeit >= 1 and laufzeit < 5)) or  ((modus == "4") and (laufzeit >= 5)) or (modus == "5") or (modus == "6") or (modus == "7"):
                    sumzinsen = sumzinsen + zinsen
                    sumbetrag = sumbetrag + Betrag
                    sumbetrag2 = sumbetrag2 + Betrag2
                    count = count + 1
                    print DkNummer + ";" + str(Betrag).replace(".",",") + ";" + str(Betrag2).replace(".",",") + ";" + str(zinsen).replace(".",",") + ";" + str(vorgemerkt) + ";" + str(Rueckzahlung)
                    # F13T-2019-023|5003.13
                    # print DkNummer + "|" + str(Betrag2)
                    # print DkNummer + ";" , str(Betrag).replace(".",",") + ";" , str(Betrag2).replace(".",",") + ";" , str(zinsen).replace(".",",") + ";" , str(vorgemerkt) + ";" , str(Rueckzahlung)
        print modus + ".Anzahl : " + str(count) + ";" + str(sumbetrag).replace(".",",") + ";" + str(sumbetrag2).replace(".",",") + ";" + str(sumzinsen).replace(".",",")
        # print "Anzahl : " + str(count) + ";" , str(sumbetrag).replace(".",",") + ";" , str(sumbetrag2).replace(".",",") + ";" , str(sumzinsen).replace(".",",")
        # print select_stmt

    elif False and modus=="7":
        print "Dks als Mietsicherheit 20" + Year2 + ": "
        # print "DkNummer;Vorname;Name;AnfangsBetrag;Betrag mit Zinsen;Zinssatz;vorgemerkt;Rueckzahlung"
        print "DkNummer;AnfangsBetrag;Betrag;Betrag mit Zinsen;Zinsen;Zinssatz;Rueckzahlung"
        # select_stmt = "SELECT Name, Vorname, DkNummer, AnfangsBetrag, SUM(Betrag), MAX(Betrag), vorgemerkt, Rueckzahlung, Zinssatz FROM db_vorjahr.DkBuchungen, db_vorjahr.DkPersonen "
        select_stmt = ""
        select_stmt += "SELECT Name, Vorname, DkNummer, SUM(AnfangsBetrag), SUM(Betrag), MAX(Betrag), vorgemerkt, Rueckzahlung, Zinssatz FROM db_vorjahr.DkBuchungen, db_vorjahr.DkPersonen "
        # select_stmt += "SELECT Name, Vorname, DkNummer, AnfangsBetrag, Betrag, vorgemerkt, Rueckzahlung, Zinssatz FROM db_vorjahr.DkBuchungen, db_vorjahr.DkPersonen "
        select_stmt += "WHERE db_vorjahr.DkPersonen.PersonId = db_vorjahr.DkBuchungen.PersonId "
        select_stmt += " AND Zinssatz = 0.5 "
        # select_stmt += " AND NOT ( round(AnfangsBetrag/100.0, 0) = Anfangsbetrag / 100.0 ) "
        select_stmt += " AND DkNummer IN (" +  dks_als_mietsicherheit + ") "
        select_stmt += " GROUP BY DkNummer ORDER BY DkNummer;"
        # print select_stmt
        stmt.execute(select_stmt)
        buchungen = stmt.fetchall()
        sumzinsen = 0
        count = 0
        for row in buchungen:
            Vorname = row[0]
            Name = row[1]
            DkNummer = row[2]
            AnfangsBetrag = row[3]
            Betrag = row[4]
            vorgemerkt = row[5]
            Rueckzahlung = row[6]
            Zinssatz = row[7]
            select_stmt2 = "SELECT DkNummer, SUM(Betrag) FROM db.DkBuchungen WHERE DkNummer ='" + DkNummer + "'";
            stmt.execute(select_stmt2)
            result = stmt.fetchone()
            if(stmt.rowcount == 0):
                print DkNummer + " nicht gefunden."
            Betrag2 = result[1]
            zinsen = Betrag2 - Betrag
            sumzinsen = sumzinsen + zinsen
            count = count + 1
            # zinsen = (Betrag * Zinssatz) / 100.0
            # print Vorname + ";" + Name + ";" + DkNummer + ";" + str(AnfangsBetrag) + ";" + str(Betrag).replace(".",",") + ";" + str(zinsen).replace(".",",") + ";" + str(Zinssatz).replace(".",",") + ";" + str(vorgemerkt) + ";" + str(Rueckzahlung)
            print DkNummer + ";" + str(AnfangsBetrag).replace(".",",") + ";" + str(Betrag).replace(".",",") + ";" + str(Betrag2).replace(".",",") + ";" + str(zinsen).replace(".",",") + ";" + str(Zinssatz).replace(".",",") + ";" + str(Rueckzahlung)
        print "Summe der Zinsen: " + str(sumzinsen).replace(".",",")
        print "Anzahl : " + str(count)

except:
    print "Unexpected error:", sys.exc_info()[0]
    print sys.exc_info()
