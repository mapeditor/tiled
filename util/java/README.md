# Java library and sample project to view Tiled files

## Folders description

* `libtiled-java` - contains source code for a library that adds support of Tiled maps to your java project.
* `tmx-viewer-java` - sample project, that uses `libtiled-java` to view Tiled map files.

Every folder contains a `README.md` related to its contents.



## Packaging the project

Make sure you have properly installed [Apache Maven](https://maven.apache.org/) on your computer.

Open current folder in terminal (cmd, power shell, etc) and run the command:

    mvn package

As the command run has completed, check the following folders:

`/libtiled-java/target` and `tmx-viewer-java/target` for generated jar files.
