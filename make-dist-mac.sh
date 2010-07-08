#!/bin/bash
# This script generates a mac release from an already
# compiled Tiled.app in the bin folder.
# TODO: look at replacing some of this with macdeployqt

# Get the version
if [ "$#" -eq "0" ]; then
    echo "Usage: make-dist-mac.sh <version>"
    exit 1
fi
name="tiled-qt-$1"

# Get various directories
qtFrameworkDir="/Library/Frameworks"
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
cp -R "$binDir/Tiled.app" "$tempDir/"

# Create symlink to application directory
ln -s /Applications "$tempDir/Applications"

# Get various in-bundle directories
pluginsDir="$tempDir/Tiled.app/Contents/PlugIns"
macOSDir="$tempDir/Tiled.app/Contents/MacOS"
frameworksDir="$tempDir/Tiled.app/Contents/Frameworks"

# Copy Qt frameworks to app bundle
cp -R "$qtFrameworkDir/QtCore.framework" "$frameworksDir/"
cp -R "$qtFrameworkDir/QtGui.framework" "$frameworksDir/"
qtCoreDir="$frameworksDir/QtCore.framework"
qtGuiDir="$frameworksDir/QtGui.framework"

# Delete debug and header files from Qt frameworks
rm -rf "$qtCoreDir/"*debug*
rm -rf "$qtGuiDir/"*debug*
rm -rf "$qtCoreDir/Versions/4/"*debug*
rm -rf "$qtGuiDir/Versions/4/"*debug*
rm -rf "$qtCoreDir/Versions/4/Headers"
rm -rf "$qtGuiDir/Versions/4/Headers"

# Modify Qt frameworks to have correct self identification names
install_name_tool -id "@executable_path/../Frameworks/QtCore.framework/Versions/4/QtCore" "$qtCoreDir/Versions/4/QtCore"
install_name_tool -id "@executable_path/../Frameworks/QtGui.framework/Versions/4/QtGui" "$qtGuiDir/Versions/4/QtGui"

# Modify all code to link in local Qt frameworks
install_name_tool -change "QtCore.framework/Versions/4/QtCore" "@executable_path/../Frameworks/QtCore.framework/Versions/4/QtCore" "$qtGuiDir/Versions/4/QtGui"
install_name_tool -change "QtCore.framework/Versions/4/QtCore" "@executable_path/../Frameworks/QtCore.framework/Versions/4/QtCore" "$macOSDir/Tiled"
install_name_tool -change "QtCore.framework/Versions/4/QtCore" "@executable_path/../Frameworks/QtCore.framework/Versions/4/QtCore" "$frameworksDir/libtiled.dylib"
install_name_tool -change "QtCore.framework/Versions/4/QtCore" "@executable_path/../Frameworks/QtCore.framework/Versions/4/QtCore" "$pluginsDir/libtmw.dylib"

install_name_tool -change "QtGui.framework/Versions/4/QtGui" "@executable_path/../Frameworks/QtGui.framework/Versions/4/QtGui" "$macOSDir/Tiled"
install_name_tool -change "QtGui.framework/Versions/4/QtGui" "@executable_path/../Frameworks/QtGui.framework/Versions/4/QtGui" "$frameworksDir/libtiled.dylib"
install_name_tool -change "QtGui.framework/Versions/4/QtGui" "@executable_path/../Frameworks/QtGui.framework/Versions/4/QtGui" "$pluginsDir/libtmw.dylib"

# Create dmg from the temp directory
hdiutil create "$baseDir/$name.dmg" -srcfolder "$tempDir" -volname "Tiled $1"

# Delete the temp directory
rm -rf "$tempDir"
