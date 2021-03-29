######################################################################
# Automatically generated by qmake (3.0) ?? 2? 21 22:25:40 2013
######################################################################

TEMPLATE = app
TARGET = DkVerwaltungQt
DEPENDPATH += .
INCLUDEPATH += .

win32:CONFIG -= debug_and_release

APP_RESOURCES_FILES.files = ./Jahreskontoauszug.html ./Zinsbescheinigung.html ./F13TurleyGmbH2.gif ./sendDKJAKtos.py ./printCommandDescription.sh ./mail-content.txt
APP_RESOURCES_FILES.path = Contents/Resources

QMAKE_BUNDLE_DATA += APP_RESOURCES_FILES

# macx:ICON = $${TARGET}.icns
# macx:QMAKE_INFO_PLIST = Info.plist

CONFIG( release, debug|release ){
   # Release verwendet kein shadow-build, deshalb muss man den macdeployqt Aufruf nicht anpassen
   macx:QMAKE_POST_LINK += $$[QT_INSTALL_BINS]/macdeployqt $${TARGET}.app -dmg
}else{
   # Debug verwendet shadow-build, deshalb müsste man den macdeployqt Aufruf anpassen
   # macx:QMAKE_POST_LINK += $$[QT_INSTALL_BINS]/macdeployqt $${TARGET}.app -no-strip # -use-debug-libs
}


QMAKE_POST_LINK = echo starting post link steps...

QMAKE_POST_LINK +=  && set -x
QMAKE_POST_LINK +=  && rm -fr $${OUT_PWD}/appdir

COPY=cp
win32:COPY=copy
QMAKE_POST_LINK +=  && $${COPY} $${PWD}/Jahreskontoauszug.html $${OUT_PWD}/
QMAKE_POST_LINK +=  && $${COPY} $${PWD}/Zinsbescheinigung.html $${OUT_PWD}/
QMAKE_POST_LINK +=  && $${COPY} $${PWD}/F13TurleyGmbH2.gif $${OUT_PWD}/
QMAKE_POST_LINK +=  && $${COPY} $${PWD}/mail-content.txt $${OUT_PWD}/

macx:QMAKE_POST_LINK +=  && $${COPY} $${PWD}/sendDKJAKtos.py $${OUT_PWD}/
macx:QMAKE_POST_LINK +=  && $${COPY} $${PWD}/printCommandDescription.sh $${OUT_PWD}/

CONFIG( release, debug|release ){
   unix:!macx::QMAKE_POST_LINK +=  && $${PWD}/deploylinux.sh $${OUT_PWD}
}

QMAKE_POST_LINK += && echo finished post link steps...

# RESOURCES += $${TARGET}.qrc
# RC_FILE += $${TARGET}.rc

QT += widgets sql network
QT += webengine webenginewidgets printsupport

# Input
HEADERS += mainform.h \
    ansparrechner.h \
    html2pdfconverter.h \
    mainwindow.h \
    personform.h \
    buchungform.h \
    dbfkts.h \
    webenginepdfviewer.h

SOURCES += main.cpp \
    $$PWD/mainform.cpp \
    ansparrechner.cpp \
    html2pdfconverter.cpp \
    mainwindow.cpp \
    personform.cpp \
    buchungform.cpp \
    dbfkts.cpp \
    webenginepdfviewer.cpp
