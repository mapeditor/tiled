#!/usr/bin/ruby
# This script generates a mac release from an already compiled Tiled.app in
# the bin folder. You should compile the release before running this:
# tiled$ qmake -r -spec macx-g++ CONFIG+=release CONFIG+=x86_64
# tiled$ make
# tiled$ ./dist/make-dist-mac.rb

# Dependencies
require 'tmpdir'

# Get various directories
baseDir = File.join File.dirname(__FILE__), '..'
binDir = File.join baseDir, 'bin'
binAppDir = File.join binDir, 'Tiled.app'
raise "No application at #{binAppDir}" unless File.directory? binAppDir

# Get the version from the info plist
if ARGV[0]
    version = ARGV[0]
else
    plistData = File.open(File.join(binAppDir, "Contents/Info.plist")).read
    version = plistData.match(/CFBundleVersion\<\/key>\s*\<string\>([\d\.]+)\<\/string\>/m)[1]
end
puts "Version is #{version}"

# Create a temporary staging directory
# This directory will serve as the content for the dmg file
Dir.mktmpdir do |tempDir|
    # Copy things to temp directory
    puts "Copying files"
    ['LICENSE.GPL', 'LICENSE.BSD', 'AUTHORS', 'COPYING', 'NEWS', 'README.md'].each do |file|
        FileUtils.cp File.join(baseDir, file), tempDir
    end
    FileUtils.cp_r File.join(baseDir, 'examples'), tempDir
    FileUtils.cp_r binAppDir, tempDir
    FileUtils.ln_s '/Applications', File.join(tempDir, 'Applications') #Symlink to Applications for easy install
    FileUtils.cp File.join(baseDir, 'src/tiled/images/tmx-icon-mac.icns'), File.join(tempDir, 'Tiled.app/Contents/Resources')

    # Use macdeployqt to copy Qt frameworks to the app
    puts "Running macdeployqt"
    `macdeployqt "#{tempDir}/Tiled.app"`
    raise "macdeployqt error #{$?}" unless $? == 0

    # Modify plugins to use Qt frameworks contained within the app bundle (is there some way to get macdeployqt to do this?)
    Dir["#{File.join tempDir, 'Tiled.app'}/**/*.dylib"].each do |library|
        ["QtCore", "QtGui"].each do |qtlib|
            #find any qt dependencies within this library
            qtdependency = `otool -L "#{library}" | grep #{qtlib}`.split(' ')[0]
            next unless qtdependency

            #skip depedencies that are already using relative paths
            #macdeployqt seems to fix some of the plugins
            next if qtdependency.include? "@executable_path"

            #if we get here, this library has a dependency on a qtlib with a hard path on the build systems disk
            puts "Fixing library #{library} dependency on #{qtlib}"
            `install_name_tool -change "#{qtdependency}" "@executable_path/../Frameworks/#{qtlib}.framework/Versions/4/#{qtlib}" "#{library}"`
            raise "install_name_tool error #{$?}" unless $? == 0
        end
    end

    # Create dmg from the temp directory
    dmgPath = File.join(baseDir, 'tiled-qt-' + version + '.dmg')
    puts "Creating dmg at #{dmgPath}"
    `hdiutil create "#{dmgPath}" -srcfolder "#{tempDir}" -volname "Tiled #{version}"`
    raise "hdiutil error #{$?}" unless $? == 0
end

puts "Done"
