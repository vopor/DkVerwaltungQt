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
    select_stmt = 'SELECT COUNT(DISTINCT DkNummer) FROM db_from.DKBuchungen '
    # select_stmt += 'WHERE Anfangsdatum <> "" AND DkNummer <> "Stammkapital" '
    stmt.execute(select_stmt)
    print stmt.fetchone()[0]
    print "Anzahl Vertraege:"
    select_stmt = 'SELECT COUNT(*) FROM db_to.Vertraege'
    stmt.execute(select_stmt)
    print stmt.fetchone()[0]
    # Summierte DkBuchungen und Vertraege
    print "Anzahl DKBuchungen:"
    select_stmt = 'SELECT COUNT(*) FROM db_from.DKBuchungen '
    # select_stmt += 'WHERE Anfangsdatum <> "" AND DkNummer <> "Stammkapital" '
    stmt.execute(select_stmt)
    print stmt.fetchone()[0]
    print "Anzahl Buchungen:"
    select_stmt = 'SELECT COUNT(*) FROM db_to.Buchungen'
    stmt.execute(select_stmt)
    print stmt.fetchone()[0]

    print "Vertraege, die nicht übernommen wurden:"
    select_stmt = 'SELECT * FROM db_from.DKBuchungen '
    select_stmt += 'WHERE Anfangsdatum <> "" AND DkNummer <> "Stammkapital" '
    select_stmt += 'AND NOT EXISTS'
    select_stmt += '('
    select_stmt += 'SELECT * FROM db_to.Vertraege WHERE db_from.DKBuchungen.DkNummer = db_to.Vertraege.Kennung '    
    select_stmt += ')'
    stmt.execute(select_stmt)
    buchungen = stmt.fetchall()
    for row in buchungen:
        print row
    print "Anzahl: ", len(buchungen)

    if True:
        print "VertragsId prüfen"
        select_stmt = 'SELECT c.id FROM db_to.Buchungen, db_from.DKBuchungen b, db_to.Vertraege c '
        select_stmt += 'WHERE db_to.Buchungen.id = b.BuchungId '
        select_stmt += 'AND b.DkNummer = c.Kennung '
        select_stmt += 'AND c.id IS NULL '
        stmt.execute(select_stmt);
        buchungen = stmt.fetchall()
        if len(buchungen) > 0:
            print "buchungen", len(buchungen)
            for row in buchungen:
                print row
        print "Anzahl: ", len(buchungen)

    print "Vertraege mit abweichendem aufsummierten Betrag:"
    
    select_stmt = '';
    select_stmt += 'SELECT KennungA, SummeA '
    select_stmt += 'FROM '
    select_stmt += '( '
    select_stmt += 'SELECT  db_to.Vertraege.Kennung AS KennungA, SUM(db_to.Buchungen.Betrag)/100. AS SummeA '
    select_stmt += 'FROM db_to.Vertraege, db_to.Buchungen '
    select_stmt += 'WHERE db_to.Vertraege.Id = db_to.Buchungen.VertragsId '
    select_stmt += 'GROUP BY Vertraege.Id '
    select_stmt += 'ORDER BY Vertraege.Id '
    select_stmt += ') '
    select_stmt += 'WHERE EXISTS '
    select_stmt += '( '
    select_stmt += 'SELECT KennungB, SummeB '
    select_stmt += 'FROM '
    select_stmt += '( '
    select_stmt += 'SELECT db_from.DKBuchungen.DkNummer AS KennungB, SUM(Betrag) AS SummeB '
    select_stmt += 'FROM db_from.DKBuchungen '
    select_stmt += 'WHERE KennungA = KennungB '
    select_stmt += 'GROUP BY KennungB '
    select_stmt += 'HAVING SUM(Betrag) <> SummeA'
    select_stmt += ') '
    select_stmt += ') '
    stmt.execute(select_stmt)
    vertraege = stmt.fetchall()
    for row in vertraege:
        print row
    print "Anzahl: ", len(vertraege)
    
    if False:
        sys.exit(0)
    
    print "Vertraege mit abweichenden aufsummierten Zinsen:"

    # DKV2: vStat_aktiverVertraege_thesa

    # CREATE VIEW vStat_aktiverVertraege_thesa AS SELECT *, ROUND(100* Jahreszins/Wert,6) as gewMittel 
    # FROM (SELECT count(*) as Anzahl, SUM(Wert) as Wert, SUM(ROUND(Zinssatz *Wert /100,2)) AS Jahreszins,
    # ROUND(AVG(Zinssatz),4) as mittlereRate FROM vVertraege_aktiv WHERE thesa)

    # CREATE VIEW vVertraege_aktiv AS SELECT id ,Kreditorin,Vertragskennung,Zinssatz,Wert,Aktivierungsdatum,Kuendigungsfrist
    # ,Vertragsende,thesa,KreditorId FROM vVertraege_aktiv_detail

    # CREATE VIEW vVertraege_aktiv_detail AS SELECT Vertraege.id AS id, Vertraege.Kennung AS Vertragskennung
    # , Vertraege.ZSatz /100. AS Zinssatz, SUM(Buchungen.Betrag) /100. AS Wert, MIN(Buchungen.Datum)  AS Aktivierungsdatum
    # , Vertraege.Kfrist AS Kuendigungsfrist, Vertraege.LaufzeitEnde  AS Vertragsende, Vertraege.thesaurierend AS thesa
    # , Kreditoren.Nachname || ', ' || Kreditoren.Vorname AS Kreditorin, Kreditoren.id AS KreditorId,Kreditoren.Nachname AS Nachname
    # , Kreditoren.Vorname AS Vorname,Kreditoren.Strasse AS Strasse
    # , Kreditoren.Plz AS Plz, Kreditoren.Stadt AS Stadt, Kreditoren.Email AS Email, Kreditoren.IBAN AS Iban, Kreditoren.BIC AS Bic
    # FROM Vertraege INNER JOIN Buchungen  ON Buchungen.VertragsId = Vertraege.id 
    # INNER JOIN Kreditoren ON Kreditoren.id = Vertraege.KreditorId GROUP BY Vertraege.id

    select_stmt = ''
    select_stmt += 'SELECT KennungA, WertA, ZinssatzA, ZinsenA '
    select_stmt += 'FROM '
    select_stmt += '( '
    select_stmt += 'SELECT db_to.Vertraege.Kennung AS KennungA, '
    # select_stmt += 'Vertraege.ZSatz AS ZSatz, '
    select_stmt += 'Vertraege.ZSatz /100. AS ZinssatzA, '
    select_stmt += '(SUM(Buchungen.Betrag) /100.) AS WertA, '
    select_stmt += 'ROUND( (( Vertraege.ZSatz /100.) * (SUM(Buchungen.Betrag) /100.)) /100,2) AS ZinsenA '
    # select_stmt += 'SUM((( ROUND((Vertraege.ZSatz /100.),2) * ((Buchungen.Betrag) /100. )) / 100.),2) AS ZinsenA '
    # select_stmt += 'ROUND((( ROUND((Vertraege.ZSatz /100.),2) * (SUM(Buchungen.Betrag) /100. )) / 100.),2) AS ZinsenA '
    # select_stmt += 'SUM((( ROUND((Vertraege.ZSatz /100.),2) * ((Buchungen.Betrag) /100. )) / 100.),2) AS ZinsenA '
    select_stmt += 'FROM db_to.Vertraege '
    select_stmt += 'INNER JOIN Buchungen ON Buchungen.VertragsId = Vertraege.id '
    select_stmt += 'INNER JOIN Kreditoren ON Kreditoren.id = db_to.Vertraege.KreditorId GROUP BY Vertraege.id'
    select_stmt += ') '
    select_stmt += 'WHERE EXISTS '
    select_stmt += '( '
    select_stmt += 'SELECT KennungB, WertB, ZinssatzB, ZinsenB '
    select_stmt += 'FROM '
    select_stmt += '( '
    select_stmt += 'SELECT db_from.DKBuchungen.DkNummer AS KennungB, Zinssatz AS ZinssatzB, SUM(Betrag) AS WertB, ROUND (SUM( (Betrag * Zinssatz) / 100.0 ), 2) AS ZinsenB '
    select_stmt += 'FROM db_from.DKBuchungen '
    select_stmt += 'WHERE KennungA = KennungB '
    select_stmt += 'GROUP BY KennungB '
    select_stmt += 'HAVING (ROUND (SUM( (Betrag * Zinssatz) / 100.0 ), 2)) <> ZinsenA'
    select_stmt += ') '
    select_stmt += ') '
    stmt.execute(select_stmt)
    vertraege = stmt.fetchall()
    for row in vertraege:
        print row
    print "Anzahl: ", len(vertraege)

    sys.exit(0)

except SystemExit:
    pass
except:
    print "Unexpected error:", sys.exc_info()[0]
    print sys.exc_info()

sys.exit(0)
