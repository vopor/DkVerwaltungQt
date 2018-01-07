#!/usr/bin/python
# coding: utf8

# -*- coding: utf-8 -*-

#
# Das Script versendet alle Pdf-Dateien im aktuellen Verzeichnis an die jeweiligen EMail-Empfänger mit Anschreiben.
# 
# Die EMail-Adresse und der Name des Empfängers müssen dabei in dem Namen der Pdf-Datei enthalten sein.
# <laufende Nummer>_<EMail-Adresse>_<Vorname>_<Nachname>
#
# Der Text für das Anschreiben befindet sich im Quelltext dieses Scripts (s.u.).
#
# Aufruf: ./sendDKJAtos.py
#

# original from:
# http://nathangrigg.net/2012/04/send-emails-from-the-command-line/
# help:
# ./mailapp.py --help
# example:
# ls | python mailapp.py -s "Here's how my home directory looks"

import sys
import argparse
from subprocess import Popen,PIPE
import os
import shutil

debug_output = False

def escape(s):
    """Escape backslashes and quotes to appease AppleScript"""
    s = s.replace("\\","\\\\")
    s = s.replace('"','\\"')
    return s

def make_message(content,subject=None,to_addr=None,from_addr=None,send=False,cc_addr=None,bcc_addr=None,attach=None):
    """Use applescript to create a mail message"""
    if send:
        properties = ["visible:false"]
    else:
        properties = ["visible:true"]

    # mailbox:"Drafts"
    # properties.append("mailbox:Drafts")
    # content = "content"

    if subject:
        properties.append('subject:"%s"' % escape(subject))
    if from_addr:
        properties.append('sender:"%s"' % escape(from_addr))
    if len(content) > 0:
        properties.append('content:"%s"' % escape(content))
    properties_string = ",".join(properties)

    template = 'make new %s with properties {%s:"%s"}'
    make_new = []
    if to_addr:
       # make_new.extend([template % ("to recipient","address",escape(addr)) for addr in to_addr])
       make_new.extend([template % ("to recipient","address",escape(to_addr))])
    if cc_addr:
        # make_new.extend([template % ("cc recipient","address",escape(addr)) for addr in cc_addr])
        make_new.extend([template % ("cc recipient","address",escape(cc_addr))])
    if bcc_addr:
        # make_new.extend([template % ("bcc recipient","address",escape(addr)) for addr in bcc_addr])
        make_new.extend([template % ("bcc recipient","address",escape(bcc_addr))])
    if attach:
        # make_new.extend([template % ("attachment","file name",escape(os.path.abspath(f))) for f in attach])
        make_new.extend([template % ("attachment","file name",escape(os.path.abspath(attach)))])

    # make_new.extend('at folder "Drafts")
    # make_new.extend(['at folder "%s"' % escape("Drafts")])
    # drafts_string = 'at folder "%s"' % escape("Drafts")
    # drafts_string = 'at folder id 2'
    if send:
        make_new.append('send')
        
    if len(make_new) > 0:
        make_new_string = "tell result\n" + "\n".join(make_new) + "\nend tell\n"
    else:
        make_new_string = ""

    # set_html_content_string = "set html content to textBody" # -- must set the HTML CONTENT rather than the CONTENT
    # set_html_content_string = ""
    # set_Drafts_string = escape("Drafts")
    script = """tell application "Mail"
    set newmessage to make new outgoing message with properties {%s}
    %s
    save newmessage
    end tell
    """ % (properties_string, make_new_string)

    if debug_output:
        print "script:\n"
        print script
        print "\n"

    # run applescript
    p = Popen('/usr/bin/osascript',stdin=PIPE,stdout=PIPE)
    p.communicate(script) # send script to stdin
    return p.returncode

def parse_arguments():
    parser = argparse.ArgumentParser(
    description="Create a new mail message using Mail.app")
    parser.add_argument('recipient',metavar="to-addr",nargs="*",
    help="message recipient(s)")
    parser.add_argument('-s',metavar="subject",help="message subject")
    parser.add_argument('-c',metavar="addr",nargs="+",
    help="carbon copy recipient(s)")
    parser.add_argument('-b',metavar="addr",nargs="+",
    help="blind carbon copy recipient(s)")
    parser.add_argument('-r',metavar="addr",help="from address")
    parser.add_argument('-a',metavar="file",nargs="+",
    help="attachment(s)")
    parser.add_argument('--input',metavar="file",help="Input file",
    type=argparse.FileType('r'),default=sys.stdin)
    parser.add_argument('--send',action="store_true",
    help="Send the message")
    return parser.parse_args()

# if __name__ == "__main__":
def false_Main():
    args = parse_arguments()
    content = args.input.read()
    code = make_message(
        content,
        subject = args.s,
        to_addr = args.recipient,
        from_addr = args.r,
        send = args.send,
        cc_addr = args.c,
        bcc_addr = args.b,
        attach = args.a)
 
def ask_to_continue():
    answer = input('Please indicate approval: [y/n]')
    if not answer or answer[0].lower() != 'y':
        print('You did not indicate approval')
        return False
    return True

def ask_to_continue2():
    msg = 'Shall I?'
    shall = raw_input("%s (y/N) " % msg).lower() == 'y'
    return shall
    
def getContent():
	return """
	die Bewohner*innen des Wohnprojekts 472 wünschen ein schönes neues Jahr und bedanken sich herzlich für die Unterstützung. 
	Dies ist der Kontoauszug Deiner/Ihrer Direktkredite für das Jahr 2017 bei der F13 Turley GmbH. 
	Die Zinsen wurden dem Direktkreditkonto gutgeschrieben. 
	Auf Wunsch erstellen wir eine gesonderte Zinsbescheinigung für die Steuererklärung. 
	Wir bitten um Überprüfung. Falls etwas nicht stimmt oder unverständlich ist, bitte einfach per E-Mail (Kontakt-DK@13hafreiheit) oder Telefon (01575-1183759) bei uns melden. 
	Wir erhoffen und wünschen uns auch im neuen Jahre Eure/Ihre Solidarität. 
	Denn für die weitere Umschuldung brauchen wir weiterhin Eure/Ihre Hilfe in Form von Direktkrediten. 
	Also empfehlt 13ha Freiheit weiter an Freund*innen, Bekannte und Verwandte. 

	Herzliche Grüße 



	Hans-Peter Neumann	
	AG Direktkredite 
	""".replace('\t', '')

# main

ss = "@"
subdir = os.getcwd() # + "/16-01-01 Export"
for i in os.listdir(subdir):
    if i.endswith(".pdf"):
        if ss in i:
            # if not (("fischer-stefan@live.de" in i) or (("hapeneumann@googlemail.com" in i))):
            # if not (("fischer-stefan@live.de" in i)):
            # if not (("hapeneumann@googlemail.com" in i)):
            # if not (("evebeyer@gmx.net" in i)):
            # if not ("VPorzelt@gmx.de" in i):
            #     continue
            filename = i
            textfilename = i
            textfilename = textfilename.replace(".pdf", ".txt")
            words = i.split("_")
            for j, word in enumerate(words):
                if ss in word:
                    # TODO: EMail ist so abgeschnitten, wenn sie "_" underlines enthält
                    email = word
                    name = " ".join(words[j+1:])
                    name = name.replace(".pdf", "")
                    newfilename = "_".join(words[j+1:])
                    newfilename = newfilename.replace("0.pdf", ".pdf")
                    if debug_output:
                        print "email: " + email # evebeyer@gmx.net
                        print "filename: " + filename # 0010_evebeyer@gmx.net_Evelyn_Beyer0.pdf
                        print "textfilename: " + textfilename # 0010_evebeyer@gmx.net_Evelyn_Beyer0.txt
                        print "newfilename: " + newfilename
                        
                    # copy filename nach tmp-newfilename
                    shutil.copy2(filename, newfilename)
                    newcontent = ""
                    newcontent += "\n"
                    newcontent += "Liebe*r " + name + "," + "\n"
                    newcontent += "\n"
                    # newcontent += getContent()
                    newcontent += "die Bewohner*innen des Wohnprojekts 472 wünschen ein schönes neues Jahr und bedanken sich herzlich für die Unterstützung." + "\n"
                    newcontent += "Dies ist der Kontoauszug Deiner/Ihrer Direktkredite für das Jahr 2017 bei der F13 Turley GmbH." + "\n"
                    newcontent += "Die Zinsen wurden dem Direktkreditkonto gutgeschrieben." + "\n"
                    newcontent += "Auf Wunsch erstellen wir eine gesonderte Zinsbescheinigung für die Steuererklärung." + "\n"
                    newcontent += "Wir bitten um Überprüfung. Falls etwas nicht stimmt oder unverständlich ist, bitte einfach per E-Mail (Kontakt-DK@13hafreiheit) oder Telefon (01575-1183759) bei uns melden." + "\n"
                    newcontent += "Wir erhoffen und wünschen uns auch im neuen Jahre Eure/Ihre Solidarität." + "\n"
                    newcontent += "Denn für die weitere Umschuldung brauchen wir weiterhin Eure/Ihre Hilfe in Form von Direktkrediten." + "\n"
                    newcontent += "Also empfehlt 13ha Freiheit weiter an Freund*innen, Bekannte und Verwandte." + "\n"
                    newcontent += "\n"
                    newcontent += "Herzliche Grüße" + "\n" 
                    newcontent += "\n"
                    newcontent += "\n"
                    newcontent += "\n"
                    newcontent += "\n"
                    newcontent += "Hans-Peter Neumann" + "\n"
                    newcontent += "AG Direktkredite" + "\n"
                    newcontent += "\n"
                    sendIt = True
                    if sendIt:
                        code = make_message(
                          content = newcontent,
                          subject = "Kontoauszug 2017 DK F13 Turley GmbH",
                          to_addr = email,
                          # from_addr = args.r,
                          send = False,
                          # cc_addr = args.c,
                          # bcc_addr = args.b,
                          attach = newfilename)
                    # remove tmp-newfilename
                    os.remove(newfilename)
            if not ask_to_continue2():
                exit(1)
        else:
            print ss + " not in " + i
