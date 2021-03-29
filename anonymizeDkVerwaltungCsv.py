#!/usr/bin/python
# coding: utf8

# -*- coding: utf-8 -*-

#
# Das Script anonymisiert die Daten der DkVerwaltung-Csv-Datei.
# 
# Aufruf: ./anonymizeDkVerwaltungCsv.py <DkVerwaltung-csv-source-file> [<DkVerwaltung-csv-target-file>]
#  

import os
import sys
import csv
# from faker import Faker
from collections import defaultdict

def anonymize_rows_xxx(rows):
    """
    Rows is an iterable of dictionaries that contain name and
    email fields that need to be anonymized.
    """
    # Load the faker and its providers
    faker  = Faker()
    # Create mappings of names & emails to faked names & emails.
    names  = defaultdict(faker.name)
    emails = defaultdict(faker.email)
    # Iterate over the rows and yield anonymized rows.
    for row in rows:
        # Replace the name and email fields with faked fields.
        row['name']  = names[row['name']]
        row['email'] = emails[row['email']]
        # Yield the row back to the caller
        yield row

    # with open(DkVerwaltung_csv_file, 'r') as f: # , newline=''
    #     reader = csv.DictReader(f)
    #     print reader.fieldnames
    #     # for row in reader:
    #     #     print row

def anonymize_rows(rows):
    """
    Rows is an iterable of dictionaries that contain name and
    email fields that need to be anonymized.
    """
    # Iterate over the rows and yield anonymized rows.
    # ['Datum', 'DKNr', 'Vorname', 'Name', 'Anrede', 'DKNummer', 'Stra\xc3\x9fe', 'PLZ', 'Ort', 'Email', 'Rueckzahlung', 'vorgemerkt', 'Betrag', 'Zinssatz', 'Bemerkung']
    # QString anonymizeDkPersonen = "UPDATE DkPersonen SET Vorname='Vorname_' || PersonId, Name='Name_' || PersonId, Anrede='Anrede_' || PersonId, Straße='Straße_' || PersonId, PLZ='PLZ_' || PersonId, Ort='Ort_' || PersonId, Email='Email_' || PersonId";
    count = 0
    for row in rows:
        count = count + 1
        row['Vorname']  = 'Vorname_' + str(count)
        row['Name']  = 'Name_' + str(count)
        row['Anrede']  = 'Anrede_' + str(count)
        row['DK Nummer']  = 'DK Nummer ' + str(count)
        row['Straße']  = 'Straße_' + str(count)
        row['PLZ']  = 'PLZ_' + str(count)
        row['Ort']  = 'Ort_' + str(count)
        row['Email'] = 'Email_' + str(count)
        row['Bemerkung'] = 'Bemerkung_' + str(count)
        # Yield the row back to the caller
        yield row

def anonymize(source, target):
    """
    The source argument is a path to a CSV file containing data to anonymize,
    while target is a path to write the anonymized CSV data to.
    """
    with open(source, 'r') as f:
        with open(target, 'w') as o:
            # Use the DictReader to easily extract fields
            reader = csv.DictReader(f)
            writer = csv.DictWriter(o, reader.fieldnames)
            writer.writeheader()
            # Read and anonymize data, writing to target file.
            for row in anonymize_rows(reader):
                writer.writerow(row)

if len(sys. argv) < 2:
    print "Aufruf: ./" + os.path.basename(__file__) + " <DkVerwaltung-csv-source-file> [<DkVerwaltung-csv-target-file>]"
    sys.exit(1)

DkVerwaltung_csv_source_file=sys.argv[1]
if len(sys. argv) < 3:
    DkVerwaltung_csv_target_file=DkVerwaltung_csv_source_file + ".anonymized.csv"
else:
    DkVerwaltung_csv_target_file=sys.argv[2]

if not os.path.isfile(DkVerwaltung_csv_source_file):
    print DkVerwaltung_csv_source_file + " existiert nicht."
    sys.exit(2)

anonymize(DkVerwaltung_csv_source_file, DkVerwaltung_csv_target_file)
