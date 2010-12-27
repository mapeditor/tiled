Java TMX Viewer example
=======================

This is a simple example application showing how to use libtiled-java to load
and view a Tiled map file.

Trying it out in IntelliJ IDEA
------------------------------

Open tiled-java.ipr in IntelliJ IDEA and press Run to launch it. It will popup
a dialog with a field to enter parameters. The map you want to view should be
entered there.

Trying it out from the command line
-----------------------------------

The tmxviewer-java needs libtiled-java to be compiled first, and its .jar file
needs to be in the CLASSPATH both during compiling and running.

The Apache 'ant' build tool is used to build from the command line.

* Run 'ant' in the tmxviewer-java directory to provide libtiled-java.jar
* Run 'export CLASSPATH=$PWD/libtiled-java.jar' afterwards to add the jar file
  to the CLASSPATH
* Go to the tmxviewer-java directory and run 'ant' there to compile it
* Run 'export CLASSPATH=build:$CLASSPATH' to add the 'build' directory to the
  CLASSPATH along with libtiled-java.jar
* Launch the TMX Viewer using 'java TMXViewer [file]'
