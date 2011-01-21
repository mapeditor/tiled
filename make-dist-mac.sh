#!/bin/bash
# This script generates a mac release from an already compiled Tiled.app in
# the bin folder. You should compile the release before running this:
# qmake -r -spec macx-g++ CONFIG+=release CONFIG+=x86 CONFIG+=x86_64
# make

# Get the version
if [ "$#" -eq "0" ]; then
    echo "Usage: make-dist-mac.sh <version>"
    exit 1
fi
name="tiled-qt-$1"

# Get various directories
baseDir=`dirname $01`
binDir="$baseDir/bin"

# Create a temporary staging directory
tempDir=`mktemp -d /tmp/${name}.XXXXXX` || exit 1
echo $tempDir

# Copy things to temp directory
cp "$baseDir/AUTHORS" "$tempDir/"
cp "$baseDir/COPYING" "$tempDir/"
cp "$baseDir/NEWS" "$tempDir/"
cp "$baseDir/README" "$tempDir/"
cp -R "$baseDir/examples" "$tempDir/"
cp -R "$binDir/Tiled.app" "$tempDir/"

# Create symlink to application directory
ln -s /Applications "$tempDir/Applications"

# Get various in-bundle directories
pluginsDir="$tempDir/Tiled.app/Contents/PlugIns"
macOSDir="$tempDir/Tiled.app/Contents/MacOS"
frameworksDir="$tempDir/Tiled.app/Contents/Frameworks"

# Use macdeployqt to copy Qt frameworks to the app
macdeployqt "$tempDir/Tiled.app"
qtCoreDir="$frameworksDir/QtCore.framework"
qtGuiDir="$frameworksDir/QtGui.framework"

# Modify plugins to use Qt frameworks contained within the app bundle (perhaps theres some way to get macdeployqt to do this?)
install_name_tool -change "QtCore.framework/Versions/4/QtCore" "@executable_path/../Frameworks/QtCore.framework/Versions/4/QtCore" "$frameworksDir/libtiled.dylib"
install_name_tool -change "QtCore.framework/Versions/4/QtCore" "@executable_path/../Frameworks/QtCore.framework/Versions/4/QtCore" "$pluginsDir/libtmw.dylib"
install_name_tool -change "QtCore.framework/Versions/4/QtCore" "@executable_path/../Frameworks/QtCore.framework/Versions/4/QtCore" "$pluginsDir/libtengine.dylib"
install_name_tool -change "QtGui.framework/Versions/4/QtGui" "@executable_path/../Frameworks/QtGui.framework/Versions/4/QtGui" "$frameworksDir/libtiled.dylib"
install_name_tool -change "QtGui.framework/Versions/4/QtGui" "@executable_path/../Frameworks/QtGui.framework/Versions/4/QtGui" "$pluginsDir/libtmw.dylib"
install_name_tool -change "QtGui.framework/Versions/4/QtGui" "@executable_path/../Frameworks/QtGui.framework/Versions/4/QtGui" "$pluginsDir/libtengine.dylib"

# Create dmg from the temp directory
hdiutil create "$baseDir/$name.dmg" -srcfolder "$tempDir" -volname "Tiled $1"

# Delete the temp directory
rm -rf "$tempDir"
