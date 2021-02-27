# DkVerwaltungQt

## Die Entwicklung des Programms ist eingestellt !!!
  
## Ich empfehle stattdessen DKV2:  
https://github.com/Schachigel/DKV2/  
  
Ich benutze das Programm nur dafür, die Kontoauszüge am Jahresende zu generieren und als Email zu verschicken.

Und ich verwende die Skripte CreateDkVerwaltungQt.sql und UpdateDkVerwaltungQt.sql, um die Daten aus dem OO-Programm aus Tübingen in eine SQLite-Datenbank umzuwandeln, um bestimmte SQL-Abfragen generieren zu können.

Für DKV2 bin ich dabei, einen Import aus dem OO-Programm aus Tübingen zu schreiben:  

Dies ist derzeit ein 2-stufiger Prozess:  

1) Umwandeln des OO-Journals in einer zu DkVerwaltungQt kompatiblen SQLite-Datenbank z.B. DkVerwaltungQt.db3:  

https://github.com/vopor/DkVerwaltungQt/blob/master/CreateDkVerwaltungQt.sql  
https://github.com/vopor/DkVerwaltungQt/blob/master/UpdateDkVerwaltungQt.sql  

2) Umwandeln dieser Datenbank in eine zu DKV2 kompatiblen Datenbank:  

https://github.com/vopor/DkVerwaltungQt/blob/master/exportToDKV2V2.py  
  

---
  
  
## Datenübernahme aus der Dk-Verwaltung von Volker aus Tübingen in das "DkVerwaltungQt"-Format 
  
Daten exportieren  
  
- Auf das Journal wechseln  
- Die Datei abspeichern als CSV mit dem Namen Dk-Verwaltung.csv  
- Als Feld-Trennzeichen , auswählen (default)  

Daten importieren  
  
- Daten aus Dk-Verwaltung.csv übernehmen:  
sqlite3 DkVerwaltungQt.db3 < CreateDkVerwaltungQt.sql  
Es werden die Tabellen DkPersonen und DkBuchungen und eine Tabelle DkZinssaetze mit den aktuell vergebenen Zinssätzen wird erstellt.  

- Buchungen, die zu derselben Person (gleicher Vorname und Nachname gehören) zusammenfassen:  
sqlite3 DkVerwaltungQt.db3 < UpdateDkVerwaltungQt.sql  
  
  
## Datenübernahme des "DkVerwaltungQt"-Formats nach DKV2
  
  

./exportToDKV2V2.py DkVerwaltungQt.db3 DKV2.db
  
  

  
## Tools (z.Zt. nur macOS)  
  

Habe ich benötigt, um die in Html-generierten Jahreszinsbescheinigungen in Pdf-Dokumente umzuwandeln und per Mail zu verschicken.  
Mit etwas Aufwand kann man diese Tools auch auf andere Plattformen portieren oder in Quelltext umwandeln.  
   
### url2pdf  
Url2pdf generiert aus einer html-Datei ein Pdf-Dokument.  
Man findet es im Internet z.B. hier:  
https://github.com/scottgarner/URL2PDF  
Ich habe es verwendet, um die in Html generierten Jahreszinsbescheinigungen in Pdf umzuwandeln, um sie anschliessend verschicken zu können.  
html2pdf.sh  
Bash-Script, um alle Html-Dateien in einem Verzeichnis in Pdf-Dokumente umzuwandeln.  

### sendDKJAKtos.py  
  

Python-Script, um alle PDF-Dokumente in einem Verzeichnis zu versenden.  
Die Lösung basiert auf Apple-Mail und Apple-Script.  
Die Idee habe ich aus dem Internet:   
http://nathangrigg.net/2012/04/send-emails-from-the-command-line/  
Voraussetzung ist, dass man die Pdf-Dokumente zuvor in einem bestimmten Format generiert hat:  
Email-Adresse_Vorname_Nachname.pdf  
 

## DkVerwaltungQt

### Installation  bzw. Erstellung  
  
Das Programm kann mit QtCreator und Qt 5.12.0 für Windows, Mac und Linux erstellt werden.  
Installation der OO-Bibliothek für Anschreiben  
Die OO-BIBLIOTHEK befindet sich in der Datei script.xlb.  
Diese muss in OpenOffice unter Extras – Makros – Makros verwalten – OpenOffice Basic...  
MyMacros bzw. Meine Makros hinzugefügt werden, damit später aus dem Programm heraus Anschreiben an die Kreditgeber vorausgefüllt werden können.  
Es ist geplant, die Vorlage auch auf Html umzustellen, sodass man sich die Macro-Verwaltung in OO sparen kann.  
#### DkVerwaltungQt.ini  
In der Datei DkVerwaltungQt.ini im Programmverzeichnis werden für jeden Benutzer/Computer Konfigurationsdaten abgelegt wie z.B. der Pfad zur Datenbank (DkVerwaltungQt.db3), der Pfad zu OpenOffice, …  
Diese Daten werden zur Laufzeit abgefragt, wenn sie noch nicht hinterlegt sind.  
  
