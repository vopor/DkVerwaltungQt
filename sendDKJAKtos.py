#!/usr/bin/python
# from:
# http://nathangrigg.net/2012/04/send-emails-from-the-command-line/
# help:
# ./mailapp.py --help
# example:
# ls | python mailapp.py -s "Here's how my home directory looks"
# ls | python mailapp.py -s "Here's how my home directory looks"

import sys
import argparse
#import os.path
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
    if send:
        make_new.append('send')
    if len(make_new) > 0:
        make_new_string = "tell result\n" + "\n".join(make_new) + "\nend tell\n"
    else:
        make_new_string = ""

    # set_html_content_string = "set html content to textBody" # -- must set the HTML CONTENT rather than the CONTENT
    set_html_content_string = ""

    script = """tell application "Mail"
    make new outgoing message with properties {%s}
    %s
    %s end tell
    """ % (properties_string, set_html_content_string, make_new_string)

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
    sys.exit(code)

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
    
##!/usr/bin/python

#import os
#import sys
#import shutil

ss = "@"
subdir = os.getcwd() # + "/16-01-01 Export"
for i in os.listdir(subdir):
    if i.endswith(".pdf"):
        if ss in i:
            # if not (("fischer-stefan@live.de" in i) or (("hapeneumann@googlemail.com" in i))):
            # if not ("VPorzelt@gmx.de" in i):
            # if not (("fischer-stefan@live.de" in i) or (("hapeneumann@googlemail.com" in i))):
            if not (("fischer-stefan@live.de" in i)):
                continue
            filename = i
            textfilename = i
            textfilename = textfilename.replace(".pdf", ".txt")
            words = i.split("_")
            for j, word in enumerate(words):
                if ss in word:
                    email = word
                    newfilename = "_".join(words[j+1:])
                    newfilename = newfilename.replace("0.pdf", ".pdf")
                    # if not os.path.exists("tmp"):
                    #     os.makedirs("tmp")
                    # newfilename = "tmp/" + newfilename
                    if debug_output:
                        print "email: " + email # evebeyer@gmx.net
                        print "filename: " + filename # 0010_evebeyer@gmx.net_Evelyn_Beyer0.pdf
                        print "textfilename: " + textfilename # 0010_evebeyer@gmx.net_Evelyn_Beyer0.txt
                        print "newfilename: " + newfilename
                        
                    # copy filename nach tmp-newfilename
                    shutil.copy2(filename, newfilename)
                    # make mail
                    file = open(textfilename, 'r')
                    content = file.read()
                    file.close()
                    newcontent = ""
                    newlines = []
                    lines = content.split("\n")
                    start = 0
                    for k, line in enumerate(lines):
                        # print str(k) + " " + lines[k]
                        if lines[k].startswith("Kontoauszug Direktkredit"):
                            start = k
                        if lines[k].startswith("Dies ist der Kontoauszug des Direktkredits mit der Nummer"):
                            # lines[k] = "Den Kontoauszug Deines Direktkredits findest Du als Pdf-Datei im Anhang."
                            newlines.extend(lines[start:k])
                            newlines.append("Den Kontoauszug Deines Direktkredits findest Du als Pdf-Datei im Anhang.")
                            "Den Kontoauszug Deines Direktkredits findest Du als Pdf-Datei im Anhang."
                            newlines.extend(lines[k+5:])
                            newlines.append("\n")
                            # newcontent = lines[:k-1].join("\n")
                            # newcontent += "Den Kontoauszug Deines Direktkredits findest Du als Pdf-Datei im Anhang."
                            # "Den Kontoauszug Deines Direktkredits findest Du als Pdf-Datei im Anhang."
                            # newcontent += lines[k+3:].join("\n")
                            break
                    # content = lines.join("\n")
                    newcontent = "\n".join(newlines)
                    # newcontent = newlines.join("\n")
                    # print newcontent
                    sendIt = True
                    if sendIt:
                        code = make_message(
                          content = newcontent,
                          subject = "Kontoauszug 2015 DK F13 Turley GmbH",
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
