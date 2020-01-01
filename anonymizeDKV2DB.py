#!/usr/bin/python
# coding: utf8

# -*- coding: utf-8 -*-

#
# Das Script anonymisiert die Daten der Datenbank aus DKV2.
# 
# Aufruf: ./anonymizeDKV2DB.py <DKV2-db3-file>
#

import os
import sys
import sqlite3
# print sqlite3.version
# print sys.argv
if len(sys. argv) < 2:
    print "Aufruf: ./anonymizeDKV2DB.py <DKV2-db3-file>"
    exit(1)

DKV2_db3_file=sys.argv[1]
if not os.path.isfile(DKV2_db3_file):
    print DKV2_db3_file + "existiert nicht."
    exit(2)

try:
    conn = sqlite3.connect(DKV2_db3_file)
    stmt = conn.cursor()
    # Kreditoren
    update_stmt =  'UPDATE Kreditoren '
    update_stmt += 'SET Vorname="Vorname_" || id, Nachname="Nachname_" || id, Strasse="Strasse_" || id, EMail="EMail_" || id'
    stmt.execute(update_stmt);
    conn.commit()
except:
    print "Unexpected error:", sys.exc_info()[0]
    print sys.exc_info()

exit(0)
