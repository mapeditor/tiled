#!/bin/bash
# This script generates a mac release from an already compiled Tiled.app in
# the bin folder. You should compile the release before running this:
# tiled$ qmake -r -spec macx-g++ CONFIG+=release CONFIG+=x86_64
# tiled$ make
# tiled$ ./dist/make-dist-mac.sh version

# Get the version
if [ "$#" -eq "0" ]; then
    echo "Usage: make-dist-mac.sh <version>"
    exit 1
fi
name="tiled-qt-$1"

# Get various directories
baseDir=`dirname $01`/..
binDir="$baseDir/bin"

# Create a temporary staging directory
echo Creating staging directory
tempDir=`mktemp -d /tmp/${name}.XXXXXX` || exit 1
echo

# Copy things to temp directory
echo Copying files
cp "$baseDir/AUTHORS" "$tempDir/"
cp "$baseDir/COPYING" "$tempDir/"
cp "$baseDir/NEWS" "$tempDir/"
cp "$baseDir/README.md" "$tempDir/"
cp -R "$baseDir/examples" "$tempDir/"
cp -R "$binDir/Tiled.app" "$tempDir/"
ln -s /Applications "$tempDir/Applications" #Symlink to Applciations for easy install
cp "$baseDir/src/tiled/images/tmx-icon-mac.icns" "$tempDir/Tiled.app/Contents/Resources/"
echo

# Get various in-bundle directories
pluginsDir="$tempDir/Tiled.app/Contents/PlugIns"
macOSDir="$tempDir/Tiled.app/Contents/MacOS"
frameworksDir="$tempDir/Tiled.app/Contents/Frameworks"

# Use macdeployqt to copy Qt frameworks to the app
echo Running macdeployqt
macdeployqt "$tempDir/Tiled.app"
echo

# Modify plugins to use Qt frameworks contained within the app bundle (is there some way to get macdeployqt to do this?)
echo Fixing plugins
for plugin in $(echo $(ls $frameworksDir/*.dylib) $(ls $pluginsDir/*.dylib) )
do
  pluginname=`basename $plugin`
  echo "  Fixing $pluginname"
  install_name_tool -change "$QTDIR"lib/QtCore.framework/Versions/4/QtCore "@executable_path/../Frameworks/QtCore.framework/Versions/4/QtCore" "$plugin"
  install_name_tool -change "$QTDIR"lib/QtGui.framework/Versions/4/QtGui "@executable_path/../Frameworks/QtGui.framework/Versions/4/QtGui" "$plugin"
done
echo

# Create dmg from the temp directory
echo Creating dmg file
hdiutil create "$baseDir/$name.dmg" -srcfolder "$tempDir" -volname "Tiled $1"
echo

# Delete the temp directory
echo Removing staging directory $tempDir 
rm -rf "$tempDir"
echo

# Exit
echo Done
