/* This file is part of the KDE libraries
   SPDX-FileCopyrightText: 2011 Mario Bensi <mbensi@ipsquad.net>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "k7zip.h"
#include "karchive_p.h"
#include "loggingcategory.h"

#include <QDebug>
#include <QDir>
#include <QBuffer>
#include <QFile>
#include <qplatformdefs.h>

#include "kcompressiondevice.h"
#include <kfilterbase.h>
#include <kxzfilter.h>
#include "klimitediodevice_p.h"

#include <time.h> // time()
#include <memory>
#include "zlib.h"

#ifndef QT_STAT_LNK
#       define QT_STAT_LNK 0120000
#endif // QT_STAT_LNK

////////////////////////////////////////////////////////////////////////
/////////////////////////// K7Zip //////////////////////////////////////
////////////////////////////////////////////////////////////////////////

#define BUFFER_SIZE 8*1024

static const unsigned char k7zip_signature[6] = {'7', 'z', 0xBC, 0xAF, 0x27, 0x1C};
//static const unsigned char XZ_HEADER_MAGIC[6] = { 0xFD, '7', 'z', 'X', 'Z', 0x00 };

#define GetUi16(p, offset) (((unsigned char)p[offset+0]) | (((unsigned char)p[1]) << 8))

#define GetUi32(p, offset) ( \
                             ((unsigned char)p[offset+0])        | \
                             (((unsigned char)p[offset+1]) <<  8) | \
                             (((unsigned char)p[offset+2]) << 16) | \
                             (((unsigned char)p[offset+3]) << 24))

#define GetUi64(p, offset) ((quint32)GetUi32(p, offset) | (((quint64)GetUi32(p, offset + 4)) << 32))

#define LZMA2_DIC_SIZE_FROM_PROP(p) (((quint32)2 | ((p) & 1)) << ((p) / 2 + 11))

#define FILE_ATTRIBUTE_READONLY             1
#define FILE_ATTRIBUTE_HIDDEN               2
#define FILE_ATTRIBUTE_SYSTEM               4
#define FILE_ATTRIBUTE_DIRECTORY           16
#define FILE_ATTRIBUTE_ARCHIVE             32
#define FILE_ATTRIBUTE_DEVICE              64
#define FILE_ATTRIBUTE_NORMAL             128
#define FILE_ATTRIBUTE_TEMPORARY          256
#define FILE_ATTRIBUTE_SPARSE_FILE        512
#define FILE_ATTRIBUTE_REPARSE_POINT     1024
#define FILE_ATTRIBUTE_COMPRESSED        2048
#define FILE_ATTRIBUTE_OFFLINE          0x1000
#define FILE_ATTRIBUTE_ENCRYPTED        0x4000
#define FILE_ATTRIBUTE_UNIX_EXTENSION   0x8000   /* trick for Unix */

enum HeaderType {
    kEnd,

    kHeader,

    kArchiveProperties,

    kAdditionalStreamsInfo,
    kMainStreamsInfo,
    kFilesInfo,

    kPackInfo,
    kUnpackInfo,
    kSubStreamsInfo,

    kSize,
    kCRC,

    kFolder,

    kCodersUnpackSize,
    kNumUnpackStream,

    kEmptyStream,
    kEmptyFile,
    kAnti,

    kName,
    kCTime,
    kATime,
    kMTime,
    kAttributes,
    kComment,

    kEncodedHeader,

    kStartPos,
    kDummy
};

// Method ID
// static const quint64 k_Copy = 0x00;
// static const quint64 k_Delta = 0x03;
// static const quint64 k_x86 = 0x04; //BCJ
// static const quint64 k_PPC = 0x05; // BIG Endian
// static const quint64 k_IA64 = 0x06;
// static const quint64 k_ARM = 0x07; // little Endian
// static const quint64 k_ARM_Thumb = 0x08; // little Endian
// static const quint64 k_SPARC = 0x09;
static const quint64 k_LZMA2 = 0x21;
// static const quint64 k_Swap2 = 0x020302;
// static const quint64 k_Swap4 = 0x020304;
static const quint64 k_LZMA = 0x030101;
static const quint64 k_BCJ = 0x03030103;
static const quint64 k_BCJ2 = 0x0303011B;
// static const quint64 k_7zPPC = 0x03030205;
// static const quint64 k_Alpha = 0x03030301;
// static const quint64 k_7zIA64 = 0x03030401;
// static const quint64 k_7zARM = 0x03030501;
// static const quint64 k_M68 = 0x03030605; //Big Endian
// static const quint64 k_ARMT = 0x03030701;
// static const quint64 k_7zSPARC = 0x03030805;
static const quint64 k_PPMD = 0x030401;
// static const quint64 k_Experimental = 0x037F01;
// static const quint64 k_Shrink = 0x040101;
// static const quint64 k_Implode = 0x040106;
// static const quint64 k_Deflate = 0x040108;
// static const quint64 k_Deflate64 = 0x040109;
// static const quint64 k_Imploding = 0x040110;
// static const quint64 k_Jpeg = 0x040160;
// static const quint64 k_WavPack = 0x040161;
// static const quint64 k_PPMd = 0x040162;
// static const quint64 k_wzAES = 0x040163;
static const quint64 k_BZip2 = 0x040202;
// static const quint64 k_Rar15 = 0x040301;
// static const quint64 k_Rar20 = 0x040302;
// static const quint64 k_Rar29 = 0x040303;
// static const quint64 k_Arj = 0x040401; //1 2 3
// static const quint64 k_Arj4 = 0x040402;
// static const quint64 k_Z = 0x0405;
// static const quint64 k_Lzh = 0x0406;
// static const quint64 k_Cab = 0x0408;
// static const quint64 k_DeflateNSIS = 0x040901;
// static const quint64 k_Bzip2NSIS = 0x040902;
static const quint64 k_AES = 0x06F10701;

/**
 * A K7ZipFileEntry represents a file in a 7zip archive.
 */
class KARCHIVE_EXPORT K7ZipFileEntry : public KArchiveFile
{
public:
    K7ZipFileEntry(K7Zip *zip, const QString &name, int access, const QDateTime &date,
                   const QString &user, const QString &group, const QString &symlink,
                   qint64 pos, qint64 size, const QByteArray &data);

    ~K7ZipFileEntry();

    /**
     * @return the content of this file.
     * Call data() with care (only once per file), this data isn't cached.
     */
    QByteArray data() const override;

    /**
     * This method returns QIODevice (internal class: KLimitedIODevice)
     * on top of the underlying QIODevice. This is obviously for reading only.
     *
     * WARNING: Note that the ownership of the device is being transferred to the caller,
     * who will have to delete it.
     *
     * The returned device auto-opens (in readonly mode), no need to open it.
     * @return the QIODevice of the file
     */
    QIODevice *createDevice() const override;

private:
    const QByteArray m_data;
    QBuffer *m_buffer;
};

K7ZipFileEntry::K7ZipFileEntry(K7Zip *zip, const QString &name, int access, const QDateTime &date,
                               const QString &user, const QString &group, const QString &symlink,
                               qint64 pos, qint64 size, const QByteArray &data)
    : KArchiveFile(zip, name, access, date, user, group, symlink, pos, size)
    , m_data(data)
    , m_buffer(new QBuffer)
{
    m_buffer->setData(m_data);
    m_buffer->open(QIODevice::ReadOnly);
}

K7ZipFileEntry::~K7ZipFileEntry()
{
    delete m_buffer;
}

QByteArray K7ZipFileEntry::data() const
{
    return m_data.mid(position(), size());
}

QIODevice *K7ZipFileEntry::createDevice() const
{
    return new KLimitedIODevice(m_buffer, position(), size());
}

class FileInfo
{
public:
    FileInfo()
        : size(0)
        , attributes(0)
        , crc(0)
        , attribDefined(false)
        , crcDefined(false)
        , hasStream(false)
        , isDir(false)
    {
    }

    QString path;
    quint64 size;
    quint32 attributes;
    quint32 crc;
    bool attribDefined;
    bool crcDefined;
    bool hasStream;
    bool isDir;
};

class Folder
{
public:
    class FolderInfo
    {
    public:
        FolderInfo()
            : numInStreams(0)
            , numOutStreams(0)
            , methodID(0)
        {
        }

        bool isSimpleCoder() const
        {
            return (numInStreams == 1) && (numOutStreams == 1);
        }

        int numInStreams;
        int numOutStreams;
        QVector<unsigned char> properties;
        quint64 methodID;
    };

    Folder()
        : unpackCRCDefined(false)
        , unpackCRC(0)
    {
    }

    ~Folder()
    {
        qDeleteAll(folderInfos);
    }

    Q_DISABLE_COPY(Folder)

    quint64 getUnpackSize() const
    {
        if (unpackSizes.isEmpty()) {
            return 0;
        }
        for (int i = unpackSizes.size() - 1; i >= 0; i--) {
            if (findBindPairForOutStream(i) < 0) {
                return unpackSizes.at(i);
            }
        }
        return 0;
    }

    int getNumOutStreams() const
    {
        int result = 0;
        for (int i = 0; i < folderInfos.size(); i++) {
            result += folderInfos.at(i)->numOutStreams;
        }
        return result;
    }

    quint32 getCoderInStreamIndex(quint32 coderIndex) const
    {
        quint32 streamIndex = 0;
        for (quint32 i = 0; i < coderIndex; i++) {
            streamIndex += folderInfos.at(i)->numInStreams;
        }
        return streamIndex;
    }

    quint32 getCoderOutStreamIndex(quint32 coderIndex) const
    {
        quint32 streamIndex = 0;
        for (quint32 i = 0; i < coderIndex; i++) {
            streamIndex += folderInfos.at(i)->numOutStreams;
        }
        return streamIndex;
    }

    int findBindPairForInStream(size_t inStreamIndex) const
    {
        for (int i = 0; i < inIndexes.size(); i++) {
            if (inIndexes[i] == inStreamIndex) {
                return i;
            }
        }
        return -1;
    }

    int findBindPairForOutStream(size_t outStreamIndex) const
    {
        for (int i = 0; i < outIndexes.size(); i++) {
            if (outIndexes[i] == outStreamIndex) {
                return i;
            }
        }
        return -1;
    }

    int findPackStreamArrayIndex(size_t inStreamIndex) const
    {
        for (int i = 0; i < packedStreams.size(); i++) {
            if (packedStreams[i] == inStreamIndex) {
                return i;
            }
        }
        return -1;
    }

    void findInStream(quint32 streamIndex, quint32 &coderIndex, quint32 &coderStreamIndex) const
    {
        for (coderIndex = 0; coderIndex < (quint32)folderInfos.size(); coderIndex++) {
            quint32 curSize = folderInfos[coderIndex]->numInStreams;
            if (streamIndex < curSize) {
                coderStreamIndex = streamIndex;
                return;
            }
            streamIndex -= curSize;
        }
    }

    void findOutStream(quint32 streamIndex, quint32 &coderIndex, quint32 &coderStreamIndex) const
    {
        for (coderIndex = 0; coderIndex < (quint32)folderInfos.size(); coderIndex++) {
            quint32 curSize = folderInfos[coderIndex]->numOutStreams;
            if (streamIndex < curSize) {
                coderStreamIndex = streamIndex;
                return;
            }
            streamIndex -= curSize;
        }
    }

    bool isEncrypted() const
    {
        for (int i = folderInfos.size() - 1; i >= 0; i--) {
            if (folderInfos.at(i)->methodID == k_AES) {
                return true;
            }
        }
        return false;
    }

    //bool CheckStructure() const;

    bool unpackCRCDefined;
    quint32 unpackCRC;
    QVector<FolderInfo *> folderInfos;
    QVector<quint64> inIndexes;
    QVector<quint64> outIndexes;
    QVector<quint64> packedStreams;
    QVector<quint64> unpackSizes;
};

class Q_DECL_HIDDEN K7Zip::K7ZipPrivate
{
public:
    K7ZipPrivate(K7Zip *parent)
        : q(parent)
        , packPos(0)
        , numPackStreams(0)
        , buffer(nullptr)
        , pos(0)
        , end(0)
        , headerSize(0)
        , countSize(0)
        , m_currentFile(nullptr)
    {
    }

    ~K7ZipPrivate()
    {
        qDeleteAll(folders);
        qDeleteAll(fileInfos);
    }

    K7Zip *q;

    QVector<bool> packCRCsDefined;
    QVector<quint32> packCRCs;
    QVector<quint64> numUnpackStreamsInFolders;

    QVector<Folder *> folders;
    QVector<FileInfo *> fileInfos;
    // File informations
    QVector<bool> cTimesDefined;
    QVector<quint64> cTimes;
    QVector<bool> aTimesDefined;
    QVector<quint64> aTimes;
    QVector<bool> mTimesDefined;
    QVector<quint64> mTimes;
    QVector<bool> startPositionsDefined;
    QVector<quint64> startPositions;
    QVector<int> fileInfoPopIDs;

    quint64 packPos;
    quint64 numPackStreams;
    QVector<quint64> packSizes;
    QVector<quint64> unpackSizes;
    QVector<bool> digestsDefined;
    QVector<quint32> digests;

    QVector<bool> isAnti;

    const char *buffer;
    quint64 pos;
    quint64 end;
    quint64 headerSize;
    quint64 countSize;

    //Write
    QByteArray header;
    QByteArray outData; // Store data in this buffer before compress and write in archive.
    K7ZipFileEntry *m_currentFile;
    QVector<KArchiveEntry *> m_entryList;

    void clear()
    {
        packCRCsDefined.clear();
        packCRCs.clear();
        numUnpackStreamsInFolders.clear();
        qDeleteAll(folders);
        folders.clear();
        qDeleteAll(fileInfos);
        fileInfos.clear();
        cTimesDefined.clear();
        cTimes.clear();
        aTimesDefined.clear();
        aTimes.clear();
        mTimesDefined.clear();
        mTimes.clear();
        startPositionsDefined.clear();
        startPositions.clear();
        fileInfoPopIDs.clear();
        packSizes.clear();
        unpackSizes.clear();
        digestsDefined.clear();
        digests.clear();
        isAnti.clear();

        buffer = nullptr;
        pos = 0;
        end = 0;
        headerSize = 0;
        countSize = 0;
    }

    // Read
    int readByte();
    quint32 readUInt32();
    quint64 readUInt64();
    quint64 readNumber();
    QString readString();
    void readHashDigests(int numItems, QVector<bool> &digestsDefined, QVector<quint32> &digests);
    void readBoolVector(int numItems, QVector<bool> &v);
    void readBoolVector2(int numItems, QVector<bool> &v);
    void skipData(int size);
    bool findAttribute(int attribute);
    bool readUInt64DefVector(int numFiles, QVector<quint64> &values, QVector<bool> &defined);

    Folder *folderItem();
    bool readMainStreamsInfo();
    bool readPackInfo();
    bool readUnpackInfo();
    bool readSubStreamsInfo();
    QByteArray readAndDecodePackedStreams(bool readMainStreamInfo = true);

    //Write
    void createItemsFromEntities(const KArchiveDirectory *, const QString &, QByteArray &);
    void writeByte(unsigned char b);
    void writeNumber(quint64 value);
    void writeBoolVector(const QVector<bool> &boolVector);
    void writeUInt32(quint32 value);
    void writeUInt64(quint64 value);
    void writeHashDigests(const QVector<bool> &digestsDefined, const QVector<quint32> &digests);
    void writeAlignedBoolHeader(const QVector<bool> &v, int numDefined, int type, unsigned itemSize);
    void writeUInt64DefVector(const QVector<quint64> &v, const QVector<bool> &defined, int type);
    void writeFolder(const Folder *folder);
    void writePackInfo(quint64 dataOffset, QVector<quint64> &packedSizes, QVector<bool> &packedCRCsDefined, QVector<quint32> &packedCRCs);
    void writeUnpackInfo(const QVector<Folder *> &folderItems);
    void writeSubStreamsInfo(const QVector<quint64> &unpackSizes, const QVector<bool> &digestsDefined, const QVector<quint32> &digests);
    void writeHeader(quint64 &headerOffset);
    void writeSignature();
    void writeStartHeader(const quint64 nextHeaderSize, const quint32 nextHeaderCRC, const quint64 nextHeaderOffset);
    QByteArray encodeStream(QVector<quint64> &packSizes, QVector<Folder *> &folds);
};

K7Zip::K7Zip(const QString &fileName)
    : KArchive(fileName)
    , d(new K7ZipPrivate(this))
{
}

K7Zip::K7Zip(QIODevice *dev)
    : KArchive(dev)
    , d(new K7ZipPrivate(this))
{
    Q_ASSERT(dev);
}

K7Zip::~K7Zip()
{
    if (isOpen()) {
        close();
    }

    delete d;
}

int K7Zip::K7ZipPrivate::readByte()
{
    if (!buffer || pos + 1 > end) {
        return -1;
    }
    return buffer[pos++];
}

quint32 K7Zip::K7ZipPrivate::readUInt32()
{
    if (!buffer || (quint64)(pos + 4) > end) {
        qCDebug(KArchiveLog) << "error size";
        return 0;
    }

    quint32 res = GetUi32(buffer, pos);
    pos += 4;
    return res;
}

quint64 K7Zip::K7ZipPrivate::readUInt64()
{
    if (!buffer || (quint64)(pos + 8) > end) {
        qCDebug(KArchiveLog) << "error size";
        return 0;
    }

    quint64 res = GetUi64(buffer, pos);
    pos += 8;
    return res;
}

quint64 K7Zip::K7ZipPrivate::readNumber()
{
    if (!buffer || (quint64)(pos + 8) > end) {
        return 0;
    }

    unsigned char firstByte = buffer[pos++];
    unsigned char mask = 0x80;
    quint64 value = 0;
    for (int i = 0; i < 8; i++) {
        if ((firstByte & mask) == 0) {
            quint64 highPart = firstByte & (mask - 1);
            value += (highPart << (i * 8));
            return value;
        }
        value |= ((unsigned char)buffer[pos++] << (8 * i));
        mask >>= 1;
    }
    return value;
}

QString K7Zip::K7ZipPrivate::readString()
{
    if (!buffer) {
        return QString();
    }

    const char *buf = buffer + pos;
    size_t rem = (end - pos) / 2 * 2;
    {
        size_t i;
        for (i = 0; i < rem; i += 2) {
            if (buf[i] == 0 && buf[i + 1] == 0) {
                break;
            }
        }
        if (i == rem) {
            qCDebug(KArchiveLog) << "read string error";
            return QString();
        }
        rem = i;
    }

    int len = (int)(rem / 2);
    if (len < 0 || (size_t)len * 2 != rem) {
        qCDebug(KArchiveLog) << "read string unsupported";
        return QString();
    }

    QString p;
    for (int i = 0; i < len; i++, buf += 2) {
        p += (wchar_t)GetUi16(buf, 0);
    }

    pos += rem + 2;
    return p;
}

void K7Zip::K7ZipPrivate::skipData(int size)
{
    if (!buffer || pos + size > end) {
        return;
    }
    pos += size;
}

bool K7Zip::K7ZipPrivate::findAttribute(int attribute)
{
    if (!buffer) {
        return false;
    }

    for (;;) {
        int type = readByte();
        if (type == attribute) {
            return true;
        }
        if (type == kEnd) {
            return false;
        }
        skipData(readNumber());
    }
}

void K7Zip::K7ZipPrivate::readBoolVector(int numItems, QVector<bool> &v)
{
    if (!buffer) {
        return;
    }

    unsigned char b = 0;
    unsigned char mask = 0;
    for (int i = 0; i < numItems; i++) {
        if (mask == 0) {
            b = readByte();
            mask = 0x80;
        }
        v.append((b & mask) != 0);
        mask >>= 1;
    }
}

void K7Zip::K7ZipPrivate::readBoolVector2(int numItems, QVector<bool> &v)
{
    if (!buffer) {
        return;
    }

    int allAreDefined = readByte();
    if (allAreDefined == 0) {
        readBoolVector(numItems, v);
        return;
    }

    for (int i = 0; i < numItems; i++) {
        v.append(true);
    }
}

void K7Zip::K7ZipPrivate::readHashDigests(int numItems,
                                          QVector<bool> &digestsDefined,
                                          QVector<quint32> &digests)
{
    if (!buffer) {
        return;
    }

    readBoolVector2(numItems, digestsDefined);
    for (int i = 0; i < numItems; i++) {
        quint32 crc = 0;
        if (digestsDefined[i]) {
            crc = GetUi32(buffer, pos);
            pos += 4;
        }
        digests.append(crc);
    }
}

Folder *K7Zip::K7ZipPrivate::folderItem()
{
    if (!buffer) {
        return nullptr;
    }

    Folder *folder = new Folder;
    int numCoders = readNumber();

    quint64 numInStreamsTotal = 0;
    quint64 numOutStreamsTotal = 0;
    for (int i = 0; i < numCoders; ++i) {
        //BYTE
        //    {
        //      0:3 CodecIdSize
        //      4:  Is Complex Coder
        //      5:  There Are Attributes
        //      6:  Reserved
        //      7:  There are more alternative methods. (Not used
        //      anymore, must be 0).
        //    }
        unsigned char coderInfo = readByte();
        int codecIdSize = (coderInfo & 0xF);
        if (codecIdSize > 8) {
            qCDebug(KArchiveLog) << "unsupported codec id size";
            delete folder;
            return nullptr;
        }
        Folder::FolderInfo *info = new Folder::FolderInfo();
        std::unique_ptr<unsigned char[]> codecID(new unsigned char[codecIdSize]);
        for (int i = 0; i < codecIdSize; ++i) {
            codecID[i] = readByte();
        }

        int id = 0;
        for (int j = 0; j < codecIdSize; j++) {
            id |= codecID[codecIdSize - 1 - j] << (8 * j);
        }
        info->methodID = id;

        //if (Is Complex Coder)
        if ((coderInfo & 0x10) != 0) {
            info->numInStreams = readNumber();
            info->numOutStreams = readNumber();
        } else {
            info->numInStreams = 1;
            info->numOutStreams = 1;
        }

        //if (There Are Attributes)
        if ((coderInfo & 0x20) != 0) {
            int propertiesSize = readNumber();
            for (int i = 0; i < propertiesSize; ++i) {
                info->properties.append(readByte());
            }
        }

        if ((coderInfo & 0x80) != 0) {
            qCDebug(KArchiveLog) << "unsupported";
            delete info;
            delete folder;
            return nullptr;
        }

        numInStreamsTotal += info->numInStreams;
        numOutStreamsTotal += info->numOutStreams;
        folder->folderInfos.append(info);
    }

    int numBindPairs = numOutStreamsTotal - 1;
    for (int i = 0; i < numBindPairs; i++) {
        folder->inIndexes.append(readNumber());
        folder->outIndexes.append(readNumber());
    }

    int numPackedStreams = numInStreamsTotal - numBindPairs;
    if (numPackedStreams > 1) {
        for (int i = 0; i < numPackedStreams; ++i) {
            folder->packedStreams.append(readNumber());
        }
    } else {
        if (numPackedStreams == 1) {
            for (quint64 i = 0; i < numInStreamsTotal; i++) {
                if (folder->findBindPairForInStream(i) < 0) {
                    folder->packedStreams.append(i);
                    break;
                }
            }
            if (folder->packedStreams.size() != 1) {
                delete folder;
                return nullptr;
            }
        }
    }
    return folder;
}

bool K7Zip::K7ZipPrivate::readUInt64DefVector(int numFiles, QVector<quint64> &values, QVector<bool> &defined)
{
    if (!buffer) {
        return false;
    }

    readBoolVector2(numFiles, defined);

    int external = readByte();
    if (external != 0) {
        int dataIndex = readNumber();
        if (dataIndex < 0 /*|| dataIndex >= dataVector->Size()*/) {
            qCDebug(KArchiveLog) << "wrong data index";
            return false;
        }

        // TODO : go to the new index
    }

    for (int i = 0; i < numFiles; i++) {
        quint64 t = 0;
        if (defined[i]) {
            t = readUInt64();
        }
        values.append(t);
    }
    return true;
}

bool K7Zip::K7ZipPrivate::readPackInfo()
{
    if (!buffer) {
        return false;
    }

    packPos = readNumber();
    numPackStreams = readNumber();
    packSizes.clear();

    packCRCsDefined.clear();
    packCRCs.clear();

    if (!findAttribute(kSize)) {
        qCDebug(KArchiveLog) << "kSize not found";
        return false;
    }

    for (quint64 i = 0; i < numPackStreams; ++i) {
        packSizes.append(readNumber());
    }

    for (;;) {
        int type = readByte();
        if (type == kEnd) {
            break;
        }
        if (type == kCRC) {
            readHashDigests(numPackStreams, packCRCsDefined, packCRCs);
            continue;
        }
        skipData(readNumber());
    }

    if (packCRCs.isEmpty()) {
        for (quint64 i = 0; i < numPackStreams; ++i) {
            packCRCsDefined.append(false);
            packCRCs.append(0);
        }
    }
    return true;
}

bool K7Zip::K7ZipPrivate::readUnpackInfo()
{
    if (!buffer) {
        return false;
    }

    if (!findAttribute(kFolder)) {
        qCDebug(KArchiveLog) << "kFolder not found";
        return false;
    }

    int numFolders = readNumber();
    qDeleteAll(folders);
    folders.clear();
    int external = readByte();
    switch (external) {
    case 0: {
        for (int i = 0; i < numFolders; ++i) {
            folders.append(folderItem());
        }
        break;
    }
    case 1: {
        int dataIndex = readNumber();
        if (dataIndex < 0 /*|| dataIndex >= dataVector->Size()*/) {
            qCDebug(KArchiveLog) << "wrong data index";
        }
        // TODO : go to the new index
        break;
    }
    default:
        qCDebug(KArchiveLog) << "external error";
        return false;
    }

    if (!findAttribute(kCodersUnpackSize)) {
        qCDebug(KArchiveLog) << "kCodersUnpackSize not found";
        return false;
    }

    for (int i = 0; i < numFolders; ++i) {
        Folder *folder = folders.at(i);
        int numOutStreams = folder->getNumOutStreams();
        for (int j = 0; j < numOutStreams; ++j) {
            folder->unpackSizes.append(readNumber());
        }
    }

    for (;;) {
        int type = readByte();
        if (type == kEnd) {
            break;
        }
        if (type == kCRC) {
            QVector<bool> crcsDefined;
            QVector<quint32> crcs;
            readHashDigests(numFolders, crcsDefined, crcs);
            for (int i = 0; i < numFolders; i++) {
                Folder *folder = folders.at(i);
                folder->unpackCRCDefined = crcsDefined[i];
                folder->unpackCRC = crcs[i];
            }
            continue;
        }
        skipData(readNumber());
    }
    return true;
}

bool K7Zip::K7ZipPrivate::readSubStreamsInfo()
{
    if (!buffer) {
        return false;
    }

    numUnpackStreamsInFolders.clear();

    int type;
    for (;;) {
        type = readByte();
        if (type == kNumUnpackStream) {
            for (int i = 0; i < folders.size(); i++) {
                numUnpackStreamsInFolders.append(readNumber());
            }
            continue;
        }
        if (type == kCRC || type == kSize) {
            break;
        }
        if (type == kEnd) {
            break;
        }
        skipData(readNumber());
    }

    if (numUnpackStreamsInFolders.isEmpty()) {
        for (int i = 0; i < folders.size(); i++) {
            numUnpackStreamsInFolders.append(1);
        }
    }

    for (int i = 0; i < numUnpackStreamsInFolders.size(); i++) {
        quint64 numSubstreams = numUnpackStreamsInFolders.at(i);
        if (numSubstreams == 0) {
            continue;
        }
        quint64 sum = 0;
        for (quint64 j = 1; j < numSubstreams; j++) {
            if (type == kSize) {
                int size = readNumber();
                unpackSizes.append(size);
                sum += size;
            }
        }
        unpackSizes.append(folders.at(i)->getUnpackSize() - sum);
    }

    if (type == kSize) {
        type = readByte();
    }

    int numDigests = 0;
    int numDigestsTotal = 0;
    for (int i = 0; i < folders.size(); i++) {
        quint64 numSubstreams = numUnpackStreamsInFolders.at(i);
        if (numSubstreams != 1 || !folders.at(i)->unpackCRCDefined) {
            numDigests += numSubstreams;
        }
        numDigestsTotal += numSubstreams;
    }

    for (;;) {
        if (type == kCRC) {
            QVector<bool> digestsDefined2;
            QVector<quint32> digests2;
            readHashDigests(numDigests, digestsDefined2, digests2);
            int digestIndex = 0;
            for (int i = 0; i < folders.size(); i++) {
                quint64 numSubstreams = numUnpackStreamsInFolders.at(i);
                const Folder *folder = folders.at(i);
                if (numSubstreams == 1 && folder->unpackCRCDefined) {
                    digestsDefined.append(true);
                    digests.append(folder->unpackCRC);
                } else {
                    for (quint64 j = 0; j < numSubstreams; j++, digestIndex++) {
                        digestsDefined.append(digestsDefined2[digestIndex]);
                        digests.append(digests2[digestIndex]);
                    }
                }
            }
        } else if (type == kEnd) {
            if (digestsDefined.isEmpty()) {
                for (int i = 0; i < numDigestsTotal; i++) {
                    digestsDefined.append(false);
                    digests.append(0);
                }
            }

            break;
        } else {
            skipData(readNumber());
        }

        type = readByte();
    }
    return true;
}

#define TICKSPERSEC        10000000
#define TICKSPERMSEC       10000
#define SECSPERDAY         86400
#define SECSPERHOUR        3600
#define SECSPERMIN         60
#define EPOCHWEEKDAY       1  /* Jan 1, 1601 was Monday */
#define DAYSPERWEEK        7
#define DAYSPERQUADRICENTENNIUM (365 * 400 + 97)
#define DAYSPERNORMALQUADRENNIUM (365 * 4 + 1)
#define TICKS_1601_TO_1970 (SECS_1601_TO_1970 * TICKSPERSEC)
#define SECS_1601_TO_1970  ((369 * 365 + 89) * (unsigned long long)SECSPERDAY)

static uint toTimeT(const long long liTime)
{
    long long time = liTime / TICKSPERSEC;

    /* The native version of RtlTimeToTimeFields does not take leap seconds
     * into account */

    /* Split the time into days and seconds within the day */
    long int days = time / SECSPERDAY;
    int secondsInDay = time % SECSPERDAY;

    /* compute time of day */
    short hour = (short)(secondsInDay / SECSPERHOUR);
    secondsInDay = secondsInDay % SECSPERHOUR;
    short minute = (short)(secondsInDay / SECSPERMIN);
    short second = (short)(secondsInDay % SECSPERMIN);

    /* compute year, month and day of month. */
    long int cleaps = (3 * ((4 * days + 1227) / DAYSPERQUADRICENTENNIUM) + 3) / 4;
    days += 28188 + cleaps;
    long int years = (20 * days - 2442) / (5 * DAYSPERNORMALQUADRENNIUM);
    long int yearday = days - (years * DAYSPERNORMALQUADRENNIUM) / 4;
    long int months = (64 * yearday) / 1959;
    /* the result is based on a year starting on March.
     * To convert take 12 from Januari and Februari and
     * increase the year by one. */

    short month, year;
    if (months < 14) {
        month = (short)(months - 1);
        year = (short)(years + 1524);
    } else {
        month = (short)(months - 13);
        year = (short)(years + 1525);
    }
    /* calculation of day of month is based on the wonderful
     * sequence of INT( n * 30.6): it reproduces the·
     * 31-30-31-30-31-31 month lengths exactly for small n's */
    short day = (short)(yearday - (1959 * months) / 64);

    QDateTime t(QDate(year, month, day), QTime(hour, minute, second));
    t.setTimeSpec(Qt::UTC);
    return t.toSecsSinceEpoch();
}

long long rtlSecondsSince1970ToSpecTime(quint32 seconds)
{
    long long secs = seconds * (long long)TICKSPERSEC + TICKS_1601_TO_1970;
    return secs;
}

bool K7Zip::K7ZipPrivate::readMainStreamsInfo()
{
    if (!buffer) {
        return false;
    }

    quint32 type;
    for (;;) {
        type = readByte();
        if (type > ((quint32)1 << 30)) {
            qCDebug(KArchiveLog) << "type error";
            return false;
        }
        switch (type) {
        case kEnd:
            return true;
        case kPackInfo: {
            if (!readPackInfo()) {
                qCDebug(KArchiveLog) << "error during read pack information";
                return false;
            }
            break;
        }
        case kUnpackInfo: {
            if (!readUnpackInfo()) {
                qCDebug(KArchiveLog) << "error during read pack information";
                return false;
            }
            break;
        }
        case kSubStreamsInfo: {
            if (!readSubStreamsInfo()) {
                qCDebug(KArchiveLog) << "error during read substreams information";
                return false;
            }
            break;
        }
        default:
            qCDebug(KArchiveLog) << "Wrong type";
            return false;
        }
    }

    qCDebug(KArchiveLog) << "should not reach";
    return false;
}

static bool getInStream(const Folder *folder, quint32 streamIndex, int &seqInStream, quint32 &coderIndex)
{
    for (int i = 0; i < folder->packedStreams.size(); i++) {
        if (folder->packedStreams[i] == streamIndex) {
            seqInStream = i;
            return  true;
        }
    }

    int binderIndex = folder->findBindPairForInStream(streamIndex);
    if (binderIndex < 0) {
        return false;
    }

    quint32 coderStreamIndex;
    folder->findOutStream(folder->outIndexes[binderIndex],
                          coderIndex, coderStreamIndex);

    quint32 startIndex = folder->getCoderInStreamIndex(coderIndex);

    if (folder->folderInfos[coderIndex]->numInStreams > 1) {
        return false;
    }

    for (int i = 0; i < (int)folder->folderInfos[coderIndex]->numInStreams; i++) {
        getInStream(folder, startIndex + i, seqInStream, coderIndex);
    }

    return true;
}

static bool getOutStream(const Folder *folder, quint32 streamIndex, int &seqOutStream)
{
    QVector<quint32> outStreams;
    quint32 outStreamIndex = 0;
    for (int i = 0; i < folder->folderInfos.size(); i++) {
        const Folder::FolderInfo *coderInfo = folder->folderInfos.at(i);

        for (int j = 0; j < coderInfo->numOutStreams; j++, outStreamIndex++) {
            if (folder->findBindPairForOutStream(outStreamIndex) < 0) {
                outStreams.append(outStreamIndex);
            }
        }
    }

    for (int i = 0; i < outStreams.size(); i++) {
        if (outStreams[i] == streamIndex) {
            seqOutStream = i;
            return true;
        }
    }

    int binderIndex = folder->findBindPairForOutStream(streamIndex);
    if (binderIndex < 0) {
        return false;
    }

    quint32 coderIndex, coderStreamIndex;
    folder->findInStream(folder->inIndexes[binderIndex],
                         coderIndex, coderStreamIndex);

    quint32 startIndex = folder->getCoderOutStreamIndex(coderIndex);

    if (folder->folderInfos[coderIndex]->numOutStreams > 1) {
        return false;
    }

    for (int i = 0; i < (int)folder->folderInfos[coderIndex]->numOutStreams; i++) {
        getOutStream(folder, startIndex + i, seqOutStream);
    }

    return true;
}

const int kNumTopBits = 24;
const quint32 kTopValue = (1 << kNumTopBits);

class RangeDecoder
{
    int pos;

public:
    QByteArray stream;
    quint32 range;
    quint32 code;

    RangeDecoder()
        : pos(0)
    {
    }

    unsigned char readByte()
    {
        return stream[pos++];
    }

    void normalize()
    {
        while (range < kTopValue) {
            code = (code << 8) | readByte();
            range <<= 8;
        }
    }

    void setStream(const QByteArray &s)
    {
        stream = s;
    }

    void init()
    {
        code = 0;
        range = 0xFFFFFFFF;
        for (int i = 0; i < 5; i++) {
            code = (code << 8) | readByte();
        }
    }

    quint32 getThreshold(quint32 total)
    {
        return (code) / (range /= total);
    }

    void decode(quint32 start, quint32 size)
    {
        code -= start * range;
        range *= size;
        normalize();
    }

    quint32 decodeDirectBits(int numTotalBits)
    {
        quint32 r = range;
        quint32 c = code;
        quint32 result = 0;
        for (int i = numTotalBits; i != 0; i--) {
            r >>= 1;
            quint32 t = (c - r) >> 31;
            c -= r & (t - 1);
            result = (result << 1) | (1 - t);

            if (r < kTopValue) {
                c = (c << 8) | readByte();
                r <<= 8;
            }
        }
        range = r;
        code = c;
        return result;
    }

    quint32 DecodeBit(quint32 size0, quint32 numTotalBits)
    {
        quint32 newBound = (range >> numTotalBits) * size0;
        quint32 symbol;
        if (code < newBound) {
            symbol = 0;
            range = newBound;
        } else {
            symbol = 1;
            code -= newBound;
            range -= newBound;
        }
        normalize();
        return symbol;
    }
};

const int kNumBitModelTotalBits  = 11;
const quint32 kBitModelTotal = (1 << kNumBitModelTotalBits);

template <int numMoveBits>
class CBitModel
{
public:
    quint32 prob;
    void updateModel(quint32 symbol)
    {
        if (symbol == 0) {
            prob += (kBitModelTotal - prob) >> numMoveBits;
        } else {
            prob -= (prob) >> numMoveBits;
        }
    }

    void init()
    {
        prob = kBitModelTotal / 2;
    }
};

template <int numMoveBits>
class CBitDecoder : public CBitModel<numMoveBits>
{
public:
    quint32 decode(RangeDecoder *decoder)
    {
        quint32 newBound = (decoder->range >> kNumBitModelTotalBits) * this->prob;
        if (decoder->code < newBound) {
            decoder->range = newBound;
            this->prob += (kBitModelTotal - this->prob) >> numMoveBits;
            if (decoder->range < kTopValue) {
                decoder->code = (decoder->code << 8) | decoder->readByte();
                decoder->range <<= 8;
            }
            return 0;
        } else {
            decoder->range -= newBound;
            decoder->code -= newBound;
            this->prob -= (this->prob) >> numMoveBits;
            if (decoder->range < kTopValue) {
                decoder->code = (decoder->code << 8) | decoder->readByte();
                decoder->range <<= 8;
            }
            return 1;
        }
    }
};

inline bool isJcc(unsigned char b0, unsigned char b1)
{
    return (b0 == 0x0F && (b1 & 0xF0) == 0x80);
}
inline bool isJ(unsigned char b0, unsigned char b1)
{
    return ((b1 & 0xFE) == 0xE8 || isJcc(b0, b1));
}
inline unsigned getIndex(unsigned char b0, unsigned char b1)
{
    return ((b1 == 0xE8) ? b0 : ((b1 == 0xE9) ? 256 : 257));
}

const int kNumMoveBits = 5;

static QByteArray decodeBCJ2(const QByteArray &mainStream, const QByteArray &callStream, const QByteArray &jumpStream, const QByteArray &rangeBuffer)
{
    unsigned char prevByte = 0;
    QByteArray outStream;
    int mainStreamPos = 0;
    int callStreamPos = 0;
    int jumpStreamPos = 0;

    RangeDecoder rangeDecoder;
    rangeDecoder.setStream(rangeBuffer);
    rangeDecoder.init();

    QVector<CBitDecoder<kNumMoveBits> > statusDecoder(256 + 2);

    for (int i = 0; i < 256 + 2; i++) {
        statusDecoder[i].init();
    }

    for (;;) {
        quint32 i;
        unsigned char b = 0;
        const quint32 kBurstSize = (1 << 18);
        for (i = 0; i < kBurstSize; i++) {
            if (mainStreamPos == mainStream.size()) {
                return outStream;
            }

            b = mainStream[mainStreamPos++];
            outStream.append(b);

            if (isJ(prevByte, b)) {
                break;
            }
            prevByte = b;
        }

        if (i == kBurstSize) {
            continue;
        }

        unsigned index = getIndex(prevByte, b);
        if (statusDecoder[index].decode(&rangeDecoder) == 1) {
            if (b == 0xE8) {
                if (callStreamPos + 4 > callStream.size()) {
                    return QByteArray();
                }
            } else {
                if (jumpStreamPos + 4 > jumpStream.size()) {
                    return QByteArray();
                }
            }
            quint32 src = 0;
            for (int i = 0; i < 4; i++) {
                unsigned char b0;
                if (b == 0xE8) {
                    b0 = callStream[callStreamPos++];
                } else {
                    b0 = jumpStream[jumpStreamPos++];
                }
                src <<= 8;
                src |= ((quint32)b0);
            }

            quint32 dest = src - (quint32(outStream.size()) + 4);
            outStream.append((unsigned char)(dest));
            outStream.append((unsigned char)(dest >> 8));
            outStream.append((unsigned char)(dest >> 16));
            outStream.append((unsigned char)(dest >> 24));
            prevByte = (unsigned char)(dest >> 24);
        } else {
            prevByte = b;
        }
    }
}

QByteArray K7Zip::K7ZipPrivate::readAndDecodePackedStreams(bool readMainStreamInfo)
{
    if (!buffer) {
        return QByteArray();
    }

    if (readMainStreamInfo) {
        readMainStreamsInfo();
    }

    QByteArray inflatedData;

    quint64 startPos = 32 + packPos;
    for (int i = 0; i < folders.size(); i++) {
        const Folder *folder = folders.at(i);
        quint64 unpackSize64 = folder->getUnpackSize();;
        size_t unpackSize = (size_t)unpackSize64;
        if (unpackSize != unpackSize64) {
            qCDebug(KArchiveLog) << "unsupported";
            return inflatedData;
        }

        // Find main coder
        quint32 mainCoderIndex = 0;
        QVector<int> outStreamIndexed;
        int outStreamIndex = 0;
        for (int j = 0; j < folder->folderInfos.size(); j++) {
            const Folder::FolderInfo *info = folder->folderInfos[j];
            for (int k = 0; k < info->numOutStreams; k++, outStreamIndex++) {
                if (folder->findBindPairForOutStream(outStreamIndex) < 0) {
                    outStreamIndexed.append(outStreamIndex);
                    break;
                }
            }
        }

        quint32 temp = 0;
        if (!outStreamIndexed.isEmpty()) {
            folder->findOutStream(outStreamIndexed[0], mainCoderIndex, temp);
        }

        quint32 startInIndex = folder->getCoderInStreamIndex(mainCoderIndex);
        quint32 startOutIndex = folder->getCoderOutStreamIndex(mainCoderIndex);

        Folder::FolderInfo *mainCoder = folder->folderInfos[mainCoderIndex];

        QVector<int> seqInStreams;
        QVector<quint32> coderIndexes;
        seqInStreams.reserve(mainCoder->numInStreams);
        coderIndexes.reserve(mainCoder->numInStreams);
        for (int j = 0; j < (int)mainCoder->numInStreams; j++) {
            int seqInStream;
            quint32 coderIndex;
            getInStream(folder, startInIndex + j, seqInStream, coderIndex);
            seqInStreams.append(seqInStream);
            coderIndexes.append(coderIndex);
        }

        QVector<int> seqOutStreams;
        seqOutStreams.reserve(mainCoder->numOutStreams);
        for (int j = 0; j < (int)mainCoder->numOutStreams; j++) {
            int seqOutStream;
            getOutStream(folder, startOutIndex + j, seqOutStream);
            seqOutStreams.append(seqOutStream);
        }

        QVector<QByteArray> datas;
        for (int j = 0; j < (int)mainCoder->numInStreams; j++) {
            int size = packSizes[j + i];
            std::unique_ptr<char[]> encodedBuffer(new char[size]);
            QIODevice *dev = q->device();
            dev->seek(startPos);
            quint64 n = dev->read(encodedBuffer.get(), size);
            if (n != (quint64)size) {
                qCDebug(KArchiveLog) << "Failed read next size, should read " << size << ", read " << n;
                return inflatedData;
            }
            QByteArray deflatedData(encodedBuffer.get(), size);
            datas.append(deflatedData);
            startPos += size;
            pos += size;
            headerSize += size;
        }

        QVector<QByteArray> inflatedDatas;
        QByteArray deflatedData;
        for (int j = 0; j < seqInStreams.size(); ++j) {
            Folder::FolderInfo *coder = nullptr;
            if ((quint32)j != mainCoderIndex) {
                coder = folder->folderInfos[coderIndexes[j]];
            } else {
                coder = folder->folderInfos[mainCoderIndex];
            }

            deflatedData = datas[seqInStreams[j]];

            KFilterBase *filter = nullptr;

            switch (coder->methodID) {
            case k_LZMA:
                filter = KCompressionDevice::filterForCompressionType(KCompressionDevice::Xz);
                if (!filter) {
                    qCDebug(KArchiveLog) << "filter not found";
                    return QByteArray();
                }
                static_cast<KXzFilter *>(filter)->init(QIODevice::ReadOnly, KXzFilter::LZMA, coder->properties);
                break;
            case k_LZMA2:
                filter = KCompressionDevice::filterForCompressionType(KCompressionDevice::Xz);
                if (!filter) {
                    qCDebug(KArchiveLog) << "filter not found";
                    return QByteArray();
                }
                static_cast<KXzFilter *>(filter)->init(QIODevice::ReadOnly, KXzFilter::LZMA2, coder->properties);
                break;
            case k_PPMD: {
                /*if (coder->properties.size() == 5) {
                    //Byte order = *(const Byte *)coder.Props;
                    qint32 dicSize = ((unsigned char)coder->properties[1]        |
                                     (((unsigned char)coder->properties[2]) <<  8) |
                                     (((unsigned char)coder->properties[3]) << 16) |
                                     (((unsigned char)coder->properties[4]) << 24));
                }*/
                break;
            }
            case k_AES:
                if (coder->properties.size() >= 1) {
                    //const Byte *data = (const Byte *)coder.Props;
                    //Byte firstByte = *data++;
                    //UInt32 numCyclesPower = firstByte & 0x3F;
                }
                break;
            case k_BCJ:
                filter = KCompressionDevice::filterForCompressionType(KCompressionDevice::Xz);
                if (!filter) {
                    qCDebug(KArchiveLog) << "filter not found";
                    return QByteArray();
                }
                static_cast<KXzFilter *>(filter)->init(QIODevice::ReadOnly, KXzFilter::BCJ, coder->properties);
                break;
            case k_BCJ2: {
                QByteArray bcj2 = decodeBCJ2(inflatedDatas[0], inflatedDatas[1], inflatedDatas[2], deflatedData);
                inflatedDatas.clear();
                inflatedDatas.append(bcj2);
                break;
            }
            case k_BZip2:
                filter = KCompressionDevice::filterForCompressionType(KCompressionDevice::BZip2);
                if (!filter) {
                    qCDebug(KArchiveLog) << "filter not found";
                    return QByteArray();
                }
                filter->init(QIODevice::ReadOnly);
                break;
            }

            if (coder->methodID == k_BCJ2) {
                continue;
            }

            if (!filter) {
                return QByteArray();
            }

            filter->setInBuffer(deflatedData.data(), deflatedData.size());

            QByteArray outBuffer;
            // reserve memory
            outBuffer.resize(unpackSize);

            KFilterBase::Result result = KFilterBase::Ok;
            QByteArray inflatedDataTmp;
            while (result != KFilterBase::End && result != KFilterBase::Error && !filter->inBufferEmpty()) {
                filter->setOutBuffer(outBuffer.data(), outBuffer.size());
                result = filter->uncompress();
                if (result == KFilterBase::Error) {
                    qCDebug(KArchiveLog) << " decode error";
                    filter->terminate();
                    delete filter;
                    return QByteArray();
                }
                int uncompressedBytes = outBuffer.size() - filter->outBufferAvailable();

                // append the uncompressed data to inflate buffer
                inflatedDataTmp.append(outBuffer.data(), uncompressedBytes);

                if (result == KFilterBase::End) {
                    //qCDebug(KArchiveLog) << "Finished unpacking";
                    break; // Finished.
                }
            }

            if (result != KFilterBase::End && !filter->inBufferEmpty()) {
                qCDebug(KArchiveLog) << "decode failed result" << result;
                filter->terminate();
                delete filter;
                return QByteArray();
            }

            filter->terminate();
            delete filter;

            inflatedDatas.append(inflatedDataTmp);
        }

        QByteArray inflated;
        for (const QByteArray& data : qAsConst(inflatedDatas)) {
            inflated.append(data);
        }

        inflatedDatas.clear();

        if (folder->unpackCRCDefined) {
            if ((size_t)inflated.size() < unpackSize) {
                qCDebug(KArchiveLog) << "wrong crc size data";
                return QByteArray();
            }
            quint32 crc = crc32(0, (Bytef *)(inflated.data()), unpackSize);
            if (crc != folder->unpackCRC) {
                qCDebug(KArchiveLog) << "wrong crc";
                return QByteArray();
            }
        }

        inflatedData.append(inflated);
    }

    return inflatedData;
}

///////////////// Write ////////////////////

void K7Zip::K7ZipPrivate::createItemsFromEntities(const KArchiveDirectory *dir, const QString &path, QByteArray &data)
{
    const QStringList l = dir->entries();
    QStringList::ConstIterator it = l.begin();
    for (; it != l.end(); ++it) {
        const KArchiveEntry *entry = dir->entry((*it));

        FileInfo *fileInfo = new FileInfo;
        fileInfo->attribDefined = true;

        fileInfo->path = path + entry->name();
        mTimesDefined.append(true);
        mTimes.append(rtlSecondsSince1970ToSpecTime(entry->date().toSecsSinceEpoch()));

        if (entry->isFile()) {
            const K7ZipFileEntry *fileEntry = static_cast<const K7ZipFileEntry *>(entry);

            fileInfo->attributes = FILE_ATTRIBUTE_ARCHIVE;
            fileInfo->attributes |= FILE_ATTRIBUTE_UNIX_EXTENSION + ((entry->permissions() & 0xFFFF) << 16);
            fileInfo->size = fileEntry->size();
            QString symLink = fileEntry->symLinkTarget();
            if (fileInfo->size > 0) {
                fileInfo->hasStream = true;
                data.append(outData.mid(fileEntry->position(), fileEntry->size()));
                unpackSizes.append(fileInfo->size);
            } else if (!symLink.isEmpty()) {
                fileInfo->hasStream = true;
                data.append(symLink.toUtf8());
                unpackSizes.append(symLink.size());
            }
            fileInfos.append(fileInfo);
        } else if (entry->isDirectory()) {
            fileInfo->attributes = FILE_ATTRIBUTE_DIRECTORY;
            fileInfo->attributes |= FILE_ATTRIBUTE_UNIX_EXTENSION + ((entry->permissions() & 0xFFFF) << 16);
            fileInfo->isDir = true;
            fileInfos.append(fileInfo);
            createItemsFromEntities((KArchiveDirectory *)entry, path + (*it) + QLatin1Char('/'), data);

        }
    }
}

void K7Zip::K7ZipPrivate::writeByte(unsigned char b)
{
    header.append(b);
    countSize++;
}

void K7Zip::K7ZipPrivate::writeNumber(quint64 value)
{
    int firstByte = 0;
    short mask = 0x80;
    int i;
    for (i = 0; i < 8; i++) {
        if (value < ((quint64(1) << (7  * (i + 1))))) {
            firstByte |= (int)(value >> (8 * i));
            break;
        }
        firstByte |= mask;
        mask >>= 1;
    }
    writeByte(firstByte);
    for (; i > 0; i--) {
        writeByte((int)value);
        value >>= 8;
    }
}

void K7Zip::K7ZipPrivate::writeBoolVector(const QVector<bool> &boolVector)
{
    int b = 0;
    short mask = 0x80;
    for (int i = 0; i < boolVector.size(); i++) {
        if (boolVector[i]) {
            b |= mask;
        }
        mask >>= 1;
        if (mask == 0) {
            writeByte(b);
            mask = 0x80;
            b = 0;
        }
    }
    if (mask != 0x80) {
        writeByte(b);
    }
}

void K7Zip::K7ZipPrivate::writeUInt32(quint32 value)
{
    for (int i = 0; i < 4; i++) {
        writeByte((unsigned char)value);
        value >>= 8;
    }
}

void K7Zip::K7ZipPrivate::writeUInt64(quint64 value)
{
    for (int i = 0; i < 8; i++) {
        writeByte((unsigned char)value);
        value >>= 8;
    }
}

void K7Zip::K7ZipPrivate::writeAlignedBoolHeader(const QVector<bool> &v, int numDefined, int type, unsigned itemSize)
{
    const unsigned bvSize = (numDefined == v.size()) ? 0 : ((unsigned)v.size() + 7) / 8;
    const quint64 dataSize = (quint64)numDefined * itemSize + bvSize + 2;
    //SkipAlign(3 + (unsigned)bvSize + (unsigned)GetBigNumberSize(dataSize), itemSize);

    writeByte(type);
    writeNumber(dataSize);
    if (numDefined == v.size()) {
        writeByte(1);
    } else {
        writeByte(0);
        writeBoolVector(v);
    }
    writeByte(0);
}

void K7Zip::K7ZipPrivate::writeUInt64DefVector(const QVector<quint64> &v, const QVector<bool> &defined, int type)
{
    int numDefined = 0;

    for (int i = 0; i < defined.size(); i++) {
        if (defined[i]) {
            numDefined++;
        }
    }

    if (numDefined == 0) {
        return;
    }

    writeAlignedBoolHeader(defined, numDefined, type, 8);

    for (int i = 0; i < defined.size(); i++) {
        if (defined[i]) {
            writeUInt64(v[i]);
        }
    }
}

void K7Zip::K7ZipPrivate::writeHashDigests(
    const QVector<bool> &digestsDefined,
    const QVector<quint32> &digests)
{
    int numDefined = 0;
    int i;
    for (i = 0; i < digestsDefined.size(); i++) {
        if (digestsDefined[i]) {
            numDefined++;
        }
    }

    if (numDefined == 0) {
        return;
    }

    writeByte(kCRC);
    if (numDefined == digestsDefined.size()) {
        writeByte(1);
    } else {
        writeByte(0);
        writeBoolVector(digestsDefined);
    }

    for (i = 0; i < digests.size(); i++) {
        if (digestsDefined[i]) {
            writeUInt32(digests[i]);
        }
    }
}

void K7Zip::K7ZipPrivate::writePackInfo(quint64 dataOffset, QVector<quint64> &packedSizes, QVector<bool> &packedCRCsDefined, QVector<quint32> &packedCRCs)
{
    if (packedSizes.isEmpty()) {
        return;
    }
    writeByte(kPackInfo);
    writeNumber(dataOffset);
    writeNumber(packedSizes.size());
    writeByte(kSize);

    for (int i = 0; i < packedSizes.size(); i++) {
        writeNumber(packedSizes[i]);
    }

    writeHashDigests(packedCRCsDefined, packedCRCs);

    writeByte(kEnd);
}

void K7Zip::K7ZipPrivate::writeFolder(const Folder *folder)
{
    writeNumber(folder->folderInfos.size());
    for (int i = 0; i < folder->folderInfos.size(); i++) {
        const Folder::FolderInfo *info = folder->folderInfos.at(i);
        {
            size_t propsSize = info->properties.size();

            quint64 id = info->methodID;
            size_t idSize;
            for (idSize = 1; idSize < sizeof (id); idSize++) {
                if ((id >> (8 * idSize)) == 0) {
                    break;
                }
            }

            int longID[15];
            for (int t = idSize - 1; t >= 0; t--, id >>= 8) {
                longID[t] = (int)(id & 0xFF);
            }

            int b;
            b = (int)(idSize & 0xF);
            bool isComplex = !info->isSimpleCoder();
            b |= (isComplex ? 0x10 : 0);
            b |= ((propsSize != 0) ? 0x20 : 0);

            writeByte(b);
            for (size_t j = 0; j < idSize; ++j) {
                writeByte(longID[j]);
            }

            if (isComplex) {
                writeNumber(info->numInStreams);
                writeNumber(info->numOutStreams);
            }

            if (propsSize == 0) {
                continue;
            }

            writeNumber(propsSize);
            for (size_t j = 0; j < propsSize; ++j) {
                writeByte(info->properties[j]);
            }
        }
    }

    for (int i = 0; i < folder->inIndexes.size(); i++) {
        writeNumber(folder->inIndexes[i]);
        writeNumber(folder->outIndexes[i]);
    }

    if (folder->packedStreams.size() > 1) {
        for (int i = 0; i < folder->packedStreams.size(); i++) {
            writeNumber(folder->packedStreams[i]);
        }
    }
}

void K7Zip::K7ZipPrivate::writeUnpackInfo(const QVector<Folder *> &folderItems)
{
    if (folderItems.isEmpty()) {
        return;
    }

    writeByte(kUnpackInfo);

    writeByte(kFolder);
    writeNumber(folderItems.size());
    {
        writeByte(0);
        for (int i = 0; i < folderItems.size(); i++) {
            writeFolder(folderItems[i]);
        }
    }

    writeByte(kCodersUnpackSize);
    int i;
    for (i = 0; i < folderItems.size(); i++) {
        const Folder *folder = folderItems[i];
        for (int j = 0; j < folder->unpackSizes.size(); j++) {
            writeNumber(folder->unpackSizes.at(j));
        }
    }

    QVector<bool> unpackCRCsDefined;
    QVector<quint32> unpackCRCs;
    unpackCRCsDefined.reserve(folderItems.size());
    unpackCRCs.reserve(folderItems.size());
    for (i = 0; i < folderItems.size(); i++) {
        const Folder *folder = folderItems[i];
        unpackCRCsDefined.append(folder->unpackCRCDefined);
        unpackCRCs.append(folder->unpackCRC);
    }
    writeHashDigests(unpackCRCsDefined, unpackCRCs);

    writeByte(kEnd);
}

void K7Zip::K7ZipPrivate::writeSubStreamsInfo(
    const QVector<quint64> &unpackSizes,
    const QVector<bool> &digestsDefined,
    const QVector<quint32> &digests)
{
    writeByte(kSubStreamsInfo);

    for (int i = 0; i < numUnpackStreamsInFolders.size(); i++) {
        if (numUnpackStreamsInFolders.at(i) != 1) {
            writeByte(kNumUnpackStream);
            for (int j = 0; j < numUnpackStreamsInFolders.size(); j++) {
                writeNumber(numUnpackStreamsInFolders.at(j));
            }
            break;
        }
    }

    bool needFlag = true;
    int index = 0;
    for (int i = 0; i < numUnpackStreamsInFolders.size(); i++) {
        for (quint32 j = 0; j < numUnpackStreamsInFolders.at(i); j++) {
            if (j + 1 != numUnpackStreamsInFolders.at(i)) {
                if (needFlag) {
                    writeByte(kSize);
                }
                needFlag = false;
                writeNumber(unpackSizes[index]);
            }
            index++;
        }
    }

    QVector<bool> digestsDefined2;
    QVector<quint32> digests2;

    int digestIndex = 0;
    for (int i = 0; i < folders.size(); i++) {
        int numSubStreams = (int)numUnpackStreamsInFolders.at(i);
        if (numSubStreams == 1 && folders.at(i)->unpackCRCDefined) {
            digestIndex++;
        } else {
            for (int j = 0; j < numSubStreams; j++, digestIndex++) {
                digestsDefined2.append(digestsDefined[digestIndex]);
                digests2.append(digests[digestIndex]);
            }
        }
    }
    writeHashDigests(digestsDefined2, digests2);
    writeByte(kEnd);
}

QByteArray K7Zip::K7ZipPrivate::encodeStream(QVector<quint64> &packSizes, QVector<Folder *> &folds)
{
    Folder *folder = new Folder;
    folder->unpackCRCDefined = true;
    folder->unpackCRC = crc32(0, (Bytef *)(header.data()), header.size());
    folder->unpackSizes.append(header.size());

    Folder::FolderInfo *info = new Folder::FolderInfo();
    info->numInStreams = 1;
    info->numOutStreams = 1;
    info->methodID = k_LZMA2;

    quint32 dictSize = header.size();
    const quint32 kMinReduceSize = (1 << 16);
    if (dictSize < kMinReduceSize) {
        dictSize = kMinReduceSize;
    }

    int dict;
    for (dict = 0; dict < 40; dict++) {
        if (dictSize <= LZMA2_DIC_SIZE_FROM_PROP(dict)) {
            break;
        }
    }

    info->properties.append(dict);
    folder->folderInfos.append(info);

    folds.append(folder);

    //compress data
    QByteArray encodedData;
    if (!header.isEmpty()) {
        QByteArray enc;
        QBuffer inBuffer(&enc);

        KCompressionDevice flt(&inBuffer, false, KCompressionDevice::Xz);
        flt.open(QIODevice::WriteOnly);

        KFilterBase *filter = flt.filterBase();

        static_cast<KXzFilter *>(filter)->init(QIODevice::WriteOnly, KXzFilter::LZMA2, info->properties);

        const int ret = flt.write(header);
        if (ret != header.size()) {
            qCDebug(KArchiveLog) << "write error write " << ret << "expected" << header.size();
            return encodedData;
        }

        flt.close();
        encodedData = inBuffer.data();
    }

    packSizes.append(encodedData.size());
    return encodedData;
}

void K7Zip::K7ZipPrivate::writeHeader(quint64 &headerOffset)
{
    quint64 packedSize = 0;
    for (int i = 0; i < packSizes.size(); ++i) {
        packedSize += packSizes[i];
    }

    headerOffset = packedSize;

    writeByte(kHeader);

    // Archive Properties

    if (!folders.isEmpty()) {
        writeByte(kMainStreamsInfo);
        writePackInfo(0, packSizes, packCRCsDefined, packCRCs);

        writeUnpackInfo(folders);

        QVector<quint64> unpackFileSizes;
        QVector<bool> digestsDefined;
        QVector<quint32> digests;
        for (int i = 0; i < fileInfos.size(); i++) {
            const FileInfo *file = fileInfos.at(i);
            if (!file->hasStream) {
                continue;
            }
            unpackFileSizes.append(file->size);
            digestsDefined.append(file->crcDefined);
            digests.append(file->crc);
        }

        writeSubStreamsInfo(unpackSizes, digestsDefined, digests);
        writeByte(kEnd);
    }

    if (fileInfos.isEmpty()) {
        writeByte(kEnd);
        return;
    }

    writeByte(kFilesInfo);
    writeNumber(fileInfos.size());

    {
        /* ---------- Empty Streams ---------- */
        QVector<bool> emptyStreamVector;
        int numEmptyStreams = 0;
        for (int i = 0; i < fileInfos.size(); i++) {
            if (fileInfos.at(i)->hasStream) {
                emptyStreamVector.append(false);
            } else {
                emptyStreamVector.append(true);
                numEmptyStreams++;
            }
        }

        if (numEmptyStreams > 0) {
            writeByte(kEmptyStream);
            writeNumber(((unsigned)emptyStreamVector.size() + 7) / 8);
            writeBoolVector(emptyStreamVector);

            QVector<bool> emptyFileVector, antiVector;
            int numEmptyFiles = 0, numAntiItems = 0;
            for (int i = 0; i < fileInfos.size(); i++) {
                const FileInfo *file = fileInfos.at(i);
                if (!file->hasStream) {
                    emptyFileVector.append(!file->isDir);
                    if (!file->isDir) {
                        numEmptyFiles++;
                        bool isAnti = (i < this->isAnti.size() && this->isAnti[i]);
                        antiVector.append(isAnti);
                        if (isAnti) {
                            numAntiItems++;
                        }
                    }
                }
            }

            if (numEmptyFiles > 0) {
                writeByte(kEmptyFile);
                writeNumber(((unsigned)emptyFileVector.size() + 7) / 8);
                writeBoolVector(emptyFileVector);
            }

            if (numAntiItems > 0) {
                writeByte(kAnti);
                writeNumber(((unsigned)antiVector.size() + 7) / 8);
                writeBoolVector(antiVector);
            }
        }
    }

    {
        /* ---------- Names ---------- */

        int numDefined = 0;
        size_t namesDataSize = 0;
        for (int i = 0; i < fileInfos.size(); i++) {
            const QString &name = fileInfos.at(i)->path;
            if (!name.isEmpty()) {
                numDefined++;
                namesDataSize += (name.length() + 1) * 2;
            }
        }

        if (numDefined > 0) {
            namesDataSize++;
            //SkipAlign(2 + GetBigNumberSize(namesDataSize), 2);

            writeByte(kName);
            writeNumber(namesDataSize);
            writeByte(0);
            for (int i = 0; i < fileInfos.size(); i++) {
                const QString &name = fileInfos.at(i)->path;
                for (int t = 0; t < name.length(); t++) {
                    wchar_t c = name[t].toLatin1();
                    writeByte((unsigned char)c);
                    writeByte((unsigned char)(c >> 8));
                }
                // End of string
                writeByte(0);
                writeByte(0);
            }
        }
    }

    writeUInt64DefVector(mTimes, mTimesDefined, kMTime);

    writeUInt64DefVector(startPositions, startPositionsDefined, kStartPos);

    {
        /* ---------- Write Attrib ---------- */
        QVector<bool> boolVector;
        int numDefined = 0;
        boolVector.reserve(fileInfos.size());
        for (int i = 0; i < fileInfos.size(); i++) {
            bool defined = fileInfos.at(i)->attribDefined;
            boolVector.append(defined);
            if (defined) {
                numDefined++;
            }
        }

        if (numDefined > 0) {
            writeAlignedBoolHeader(boolVector, numDefined, kAttributes, 4);
            for (int i = 0; i < fileInfos.size(); i++) {
                const FileInfo *file = fileInfos.at(i);
                if (file->attribDefined) {
                    writeUInt32(file->attributes);
                }
            }
        }
    }

    writeByte(kEnd); // for files
    writeByte(kEnd); // for headers*/
}

static void setUInt32(unsigned char *p, quint32 d)
{
    for (int i = 0; i < 4; i++, d >>= 8) {
        p[i] = (unsigned)d;
    }
}

static void setUInt64(unsigned char *p, quint64 d)
{
    for (int i = 0; i < 8; i++, d >>= 8) {
        p[i] = (unsigned char)d;
    }
}

void K7Zip::K7ZipPrivate::writeStartHeader(const quint64 nextHeaderSize, const quint32 nextHeaderCRC, const quint64 nextHeaderOffset)
{
    unsigned char buf[24];
    setUInt64(buf + 4, nextHeaderOffset);
    setUInt64(buf + 12, nextHeaderSize);
    setUInt32(buf + 20, nextHeaderCRC);
    setUInt32(buf, crc32(0, (Bytef *)(buf + 4), 20));
    q->device()->write((char *)buf, 24);
}

void K7Zip::K7ZipPrivate::writeSignature()
{
    unsigned char buf[8];
    memcpy(buf, k7zip_signature, 6);
    buf[6] = 0/*kMajorVersion*/;
    buf[7] = 3;
    q->device()->write((char *)buf, 8);
}

bool K7Zip::openArchive(QIODevice::OpenMode mode)
{
    if (!(mode & QIODevice::ReadOnly)) {
        return true;
    }

    QIODevice *dev = device();

    if (!dev) {
        setErrorString(tr("Could not get underlying device"));
        return false;
    }

    char header[32];
    // check signature
    qint64 n = dev->read(header, 32);
    if (n != 32) {
        setErrorString(tr("Read header failed"));
        return false;
    }

    for (int i = 0; i < 6; ++i) {
        if ((unsigned char)header[i] != k7zip_signature[i]) {
            setErrorString(tr("Check signature failed"));
            return false;
        }
    }

    // get Archive Version
    int major = header[6];
    int minor = header[7];

    /*if (major > 0 || minor > 2) {
        qCDebug(KArchiveLog) << "wrong archive version";
        return false;
    }*/

    // get Start Header CRC
    quint32 startHeaderCRC = GetUi32(header, 8);
    quint64 nextHeaderOffset = GetUi64(header, 12);
    quint64 nextHeaderSize = GetUi64(header, 20);
    quint32 nextHeaderCRC = GetUi32(header, 28);

    quint32 crc = crc32(0, (Bytef *)(header + 0xC), 20);

    if (crc != startHeaderCRC) {
        setErrorString(tr("Bad CRC"));
        return false;
    }

    if (nextHeaderSize == 0) {
        return true;
    }

    if (nextHeaderSize > (quint64)0xFFFFFFFF) {
        setErrorString(tr("Next header size is too big"));
        return false;
    }

    if ((qint64)nextHeaderOffset < 0) {
        setErrorString(tr("Next header size is less than zero"));
        return false;
    }

    dev->seek(nextHeaderOffset + 32);

    QByteArray inBuffer;
    inBuffer.resize(nextHeaderSize);

    n = dev->read(inBuffer.data(), inBuffer.size());
    if (n != (qint64)nextHeaderSize) {
        setErrorString(
              tr("Failed read next header size; should read %1, read %2")
              .arg(nextHeaderSize).arg(n));
        return false;
    }
    d->buffer = inBuffer.data();
    d->end = nextHeaderSize;

    d->headerSize = 32 + nextHeaderSize;
    //int physSize = 32 + nextHeaderSize + nextHeaderOffset;

    crc = crc32(0, (Bytef *)(d->buffer), (quint32)nextHeaderSize);

    if (crc != nextHeaderCRC) {
        setErrorString(tr("Bad next header CRC"));
        return false;
    }

    int type = d->readByte();
    QByteArray decodedData;
    if (type != kHeader) {
        if (type != kEncodedHeader) {
            setErrorString(tr("Error in header"));
            return false;
        }

        decodedData = d->readAndDecodePackedStreams();

        int external = d->readByte();
        if (external != 0) {
            int dataIndex = (int)d->readNumber();
            if (dataIndex < 0) {
                //qCDebug(KArchiveLog) << "dataIndex error";
            }
            d->buffer = decodedData.constData();
            d->pos = 0;
            d->end = decodedData.size();
        }

        type = d->readByte();
        if (type != kHeader) {
            setErrorString(tr("Wrong header type"));
            return false;
        }
    }
    // read header

    type = d->readByte();

    if (type == kArchiveProperties) {
        // TODO : implement this part
        setErrorString(tr("Not implemented"));
        return false;
    }

    if (type == kAdditionalStreamsInfo) {
        // TODO : implement this part
        setErrorString(tr("Not implemented"));
        return false;
    }

    if (type == kMainStreamsInfo) {
        if (!d->readMainStreamsInfo()) {
            setErrorString(tr("Error while reading main streams information"));
            return false;
        }
        type = d->readByte();
    } else {
        for (int i = 0; i < d->folders.size(); ++i) {
            Folder *folder = d->folders.at(i);
            d->unpackSizes.append(folder->getUnpackSize());
            d->digestsDefined.append(folder->unpackCRCDefined);
            d->digests.append(folder->unpackCRC);
        }
    }

    if (type == kEnd) {
        return true;
    }

    if (type != kFilesInfo) {
        setErrorString(tr("Error while reading header"));
        return false;
    }

    //read files info
    int numFiles = d->readNumber();
    for (int i = 0; i < numFiles; ++i) {
        d->fileInfos.append(new FileInfo);
    }

    QVector<bool> emptyStreamVector;
    QVector<bool> emptyFileVector;
    QVector<bool> antiFileVector;
    int numEmptyStreams = 0;

    for (;;) {
        quint64 type = d->readByte();
        if (type == kEnd) {
            break;
        }

        quint64 size = d->readNumber();

        size_t ppp = d->pos;

        bool addPropIdToList = true;
        bool isKnownType = true;

        if (type > ((quint32)1 << 30)) {
            isKnownType = false;
        } else {
            switch (type) {
            case kEmptyStream: {
                d->readBoolVector(numFiles, emptyStreamVector);
                for (int i = 0; i < emptyStreamVector.size(); ++i) {
                    if (emptyStreamVector[i]) {
                        numEmptyStreams++;
                    }
                }

                break;
            }
            case kEmptyFile:
                d->readBoolVector(numEmptyStreams, emptyFileVector);
                break;
            case kAnti:
                d->readBoolVector(numEmptyStreams, antiFileVector);
                break;
            case kCTime:
                if (!d->readUInt64DefVector(numFiles, d->cTimes, d->cTimesDefined)) {
                    return false;
                }
                break;
            case kATime:
                if (!d->readUInt64DefVector(numFiles, d->aTimes, d->aTimesDefined)) {
                    return false;
                }
                break;
            case kMTime:
                if (!d->readUInt64DefVector(numFiles, d->mTimes, d->mTimesDefined)) {
                    setErrorString(tr("Error reading modification time"));
                    return false;
                }
                break;
            case kName: {
                int external = d->readByte();
                if (external != 0) {
                    int dataIndex = d->readNumber();
                    if (dataIndex < 0 /*|| dataIndex >= dataVector->Size()*/) {
                        qCDebug(KArchiveLog) << "wrong data index";
                    }

                    // TODO : go to the new index
                }

                QString name;
                for (int i = 0; i < numFiles; i++) {
                    name = d->readString();
                    d->fileInfos.at(i)->path = name;
                }
                break;
            }
            case kAttributes: {
                QVector<bool> attributesAreDefined;
                d->readBoolVector2(numFiles, attributesAreDefined);
                int external = d->readByte();
                if (external != 0) {
                    int dataIndex = d->readNumber();
                    if (dataIndex < 0) {
                        qCDebug(KArchiveLog) << "wrong data index";
                    }

                    // TODO : go to the new index
                }

                for (int i = 0; i < numFiles; i++) {
                    FileInfo *fileInfo = d->fileInfos.at(i);
                    fileInfo->attribDefined = attributesAreDefined[i];
                    if (fileInfo->attribDefined) {
                        fileInfo->attributes = d->readUInt32();
                    }
                }
                break;
            }
            case kStartPos:
                if (!d->readUInt64DefVector(numFiles, d->startPositions, d->startPositionsDefined)) {
                    setErrorString(tr("Error reading MTime"));
                    return false;
                }
                break;
            case kDummy: {
                for (quint64 i = 0; i < size; i++) {
                    if (d->readByte() != 0) {
                        setErrorString(tr("Invalid"));
                        return false;
                    }
                }
                addPropIdToList = false;
                break;
            }
            default:
                addPropIdToList = isKnownType = false;
            }
        }

        if (isKnownType) {
            if (addPropIdToList) {
                d->fileInfoPopIDs.append(type);
            }
        } else {
            d->skipData(d->readNumber());
        }

        bool checkRecordsSize = (major > 0 ||
                                 minor > 2);
        if (checkRecordsSize && d->pos - ppp != size) {
            setErrorString(
                tr(
                    "Read size failed "
                    "(checkRecordsSize: %1, d->pos - ppp: %2, size: %3)")
                .arg(checkRecordsSize).arg(d->pos - ppp).arg(size));
            return false;
        }
    }

    int emptyFileIndex = 0;
    int sizeIndex = 0;

    int numAntiItems = 0;

    if (emptyStreamVector.isEmpty()) {
        emptyStreamVector.fill(false, numFiles);
    }

    if (antiFileVector.isEmpty()) {
        antiFileVector.fill(false, numEmptyStreams);
    }
    if (emptyFileVector.isEmpty()) {
        emptyFileVector.fill(false, numEmptyStreams);
    }

    for (int i = 0; i < numEmptyStreams; i++) {
        if (antiFileVector[i]) {
            numAntiItems++;
        }
    }

    d->outData = d->readAndDecodePackedStreams(false);

    int oldPos = 0;
    for (int i = 0; i < numFiles; i++) {
        FileInfo *fileInfo = d->fileInfos.at(i);
        bool isAnti;
        fileInfo->hasStream = !emptyStreamVector[i];
        if (fileInfo->hasStream) {
            fileInfo->isDir = false;
            isAnti = false;
            fileInfo->size = d->unpackSizes[sizeIndex];
            fileInfo->crc = d->digests[sizeIndex];
            fileInfo->crcDefined = d->digestsDefined[sizeIndex];
            sizeIndex++;
        } else {
            fileInfo->isDir = !emptyFileVector[emptyFileIndex];
            isAnti = antiFileVector[emptyFileIndex];
            emptyFileIndex++;
            fileInfo->size = 0;
            fileInfo->crcDefined = false;
        }
        if (numAntiItems != 0) {
            d->isAnti.append(isAnti);
        }

        int access;
        bool symlink = false;
        if (fileInfo->attributes & FILE_ATTRIBUTE_UNIX_EXTENSION) {
            access = fileInfo->attributes >> 16;
            if ((access & QT_STAT_MASK) == QT_STAT_LNK) {
                symlink = true;
            }
        } else {
            if (fileInfo->isDir) {
                access = S_IFDIR | 0755;
            } else {
                access = 0100644;
            }
        }

        qint64 pos = 0;
        if (!fileInfo->isDir) {
            pos = oldPos;
            oldPos += fileInfo->size;
        }

        KArchiveEntry *e;
        QString entryName;
        int index = fileInfo->path.lastIndexOf(QLatin1Char('/'));
        if (index == -1) {
            entryName = fileInfo->path;
        } else {
            entryName = fileInfo->path.mid(index + 1);
        }
        Q_ASSERT(!entryName.isEmpty());

        QDateTime mTime;
        if (d->mTimesDefined[i]) {
            mTime = KArchivePrivate::time_tToDateTime(toTimeT(d->mTimes[i]));
        } else {
            mTime = KArchivePrivate::time_tToDateTime(time(nullptr));
        }

        if (fileInfo->isDir) {
            QString path = QDir::cleanPath(fileInfo->path);
            const KArchiveEntry *ent = rootDir()->entry(path);
            if (ent && ent->isDirectory()) {
                e = nullptr;
            } else {
                e = new KArchiveDirectory(this, entryName, access, mTime, rootDir()->user(), rootDir()->group(), QString()/*symlink*/);
            }
        } else {
            if (!symlink) {
                e = new K7ZipFileEntry(this, entryName, access, mTime, rootDir()->user(), rootDir()->group(), QString()/*symlink*/, pos, fileInfo->size, d->outData);
            } else {
                QString target = QFile::decodeName(d->outData.mid(pos, fileInfo->size));
                e = new K7ZipFileEntry(this, entryName, access, mTime, rootDir()->user(), rootDir()->group(), target, 0, 0, nullptr);
            }
        }

        if (e) {
            if (index == -1) {
                rootDir()->addEntry(e);
            } else {
                QString path = QDir::cleanPath(fileInfo->path.left(index));
                KArchiveDirectory *d = findOrCreate(path);
                d->addEntry(e);
            }
        }
    }

    return true;
}

bool K7Zip::closeArchive()
{
    // Unnecessary check (already checked by KArchive::close())
    if (!isOpen()) {
        //qCWarning(KArchiveLog) << "You must open the file before close it\n";
        return false;
    }

    if ((mode() == QIODevice::ReadOnly)) {
        return true;
    }

    d->clear();

    Folder *folder = new Folder();

    folder->unpackSizes.clear();
    folder->unpackSizes.append(d->outData.size());

    Folder::FolderInfo *info = new Folder::FolderInfo();

    info->numInStreams = 1;
    info->numOutStreams = 1;
    info->methodID = k_LZMA2;

    quint32 dictSize = d->outData.size();

    const quint32 kMinReduceSize = (1 << 16);
    if (dictSize < kMinReduceSize) {
        dictSize = kMinReduceSize;
    }

    // k_LZMA2 mehtod
    int dict;
    for (dict = 0; dict < 40; dict++) {
        if (dictSize <= LZMA2_DIC_SIZE_FROM_PROP(dict)) {
            break;
        }
    }
    info->properties.append(dict);

    folder->folderInfos.append(info);
    d->folders.append(folder);

    const KArchiveDirectory *dir = directory();
    QByteArray data;
    d->createItemsFromEntities(dir, QString(), data);
    d->outData = data;

    folder->unpackCRCDefined = true;
    folder->unpackCRC = crc32(0, (Bytef *)(d->outData.data()), d->outData.size());

    //compress data
    QByteArray encodedData;
    if (!d->outData.isEmpty()) {
        QByteArray enc;
        QBuffer inBuffer(&enc);

        KCompressionDevice flt(&inBuffer, false, KCompressionDevice::Xz);
        flt.open(QIODevice::WriteOnly);

        KFilterBase *filter = flt.filterBase();

        static_cast<KXzFilter *>(filter)->init(QIODevice::WriteOnly, KXzFilter::LZMA2, info->properties);

        const int ret = flt.write(d->outData);
        if (ret != d->outData.size()) {
            setErrorString(tr("Write error"));
            return false;
        }

        flt.close();
        encodedData = inBuffer.data();
    }

    d->packSizes.append(encodedData.size());

    int numUnpackStream = 0;
    for (int i = 0; i < d->fileInfos.size(); ++i) {
        if (d->fileInfos.at(i)->hasStream) {
            numUnpackStream++;
        }
    }
    d->numUnpackStreamsInFolders.append(numUnpackStream);

    quint64 headerOffset;
    d->writeHeader(headerOffset);

    // Encode Header
    QByteArray encodedStream;
    {
        QVector<quint64> packSizes;
        QVector<Folder *> folders;
        encodedStream = d->encodeStream(packSizes, folders);

        if (folders.isEmpty()) {
            // FIXME Not sure why this is an error. Come up with a better message
            setErrorString(tr("Failed while encoding header"));
            return false;
        }

        d->header.clear();

        d->writeByte(kEncodedHeader);
        QVector<bool> emptyDefined;
        QVector<quint32> emptyCrcs;
        d->writePackInfo(headerOffset, packSizes, emptyDefined, emptyCrcs);
        d->writeUnpackInfo(folders);
        d->writeByte(kEnd);
        for (int i = 0; i < packSizes.size(); i++) {
            headerOffset += packSizes.at(i);
        }
        qDeleteAll(folders);
    }
    // end encode header

    quint64 nextHeaderSize = d->header.size();
    quint32 nextHeaderCRC = crc32(0, (Bytef *)(d->header.data()), d->header.size());
    quint64 nextHeaderOffset = headerOffset;

    device()->seek(0);
    d->writeSignature();
    d->writeStartHeader(nextHeaderSize, nextHeaderCRC, nextHeaderOffset);
    device()->write(encodedData.data(), encodedData.size());
    device()->write(encodedStream.data(), encodedStream.size());
    device()->write(d->header.data(), d->header.size());

    return true;
}

bool K7Zip::doFinishWriting(qint64 size)
{

    d->m_currentFile->setSize(size);
    d->m_currentFile = nullptr;

    return true;
}

bool K7Zip::writeData(const char *data, qint64 size)
{
    if (!d->m_currentFile) {
        setErrorString(tr("No file currently selected"));
        return false;
    }

    if (d->m_currentFile->position() == d->outData.size()) {
        d->outData.append(data, size);
    } else {
        d->outData.remove(d->m_currentFile->position(), d->m_currentFile->size());
        d->outData.insert(d->m_currentFile->position(), data, size);
    }

    return true;
}

bool K7Zip::doPrepareWriting(const QString &name, const QString &user,
                             const QString &group, qint64 /*size*/, mode_t perm,
                             const QDateTime & /*atime*/, const QDateTime &mtime, const QDateTime & /*ctime*/)
{
    if (!isOpen()) {
        setErrorString(tr("Application error: 7-Zip file must be open before being written into"));
        qCWarning(KArchiveLog) << "doPrepareWriting failed: !isOpen()";
        return false;
    }

    if (!(mode() & QIODevice::WriteOnly)) {
        setErrorString(tr("Application error: attempted to write into non-writable 7-Zip file"));
        qCWarning(KArchiveLog) << "doPrepareWriting failed: !(mode() & QIODevice::WriteOnly)";
        return false;
    }

    // Find or create parent dir
    KArchiveDirectory *parentDir = rootDir();
    //QString fileName( name );
    // In some files we can find dir/./file => call cleanPath
    QString fileName(QDir::cleanPath(name));
    int i = name.lastIndexOf(QLatin1Char('/'));
    if (i != -1) {
        QString dir = name.left(i);
        fileName = name.mid(i + 1);
        parentDir = findOrCreate(dir);
    }

    // test if the entry already exist
    const KArchiveEntry *entry = parentDir->entry(fileName);
    if (!entry) {
        K7ZipFileEntry *e = new K7ZipFileEntry(this, fileName, perm, mtime, user, group, QString()/*symlink*/, d->outData.size(), 0 /*unknown yet*/, d->outData);
        if (!parentDir->addEntryV2(e))
            return false;
        d->m_entryList << e;
        d->m_currentFile = e;
    } else {
        // TODO : find and replace in m_entryList
        //d->m_currentFile = static_cast<K7ZipFileEntry*>(entry);
    }

    return true;
}

bool K7Zip::doWriteDir(const QString &name, const QString &user,
                       const QString &group, mode_t perm,
                       const QDateTime & /*atime*/, const QDateTime &mtime, const QDateTime & /*ctime*/)
{
    if (!isOpen()) {
        setErrorString(tr("Application error: 7-Zip file must be open before being written into"));
        qCWarning(KArchiveLog) << "doWriteDir failed: !isOpen()";
        return false;
    }

    if (!(mode() & QIODevice::WriteOnly)) {
        //qCWarning(KArchiveLog) << "You must open the tar file for writing\n";
        return false;
    }

    // In some tar files we can find dir/./ => call cleanPath
    QString dirName(QDir::cleanPath(name));

    // Remove trailing '/'
    if (dirName.endsWith(QLatin1Char('/'))) {
        dirName.remove(dirName.size() - 1, 1);
    }

    KArchiveDirectory *parentDir = rootDir();
    int i = dirName.lastIndexOf(QLatin1Char('/'));
    if (i != -1) {
        QString dir = name.left(i);
        dirName = name.mid(i + 1);
        parentDir = findOrCreate(dir);
    }

    KArchiveDirectory *e = new KArchiveDirectory(this, dirName, perm, mtime, user, group, QString()/*symlink*/);
    parentDir->addEntry(e);

    return true;
}

bool K7Zip::doWriteSymLink(const QString &name, const QString &target,
                           const QString &user, const QString &group,
                           mode_t perm, const QDateTime & /*atime*/, const QDateTime &mtime, const QDateTime & /*ctime*/)
{
    if (!isOpen()) {
        setErrorString(tr("Application error: 7-Zip file must be open before being written into"));
        qCWarning(KArchiveLog) << "doWriteSymLink failed: !isOpen()";
        return false;
    }

    if (!(mode() & QIODevice::WriteOnly)) {
        setErrorString(tr("Application error: attempted to write into non-writable 7-Zip file"));
        qCWarning(KArchiveLog) << "doWriteSymLink failed: !(mode() & QIODevice::WriteOnly)";
        return false;
    }

    // Find or create parent dir
    KArchiveDirectory *parentDir = rootDir();
    // In some files we can find dir/./file => call cleanPath
    QString fileName(QDir::cleanPath(name));
    int i = name.lastIndexOf(QLatin1Char('/'));
    if (i != -1) {
        QString dir = name.left(i);
        fileName = name.mid(i + 1);
        parentDir = findOrCreate(dir);
    }
    QByteArray encodedTarget = QFile::encodeName(target);

    K7ZipFileEntry *e = new K7ZipFileEntry(this, fileName, perm, mtime, user, group, target, 0, 0, nullptr);
    d->outData.append(encodedTarget);

    if (!parentDir->addEntryV2(e))
        return false;

    d->m_entryList << e;

    return true;
}

void K7Zip::virtual_hook(int id, void *data)
{
    KArchive::virtual_hook(id, data);
}
