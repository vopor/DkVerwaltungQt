#!/bin/sh
appname=`basename $0 | sed s,\.sh$,,`

dirname=`dirname $0`
tmp="${dirname#?}"

if [ "${dirname%$tmp}" != "/" ]; then
dirname=$PWD/$dirname
fi
LD_LIBRARY_PATH=$dirname

echo QTDIR="${QTDIR}"

# Qt libs
LD_LIBRARY_PATH="$LD_LIBRARY_PATH":/$QTDIR/lib
DYLD_FRAMEWORK_PATH="${QTDIR}/lib"
DYLD_LIBRARY_PATH="${QTDIR}/lib",
QT_PLUGIN_PATH="${QTDIR}/plugins",
QT_TRANSLATION_PATH="${QTDIR}/translations",
# QT_QPA_PLATFORM_PLUGIN_PATH=
# QT_DEBUG_PLUGIN=1
# PATH="${QTDIR}/bin:${PATH}"

# exports
export LD_LIBRARY_PATH="$LD_LIBRARY_PATH"
export DYLD_FRAMEWORK_PATH="${QTDIR}/lib"
export DYLD_LIBRARY_PATH="${QTDIR}/lib",
export QT_PLUGIN_PATH="${QTDIR}/plugins",
export QT_TRANSLATION_PATH="${QTDIR}/translations",
# export QT_QPA_PLATFORM_PLUGIN_PATH=
# export QT_DEBUG_PLUGIN=1
# export PATH="${PATH}"

$dirname/$appname "$@"
