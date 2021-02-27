
echo
echo "./html2pdf.sh"
echo
echo "html2pdf.sh wandelt alle html-Dokumente in pdf-Dateien um"
echo

echo
echo "./sendDKJAKtos.py [s]"
echo
echo "sendDKJAKtos.py versendet die pdf-Dateien mit Apple Mail"
echo "[s] öffnet die Dokumente in Apple Mail, sodass diese mit klick auf den Senden-Button verschickt werden können"
echo "wird [s] nicht angegeben, werden die Dokumente nur aufgelistet"
echo

osascript -e 'tell application "Terminal" to activate'

exit 0

echo PID=$$
echo PPID=$PPID

pid=$PPID
command="\"import Cocoa; x = Cocoa.NSRunningApplication.runningApplicationWithProcessIdentifier_((pid_t)${pid}); x.activateWithOptions_(Cocoa.NSApplicationActivateIgnoringOtherApps);\""
# echo /usr/bin/python -c "${command}"
/usr/bin/python -c "${command}"

exit 0
