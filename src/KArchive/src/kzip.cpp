/* This file is part of the KDE libraries
   SPDX-FileCopyrightText: 2000 David Faure <faure@kde.org>
   SPDX-FileCopyrightText: 2002 Holger Schroeder <holger-kde@holgis.net>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kzip.h"
#include "karchive_p.h"
#include "kfilterdev.h"
#include "klimitediodevice_p.h"
#include "loggingcategory.h"

#include <QHash>
#include <QByteArray>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QDate>
#include <QList>
#include <qplatformdefs.h>

#include <time.h>
#include <zlib.h>
#include <string.h>

#ifndef QT_STAT_LNK
#       define QT_STAT_LNK 0120000
#endif // QT_STAT_LNK

static const int max_path_len = 4095;   // maximum number of character a path may contain

static void transformToMsDos(const QDateTime &_dt, char *buffer)
{
    const QDateTime dt = _dt.isValid() ? _dt : QDateTime::currentDateTime();
    const quint16 time =
        (dt.time().hour() << 11)    // 5 bit hour
        | (dt.time().minute() << 5)   // 6 bit minute
        | (dt.time().second() >> 1);  // 5 bit double seconds

    buffer[0] = char(time);
    buffer[1] = char(time >> 8);

    const quint16 date =
        ((dt.date().year() - 1980) << 9) // 7 bit year 1980-based
        | (dt.date().month() << 5)           // 4 bit month
        | (dt.date().day());                 // 5 bit day

    buffer[2] = char(date);
    buffer[3] = char(date >> 8);
}

static uint transformFromMsDos(const char *buffer)
{
    quint16 time = (uchar)buffer[0] | ((uchar)buffer[1] << 8);
    int h = time >> 11;
    int m = (time & 0x7ff) >> 5;
    int s = (time & 0x1f) * 2;
    QTime qt(h, m, s);

    quint16 date = (uchar)buffer[2] | ((uchar)buffer[3] << 8);
    int y = (date >> 9) + 1980;
    int o = (date & 0x1ff) >> 5;
    int d = (date & 0x1f);
    QDate qd(y, o, d);

    QDateTime dt(qd, qt);
    return dt.toSecsSinceEpoch();
}

// == parsing routines for zip headers

/** all relevant information about parsing file information */
struct ParseFileInfo
{
    // file related info
    mode_t perm;            // permissions of this file
    // TODO: use quint32 instead of a uint?
    uint atime;         // last access time (UNIX format)
    uint mtime;         // modification time (UNIX format)
    uint ctime;         // creation time (UNIX format)
    int uid;            // user id (-1 if not specified)
    int gid;            // group id (-1 if not specified)
    QByteArray guessed_symlink; // guessed symlink target
    int extralen;           // length of extra field

    // parsing related info
    bool exttimestamp_seen; // true if extended timestamp extra field
    // has been parsed
    bool newinfounix_seen;  // true if Info-ZIP Unix New extra field has
    // been parsed

    ParseFileInfo()
        : perm(0100644)
        , uid(-1)
        , gid(-1)
        , extralen(0)
        , exttimestamp_seen(false)
        , newinfounix_seen(false)
    {
        ctime = mtime = atime = time(nullptr);
    }
};

/** updates the parse information with the given extended timestamp extra field.
 * @param buffer start content of buffer known to contain an extended
 *  timestamp extra field (without magic & size)
 * @param size size of field content (must not count magic and size entries)
 * @param islocal true if this is a local field, false if central
 * @param pfi ParseFileInfo object to be updated
 * @return true if processing was successful
 */
static bool parseExtTimestamp(const char *buffer, int size, bool islocal,
                              ParseFileInfo &pfi)
{
    if (size < 1) {
        //qCDebug(KArchiveLog) << "premature end of extended timestamp (#1)";
        return false;
    }/*end if*/
    int flags = *buffer;        // read flags
    buffer += 1;
    size -= 1;

    if (flags & 1) {        // contains modification time
        if (size < 4) {
            //qCDebug(KArchiveLog) << "premature end of extended timestamp (#2)";
            return false;
        }/*end if*/
        pfi.mtime = uint((uchar)buffer[0] | (uchar)buffer[1] << 8
                         | (uchar)buffer[2] << 16 | (uchar)buffer[3] << 24);
        buffer += 4;
        size -= 4;
    }/*end if*/
    // central extended field cannot contain more than the modification time
    // even if other flags are set
    if (!islocal) {
        pfi.exttimestamp_seen = true;
        return true;
    }/*end if*/

    if (flags & 2) {        // contains last access time
        if (size < 4) {
            //qCDebug(KArchiveLog) << "premature end of extended timestamp (#3)";
            return true;
        }/*end if*/
        pfi.atime = uint((uchar)buffer[0] | (uchar)buffer[1] << 8
                         | (uchar)buffer[2] << 16 | (uchar)buffer[3] << 24);
        buffer += 4;
        size -= 4;
    }/*end if*/

    if (flags & 4) {        // contains creation time
        if (size < 4) {
            //qCDebug(KArchiveLog) << "premature end of extended timestamp (#4)";
            return true;
        }/*end if*/
        pfi.ctime = uint((uchar)buffer[0] | (uchar)buffer[1] << 8
                         | (uchar)buffer[2] << 16 | (uchar)buffer[3] << 24);
        buffer += 4;
    }/*end if*/

    pfi.exttimestamp_seen = true;
    return true;
}

/** updates the parse information with the given Info-ZIP Unix old extra field.
 * @param buffer start of content of buffer known to contain an Info-ZIP
 *  Unix old extra field (without magic & size)
 * @param size size of field content (must not count magic and size entries)
 * @param islocal true if this is a local field, false if central
 * @param pfi ParseFileInfo object to be updated
 * @return true if processing was successful
 */
static bool parseInfoZipUnixOld(const char *buffer, int size, bool islocal,
                                ParseFileInfo &pfi)
{
    // spec mandates to omit this field if one of the newer fields are available
    if (pfi.exttimestamp_seen || pfi.newinfounix_seen) {
        return true;
    }

    if (size < 8) {
        //qCDebug(KArchiveLog) << "premature end of Info-ZIP unix extra field old";
        return false;
    }

    pfi.atime = uint((uchar)buffer[0] | (uchar)buffer[1] << 8
                     | (uchar)buffer[2] << 16 | (uchar)buffer[3] << 24);
    buffer += 4;
    pfi.mtime = uint((uchar)buffer[0] | (uchar)buffer[1] << 8
                     | (uchar)buffer[2] << 16 | (uchar)buffer[3] << 24);
    buffer += 4;
    if (islocal && size >= 12) {
        pfi.uid = (uchar)buffer[0] | (uchar)buffer[1] << 8;
        buffer += 2;
        pfi.gid = (uchar)buffer[0] | (uchar)buffer[1] << 8;
        buffer += 2;
    }/*end if*/
    return true;
}

#if 0 // not needed yet
/** updates the parse information with the given Info-ZIP Unix new extra field.
 * @param buffer start of content of buffer known to contain an Info-ZIP
 *      Unix new extra field (without magic & size)
 * @param size size of field content (must not count magic and size entries)
 * @param islocal true if this is a local field, false if central
 * @param pfi ParseFileInfo object to be updated
 * @return true if processing was successful
 */
static bool parseInfoZipUnixNew(const char *buffer, int size, bool islocal,
                                ParseFileInfo &pfi)
{
    if (!islocal) { // contains nothing in central field
        pfi.newinfounix = true;
        return true;
    }

    if (size < 4) {
        qCDebug(KArchiveLog) << "premature end of Info-ZIP unix extra field new";
        return false;
    }

    pfi.uid = (uchar)buffer[0] | (uchar)buffer[1] << 8;
    buffer += 2;
    pfi.gid = (uchar)buffer[0] | (uchar)buffer[1] << 8;
    buffer += 2;

    pfi.newinfounix = true;
    return true;
}
#endif

/**
 * parses the extra field
 * @param buffer start of buffer where the extra field is to be found
 * @param size size of the extra field
 * @param islocal true if this is part of a local header, false if of central
 * @param pfi ParseFileInfo object which to write the results into
 * @return true if parsing was successful
 */
static bool parseExtraField(const char *buffer, int size, bool islocal,
                            ParseFileInfo &pfi)
{
    // extra field in central directory doesn't contain useful data, so we
    // don't bother parsing it
    if (!islocal) {
        return true;
    }

    while (size >= 4) { // as long as a potential extra field can be read
        int magic = (uchar)buffer[0] | (uchar)buffer[1] << 8;
        buffer += 2;
        int fieldsize = (uchar)buffer[0] | (uchar)buffer[1] << 8;
        buffer += 2;
        size -= 4;

        if (fieldsize > size) {
            //qCDebug(KArchiveLog) << "fieldsize: " << fieldsize << " size: " << size;
            //qCDebug(KArchiveLog) << "premature end of extra fields reached";
            break;
        }

        switch (magic) {
        case 0x5455:        // extended timestamp
            if (!parseExtTimestamp(buffer, fieldsize, islocal, pfi)) {
                return false;
            }
            break;
        case 0x5855:        // old Info-ZIP unix extra field
            if (!parseInfoZipUnixOld(buffer, fieldsize, islocal, pfi)) {
                return false;
            }
            break;
#if 0   // not needed yet
        case 0x7855:        // new Info-ZIP unix extra field
            if (!parseInfoZipUnixNew(buffer, fieldsize, islocal, pfi)) {
                return false;
            }
            break;
#endif
        default:
            /* ignore everything else */
            ;
        }/*end switch*/

        buffer += fieldsize;
        size -= fieldsize;
    }/*wend*/
    return true;
}

/**
 * Checks if a token for a central or local header has been found and resets
 * the device to the begin of the token. If a token for the data descriptor is
 * found it is assumed there is a central or local header token starting right
 * behind the data descriptor, and the device is set accordingly to the begin
 * of that token.
 * To be called when a 'P' has been found.
 * @param buffer start of buffer with the 3 bytes behind 'P'
 * @param dev device that is read from
 * @param dataDescriptor only search for data descriptor
 * @return true if a local or central header begin is or could be reached
 */
static bool handlePossibleHeaderBegin(const char *buffer, QIODevice *dev, bool dataDescriptor)
{
    // we have to detect three magic tokens here:
    // PK34 for the next local header in case there is no data descriptor
    // PK12 for the central header in case there is no data descriptor
    // PK78 for the data descriptor in case it is following the compressed data
    // TODO: optimize using 32bit const data for comparison instead of byte-wise,
    // given we run at least on 32bit CPUs

    if (buffer[0] == 'K') {
        if (buffer[1] == 7 && buffer[2] == 8) {
            // data descriptor token found
            dev->seek(dev->pos() + 12); // skip the 'data_descriptor'
            return true;
        }

        if (!dataDescriptor && ((buffer[1] == 1 && buffer[2] == 2)
            || (buffer[1] == 3 && buffer[2] == 4))) {
            // central/local header token found
            dev->seek(dev->pos() - 4);
            // go back 4 bytes, so that the magic bytes can be found
            // in the next cycle...
            return true;
        }
    }
    return false;
}

/**
 * Reads the device forwards from the current pos until a token for a central or
 * local header has been found or is to be assumed.
 * @param dev device that is read from
 * @return true if a local or central header token could be reached, false on error
 */
static bool seekToNextHeaderToken(QIODevice *dev, bool dataDescriptor)
{
    bool headerTokenFound = false;
    char buffer[3];

    while (!headerTokenFound) {
        int n = dev->read(buffer, 1);
        if (n < 1) {
            //qCWarning(KArchiveLog) << "Invalid ZIP file. Unexpected end of file. (#2)";
            return false;
        }

        if (buffer[0] != 'P') {
            continue;
        }

        n = dev->read(buffer, 3);
        if (n < 3) {
            //qCWarning(KArchiveLog) << "Invalid ZIP file. Unexpected end of file. (#3)";
            return false;
        }

        if (handlePossibleHeaderBegin(buffer, dev, dataDescriptor)) {
            headerTokenFound = true;
        } else {
            for (int i = 0; i < 3; ++i) {
                if (buffer[i] == 'P') {
                    // We have another P character so we must go back a little to check if it is a magic
                    dev->seek(dev->pos() - 3 + i);
                    break;
                }
            }
        }
    }
    return true;
}

////////////////////////////////////////////////////////////////////////
/////////////////////////// KZip ///////////////////////////////////////
////////////////////////////////////////////////////////////////////////

class Q_DECL_HIDDEN KZip::KZipPrivate
{
public:
    KZipPrivate()
        : m_crc(0)
        , m_currentFile(nullptr)
        , m_currentDev(nullptr)
        , m_compression(8)
        , m_extraField(KZip::NoExtraField)
        , m_offset(0)
    {
    }

    unsigned long           m_crc;         // checksum
    KZipFileEntry          *m_currentFile; // file currently being written
    QIODevice              *m_currentDev;  // filterdev used to write to the above file
    QList<KZipFileEntry *> m_fileList;   // flat list of all files, for the index (saves a recursive method ;)
    int                     m_compression;
    KZip::ExtraField        m_extraField;
    // m_offset holds the offset of the place in the zip,
    // where new data can be appended. after openarchive it points to 0, when in
    // writeonly mode, or it points to the beginning of the central directory.
    // each call to writefile updates this value.
    quint64                 m_offset;
};

KZip::KZip(const QString &fileName)
    : KArchive(fileName)
    , d(new KZipPrivate)
{
}

KZip::KZip(QIODevice *dev)
    : KArchive(dev)
    , d(new KZipPrivate)
{
}

KZip::~KZip()
{
    //qCDebug(KArchiveLog) << this;
    if (isOpen()) {
        close();
    }
    delete d;
}

bool KZip::openArchive(QIODevice::OpenMode mode)
{
    //qCDebug(KArchiveLog);
    d->m_fileList.clear();

    if (mode == QIODevice::WriteOnly) {
        return true;
    }

    char buffer[47];

    // Check that it's a valid ZIP file
    // KArchive::open() opened the underlying device already.

    quint64 offset = 0; // holds offset, where we read
    // contains information gathered from the local file headers
    QHash<QByteArray, ParseFileInfo> pfi_map;

    QIODevice *dev = device();

    // We set a bool for knowing if we are allowed to skip the start of the file
    bool startOfFile = true;

    for (;;) { // repeat until 'end of entries' signature is reached
        //qCDebug(KArchiveLog) << "loop starts";
        //qCDebug(KArchiveLog) << "dev->pos() now : " << dev->pos();
        int n = dev->read(buffer, 4);

        if (n < 4) {
            setErrorString(tr("Invalid ZIP file. Unexpected end of file. (Error code: %1)").arg(1));
            return false;
        }

        if (!memcmp(buffer, "PK\5\6", 4)) { // 'end of entries'
            //qCDebug(KArchiveLog) << "PK56 found end of archive";
            startOfFile = false;
            break;
        }

        if (!memcmp(buffer, "PK\3\4", 4)) { // local file header
            //qCDebug(KArchiveLog) << "PK34 found local file header";
            startOfFile = false;
            // can this fail ???
            dev->seek(dev->pos() + 2); // skip 'version needed to extract'

            // read static header stuff
            n = dev->read(buffer, 24);
            if (n < 24) {
                setErrorString(tr("Invalid ZIP file. Unexpected end of file. (Error code: %1)").arg(4));
                return false;
            }

            int gpf = (uchar)buffer[0]; // "general purpose flag" not "general protection fault" ;-)
            int compression_mode = (uchar)buffer[2] | (uchar)buffer[3] << 8;
            uint mtime = transformFromMsDos(buffer + 4);

            const qint64 compr_size = uint(uchar(buffer[12])) | uint(uchar(buffer[13])) << 8 |
                                      uint(uchar(buffer[14])) << 16 | uint(uchar(buffer[15])) << 24;
            const qint64 uncomp_size = uint(uchar(buffer[16])) | uint(uchar(buffer[17])) << 8 |
                                       uint(uchar(buffer[18])) << 16 | uint(uchar(buffer[19])) << 24;
            const int namelen = uint(uchar(buffer[20])) | uint(uchar(buffer[21])) << 8;
            const int extralen = uint(uchar(buffer[22])) | uint(uchar(buffer[23])) << 8;

            /*
              qCDebug(KArchiveLog) << "general purpose bit flag: " << gpf;
              qCDebug(KArchiveLog) << "compressed size: " << compr_size;
              qCDebug(KArchiveLog) << "uncompressed size: " << uncomp_size;
              qCDebug(KArchiveLog) << "namelen: " << namelen;
              qCDebug(KArchiveLog) << "extralen: " << extralen;
              qCDebug(KArchiveLog) << "archive size: " << dev->size();
            */

            // read fileName
            if (namelen <= 0) {
                setErrorString(tr("Invalid ZIP file. Negative name length"));
                return false;
            }
            QByteArray fileName = dev->read(namelen);
            if (fileName.size() < namelen) {
                setErrorString(tr("Invalid ZIP file. Name not completely read (#2)"));
                return false;
            }

            ParseFileInfo pfi;
            pfi.mtime = mtime;

            // read and parse the beginning of the extra field,
            // skip rest of extra field in case it is too long
            unsigned int extraFieldEnd = dev->pos() + extralen;
            pfi.extralen = extralen;
            int handledextralen = qMin(extralen, (int)sizeof buffer);

            //if (handledextralen)
            //    qCDebug(KArchiveLog) << "handledextralen: " << handledextralen;

            n = dev->read(buffer, handledextralen);
            // no error msg necessary as we deliberately truncate the extra field
            if (!parseExtraField(buffer, n, true, pfi)) {
                setErrorString(tr("Invalid ZIP File. Broken ExtraField."));
                return false;
            }

            // jump to end of extra field
            dev->seek(extraFieldEnd);

            // we have to take care of the 'general purpose bit flag'.
            // if bit 3 is set, the header doesn't contain the length of
            // the file and we look for the signature 'PK\7\8'.
            if (gpf & 8) {
                // here we have to read through the compressed data to find
                // the next PKxx
                if (!seekToNextHeaderToken(dev, true)) {
                    setErrorString(tr("Could not seek to next header token"));
                    return false;
                }
            } else {
                // here we skip the compressed data and jump to the next header
                //qCDebug(KArchiveLog) << "general purpose bit flag indicates, that local file header contains valid size";
                bool foundSignature = false;
                // check if this could be a symbolic link
                if (compression_mode == NoCompression
                    && uncomp_size <= max_path_len
                    && uncomp_size > 0) {
                    // read content and store it
                    // If it's not a symlink, then we'll just discard the data for now.
                    pfi.guessed_symlink = dev->read(uncomp_size);
                    if (pfi.guessed_symlink.size() < uncomp_size) {
                        setErrorString(tr("Invalid ZIP file. Unexpected end of file. (#5)"));
                        return false;
                    }
                } else {
                    if (compr_size > dev->size()) {
                        // here we cannot trust the compressed size, so scan through the compressed
                        // data to find the next header
                        if (!seekToNextHeaderToken(dev, false)) {
                            setErrorString(tr("Could not seek to next header token"));
                            return false;
                        }
                        foundSignature = true;
                    } else {
//          qCDebug(KArchiveLog) << "before interesting dev->pos(): " << dev->pos();
                        const bool success = dev->seek(dev->pos() + compr_size);
                        if (!success) {
                            setErrorString(tr("Could not seek to file compressed size"));
                            return false;
                        }
                        /*          qCDebug(KArchiveLog) << "after interesting dev->pos(): " << dev->pos();
                                                if (success)
                                                qCDebug(KArchiveLog) << "dev->at was successful... ";
                                                else
                                                qCDebug(KArchiveLog) << "dev->at failed... ";*/
                    }

                }
                // test for optional data descriptor
                if (!foundSignature) {
//                     qCDebug(KArchiveLog) << "Testing for optional data descriptor";
                    // read static data descriptor
                    n = dev->read(buffer, 4);
                    if (n < 4) {
                        setErrorString(tr("Invalid ZIP file. Unexpected end of file. (#1)"));
                        return false;
                    }

                    if (buffer[0] != 'P' || !handlePossibleHeaderBegin(buffer + 1, dev, false)) {
                        // assume data descriptor without signature
                        dev->seek(dev->pos() + 8); // skip rest of the 'data_descriptor'
                    }
                }

// not needed any more
                /*                // here we calculate the length of the file in the zip
                // with headers and jump to the next header.
                uint skip = compr_size + namelen + extralen;
                offset += 30 + skip;*/
            }
            pfi_map.insert(fileName, pfi);
        } else if (!memcmp(buffer, "PK\1\2", 4)) { // central block
            //qCDebug(KArchiveLog) << "PK12 found central block";
            startOfFile = false;

            // so we reached the central header at the end of the zip file
            // here we get all interesting data out of the central header
            // of a file
            offset = dev->pos() - 4;

            //set offset for appending new files
            if (d->m_offset == 0) {
                d->m_offset = offset;
            }

            n = dev->read(buffer + 4, 42);
            if (n < 42) {
                setErrorString(tr(
                    "Invalid ZIP file, central entry too short "
                    "(not long enough for valid entry)"));
                return false;
            }

            //int gpf = (uchar)buffer[9] << 8 | (uchar)buffer[10];
            //qCDebug(KArchiveLog) << "general purpose flag=" << gpf;
            // length of the fileName (well, pathname indeed)
            int namelen = (uchar)buffer[29] << 8 | (uchar)buffer[28];
            if (namelen <= 0) {
                setErrorString(tr("Invalid ZIP file, file path name length smaller or equal to zero"));
                return false;
            }
            QByteArray bufferName = dev->read(namelen);
            if (bufferName.size() < namelen) {
                //qCWarning(KArchiveLog) << "Invalid ZIP file. Name not completely read";
            }

            ParseFileInfo pfi = pfi_map.value(bufferName, ParseFileInfo());

            QString name(QFile::decodeName(bufferName));

            //qCDebug(KArchiveLog) << "name: " << name;
            // only in central header ! see below.
            // length of extra attributes
            int extralen = (uchar)buffer[31] << 8 | (uchar)buffer[30];
            // length of comment for this file
            int commlen = (uchar)buffer[33] << 8 | (uchar)buffer[32];
            // compression method of this file
            int cmethod = (uchar)buffer[11] << 8 | (uchar)buffer[10];

            //qCDebug(KArchiveLog) << "cmethod: " << cmethod;
            //qCDebug(KArchiveLog) << "extralen: " << extralen;

            // crc32 of the file
            uint crc32 = (uchar)buffer[19] << 24 | (uchar)buffer[18] << 16 |
                         (uchar)buffer[17] << 8 | (uchar)buffer[16];

            // uncompressed file size
            uint ucsize = (uchar)buffer[27] << 24 | (uchar)buffer[26] << 16 |
                          (uchar)buffer[25] << 8 | (uchar)buffer[24];
            // compressed file size
            uint csize = (uchar)buffer[23] << 24 | (uchar)buffer[22] << 16 |
                         (uchar)buffer[21] << 8 | (uchar)buffer[20];

            // offset of local header
            uint localheaderoffset = (uchar)buffer[45] << 24 | (uchar)buffer[44] << 16 |
                                     (uchar)buffer[43] << 8 | (uchar)buffer[42];

            // some clever people use different extra field lengths
            // in the central header and in the local header... funny.
            // so we need to get the localextralen to calculate the offset
            // from localheaderstart to dataoffset
            int localextralen = pfi.extralen; // FIXME: this will not work if
            // no local header exists

            //qCDebug(KArchiveLog) << "localextralen: " << localextralen;

            // offset, where the real data for uncompression starts
            uint dataoffset = localheaderoffset + 30 + localextralen + namelen; //comment only in central header

            //qCDebug(KArchiveLog) << "esize: " << esize;
            //qCDebug(KArchiveLog) << "eoffset: " << eoffset;
            //qCDebug(KArchiveLog) << "csize: " << csize;

            int os_madeby = (uchar)buffer[5];
            bool isdir = false;
            int access = 0100644;

            if (os_madeby == 3) {   // good ole unix
                access = (uchar)buffer[40] | (uchar)buffer[41] << 8;
            }

            QString entryName;

            if (name.endsWith(QLatin1Char('/'))) { // Entries with a trailing slash are directories
                isdir = true;
                name = name.left(name.length() - 1);
                if (os_madeby != 3) {
                    access = S_IFDIR | 0755;
                } else {
                    access |= S_IFDIR | 0700;
                }
            }

            int pos = name.lastIndexOf(QLatin1Char('/'));
            if (pos == -1) {
                entryName = name;
            } else {
                entryName = name.mid(pos + 1);
            }
            if (entryName.isEmpty()) {
                setErrorString(tr("Invalid ZIP file, found empty entry name"));
                return false;
            }

            KArchiveEntry *entry;
            if (isdir) {
                QString path = QDir::cleanPath(name);
                const KArchiveEntry *ent = rootDir()->entry(path);
                if (ent && ent->isDirectory()) {
                    //qCDebug(KArchiveLog) << "Directory already exists, NOT going to add it again";
                    entry = nullptr;
                } else {
                    QDateTime mtime = KArchivePrivate::time_tToDateTime(pfi.mtime);
                    entry = new KArchiveDirectory(this, entryName, access, mtime, rootDir()->user(), rootDir()->group(), QString());
                    //qCDebug(KArchiveLog) << "KArchiveDirectory created, entryName= " << entryName << ", name=" << name;
                }
            } else {
                QString symlink;
                if ((access & QT_STAT_MASK) == QT_STAT_LNK) {
                    symlink = QFile::decodeName(pfi.guessed_symlink);
                }
                QDateTime mtime = KArchivePrivate::time_tToDateTime(pfi.mtime);
                entry = new KZipFileEntry(this, entryName, access, mtime,
                                          rootDir()->user(), rootDir()->group(),
                                          symlink, name, dataoffset,
                                          ucsize, cmethod, csize);
                static_cast<KZipFileEntry *>(entry)->setHeaderStart(localheaderoffset);
                static_cast<KZipFileEntry *>(entry)->setCRC32(crc32);
                //qCDebug(KArchiveLog) << "KZipFileEntry created, entryName= " << entryName << ", name=" << name;
                d->m_fileList.append(static_cast<KZipFileEntry *>(entry));
            }

            if (entry) {
                if (pos == -1) {
                    rootDir()->addEntry(entry);
                } else {
                    // In some tar files we can find dir/./file => call cleanPath
                    QString path = QDir::cleanPath(name.left(pos));
                    // Ensure container directory exists, create otherwise
                    KArchiveDirectory *tdir = findOrCreate(path);
                    if (tdir) {
                        tdir->addEntry(entry);
                    } else {
                        setErrorString(tr("File %1 is in folder %2, but %3 is actually a file.").arg(entryName, path, path));
                        delete entry;
                        return false;
                    }
                }
            }

            //calculate offset to next entry
            offset += 46 + commlen + extralen + namelen;
            const bool b = dev->seek(offset);
            if (!b) {
                setErrorString(tr("Could not seek to next entry"));
                return false;
            }
        } else if (startOfFile) {
            // The file does not start with any ZIP header (e.g. self-extractable ZIP files)
            // Therefore we need to find the first PK\003\004 (local header)
            //qCDebug(KArchiveLog) << "Try to skip start of file";
            startOfFile = false;
            bool foundSignature = false;

            while (!foundSignature) {
                n = dev->read(buffer, 1);
                if (n < 1) {
                    setErrorString(tr("Invalid ZIP file. Unexpected end of file."));
                    return false;
                }

                if (buffer[0] != 'P') {
                    continue;
                }

                n = dev->read(buffer, 3);
                if (n < 3) {
                    setErrorString(tr("Invalid ZIP file. Unexpected end of file."));
                    return false;
                }

                // We have to detect the magic token for a local header: PK\003\004
                /*
                 * Note: we do not need to check the other magics, if the ZIP file has no
                 * local header, then it has not any files!
                 */
                if (buffer[0] == 'K' && buffer[1] == 3 && buffer[2] == 4) {
                    foundSignature = true;
                    dev->seek(dev->pos() - 4); // go back 4 bytes, so that the magic bytes can be found...
                } else {
                    for (int i = 0; i < 3; ++i) {
                        if (buffer[i] == 'P') {
                            // We have another P character so we must go back a little to check if it is a magic
                            dev->seek(dev->pos() - 3 + i);
                            break;
                        }
                    }
                }
            }
        } else {
            setErrorString(
                tr("Invalid ZIP file. Unrecognized header at offset %1")
                .arg(dev->pos() - 4));
            return false;
        }
    }
    //qCDebug(KArchiveLog) << "*** done *** ";
    return true;
}

bool KZip::closeArchive()
{
    if (!(mode() & QIODevice::WriteOnly)) {
        //qCDebug(KArchiveLog) << "readonly";
        return true;
    }

    //ReadWrite or WriteOnly
    //write all central dir file entries

    // to be written at the end of the file...
    char buffer[22]; // first used for 12, then for 22 at the end
    uLong crc = crc32(0L, nullptr, 0);

    qint64 centraldiroffset = device()->pos();
    //qCDebug(KArchiveLog) << "closearchive: centraldiroffset: " << centraldiroffset;
    qint64 atbackup = centraldiroffset;
    QMutableListIterator<KZipFileEntry *> it(d->m_fileList);

    while (it.hasNext()) {
        //set crc and compressed size in each local file header
        it.next();
        if (!device()->seek(it.value()->headerStart() + 14)) {
            setErrorString(
                tr("Could not seek to next file header: %1")
                .arg(device()->errorString()));
            return false;
        }
        //qCDebug(KArchiveLog) << "closearchive setcrcandcsize: fileName:"
        //    << it.current()->path()
        //    << "encoding:" << it.current()->encoding();

        uLong mycrc = it.value()->crc32();
        buffer[0] = char(mycrc); // crc checksum, at headerStart+14
        buffer[1] = char(mycrc >> 8);
        buffer[2] = char(mycrc >> 16);
        buffer[3] = char(mycrc >> 24);

        int mysize1 = it.value()->compressedSize();
        buffer[4] = char(mysize1); // compressed file size, at headerStart+18
        buffer[5] = char(mysize1 >> 8);
        buffer[6] = char(mysize1 >> 16);
        buffer[7] = char(mysize1 >> 24);

        int myusize = it.value()->size();
        buffer[8] = char(myusize); // uncompressed file size, at headerStart+22
        buffer[9] = char(myusize >> 8);
        buffer[10] = char(myusize >> 16);
        buffer[11] = char(myusize >> 24);

        if (device()->write(buffer, 12) != 12) {
            setErrorString(
                tr("Could not write file header: %1")
                .arg(device()->errorString()));
            return false;
        }
    }
    device()->seek(atbackup);

    it.toFront();
    while (it.hasNext()) {
        it.next();
        //qCDebug(KArchiveLog) << "fileName:" << it.current()->path()
        //              << "encoding:" << it.current()->encoding();

        QByteArray path = QFile::encodeName(it.value()->path());

        const int extra_field_len = (d->m_extraField == ModificationTime) ? 9 : 0;
        const int bufferSize = extra_field_len + path.length() + 46;
        char *buffer = new char[bufferSize];

        memset(buffer, 0, 46); // zero is a nice default for most header fields

        const char head[] = {
            'P', 'K', 1, 2, // central file header signature
            0x14, 3,        // version made by (3 == UNIX)
            0x14, 0         // version needed to extract
        };

        // I do not know why memcpy is not working here
        //memcpy(buffer, head, sizeof(head));
        memmove(buffer, head, sizeof (head));

        buffer[10] = char(it.value()->encoding()); // compression method
        buffer[11] = char(it.value()->encoding() >> 8);

        transformToMsDos(it.value()->date(), &buffer[12]);

        uLong mycrc = it.value()->crc32();
        buffer[16] = char(mycrc); // crc checksum
        buffer[17] = char(mycrc >> 8);
        buffer[18] = char(mycrc >> 16);
        buffer[19] = char(mycrc >> 24);

        int mysize1 = it.value()->compressedSize();
        buffer[20] = char(mysize1); // compressed file size
        buffer[21] = char(mysize1 >> 8);
        buffer[22] = char(mysize1 >> 16);
        buffer[23] = char(mysize1 >> 24);

        int mysize = it.value()->size();
        buffer[24] = char(mysize); // uncompressed file size
        buffer[25] = char(mysize >> 8);
        buffer[26] = char(mysize >> 16);
        buffer[27] = char(mysize >> 24);

        buffer[28] = char(path.length()); // fileName length
        buffer[29] = char(path.length() >> 8);

        buffer[30] = char(extra_field_len);
        buffer[31] = char(extra_field_len >> 8);

        buffer[40] = char(it.value()->permissions());
        buffer[41] = char(it.value()->permissions() >> 8);

        int myhst = it.value()->headerStart();
        buffer[42] = char(myhst); //relative offset of local header
        buffer[43] = char(myhst >> 8);
        buffer[44] = char(myhst >> 16);
        buffer[45] = char(myhst >> 24);

        // file name
        strncpy(buffer + 46, path.constData(), path.length());
        //qCDebug(KArchiveLog) << "closearchive length to write: " << bufferSize;

        // extra field
        if (d->m_extraField == ModificationTime) {
            char *extfield = buffer + 46 + path.length();
            // "Extended timestamp" header (0x5455)
            extfield[0] = 'U';
            extfield[1] = 'T';
            extfield[2] = 5; // data size
            extfield[3] = 0;
            extfield[4] = 1 | 2 | 4;    // specify flags from local field
            // (unless I misread the spec)
            // provide only modification time
            unsigned long time = (unsigned long)it.value()->date().toSecsSinceEpoch();
            extfield[5] = char(time);
            extfield[6] = char(time >> 8);
            extfield[7] = char(time >> 16);
            extfield[8] = char(time >> 24);
        }

        crc = crc32(crc, (Bytef *)buffer, bufferSize);
        bool ok = (device()->write(buffer, bufferSize) == bufferSize);
        delete[] buffer;
        if (!ok) {
            setErrorString(
                tr("Could not write file header: %1")
                .arg(device()->errorString()));
            return false;
        }
    }
    qint64 centraldirendoffset = device()->pos();
    //qCDebug(KArchiveLog) << "closearchive: centraldirendoffset: " << centraldirendoffset;
    //qCDebug(KArchiveLog) << "closearchive: device()->pos(): " << device()->pos();

    //write end of central dir record.
    buffer[0] = 'P'; //end of central dir signature
    buffer[1] = 'K';
    buffer[2] = 5;
    buffer[3] = 6;

    buffer[4] = 0; // number of this disk
    buffer[5] = 0;

    buffer[6] = 0; // number of disk with start of central dir
    buffer[7] = 0;

    int count = d->m_fileList.count();
    //qCDebug(KArchiveLog) << "number of files (count): " << count;

    buffer[8] = char(count); // total number of entries in central dir of
    buffer[9] = char(count >> 8); // this disk

    buffer[10] = buffer[8]; // total number of entries in the central dir
    buffer[11] = buffer[9];

    int cdsize = centraldirendoffset - centraldiroffset;
    buffer[12] = char(cdsize); // size of the central dir
    buffer[13] = char(cdsize >> 8);
    buffer[14] = char(cdsize >> 16);
    buffer[15] = char(cdsize >> 24);

    //qCDebug(KArchiveLog) << "end : centraldiroffset: " << centraldiroffset;
    //qCDebug(KArchiveLog) << "end : centraldirsize: " << cdsize;

    buffer[16] = char(centraldiroffset); // central dir offset
    buffer[17] = char(centraldiroffset >> 8);
    buffer[18] = char(centraldiroffset >> 16);
    buffer[19] = char(centraldiroffset >> 24);

    buffer[20] = 0; //zipfile comment length
    buffer[21] = 0;

    if (device()->write(buffer, 22) != 22) {
        setErrorString(
            tr("Could not write central dir record: %1")
            .arg(device()->errorString()));
        return false;
    }

    return true;
}

bool KZip::doWriteDir(const QString &name, const QString &user, const QString &group, mode_t perm,
                      const QDateTime &atime, const QDateTime &mtime, const QDateTime &ctime)
{
    // Zip files have no explicit directories, they are implicitly created during extraction time
    // when file entries have paths in them.
    // However, to support empty directories, we must create a dummy file entry which ends with '/'.
    QString dirName = name;
    if (!name.endsWith(QLatin1Char('/'))) {
        dirName = dirName.append(QLatin1Char('/'));
    }
    return writeFile(dirName, QByteArray(), perm, user, group, atime, mtime, ctime);
}

bool KZip::doPrepareWriting(const QString &name, const QString &user,
                            const QString &group, qint64 /*size*/, mode_t perm,
                            const QDateTime &accessTime, const QDateTime &modificationTime, const QDateTime &creationTime)
{
    //qCDebug(KArchiveLog);
    if (!isOpen()) {
        setErrorString(tr("Application error: ZIP file must be open before being written into"));
        qCWarning(KArchiveLog) << "doPrepareWriting failed: !isOpen()";
        return false;
    }

    if (!(mode() & QIODevice::WriteOnly)) { // accept WriteOnly and ReadWrite
        setErrorString(tr("Application error: attempted to write into non-writable ZIP file"));
        qCWarning(KArchiveLog) << "doPrepareWriting failed: !(mode() & QIODevice::WriteOnly)";
        return false;
    }

    if (!device()) {
        setErrorString(tr("Cannot create a device. Disk full?"));
        return false;
    }

    // set right offset in zip.
    if (!device()->seek(d->m_offset)) {
        setErrorString(tr("Cannot seek in ZIP file. Disk full?"));
        return false;
    }

    uint atime = accessTime.toSecsSinceEpoch();
    uint mtime = modificationTime.toSecsSinceEpoch();
    uint ctime = creationTime.toSecsSinceEpoch();

    // Find or create parent dir
    KArchiveDirectory *parentDir = rootDir();
    QString fileName(name);
    int i = name.lastIndexOf(QLatin1Char('/'));
    if (i != -1) {
        QString dir = name.left(i);
        fileName = name.mid(i + 1);
        //qCDebug(KArchiveLog) << "ensuring" << dir << "exists. fileName=" << fileName;
        parentDir = findOrCreate(dir);
    }

    // delete entries in the filelist with the same fileName as the one we want
    // to save, so that we don't have duplicate file entries when viewing the zip
    // with konqi...
    // CAUTION: the old file itself is still in the zip and won't be removed !!!
    QMutableListIterator<KZipFileEntry *> it(d->m_fileList);
    //qCDebug(KArchiveLog) << "fileName to write: " << name;
    while (it.hasNext()) {
        it.next();
        //qCDebug(KArchiveLog) << "prepfileName: " << it.current()->path();
        if (name == it.value()->path()) {
            // also remove from the parentDir
            parentDir->removeEntry(it.value());
            //qCDebug(KArchiveLog) << "removing following entry: " << it.current()->path();
            delete it.value();
            it.remove();
        }

    }

    // construct a KZipFileEntry and add it to list
    KZipFileEntry *e = new KZipFileEntry(this, fileName, perm, modificationTime, user, group, QString(),
                                         name, device()->pos() + 30 + name.length(), // start
                                         0 /*size unknown yet*/, d->m_compression, 0 /*csize unknown yet*/);
    e->setHeaderStart(device()->pos());
    //qCDebug(KArchiveLog) << "wrote file start: " << e->position() << " name: " << name;
    if (!parentDir->addEntryV2(e)) {
        return false;
    }

    d->m_currentFile = e;
    d->m_fileList.append(e);

    int extra_field_len = 0;
    if (d->m_extraField == ModificationTime) {
        extra_field_len = 17;    // value also used in finishWriting()
    }

    // write out zip header
    QByteArray encodedName = QFile::encodeName(name);
    int bufferSize = extra_field_len + encodedName.length() + 30;
    //qCDebug(KArchiveLog) << "bufferSize=" << bufferSize;
    char *buffer = new char[bufferSize];

    buffer[0] = 'P'; //local file header signature
    buffer[1] = 'K';
    buffer[2] = 3;
    buffer[3] = 4;

    buffer[4] = 0x14; // version needed to extract
    buffer[5] = 0;

    buffer[6] = 0; // general purpose bit flag
    buffer[7] = 0;

    buffer[8] = char(e->encoding()); // compression method
    buffer[9] = char(e->encoding() >> 8);

    transformToMsDos(e->date(), &buffer[10]);

    buffer[14] = 'C'; //dummy crc
    buffer[15] = 'R';
    buffer[16] = 'C';
    buffer[17] = 'q';

    buffer[18] = 'C'; //compressed file size
    buffer[19] = 'S';
    buffer[20] = 'I';
    buffer[21] = 'Z';

    buffer[22] = 'U'; //uncompressed file size
    buffer[23] = 'S';
    buffer[24] = 'I';
    buffer[25] = 'Z';

    buffer[26] = (uchar)(encodedName.length()); //fileName length
    buffer[27] = (uchar)(encodedName.length() >> 8);

    buffer[28] = (uchar)(extra_field_len); // extra field length
    buffer[29] = (uchar)(extra_field_len >> 8);

    // file name
    strncpy(buffer + 30, encodedName.constData(), encodedName.length());

    // extra field
    if (d->m_extraField == ModificationTime) {
        char *extfield = buffer + 30 + encodedName.length();
        // "Extended timestamp" header (0x5455)
        extfield[0] = 'U';
        extfield[1] = 'T';
        extfield[2] = 13; // data size
        extfield[3] = 0;
        extfield[4] = 1 | 2 | 4;    // contains mtime, atime, ctime

        extfield[5] = char(mtime);
        extfield[6] = char(mtime >> 8);
        extfield[7] = char(mtime >> 16);
        extfield[8] = char(mtime >> 24);

        extfield[9] = char(atime);
        extfield[10] = char(atime >> 8);
        extfield[11] = char(atime >> 16);
        extfield[12] = char(atime >> 24);

        extfield[13] = char(ctime);
        extfield[14] = char(ctime >> 8);
        extfield[15] = char(ctime >> 16);
        extfield[16] = char(ctime >> 24);
    }

    // Write header
    bool b = (device()->write(buffer, bufferSize) == bufferSize);
    d->m_crc = 0;
    delete[] buffer;

    if (!b) {
        setErrorString(tr("Could not write to the archive. Disk full?"));
        return false;
    }

    // Prepare device for writing the data
    // Either device() if no compression, or a KFilterDev to compress
    if (d->m_compression == 0) {
        d->m_currentDev = device();
        return true;
    }

    auto compressionDevice = new KCompressionDevice(device(), false, KCompressionDevice::GZip);
    d->m_currentDev = compressionDevice;
    compressionDevice->setSkipHeaders(); // Just zlib, not gzip

    b = d->m_currentDev->open(QIODevice::WriteOnly);
    Q_ASSERT(b);

    if (!b) {
        setErrorString(
            tr("Could not open compression device: %1")
            .arg(d->m_currentDev->errorString()));
    }

    return b;
}

bool KZip::doFinishWriting(qint64 size)
{
    if (d->m_currentFile->encoding() == 8) {
        // Finish
        (void)d->m_currentDev->write(nullptr, 0);
        delete d->m_currentDev;
    }
    // If 0, d->m_currentDev was device() - don't delete ;)
    d->m_currentDev = nullptr;

    Q_ASSERT(d->m_currentFile);
    //qCDebug(KArchiveLog) << "fileName: " << d->m_currentFile->path();
    //qCDebug(KArchiveLog) << "getpos (at): " << device()->pos();
    d->m_currentFile->setSize(size);
    int extra_field_len = 0;
    if (d->m_extraField == ModificationTime) {
        extra_field_len = 17;    // value also used in finishWriting()
    }

    const QByteArray encodedName = QFile::encodeName(d->m_currentFile->path());
    int csize = device()->pos() -
                d->m_currentFile->headerStart() - 30 -
                encodedName.length() - extra_field_len;
    d->m_currentFile->setCompressedSize(csize);
    //qCDebug(KArchiveLog) << "usize: " << d->m_currentFile->size();
    //qCDebug(KArchiveLog) << "csize: " << d->m_currentFile->compressedSize();
    //qCDebug(KArchiveLog) << "headerstart: " << d->m_currentFile->headerStart();

    //qCDebug(KArchiveLog) << "crc: " << d->m_crc;
    d->m_currentFile->setCRC32(d->m_crc);

    d->m_currentFile = nullptr;

    // update saved offset for appending new files
    d->m_offset = device()->pos();
    return true;
}

bool KZip::doWriteSymLink(const QString &name, const QString &target,
                          const QString &user, const QString &group,
                          mode_t perm, const QDateTime &atime, const QDateTime &mtime, const QDateTime &ctime)
{
    // reassure that symlink flag is set, otherwise strange things happen on
    // extraction
    perm |= QT_STAT_LNK;
    Compression c = compression();
    setCompression(NoCompression);  // link targets are never compressed

    if (!doPrepareWriting(name, user, group, 0, perm, atime, mtime, ctime)) {
        setCompression(c);
        return false;
    }

    QByteArray symlink_target = QFile::encodeName(target);
    if (!writeData(symlink_target.constData(), symlink_target.length())) {
        setCompression(c);
        return false;
    }

    if (!finishWriting(symlink_target.length())) {
        setCompression(c);
        return false;
    }

    setCompression(c);
    return true;
}

void KZip::virtual_hook(int id, void *data)
{
    KArchive::virtual_hook(id, data);
}

bool KZip::writeData(const char *data, qint64 size)
{
    Q_ASSERT(d->m_currentFile);
    Q_ASSERT(d->m_currentDev);
    if (!d->m_currentFile || !d->m_currentDev) {
        setErrorString(tr("No file or device"));
        return false;
    }

    // crc to be calculated over uncompressed stuff...
    // and they didn't mention it in their docs...
    d->m_crc = crc32(d->m_crc, (const Bytef *) data, size);

    qint64 written = d->m_currentDev->write(data, size);
    //qCDebug(KArchiveLog) << "wrote" << size << "bytes.";
    const bool ok = written == size;

    if (!ok) {
        setErrorString(
            tr("Error writing data: %1")
            .arg(d->m_currentDev->errorString()));
    }

    return ok;
}

void KZip::setCompression(Compression c)
{
    d->m_compression = (c == NoCompression) ? 0 : 8;
}

KZip::Compression KZip::compression() const
{
    return (d->m_compression == 8) ? DeflateCompression : NoCompression;
}

void KZip::setExtraField(ExtraField ef)
{
    d->m_extraField = ef;
}

KZip::ExtraField KZip::extraField() const
{
    return d->m_extraField;
}

////////////////////////////////////////////////////////////////////////
////////////////////// KZipFileEntry////////////////////////////////////
////////////////////////////////////////////////////////////////////////
class Q_DECL_HIDDEN KZipFileEntry::KZipFileEntryPrivate
{
public:
    KZipFileEntryPrivate()
        : crc(0)
        , compressedSize(0)
        , headerStart(0)
        , encoding(0)
    {
    }
    unsigned long crc;
    qint64        compressedSize;
    qint64        headerStart;
    int           encoding;
    QString       path;
};

KZipFileEntry::KZipFileEntry(KZip *zip, const QString &name, int access, const QDateTime &date,
                             const QString &user, const QString &group, const QString &symlink,
                             const QString &path, qint64 start, qint64 uncompressedSize,
                             int encoding, qint64 compressedSize)
    : KArchiveFile(zip, name, access, date, user, group, symlink, start, uncompressedSize)
    , d(new KZipFileEntryPrivate)
{
    d->path = path;
    d->encoding = encoding;
    d->compressedSize = compressedSize;
}

KZipFileEntry::~KZipFileEntry()
{
    delete d;
}

int KZipFileEntry::encoding() const
{
    return d->encoding;
}

qint64 KZipFileEntry::compressedSize() const
{
    return d->compressedSize;
}

void KZipFileEntry::setCompressedSize(qint64 compressedSize)
{
    d->compressedSize = compressedSize;
}

void KZipFileEntry::setHeaderStart(qint64 headerstart)
{
    d->headerStart = headerstart;
}

qint64 KZipFileEntry::headerStart() const
{
    return d->headerStart;
}

unsigned long KZipFileEntry::crc32() const
{
    return d->crc;
}

void KZipFileEntry::setCRC32(unsigned long crc32)
{
    d->crc = crc32;
}

const QString &KZipFileEntry::path() const
{
    return d->path;
}

QByteArray KZipFileEntry::data() const
{
    QIODevice *dev = createDevice();
    QByteArray arr;
    if (dev) {
        arr = dev->readAll();
        delete dev;
    }
    return arr;
}

QIODevice *KZipFileEntry::createDevice() const
{
    //qCDebug(KArchiveLog) << "creating iodevice limited to pos=" << position() << ", csize=" << compressedSize();
    // Limit the reading to the appropriate part of the underlying device (e.g. file)
    KLimitedIODevice *limitedDev = new KLimitedIODevice(archive()->device(), position(), compressedSize());
    if (encoding() == 0 || compressedSize() == 0) { // no compression (or even no data)
        return limitedDev;
    }

    if (encoding() == 8) {
        // On top of that, create a device that uncompresses the zlib data
        KCompressionDevice *filterDev = new KCompressionDevice(limitedDev, true, KCompressionDevice::GZip);

        if (!filterDev) {
            return nullptr;    // ouch
        }
        filterDev->setSkipHeaders(); // Just zlib, not gzip
        bool b = filterDev->open(QIODevice::ReadOnly);
        Q_UNUSED(b);
        Q_ASSERT(b);
        return filterDev;
    }

    qCCritical(KArchiveLog) << "This zip file contains files compressed with method"
                << encoding() << ", this method is currently not supported by KZip,"
                << "please use a command-line tool to handle this file.";
    delete limitedDev;
    return nullptr;
}
