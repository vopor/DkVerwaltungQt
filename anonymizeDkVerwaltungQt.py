#!/usr/bin/python
# coding: utf8

# -*- coding: utf-8 -*-

#
# Das Script anonymisiert die Daten der Datenbank aus DkVerwaltungQt.
# 
# Aufruf: ./anonymizeDkVerwaltungQt.py <DkVerwaltungQt-db3-file>
#

import os
import sys
import sqlite3
# print sqlite3.version
# print sys.argv
if len(sys. argv) < 2:
    print "Aufruf: ./anonymizeDkVerwaltungQt.py <DkVerwaltungQt-db3-file>"
    exit(1)

DkVerwaltungQt_db3_file=sys.argv[1]
if not os.path.isfile(DkVerwaltungQt_db3_file):
    print DkVerwaltungQt_db3_file + "existiert nicht."
    exit(2)

try:
    conn = sqlite3.connect(DkVerwaltungQt_db3_file)
    stmt = conn.cursor()
    update_stmt = "UPDATE DkPersonen SET Vorname='Vorname_' || PersonId, Name='Name_' || PersonId, Anrede='Anrede_' || PersonId, Straße='Straße_' || PersonId, PLZ='PLZ_' || PersonId, Ort='Ort_' || PersonId, Email='Email_' || PersonId"
    stmt.execute(update_stmt);
    update_stmt = "UPDATE DkBuchungen SET Bemerkung='Bemerkung_' || BuchungId"
    stmt.execute(update_stmt);
    conn.commit()
except:
    print "Unexpected error:", sys.exc_info()[0]
    print sys.exc_info()

exit(0)
