# libtiled - Java library to work with Tiled maps

This is a small library meant to make it easy to use Tiled maps in your Java
project. It is based on the Java version of Tiled.

This small library is BSD licensed. See the LICENSE.BSD file for details.

## Authors

* Adam Turk <aturk@biggeruniverse.com>
* Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
* Mike Thomas <mikepthomas@outlook.com>

https://www.mapeditor.org/

## Maven

Add into `<dependencies>` of your `pom.xml`:

```xml
    <dependency>
      <groupId>org.mapeditor</groupId>
      <artifactId>libtiled</artifactId>
      <version>x.y.z</version>
    </dependency>
```

Consult the Maven repository for the latest [org.mapeditor/libtiled](https://mvnrepository.com/artifact/org.mapeditor/libtiled) version.

## sbt

Add the following to your `build.sbt`:

```
libraryDependencies += "org.mapeditor" % "libtiled" % "x.y.z"
```

## Building

To make libtiled.jar, install [Apache Maven](http://maven.apache.org/) and run the following command:

    mvn clean install

To run all unit-tests, run the following command:

    mvn test -P release-profile

To generate reports and documentation, run:

    mvn site
