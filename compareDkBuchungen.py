#!/usr/bin/python
# coding: utf8

# -*- coding: utf-8 -*-

#
# Das Script vergleicht die Datensätze der Tabelle DkBuchungen der Datenbank von DkVerwaltungQt mit der Datenbank des Vorjahres
# vom 01.01.
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
if len(sys. argv) < 4:
    print "Aufruf: ./compareDKkVerwaltungYears.py <DkVerwaltungQt-db3-file-vorjahr> <DkVerwaltungQt-db3-file> <Year2> "
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

try:
    conn = sqlite3.connect('')
    stmt = conn.cursor()
    stmt.execute('ATTACH DATABASE "{0}" AS db_from'.format(DkVerwaltungQt_db3_file_vorjahr)) 
    stmt.execute('ATTACH DATABASE "{0}" AS db_to'.format(DkVerwaltungQt_db3_file))
    conn.commit()
    
    
    select_stmt_from = "SELECT DkNummer, Betrag FROM db_from.DkBuchungen "
    select_stmt_from += " WHERE ((substr(Datum ,7,2) || substr(Datum ,4,2)|| substr(Datum ,1,2)) = '" + Year2 + "0101') "
    select_stmt_from += "AND DkNummer != 'Stammkapital' "
    select_stmt_from += " ORDER By DkNummer "
    stmt.execute(select_stmt_from)
    buchungen_from = stmt.fetchall()
    
    print DkVerwaltungQt_db3_file_vorjahr
    print len(buchungen_from), "Datensätze"

    select_stmt_to = "SELECT DkNummer, Betrag FROM db_to.DkBuchungen "
    select_stmt_to += " WHERE ((substr(Datum ,7,2) || substr(Datum ,4,2)|| substr(Datum ,1,2)) = '" + Year2 + "0101') "
    select_stmt_to += "AND DkNummer != 'Stammkapital' "
    select_stmt_to += " ORDER By DkNummer "
    stmt.execute(select_stmt_to)
    buchungen_to = stmt.fetchall()

    print DkVerwaltungQt_db3_file
    print len(buchungen_to), "Datensätze"

    Summe_Diff_from = 0
    Summe_Betrag_from = 0
    for i, buchung_from in enumerate(buchungen_from, start=0):   # default is zero
        DkNummer_from = buchungen_from[i][0]
        Betrag_from = buchungen_from[i][1]
        Summe_Betrag_from = Summe_Betrag_from + Betrag_from
        # print DkNummer_from, Betrag_from
        found = False
        for j, buchung_from in enumerate(buchungen_to, start=0):   # default is zero
            DkNummer_to = buchungen_to[j][0]
            Betrag_to = buchungen_to[j][1]
            if(DkNummer_from == DkNummer_to):
                found = True
                betragdiff = False
                if(Betrag_from != Betrag_to):
                    betragdiff = True
                    Summe_Diff_from = Summe_Diff_from + (Betrag_to - Betrag_from)
                    print DkNummer_from, Betrag_from, Betrag_to
                if betragdiff == True:
                    if(Betrag_from == Betrag_to):
                        Summe_Diff_from = Summe_Diff_from + (Betrag_to - Betrag_from)
                        print DkNummer_from, Betrag_from, Betrag_to

        if not found:
            print "From: " + DkNummer_from, Betrag_from, " nicht gefunden!"

    print "Summe From: ", str(Summe_Betrag_from)
    # print "Summe Diff From: ", str(Summe_Diff_from)

    Summe_Diff_to = 0
    Summe_Betrag_to = 0
    for j, buchung_from in enumerate(buchungen_to, start=0):   # default is zero
        DkNummer_to = buchungen_to[j][0]
        Betrag_to = buchungen_to[j][1]
        Summe_Betrag_to = Summe_Betrag_to + Betrag_to
        found = False
        for i, buchung_from in enumerate(buchungen_from, start=0):   # default is zero
            DkNummer_from = buchungen_from[i][0]
            Betrag_from = buchungen_from[i][1]
            # print DkNummer_from, Betrag_from
            if(DkNummer_from == DkNummer_to):
                found = True
            if found == True:
                if(DkNummer_from != DkNummer_to):
                    break
        if not found:
            Summe_Diff_to = Summe_Diff_to + Betrag_to
            print "To: " +  DkNummer_to, Betrag_to, " nicht gefunden!"
    print "Summe To: ", str(Summe_Betrag_to)
    # print "Summe Diff To: ", str(Summe_Diff_to)
    # print "Summe Diff: ", str(Summe_Diff_to - Summe_Diff_from)

except:
    print "Unexpected error:", sys.exc_info()[0]
    print sys.exc_info()
