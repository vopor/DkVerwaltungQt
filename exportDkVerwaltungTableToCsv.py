#!/usr/bin/python3
# coding: utf8

# -*- coding: utf-8 -*-

#
# Das Script exportiert die Daten der Datenbank aus DkVerwaltungQt.
# 
# Aufruf: ./exportDkVerwaltungQtToCsv.py <DKV2-db3-file> <csv-file>
#

import os
import sys
import traceback
import sqlite3
import csv

# print sqlite3.version
# print sys.argv
# print len(sys. argv)

# sys.setdefaultencoding('utf8')

if len(sys. argv) < 3:
    print ("Aufruf: ./" + os.path.basename(__file__) + " <DkVerwaltungQt-db3-file> <csv-file> ")
    sys.exit(1)

DkVerwaltungQt_db3_file=sys.argv[1]
if not os.path.isfile(DkVerwaltungQt_db3_file):
    print (DkVerwaltungQt_db3_file + "existiert nicht.")
    sys.exit(2)

csv_file=sys.argv[2]

try:
    conn = sqlite3.connect(DkVerwaltungQt_db3_file)
    stmt = conn.cursor()

    # Datum,DK-Nr.,Vorname,Name,Anrede,DK Nummer,Straße,PLZ,Ort,Email,Rückzahlung,vorgemerkt,Betrag,Zinssatz,Bemerkung
    statement = ""
    statement += "SELECT Datum, ROWID, Vorname, Name, '', DkNummer, Straße, PLZ, Ort, Email, " # CAST(DkNr AS INTEGER) # Anrede
    statement += "Rueckzahlung, vorgemerkt, CAST(replace(replace(replace(Betrag, ' €', ''), '.', ''), ',', '.') AS FLOAT), CAST(replace(replace(Zinssatz, '%', ''), ',', '.') AS FLOAT), "
    # TODO:
    # 23.01.14 F13T-2013-004
    # [auto] DK-Nr. 004 neu angelegt. Betrag: 10.000,00 €.
    # 30.06.17 [auto] DK-Nr. 004 gekündigt. Betrag: 10.000,00 €.
    statement += "'' AS Bemerkung "
    statement += "FROM DkVerwaltung "
    statement += "WHERE Betrag <> '0,00 €' "
    statement += "AND DkNummer <> 'Stammkapital' "
    # statement += "ORDER BY CAST(DkNr AS INTEGER) "

    print(statement)
    stmt.execute(statement)
    
    buchungen = stmt.fetchall()
    # for row in buchungen:
    #     print (row)
    print ("buchungen ", len(buchungen)) 

    with open(csv_file, 'w', encoding='utf8') as f:
        writer = csv.writer(f)
        # header = ['Datum','DK-Nr.','Vorname','Name','Anrede','DK Nummer','Straße','PLZ','Ort','Email','Rückzahlung','vorgemerkt','Betrag','Zinssatz','Bemerkung']
        header = [u'Datum',u'DK-Nr.',u'Vorname',u'Name',u'Anrede',u'DK Nummer',u'Straße',u'PLZ',u'Ort',u'Email',u'Rückzahlung',u'vorgemerkt',u'Betrag',u'Zinssatz',u'Bemerkung']
        # writer.writerow([s.encode("utf-8") for s in ['Datum','DK-Nr.','Vorname','Name','Anrede','DK Nummer','Straße','PLZ','Ort','Email','Rückzahlung','vorgemerkt','Betrag','Zinssatz','Bemerkung']])
        # writer.writerow([s.encode("utf-8") for s in header])
        writer.writerow(header)
        writer.writerows(buchungen)

    sys.exit(0)

except SystemExit:
    pass
except:
    print ("Unexpected error:", sys.exc_info()[0])
    print (sys.exc_info())
    print (traceback.print_exc())

sys.exit(0)
