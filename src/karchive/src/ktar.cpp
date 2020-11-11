/* This file is part of the KDE libraries
   SPDX-FileCopyrightText: 2000 David Faure <faure@kde.org>
   SPDX-FileCopyrightText: 2003 Leo Savernik <l.savernik@aon.at>

   SPDX-License-Identifier: LGPL-2.0-only
*/

#include "ktar.h"
#include "karchive_p.h"
#include "loggingcategory.h"

#include <stdlib.h> // strtol
#include <assert.h>

#include <QDebug>
#include <QDir>
#include <QFile>
#include <QMimeDatabase>
#include <QTemporaryFile>

#include <kfilterdev.h>
#include <kfilterbase.h>

////////////////////////////////////////////////////////////////////////
/////////////////////////// KTar ///////////////////////////////////
////////////////////////////////////////////////////////////////////////

// Mime types of known filters
static const char application_gzip[] = "application/x-gzip";
static const char application_bzip[] = "application/x-bzip";
static const char application_lzma[] = "application/x-lzma";
static const char application_xz[] = "application/x-xz";

class Q_DECL_HIDDEN KTar::KTarPrivate
{
public:
    KTarPrivate(KTar *parent)
        : q(parent)
        , tarEnd(0)
        , tmpFile(nullptr)
        , compressionDevice(nullptr)
    {
    }

    KTar *q;
    QStringList dirList;
    qint64 tarEnd;
    QTemporaryFile *tmpFile;
    QString mimetype;
    QByteArray origFileName;
    KCompressionDevice *compressionDevice;

    bool fillTempFile(const QString &fileName);
    bool writeBackTempFile(const QString &fileName);
    void fillBuffer(char *buffer, const char *mode, qint64 size, const QDateTime &mtime,
                    char typeflag, const char *uname, const char *gname);
    void writeLonglink(char *buffer, const QByteArray &name, char typeflag,
                       const char *uname, const char *gname);
    qint64 readRawHeader(char *buffer);
    bool readLonglink(char *buffer, QByteArray &longlink);
    qint64 readHeader(char *buffer, QString &name, QString &symlink);
};

KTar::KTar(const QString &fileName, const QString &_mimetype)
    : KArchive(fileName)
    , d(new KTarPrivate(this))
{
    d->mimetype = _mimetype;
}

KTar::KTar(QIODevice *dev)
    : KArchive(dev)
    , d(new KTarPrivate(this))
{
}

// Only called when a filename was given
bool KTar::createDevice(QIODevice::OpenMode mode)
{
    if (d->mimetype.isEmpty()) {
        // Find out mimetype manually

        QMimeDatabase db;
        QMimeType mime;
        if (mode != QIODevice::WriteOnly && QFile::exists(fileName())) {
            // Give priority to file contents: if someone renames a .tar.bz2 to .tar.gz,
            // we can still do the right thing here.
            QFile f(fileName());
            if (f.open(QIODevice::ReadOnly)) {
                mime = db.mimeTypeForData(&f);
            }
            if (!mime.isValid()) {
                // Unable to determine mimetype from contents, get it from file name
                mime = db.mimeTypeForFile(fileName(), QMimeDatabase::MatchExtension);
            }
        } else {
            mime = db.mimeTypeForFile(fileName(), QMimeDatabase::MatchExtension);
        }

        //qCDebug(KArchiveLog) << mode << mime->name();

        if (mime.inherits(QStringLiteral("application/x-compressed-tar")) || mime.inherits(QString::fromLatin1(application_gzip))) {
            // gzipped tar file (with possibly invalid file name), ask for gzip filter
            d->mimetype = QString::fromLatin1(application_gzip);
        } else if (mime.inherits(QStringLiteral("application/x-bzip-compressed-tar")) || mime.inherits(QString::fromLatin1(application_bzip))) {
            // bzipped2 tar file (with possibly invalid file name), ask for bz2 filter
            d->mimetype = QString::fromLatin1(application_bzip);
        } else if (mime.inherits(QStringLiteral("application/x-lzma-compressed-tar")) || mime.inherits(QString::fromLatin1(application_lzma))) {
            // lzma compressed tar file (with possibly invalid file name), ask for xz filter
            d->mimetype = QString::fromLatin1(application_lzma);
        } else if (mime.inherits(QStringLiteral("application/x-xz-compressed-tar")) || mime.inherits(QString::fromLatin1(application_xz))) {
            // xz compressed tar file (with possibly invalid name), ask for xz filter
            d->mimetype = QString::fromLatin1(application_xz);
        }
    }

    if (d->mimetype == QLatin1String("application/x-tar")) {
        return KArchive::createDevice(mode);
    } else if (mode == QIODevice::WriteOnly) {
        if (!KArchive::createDevice(mode)) {
            return false;
        }
        if (!d->mimetype.isEmpty()) {
            // Create a compression filter on top of the QSaveFile device that KArchive created.
            //qCDebug(KArchiveLog) << "creating KFilterDev for" << d->mimetype;
            KCompressionDevice::CompressionType type = KFilterDev::compressionTypeForMimeType(d->mimetype);
            d->compressionDevice = new KCompressionDevice(device(), false, type);
            setDevice(d->compressionDevice);
        }
        return true;
    } else {
        // The compression filters are very slow with random access.
        // So instead of applying the filter to the device,
        // the file is completely extracted instead,
        // and we work on the extracted tar file.
        // This improves the extraction speed by the tar ioslave dramatically,
        // if the archive file contains many files.
        // This is because the tar ioslave extracts one file after the other and normally
        // has to walk through the decompression filter each time.
        // Which is in fact nearly as slow as a complete decompression for each file.

        Q_ASSERT(!d->tmpFile);
        d->tmpFile = new QTemporaryFile();
        d->tmpFile->setFileTemplate(QDir::tempPath() + QStringLiteral("/") + QLatin1String("ktar-XXXXXX.tar"));
        d->tmpFile->open();
        //qCDebug(KArchiveLog) << "creating tempfile:" << d->tmpFile->fileName();

        setDevice(d->tmpFile);
        return true;
    }
}

KTar::~KTar()
{
    // mjarrett: Closes to prevent ~KArchive from aborting w/o device
    if (isOpen()) {
        close();
    }

    delete d->tmpFile;
    delete d->compressionDevice;
    delete d;
}

void KTar::setOrigFileName(const QByteArray &fileName)
{
    if (!isOpen() || !(mode() & QIODevice::WriteOnly)) {
        //qCWarning(KArchiveLog) << "KTar::setOrigFileName: File must be opened for writing first.\n";
        return;
    }
    d->origFileName = fileName;
}

qint64 KTar::KTarPrivate::readRawHeader(char *buffer)
{
    // Read header
    qint64 n = q->device()->read(buffer, 0x200);
    // we need to test if there is a prefix value because the file name can be null
    // and the prefix can have a value and in this case we don't reset n.
    if (n == 0x200 && (buffer[0] != 0 || buffer[0x159] != 0)) {
        // Make sure this is actually a tar header
        if (strncmp(buffer + 257, "ustar", 5)) {
            // The magic isn't there (broken/old tars), but maybe a correct checksum?

            int check = 0;
            for (uint j = 0; j < 0x200; ++j) {
                check += static_cast<unsigned char>(buffer[j]);
            }

            // adjust checksum to count the checksum fields as blanks
            for (uint j = 0; j < 8 /*size of the checksum field including the \0 and the space*/; j++) {
                check -= static_cast<unsigned char>(buffer[148 + j]);
            }
            check += 8 * ' ';

            QByteArray s = QByteArray::number(check, 8);   // octal

            // only compare those of the 6 checksum digits that mean something,
            // because the other digits are filled with all sorts of different chars by different tars ...
            // Some tars right-justify the checksum so it could start in one of three places - we have to check each.
            if (strncmp(buffer + 148 + 6 - s.length(), s.data(), s.length())
                && strncmp(buffer + 148 + 7 - s.length(), s.data(), s.length())
                && strncmp(buffer + 148 + 8 - s.length(), s.data(), s.length())) {
                /*qCWarning(KArchiveLog) << "KTar: invalid TAR file. Header is:" << QByteArray( buffer+257, 5 )
                               << "instead of ustar. Reading from wrong pos in file?"
                               << "checksum=" << QByteArray( buffer + 148 + 6 - s.length(), s.length() );*/
                return -1;
            }
        }/*end if*/
    } else {
        // reset to 0 if 0x200 because logical end of archive has been reached
        if (n == 0x200) {
            n = 0;
        }
    }/*end if*/
    return n;
}

bool KTar::KTarPrivate::readLonglink(char *buffer, QByteArray &longlink)
{
    qint64 n = 0;
    //qCDebug(KArchiveLog) << "reading longlink from pos " << q->device()->pos();
    QIODevice *dev = q->device();
    // read size of longlink from size field in header
    // size is in bytes including the trailing null (which we ignore)
    qint64 size = QByteArray(buffer + 0x7c, 12).trimmed().toLongLong(nullptr, 8 /*octal*/);

    size--;    // ignore trailing null
    if (size > std::numeric_limits<int>::max() - 32) { // QByteArray can't really be INT_MAX big, it's max size is something between INT_MAX - 32 and INT_MAX depending the platform so just be safe
        qCWarning(KArchiveLog) << "Failed to allocate memory for longlink of size" << size;
        return false;
    }
    if (size < 0) {
        qCWarning(KArchiveLog) << "Invalid longlink size" << size;
        return false;
    }
    longlink.resize(size);
    qint64 offset = 0;
    while (size > 0) {
        int chunksize = qMin(size, 0x200LL);
        n = dev->read(longlink.data() + offset, chunksize);
        if (n == -1) {
            return false;
        }
        size -= chunksize;
        offset += 0x200;
    }/*wend*/
    // jump over the rest
    const int skip = 0x200 - (n % 0x200);
    if (skip <= 0x200) {
        if (dev->read(buffer, skip) != skip) {
            return false;
        }
    }
    longlink.truncate(qstrlen(longlink.constData()));
    return true;
}

qint64 KTar::KTarPrivate::readHeader(char *buffer, QString &name, QString &symlink)
{
    name.truncate(0);
    symlink.truncate(0);
    while (true) {
        qint64 n = readRawHeader(buffer);
        if (n != 0x200) {
            return n;
        }

        // is it a longlink?
        if (strcmp(buffer, "././@LongLink") == 0) {
            char typeflag = buffer[0x9c];
            QByteArray longlink;
            if (readLonglink(buffer, longlink)) {
                switch (typeflag) {
                case 'L':
                    name = QFile::decodeName(longlink.constData());
                    break;
                case 'K':
                    symlink = QFile::decodeName(longlink.constData());
                    break;
                }/*end switch*/
            }
        } else {
            break;
        }/*end if*/
    }/*wend*/

    // if not result of longlink, read names directly from the header
    if (name.isEmpty())
        // there are names that are exactly 100 bytes long
        // and neither longlink nor \0 terminated (bug:101472)
    {
        name = QFile::decodeName(QByteArray(buffer, qstrnlen(buffer, 100)));
    }
    if (symlink.isEmpty()) {
        char *symlinkBuffer = buffer + 0x9d /*?*/;
        symlink = QFile::decodeName(QByteArray(symlinkBuffer, qstrnlen(symlinkBuffer, 100)));
    }

    return 0x200;
}

/*
 * If we have created a temporary file, we have
 * to decompress the original file now and write
 * the contents to the temporary file.
 */
bool KTar::KTarPrivate::fillTempFile(const QString &fileName)
{
    if (! tmpFile) {
        return true;
    }

    //qCDebug(KArchiveLog) << "filling tmpFile of mimetype" << mimetype;

    KCompressionDevice::CompressionType compressionType = KFilterDev::compressionTypeForMimeType(mimetype);
    KCompressionDevice filterDev(fileName, compressionType);

    QFile *file = tmpFile;
    Q_ASSERT(file->isOpen());
    Q_ASSERT(file->openMode() & QIODevice::WriteOnly);
    file->seek(0);
    QByteArray buffer;
    buffer.resize(8 * 1024);
    if (! filterDev.open(QIODevice::ReadOnly)) {
        q->setErrorString(
            tr("File %1 does not exist")
            .arg(fileName));
        return false;
    }
    qint64 len = -1;
    while (!filterDev.atEnd() && len != 0) {
        len = filterDev.read(buffer.data(), buffer.size());
        if (len < 0) {   // corrupted archive
            q->setErrorString(tr("Archive %1 is corrupt").arg(fileName));
            return false;
        }
        if (file->write(buffer.data(), len) != len) {   // disk full
            q->setErrorString(tr("Disk full"));
            return false;
        }
    }
    filterDev.close();

    file->flush();
    file->seek(0);
    Q_ASSERT(file->isOpen());
    Q_ASSERT(file->openMode() & QIODevice::ReadOnly);

    //qCDebug(KArchiveLog) << "filling tmpFile finished.";
    return true;
}

bool KTar::openArchive(QIODevice::OpenMode mode)
{

    if (!(mode & QIODevice::ReadOnly)) {
        return true;
    }

    if (!d->fillTempFile(fileName())) {
        return false;
    }

    // We'll use the permission and user/group of d->rootDir
    // for any directory we emulate (see findOrCreate)
    //struct stat buf;
    //stat( fileName(), &buf );

    d->dirList.clear();
    QIODevice *dev = device();

    if (!dev) {
        setErrorString(tr("Could not get underlying device"));
        qCWarning(KArchiveLog) << "Could not get underlying device";
        return false;
    }

    // read dir information
    char buffer[0x200];
    bool ende = false;
    do {
        QString name;
        QString symlink;

        // Read header
        qint64 n = d->readHeader(buffer, name, symlink);
        if (n < 0) {
            setErrorString(tr("Could not read tar header"));
            return false;
        }
        if (n == 0x200) {
            bool isdir = false;

            if (name.isEmpty()) {
                continue;
            }
            if (name.endsWith(QLatin1Char('/'))) {
                isdir = true;
                name.truncate(name.length() - 1);
            }

            QByteArray prefix = QByteArray(buffer + 0x159, 155);
            if (prefix[0] != '\0') {
                name = (QString::fromLatin1(prefix.constData()) + QLatin1Char('/') +  name);
            }

            int pos = name.lastIndexOf(QLatin1Char('/'));
            QString nm = (pos == -1) ? name : name.mid(pos + 1);

            // read access
            buffer[0x6b] = 0;
            char *dummy;
            const char *p = buffer + 0x64;
            while (*p == ' ') {
                ++p;
            }
            int access = strtol(p, &dummy, 8);

            // read user and group
            const int maxUserGroupLength = 32;
            const char *userStart = buffer + 0x109;
            const int userLen = qstrnlen(userStart, maxUserGroupLength);
            const QString user = QString::fromLocal8Bit(userStart, userLen);
            const char *groupStart = buffer + 0x129;
            const int groupLen = qstrnlen(groupStart, maxUserGroupLength);
            const QString group = QString::fromLocal8Bit(groupStart, groupLen);

            // read time
            buffer[0x93] = 0;
            p = buffer + 0x88;
            while (*p == ' ') {
                ++p;
            }
            uint time = strtol(p, &dummy, 8);

            // read type flag
            char typeflag = buffer[0x9c];
            // '0' for files, '1' hard link, '2' symlink, '5' for directory
            // (and 'L' for longlink fileNames, 'K' for longlink symlink targets)
            // 'D' for GNU tar extension DUMPDIR, 'x' for Extended header referring
            // to the next file in the archive and 'g' for Global extended header

            if (typeflag == '5') {
                isdir = true;
            }

            bool isDumpDir = false;
            if (typeflag == 'D') {
                isdir = false;
                isDumpDir = true;
            }
            //qCDebug(KArchiveLog) << nm << "isdir=" << isdir << "pos=" << dev->pos() << "typeflag=" << typeflag << " islink=" << ( typeflag == '1' || typeflag == '2' );

            if (typeflag == 'x' || typeflag == 'g') { // pax extended header, or pax global extended header
                // Skip it for now. TODO: implement reading of extended header, as per http://pubs.opengroup.org/onlinepubs/009695399/utilities/pax.html
                (void)dev->read(buffer, 0x200);
                continue;
            }

            if (isdir) {
                access |= S_IFDIR;    // f*cking broken tar files
            }

            KArchiveEntry *e;
            if (isdir) {
                //qCDebug(KArchiveLog) << "directory" << nm;
                e = new KArchiveDirectory(this, nm, access, KArchivePrivate::time_tToDateTime(time), user, group, symlink);
            } else {
                // read size
                QByteArray sizeBuffer(buffer + 0x7c, 12);
                qint64 size = sizeBuffer.trimmed().toLongLong(nullptr, 8 /*octal*/);
                //qCDebug(KArchiveLog) << "sizeBuffer='" << sizeBuffer << "' -> size=" << size;

                // for isDumpDir we will skip the additional info about that dirs contents
                if (isDumpDir) {
                    //qCDebug(KArchiveLog) << nm << "isDumpDir";
                    e = new KArchiveDirectory(this, nm, access, KArchivePrivate::time_tToDateTime(time), user, group, symlink);
                } else {

                    // Let's hack around hard links. Our classes don't support that, so make them symlinks
                    if (typeflag == '1') {
                        //qCDebug(KArchiveLog) << "Hard link, setting size to 0 instead of" << size;
                        size = 0; // no contents
                    }

                    //qCDebug(KArchiveLog) << "file" << nm << "size=" << size;
                    e = new KArchiveFile(this, nm, access, KArchivePrivate::time_tToDateTime(time), user, group, symlink,
                                         dev->pos(), size);
                }

                // Skip contents + align bytes
                qint64 rest = size % 0x200;
                qint64 skip = size + (rest ? 0x200 - rest : 0);
                //qCDebug(KArchiveLog) << "pos()=" << dev->pos() << "rest=" << rest << "skipping" << skip;
                if (! dev->seek(dev->pos() + skip)) {
                    //qCWarning(KArchiveLog) << "skipping" << skip << "failed";
                }
            }

            if (pos == -1) {
                if (nm == QLatin1String(".")) { // special case
                    if (isdir) {
                        if (KArchivePrivate::hasRootDir(this)) {
                            qWarning() << "Broken tar file has two root dir entries";
                            delete e;
                        } else {
                            setRootDir(static_cast<KArchiveDirectory *>(e));
                        }
                    } else {
                        delete e;
                    }
                } else {
                    rootDir()->addEntry(e);
                }
            } else {
                // In some tar files we can find dir/./file => call cleanPath
                QString path = QDir::cleanPath(name.left(pos));
                // Ensure container directory exists, create otherwise
                KArchiveDirectory *d = findOrCreate(path);
                if (d) {
                    d->addEntry(e);
                } else {
                    delete e;
                    return false;
                }
            }
        } else {
            //qCDebug(KArchiveLog) << "Terminating. Read " << n << " bytes, first one is " << buffer[0];
            d->tarEnd = dev->pos() - n; // Remember end of archive
            ende = true;
        }
    } while (!ende);
    return true;
}

/*
 * Writes back the changes of the temporary file
 * to the original file.
 * Must only be called if in write mode, not in read mode
 */
bool KTar::KTarPrivate::writeBackTempFile(const QString &fileName)
{
    if (!tmpFile) {
        return true;
    }

    //qCDebug(KArchiveLog) << "Write temporary file to compressed file" << fileName << mimetype;

    bool forced = false;
    if (QLatin1String(application_gzip) == mimetype || QLatin1String(application_bzip) == mimetype ||
        QLatin1String(application_lzma) == mimetype || QLatin1String(application_xz) == mimetype) {
        forced = true;
    }

    // #### TODO this should use QSaveFile to avoid problems on disk full
    // (KArchive uses QSaveFile by default, but the temp-uncompressed-file trick
    // circumvents that).

    KFilterDev dev(fileName);
    QFile *file = tmpFile;
    if (!dev.open(QIODevice::WriteOnly)) {
        file->close();
        q->setErrorString(tr("Failed to write back temp file: %1").arg(dev.errorString()));
        return false;
    }
    if (forced) {
        dev.setOrigFileName(origFileName);
    }
    file->seek(0);
    QByteArray buffer;
    buffer.resize(8 * 1024);
    qint64 len;
    while (!file->atEnd()) {
        len = file->read(buffer.data(), buffer.size());
        dev.write(buffer.data(), len); // TODO error checking
    }
    file->close();
    dev.close();

    //qCDebug(KArchiveLog) << "Write temporary file to compressed file done.";
    return true;
}

bool KTar::closeArchive()
{
    d->dirList.clear();

    bool ok = true;

    // If we are in readwrite mode and had created
    // a temporary tar file, we have to write
    // back the changes to the original file
    if (d->tmpFile && (mode() & QIODevice::WriteOnly)) {
        ok = d->writeBackTempFile(fileName());
        delete d->tmpFile;
        d->tmpFile = nullptr;
        setDevice(nullptr);
    }

    return ok;
}

bool KTar::doFinishWriting(qint64 size)
{
    // Write alignment
    int rest = size % 0x200;
    if ((mode() & QIODevice::ReadWrite) == QIODevice::ReadWrite) {
        d->tarEnd = device()->pos() + (rest ? 0x200 - rest : 0);    // Record our new end of archive
    }
    if (rest) {
        char buffer[0x201];
        for (uint i = 0; i < 0x200; ++i) {
            buffer[i] = 0;
        }
        qint64 nwritten = device()->write(buffer, 0x200 - rest);
        const bool ok = nwritten == 0x200 - rest;

        if (!ok) {
            setErrorString(
                tr("Couldn't write alignment: %1")
                .arg(device()->errorString()));
        }

        return ok;
    }
    return true;
}

/*** Some help from the tar sources
struct posix_header
{                               byte offset
  char name[100];               *   0 *     0x0
  char mode[8];                 * 100 *     0x64
  char uid[8];                  * 108 *     0x6c
  char gid[8];                  * 116 *     0x74
  char size[12];                * 124 *     0x7c
  char mtime[12];               * 136 *     0x88
  char chksum[8];               * 148 *     0x94
  char typeflag;                * 156 *     0x9c
  char linkname[100];           * 157 *     0x9d
  char magic[6];                * 257 *     0x101
  char version[2];              * 263 *     0x107
  char uname[32];               * 265 *     0x109
  char gname[32];               * 297 *     0x129
  char devmajor[8];             * 329 *     0x149
  char devminor[8];             * 337 *     ...
  char prefix[155];             * 345 *
                                * 500 *
};
*/

void KTar::KTarPrivate::fillBuffer(char *buffer,
                                   const char *mode, qint64 size, const QDateTime &mtime, char typeflag,
                                   const char *uname, const char *gname)
{
    // mode (as in stpos())
    assert(strlen(mode) == 6);
    memcpy(buffer + 0x64, mode, 6);
    buffer[0x6a] = ' ';
    buffer[0x6b] = '\0';

    // dummy uid
    strcpy(buffer + 0x6c, "   765 ");  // 501 in decimal
    // dummy gid
    strcpy(buffer + 0x74, "   144 ");  // 100 in decimal

    // size
    QByteArray s = QByteArray::number(size, 8);   // octal
    s = s.rightJustified(11, '0');
    memcpy(buffer + 0x7c, s.data(), 11);
    buffer[0x87] = ' '; // space-terminate (no null after)

    // modification time
    const QDateTime modificationTime = mtime.isValid() ? mtime : QDateTime::currentDateTime();
    s = QByteArray::number(static_cast<qulonglong>(modificationTime.toMSecsSinceEpoch() / 1000), 8);   // octal
    s = s.rightJustified(11, '0');
    memcpy(buffer + 0x88, s.data(), 11);
    buffer[0x93] = ' '; // space-terminate (no null after) -- well current tar writes a null byte

    // spaces, replaced by the check sum later
    buffer[0x94] = 0x20;
    buffer[0x95] = 0x20;
    buffer[0x96] = 0x20;
    buffer[0x97] = 0x20;
    buffer[0x98] = 0x20;
    buffer[0x99] = 0x20;

    /* From the tar sources :
       Fill in the checksum field.  It's formatted differently from the
       other fields: it has [6] digits, a null, then a space -- rather than
       digits, a space, then a null. */

    buffer[0x9a] = '\0';
    buffer[0x9b] = ' ';

    // type flag (dir, file, link)
    buffer[0x9c] = typeflag;

    // magic + version
    strcpy(buffer + 0x101, "ustar");
    strcpy(buffer + 0x107, "00");

    // user
    strcpy(buffer + 0x109, uname);
    // group
    strcpy(buffer + 0x129, gname);

    // Header check sum
    int check = 32;
    for (uint j = 0; j < 0x200; ++j) {
        check += static_cast<unsigned char>(buffer[j]);
    }
    s = QByteArray::number(check, 8);   // octal
    s = s.rightJustified(6, '0');
    memcpy(buffer + 0x94, s.constData(), 6);
}

void KTar::KTarPrivate::writeLonglink(char *buffer, const QByteArray &name, char typeflag,
                                      const char *uname, const char *gname)
{
    strcpy(buffer, "././@LongLink");
    qint64 namelen = name.length() + 1;
    fillBuffer(buffer, "     0", namelen, QDateTime(), typeflag, uname, gname);
    q->device()->write(buffer, 0x200);   // TODO error checking
    qint64 offset = 0;
    while (namelen > 0) {
        int chunksize = qMin(namelen, 0x200LL);
        memcpy(buffer, name.data() + offset, chunksize);
        // write long name
        q->device()->write(buffer, 0x200);   // TODO error checking
        // not even needed to reclear the buffer, tar doesn't do it
        namelen -= chunksize;
        offset += 0x200;
    }/*wend*/
}

bool KTar::doPrepareWriting(const QString &name, const QString &user,
                            const QString &group, qint64 size, mode_t perm,
                            const QDateTime & /*atime*/, const QDateTime &mtime, const QDateTime & /*ctime*/)
{
    if (!isOpen()) {
        setErrorString(tr("Application error: TAR file must be open before being written into"));
        qCWarning(KArchiveLog) << "doPrepareWriting failed: !isOpen()";
        return false;
    }

    if (!(mode() & QIODevice::WriteOnly)) {
        setErrorString(tr("Application error: attempted to write into non-writable 7-Zip file"));
        qCWarning(KArchiveLog) << "doPrepareWriting failed: !(mode() & QIODevice::WriteOnly)";
        return false;
    }

    // In some tar files we can find dir/./file => call cleanPath
    QString fileName(QDir::cleanPath(name));

    /*
      // Create toplevel dirs
      // Commented out by David since it's not necessary, and if anybody thinks it is,
      // he needs to implement a findOrCreate equivalent in writeDir.
      // But as KTar and the "tar" program both handle tar files without
      // dir entries, there's really no need for that
      QString tmp ( fileName );
      int i = tmp.lastIndexOf( '/' );
      if ( i != -1 )
      {
      QString d = tmp.left( i + 1 ); // contains trailing slash
      if ( !m_dirList.contains( d ) )
      {
      tmp = tmp.mid( i + 1 );
      writeDir( d, user, group ); // WARNING : this one doesn't create its toplevel dirs
      }
      }
    */

    char buffer[0x201];
    memset(buffer, 0, 0x200);
    if ((mode() & QIODevice::ReadWrite) == QIODevice::ReadWrite) {
        device()->seek(d->tarEnd);    // Go to end of archive as might have moved with a read
    }

    // provide converted stuff we need later on
    const QByteArray encodedFileName = QFile::encodeName(fileName);
    const QByteArray uname = user.toLocal8Bit();
    const QByteArray gname = group.toLocal8Bit();

    // If more than 100 bytes, we need to use the LongLink trick
    if (encodedFileName.length() > 99) {
        d->writeLonglink(buffer, encodedFileName, 'L', uname.constData(), gname.constData());
    }

    // Write (potentially truncated) name
    strncpy(buffer, encodedFileName.constData(), 99);
    buffer[99] = 0;
    // zero out the rest (except for what gets filled anyways)
    memset(buffer + 0x9d, 0, 0x200 - 0x9d);

    QByteArray permstr = QByteArray::number(static_cast<unsigned int>(perm), 8);
    permstr = permstr.rightJustified(6, '0');
    d->fillBuffer(buffer, permstr.constData(), size, mtime, 0x30, uname.constData(), gname.constData());

    // Write header
    if (device()->write(buffer, 0x200) != 0x200) {
        setErrorString(
            tr("Failed to write header: %1")
            .arg(device()->errorString()));
        return false;
    } else {
        return true;
    }
}

bool KTar::doWriteDir(const QString &name, const QString &user,
                      const QString &group, mode_t perm,
                      const QDateTime & /*atime*/, const QDateTime &mtime, const QDateTime & /*ctime*/)
{
    if (!isOpen()) {
        setErrorString(tr("Application error: TAR file must be open before being written into"));
        qCWarning(KArchiveLog) << "doWriteDir failed: !isOpen()";
        return false;
    }

    if (!(mode() & QIODevice::WriteOnly)) {
        setErrorString(tr("Application error: attempted to write into non-writable TAR file"));
        qCWarning(KArchiveLog) << "doWriteDir failed: !(mode() & QIODevice::WriteOnly)";
        return false;
    }

    // In some tar files we can find dir/./ => call cleanPath
    QString dirName(QDir::cleanPath(name));

    // Need trailing '/'
    if (!dirName.endsWith(QLatin1Char('/'))) {
        dirName += QLatin1Char('/');
    }

    if (d->dirList.contains(dirName)) {
        return true;    // already there
    }

    char buffer[0x201];
    memset(buffer, 0, 0x200);
    if ((mode() & QIODevice::ReadWrite) == QIODevice::ReadWrite) {
        device()->seek(d->tarEnd);    // Go to end of archive as might have moved with a read
    }

    // provide converted stuff we need lateron
    QByteArray encodedDirname = QFile::encodeName(dirName);
    QByteArray uname = user.toLocal8Bit();
    QByteArray gname = group.toLocal8Bit();

    // If more than 100 bytes, we need to use the LongLink trick
    if (encodedDirname.length() > 99) {
        d->writeLonglink(buffer, encodedDirname, 'L', uname.constData(), gname.constData());
    }

    // Write (potentially truncated) name
    strncpy(buffer, encodedDirname.constData(), 99);
    buffer[99] = 0;
    // zero out the rest (except for what gets filled anyways)
    memset(buffer + 0x9d, 0, 0x200 - 0x9d);

    QByteArray permstr = QByteArray::number(static_cast<unsigned int>(perm), 8);
    permstr = permstr.rightJustified(6, ' ');
    d->fillBuffer(buffer, permstr.constData(), 0, mtime, 0x35, uname.constData(), gname.constData());

    // Write header
    device()->write(buffer, 0x200);
    if ((mode() & QIODevice::ReadWrite) == QIODevice::ReadWrite) {
        d->tarEnd = device()->pos();
    }

    d->dirList.append(dirName);   // contains trailing slash
    return true; // TODO if wanted, better error control
}

bool KTar::doWriteSymLink(const QString &name, const QString &target,
                          const QString &user, const QString &group,
                          mode_t perm, const QDateTime & /*atime*/, const QDateTime &mtime, const QDateTime & /*ctime*/)
{
    if (!isOpen()) {
        setErrorString(tr("Application error: TAR file must be open before being written into"));
        qCWarning(KArchiveLog) << "doWriteSymLink failed: !isOpen()";
        return false;
    }

    if (!(mode() & QIODevice::WriteOnly)) {
        setErrorString(tr("Application error: attempted to write into non-writable TAR file"));
        qCWarning(KArchiveLog) << "doWriteSymLink failed: !(mode() & QIODevice::WriteOnly)";
        return false;
    }

    // In some tar files we can find dir/./file => call cleanPath
    QString fileName(QDir::cleanPath(name));

    char buffer[0x201];
    memset(buffer, 0, 0x200);
    if ((mode() & QIODevice::ReadWrite) == QIODevice::ReadWrite) {
        device()->seek(d->tarEnd);    // Go to end of archive as might have moved with a read
    }

    // provide converted stuff we need lateron
    QByteArray encodedFileName = QFile::encodeName(fileName);
    QByteArray encodedTarget = QFile::encodeName(target);
    QByteArray uname = user.toLocal8Bit();
    QByteArray gname = group.toLocal8Bit();

    // If more than 100 bytes, we need to use the LongLink trick
    if (encodedTarget.length() > 99) {
        d->writeLonglink(buffer, encodedTarget, 'K', uname.constData(), gname.constData());
    }
    if (encodedFileName.length() > 99) {
        d->writeLonglink(buffer, encodedFileName, 'L', uname.constData(), gname.constData());
    }

    // Write (potentially truncated) name
    strncpy(buffer, encodedFileName.constData(), 99);
    buffer[99] = 0;
    // Write (potentially truncated) symlink target
    strncpy(buffer + 0x9d, encodedTarget.constData(), 99);
    buffer[0x9d + 99] = 0;
    // zero out the rest
    memset(buffer + 0x9d + 100, 0, 0x200 - 100 - 0x9d);

    QByteArray permstr = QByteArray::number(static_cast<unsigned int>(perm), 8);
    permstr = permstr.rightJustified(6, ' ');
    d->fillBuffer(buffer, permstr.constData(), 0, mtime, 0x32, uname.constData(), gname.constData());

    // Write header
    bool retval = device()->write(buffer, 0x200) == 0x200;
    if ((mode() & QIODevice::ReadWrite) == QIODevice::ReadWrite) {
        d->tarEnd = device()->pos();
    }
    return retval;
}

void KTar::virtual_hook(int id, void *data)
{
    KArchive::virtual_hook(id, data);
}
