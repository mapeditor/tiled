# KArchive

Reading, creating, and manipulating file archives

## Introduction

KArchive provides classes for easy reading, creation and manipulation of
"archive" formats like ZIP and TAR.

It also provides transparent compression and decompression of data, like the
GZip format, via a subclass of QIODevice.

## Usage

If you want to read and write compressed data, just create an instance of
KCompressionDevice and write to or read from that.

If you want to read and write archive formats, create an instance of the
appropriate subclass of KArchive (eg: K7Zip for 7-Zip files).  You may need to
combine this with usage of KCompressionDevice (see the API documentation for the
relevant KArchive subclass for details).

