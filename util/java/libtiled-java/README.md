# libtiled - Java library to work with Tiled maps
[![Maven Central Version](https://img.shields.io/maven-central/v/org.mapeditor/libtiled?label=libtiled-java)](https://central.sonatype.com/artifact/org.mapeditor/libtiled)

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

## Excluding unused dependencies

libtiled depends on [zstd-jni](https://github.com/luben/zstd-jni) to read and
write zstd-compressed layer data and on
[jsvg](https://github.com/weisJ/jsvg) to render SVG tilesets. If your project
uses neither, you can exclude them to reduce its size:

```xml
    <dependency>
      <groupId>org.mapeditor</groupId>
      <artifactId>libtiled</artifactId>
      <version>x.y.z</version>
      <exclusions>
        <exclusion>
          <groupId>com.github.luben</groupId>
          <artifactId>zstd-jni</artifactId>
        </exclusion>
        <exclusion>
          <groupId>com.github.weisj</groupId>
          <artifactId>jsvg</artifactId>
        </exclusion>
      </exclusions>
    </dependency>
```

Loading a zstd-compressed map or an SVG tileset without the respective
dependency fails with an error naming the missing dependency.

## Building

To make libtiled.jar, install [Apache Maven](http://maven.apache.org/) and run the following command:

    mvn clean install

To run all unit-tests, run the following command:

    mvn test -P release-profile

To generate reports and documentation, run:

    mvn site
