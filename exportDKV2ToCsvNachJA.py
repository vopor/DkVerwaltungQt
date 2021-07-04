#!/usr/bin/python3
# coding: utf8

# -*- coding: utf-8 -*-

#
# Das Script exportiert die Daten der Datenbank aus DKV2.
# 
# Aufruf: ./exportDKV2ToCsvNachJA.py <DKV2-db3-file> <csv-file>
#

import os
import sys
import traceback
import sqlite3
import csv
# import panda
import datetime

# https://stackoverflow.com/questions/51832672/pandas-excel-days360-equivalent
def days360(start_date_str, end_date_str):
    method_eu=True
    # print("start_date_str", start_date_str)
    # print("end_date_str", end_date_str)
    start_date = datetime.date.fromisoformat(start_date_str)
    end_date = datetime.date.fromisoformat(end_date_str)
    start_day = start_date.day
    start_month = start_date.month
    start_year = start_date.year
    end_day = end_date.day
    end_month = end_date.month
    end_year = end_date.year

    if (
        start_day == 31 or
        (
            method_eu is False and
            start_month == 2 and (
                start_day == 29 or (
                    start_day == 28 and
                    start_date.is_leap_year is False
                )
            )
        )
    ):
        start_day = 30

    if end_day == 31:
        if method_eu is False and start_day != 30:
            end_day = 1

            if end_month == 12:
                end_year += 1
                end_month = 1
            else:
                end_month += 1
        else:
            end_day = 30

    ret = (
        end_day + end_month * 30 + end_year * 360 -
        start_day - start_month * 30 - start_year * 360)
    # print("ret", ret)
    return ret
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
    conn.create_function("days360", 2, days360)
    stmt = conn.cursor()

    # Datum,DK-Nr.,Vorname,Name,Anrede,DK Nummer,Straße,PLZ,Ort,Email,Rückzahlung,vorgemerkt,Betrag,Zinssatz,Bemerkung
    statement = ""
    statement += "SELECT ( substr(b.Datum,9,2) || '.' || substr(b.Datum,6,2) || '.' || substr(b.Datum,3,2)) AS Datum, "
    statement += "b.Id AS BuchungId, k.Vorname, k.Nachname, '', v.Kennung, k.Strasse, k.Plz, k.Stadt, k.Email, "
    
    # statement += "CASE WHEN (v.LaufzeitEnde != '9999-12-31') '
    # statement += "THEN CASE WHEN b.Ex = 1 THEN (( substr(v.LaufzeitEnde,9,2) || '.' || substr(v.LaufzeitEnde,6,2) || '.' || substr(v.LaufzeitEnde,3,2)) ELSE '' END "
    # statement += "ELSE '' END AS Rueckzahlung, "
    
    # statement += "CASE WHEN ((b.Ex = 1)) THEN (substr(b.Datum,9,2) || '.' || substr(b.Datum,6,2) || '.' || substr(b.Datum,3,2)) ELSE '' END AS Rueckzahlung, "
    statement += "CASE WHEN ((b.Ex = 1)) THEN ( substr(v.LaufzeitEnde,9,2) || '.' || substr(v.LaufzeitEnde,6,2) || '.' || substr(v.LaufzeitEnde,3,2)) ELSE '' END AS Rueckzahlung, "
    
    statement += "CASE WHEN ((v.LaufzeitEnde != '9999-12-31') AND (b.Ex = 0)) THEN ( substr(v.LaufzeitEnde,9,2) || '.' || substr(v.LaufzeitEnde,6,2) || '.' || substr(v.LaufzeitEnde,3,2)) END AS vorgemerkt, "
    
    # statement += "CASE WHEN (v.LaufzeitEnde != '9999-12-31') "
    # statement += "THEN (CASE WHEN (b.Ex = 0) THEN (( substr(v.LaufzeitEnde,9,2) || '.' || substr(v.LaufzeitEnde,6,2) || '.' || substr(v.LaufzeitEnde,3,2)) ELSE '' END) "
    # statement += "ELSE '' END AS vorgemerkt, "
    
    # statement += "ROUND((b.Betrag / 100.0),2) AS Betrag, "
    # statement += "(ROUND(b.Betrag,2) / 100.0) AS Betrag, "
    # statement += "(b.Betrag / 100.0) AS Betrag, "
    # statement += "ROUND(( (b.Betrag / 100.0) + (((b.Betrag * (v.ZSatz / 100.0)) / 100.0) / 360.0) ), 2) AS Betrag, "
    # statement += "CASE WHEN (Buchungsart = 2) THEN ( -1 * (b.Betrag / 100.0) ) ELSE (b.Betrag / 100.0) END AS Betrag, ";
    
    # statement += "ROUND(( (b.Betrag / 100.0) + (z.Zinsen / 100.0 / 360.0) ), 2) AS Betrag, "
    # kleinere Rundungsfehler bei 2. Nachkommastelle, da b.Betrag bereits gerundet
    statement += "ROUND( ( (b.Betrag + (z.Zinsen / 360.0) ) / 100.0), 2) AS Betrag, "

    # Runden(cAnfangsbetrag*pZinssatz*Zinstage(dAnfangsdatum, dEnddatum)/360, 2)
    # Exakte Übereinstimmung mit OO, da dieselbe Berechnung verwendet wird
    # statement += "ROUND( (a.Anfangsbetrag / 100.0) + ((a.Anfangsbetrag / 100.0) * ( (v.ZSatz / 100.0) / 100.0) * days360(Anfangsdatum, '2022-01-01') / 360.0) ,2) AS Betrag, "
       
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

    statement += "FROM "
    statement += "("
    # statement += "SELECT *, 0 AS Ex FROM Buchungen "
    statement += "SELECT MIN(Buchungen.Id) AS Id, Buchungen.VertragsId AS VertragsId, '2022-01-01' AS Datum, 1 AS BuchungsArt, SUM(Betrag) AS Betrag, MIN(Buchungen.Zeitstempel), 0 AS Ex FROM Buchungen GROUP BY Buchungen.VertragsId "
    # statement += "UNION "
    # statement += "SELECT *, 1 AS Ex FROM ExBuchungen "
    statement += ") b "
    statement += "LEFT OUTER JOIN (SELECT a1.VertragsId as VertragsId, a1.Betrag AS Anfangsbetrag, a1.Datum AS Anfangsdatum FROM Buchungen a1 WHERE a1.Id = (SELECT MIN(a2.Id) FROM Buchungen a2 WHERE a1.VertragsId = a2.VertragsId)) a ON b.VertragsId = a.VertragsId "
    statement += "LEFT OUTER JOIN (SELECT z1.VertragsId as VertragsId, z1.Betrag AS Zinsen FROM Buchungen z1 WHERE z1.Id = (SELECT MAX(z2.Id) FROM Buchungen z2 WHERE z1.VertragsId = z2.VertragsId)) z ON b.VertragsId = z.VertragsId "
    statement += "LEFT OUTER JOIN (SELECT *, 0 AS Ex FROM Vertraege UNION SELECT *, 1 AS Ex FROM ExVertraege) v ON b.VertragsId = v.Id "
    statement += "LEFT OUTER JOIN Kreditoren k ON v.KreditorId = k.Id "
    statement += "WHERE b.Betrag <> 0 "
    # statement += "ORDER BY CAST(b.id AS TEXT); "
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
