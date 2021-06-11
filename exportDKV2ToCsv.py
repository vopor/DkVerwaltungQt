#!/usr/bin/python3
# coding: utf8

# -*- coding: utf-8 -*-

#
# Das Script exportiert die Daten der Datenbank aus DKV2.
# 
# Aufruf: ./exportDKV2ToCsv.py <DKV2-db3-file> <csv-file>
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
    print ("Aufruf: ./" + os.path.basename(__file__) + " <DKV2-db3-file> <csv-file> ")
    sys.exit(1)

DKV2_db3_file=sys.argv[1]
if not os.path.isfile(DKV2_db3_file):
    print (DKV2_db3_file + "existiert nicht.")
    sys.exit(2)

csv_file=sys.argv[2]

try:
    conn = sqlite3.connect(DKV2_db3_file)
    stmt = conn.cursor()

    # Datum,DK-Nr.,Vorname,Name,Anrede,DK Nummer,Straße,PLZ,Ort,Email,Rückzahlung,vorgemerkt,Betrag,Zinssatz,Bemerkung
    statement = ""
    statement += "SELECT ( substr(b.Datum,9,2) || '.' || substr(b.Datum,6,2) || '.' || substr(b.Datum,3,2)) AS Datum, "
    statement += "b.Id AS BuchungId, k.Vorname, k.Nachname, '', v.Kennung, k.Strasse, k.Plz, k.Stadt, k.Email, "
    
    # statement += "CASE WHEN (v.LaufzeitEnde != '9999-12-31') '
    # statement += "THEN CASE WHEN b.Ex = 1 THEN (( substr(v.LaufzeitEnde,9,2) || '.' || substr(v.LaufzeitEnde,6,2) || '.' || substr(v.LaufzeitEnde,3,2)) ELSE '' END "
    # statement += "ELSE '' END AS Rueckzahlung, "
    statement += "CASE WHEN (b.Betrag < 0) THEN (substr(b.Datum,9,2) || '.' || substr(b.Datum,6,2) || '.' || substr(b.Datum,3,2)) ELSE '' END AS Rueckzahlung, "
    
    statement += "CASE WHEN (v.LaufzeitEnde != '9999-12-31') THEN ( substr(v.LaufzeitEnde,9,2) || '.' || substr(v.LaufzeitEnde,6,2) || '.' || substr(v.LaufzeitEnde,3,2)) END AS vorgemerkt, "
    
    # statement += "CASE WHEN (v.LaufzeitEnde != '9999-12-31') "
    # statement += "THEN (CASE WHEN (b.Ex = 0) THEN (( substr(v.LaufzeitEnde,9,2) || '.' || substr(v.LaufzeitEnde,6,2) || '.' || substr(v.LaufzeitEnde,3,2)) ELSE '' END) "
    # statement += "ELSE '' END AS vorgemerkt, "
    
    statement += "(b.Betrag / 100.0) AS Betrag, "
    # statement += "CASE WHEN (Buchungsart = 2) THEN ( -1 * (b.Betrag / 100.0) ) ELSE (b.Betrag / 100.0) END AS Betrag, ";
    
    statement += "(v.ZSatz / 100.0) AS Zinssatz, "
    # TODO:
    # 23.01.14 F13T-2013-004
    # [auto] DK-Nr. 004 neu angelegt. Betrag: 10.000,00 €.
    # 30.06.17 [auto] DK-Nr. 004 gekündigt. Betrag: 10.000,00 €.
    statement += "'' AS Bemerkung "

    # statement += "FROM "
    # statement += "(SELECT *, 0 AS Ex FROM Buchungen UNION SELECT *, 1 AS Ex FROM ExBuchungen) b, "
    # statement += "(SELECT *, 0 AS Ex FROM Vertraege UNION SELECT *, 1 AS Ex FROM ExVertraege) v, "
    # statement += "Kreditoren k "
    # statement += "WHERE b.VertragsId = v.Id "
    # statement += "AND v.KreditorId = k.Id "

    statement += "FROM (SELECT *, 0 AS Ex FROM Buchungen UNION SELECT *, 1 AS Ex FROM ExBuchungen) b "
    statement += "LEFT OUTER JOIN (SELECT *, 0 AS Ex FROM Vertraege UNION SELECT *, 1 AS Ex FROM ExVertraege) v ON b.VertragsId = v.Id "
    statement += "LEFT OUTER JOIN Kreditoren k ON v.KreditorId = k.Id;"

    # statement += "ORDER BY CAST(b.id AS TEXT); "
    print(statement)
    stmt.execute(statement)
    
    buchungen = stmt.fetchall()
    print ("buchungen", len(buchungen))
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
