#!/usr/bin/ruby
# Modifies plugins and other binaries to use Qt frameworks contained within the app bundle (is there some way to get macdeployqt to do this?)

binAppDir = 'Tiled.app'
raise "No application at #{binAppDir}" unless File.directory? binAppDir

# Modify plugins to use Qt frameworks contained within the app bundle (is there some way to get macdeployqt to do this?)
Dir["Tiled.app/**/*.dylib",
    "Tiled.app/Contents/MacOS/tmxrasterizer",
    "Tiled.app/Contents/MacOS/terraingenerator"].each do |library|
    ["QtCore", "QtGui", "QtWidgets", "QtNetwork", "QtQml"].each do |qtlib|
        #find any qt dependencies within this library
        qtdependency = `otool -L "#{library}" | grep #{qtlib}`.split(' ')[0]
        next unless qtdependency

        #skip depedencies that are already using relative paths
        #macdeployqt seems to fix some of the plugins
        next if qtdependency.include? "@executable_path"

        #if we get here, this library has a dependency on a qtlib with a hard path on the build systems disk
        puts "Fixing #{library} dependency on #{qtlib}"
        `install_name_tool -change "#{qtdependency}" "@executable_path/../Frameworks/#{qtlib}.framework/Versions/5/#{qtlib}" "#{library}"`
        raise "install_name_tool error #{$?}" unless $? == 0
    end
end

puts "Done"
