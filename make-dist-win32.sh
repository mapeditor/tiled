#!/bin/bash
# This script generates a Windows release from an already compiled Tiled
# A "release" build is assumed (release dlls are copied)

SOURCEDIR="."
BUILDDIR="../tiled-build-desktop"
QTSDKDIR="/c/QtSDK"
QTDIR="$QTSDKDIR/Desktop/Qt/4.7.1/mingw"
MINGWDIR="$QTSDKDIR/mingw"
DESTDIR="../tiled-release"
V="" #"-v"

echo "Copying files from the source directory..."
mkdir -p $V $DESTDIR
cp $V $SOURCEDIR/COPYING $DESTDIR/COPYING.txt
cp $V $SOURCEDIR/AUTHORS $DESTDIR/AUTHORS.txt
cp $V $SOURCEDIR/README $DESTDIR/README.txt
cp $V $SOURCEDIR/NEWS $DESTDIR/NEWS.txt
cp $V $SOURCEDIR/LICENSE.BSD $DESTDIR/LICENSE.BSD.txt
cp $V $SOURCEDIR/LICENSE.GPL $DESTDIR/LICENSE.GPL.txt
cp $V $SOURCEDIR/LICENSE.LGPL $DESTDIR/LICENSE.LGPL.txt
cp -r $V $SOURCEDIR/examples $DESTDIR

mkdir -p $V $DESTDIR/docs
cp $V $SOURCEDIR/docs/map.{dtd,xsd} $DESTDIR/docs

cp -r $V $SOURCEDIR/util $DESTDIR
rm $V $DESTDIR/util/java/.gitignore
rm $V $DESTDIR/util/java/tmxviewer-java/.gitignore
rm $V $DESTDIR/util/java/libtiled-java/.gitignore
mv $V $DESTDIR/util/java/libtiled-java/README{,.txt}
mv $V $DESTDIR/util/java/tmxviewer-java/README{,.txt}


echo "Copying files from the build directory..."
cp $V $BUILDDIR/tiled.dll $DESTDIR
cp $V $BUILDDIR/tiled.exe $DESTDIR
cp $V $BUILDDIR/tmxviewer.exe $DESTDIR

mkdir -p $V $DESTDIR/plugins/tiled
cp $V $BUILDDIR/plugins/tiled/*.dll $DESTDIR/plugins/tiled

mkdir -p $V $DESTDIR/translations
cp $V $BUILDDIR/translations/*.qm $DESTDIR/translations


echo "Copying files from the Qt SDK..."
cp $V $MINGWDIR/bin/mingwm10.dll $DESTDIR
cp $V $MINGWDIR/bin/libgcc_s_dw2-1.dll $DESTDIR
cp $V $QTDIR/bin/QtCore4.dll $DESTDIR
cp $V $QTDIR/bin/QtGui4.dll $DESTDIR
cp $V $QTDIR/bin/QtOpenGL4.dll $DESTDIR

mkdir -p $V $DESTDIR/plugins/codecs
cp $V $QTDIR/plugins/codecs/qcncodecs4.dll $DESTDIR/plugins/codecs
cp $V $QTDIR/plugins/codecs/qjpcodecs4.dll $DESTDIR/plugins/codecs
cp $V $QTDIR/plugins/codecs/qkrcodecs4.dll $DESTDIR/plugins/codecs
cp $V $QTDIR/plugins/codecs/qtwcodecs4.dll $DESTDIR/plugins/codecs

mkdir -p $V $DESTDIR/plugins/imageformats
cp $V $QTDIR/plugins/imageformats/qgif4.dll $DESTDIR/plugins/imageformats
cp $V $QTDIR/plugins/imageformats/qjpeg4.dll $DESTDIR/plugins/imageformats
cp $V $QTDIR/plugins/imageformats/qtiff4.dll $DESTDIR/plugins/imageformats

cp $V $QTDIR/translations/qt_cs.qm $DESTDIR/translations
cp $V $QTDIR/translations/qt_de.qm $DESTDIR/translations
cp $V $QTDIR/translations/qt_es.qm $DESTDIR/translations
cp $V $QTDIR/translations/qt_fr.qm $DESTDIR/translations
cp $V $QTDIR/translations/qt_he.qm $DESTDIR/translations
cp $V $QTDIR/translations/qt_ja.qm $DESTDIR/translations
cp $V $QTDIR/translations/qt_pt.qm $DESTDIR/translations
cp $V $QTDIR/translations/qt_zh_CN.qm $DESTDIR/translations
cp $V $QTDIR/translations/qt_zh_TW.qm $DESTDIR/translations

echo "[Paths]
Plugins = plugins
Translations = translations" > $DESTDIR/qt.conf


# TODO: Zip it up
