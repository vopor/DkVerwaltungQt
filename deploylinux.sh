#!/bin/bash

set -x

APPNAME=DkVerwaltungQt
VERSION=1.0.0
ARCH=x86_64

export APPNAME="$APPNAME"
export VERSION="$VERSION"
export ARCH="$ARCH"

function usage()
{
    echo 
    echo "usage: deploylinux.sh <path to out_dir> <QTDIR>"
    echo 
}

SRC_DIR=`dirname "$0"`
OUT_DIR=$1
if [ -z $1 ]; then
   usage
   exit 1
fi
echo SRC_DIR=$SRC_DIR
echo OUT_DIR=$OUT_DIR

QT_DIR=$2
if [ -z $2 ]; then
   usage
   exit 2
fi

export QTDIR="$QT_DIR"
export QT_PLUGIN_PATH="$QT_DIR/plugins"
export LD_LIBRARY_PATH="$QT_DIR/lib:$LD_LIBRARY_PATH"

echo QTDIR="$QTDIR"
echo QT_PLUGIN_PATH="$QT_PLUGIN_PATH"
echo LD_LIBRARY_PATH="$LD_LIBRARY_PATH"

LINUX_DEPLOY_QT="/opt/linuxdeployqt-7/squashfs-root/AppRun"
# APPIMAGETOOL="/opt/appimagetool-12/squashfs-root/AppRun "
APPIMAGETOOL="/opt/linuxdeployqt-7/squashfs-root/usr/bin/appimagetool"

echo starting linux deploy...

rm -fr "$OUT_DIR"/appdir

mkdir -p "$OUT_DIR"/appdir
cp "$SRC_DIR"/$APPNAME.png "$OUT_DIR"/appdir/
cp "$SRC_DIR"/$APPNAME.desktop "$OUT_DIR"/appdir/
cp "$OUT_DIR"/$APPNAME "$OUT_DIR"/appdir/

# mkdir -p "$OUT_DIR"/appdir/usr/bin
# cp "$OUT_DIR"/"$APPNAME" "$OUT_DIR"/appdir/usr/bin/ 

cp "$SRC_DIR"/Jahreskontoauszug.html "$OUT_DIR"/appdir/ # usr/bin/ 
cp "$SRC_DIR"/Zinsbescheinigung.html "$OUT_DIR"/appdir/ # usr/bin/  
cp "$SRC_DIR"/F13TurleyGmbH2.gif "$OUT_DIR"/appdir/ # usr/bin/ 
cp "$SRC_DIR"/mail-content.txt "$OUT_DIR"/appdir/ # usr/bin/ 

cp -r "$SRC_DIR"/guac-gui/root "$OUT_DIR"/appdir/

# mkdir -p "$OUT_DIR"/appdir/usr/share/applications
# cp "$SRC_DIR"/$APPNAME.desktop "$OUT_DIR"/appdir/usr/share/applications/ 

# mkdir -p "$OUT_DIR"/appdir/usr/share/metainfo
# cp "$SRC_DIR"/$APPNAME.desktop.appdata.xml "$OUT_DIR"/appdir/usr/share/metainfo/ 

# mkdir -p "$OUT_DIR"/appdir/usr/share/icons/hicolor/scalable/apps 
# cp "$SRC_DIR"/$APPNAME.svg "$OUT_DIR"/appdir/usr/share/icons/hicolor/scalable/apps/ 

# "$OUT_DIR"/appdir/usr/share/applications/$APPNAME.desktop
# $LINUX_DEPLOY_QT "$OUT_DIR"/appdir/"$APPNAME" -appimage -unsupported-allow-new-glibc  -extra-plugins=platforms/

LINUX_DEPLOY_QT_PARAMS="$LINUX_DEPLOY_QT_PARAMS -bundle-non-qt-libs" 
# LINUX_DEPLOY_QT_PARAMS="$LINUX_DEPLOY_QT_PARAMS -verbose=3" 
LINUX_DEPLOY_QT_PARAMS="$LINUX_DEPLOY_QT_PARAMS -extra-plugins=platforms/"
# LINUX_DEPLOY_QT_PARAMS="$LINUX_DEPLOY_QT_PARAMS -executable=<additionsl executable>"
LINUX_DEPLOY_QT_PARAMS="$LINUX_DEPLOY_QT_PARAMS -unsupported-allow-new-glibc"
$LINUX_DEPLOY_QT "$OUT_DIR"/appdir/"$APPNAME" $LINUX_DEPLOY_QT_PARAMS -appimage

# chmod -R "u+rw,g+rw,o+r" "$OUT_DIR"/appdir/

pushd "$OUT_DIR"

$APPIMAGETOOL "$OUT_DIR"/appdir

popd

echo ...finished linux deploy

