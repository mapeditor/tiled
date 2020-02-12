# Java TMX Viewer example

This is a simple example application showing how to use libtiled-java to load
and view a Tiled map file.

## Opening the project in a modern IDE

Just import this project as a `Maven project`, specifying the project `pom.xml` 
file location.

## Trying it out from the command line

The tmxviewer-java needs libtiled-java to be compiled first to add libtiled
dependency to your local Maven repository.

The [Apache Maven](https://maven.apache.org/) build tool is used to build from the command line.

* Run `mvn clean install` in the libtiled-java directory to build libtiled.jar
* Go to the `tmxviewer-java` directory and run `mvn clean install` there to compile it
* Launch the `TMX Viewer` using `java -jar tmxviewer-<Version>.jar [file]`
