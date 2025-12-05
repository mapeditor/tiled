/* This file is part of the KDE libraries
   SPDX-FileCopyrightText: 2014 David Faure <faure@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/
#ifndef KRCC_H
#define KRCC_H

#include <karchive.h>

/*!
 * \class KRcc
 * \inmodule KArchive
 *
 * KRcc is a class for reading dynamic binary resources created by Qt's rcc tool
 * from a .qrc file and the files it points to.
 *
 * Writing is not supported.
 * \brief A class for reading rcc resources.
 * \since 5.3
 */
class KARCHIVE_EXPORT KRcc : public KArchive
{
    Q_DECLARE_TR_FUNCTIONS(KRcc)

public:
    /*!
     * Creates an instance that operates on the given filename.
     *
     * \a filename is a local path (e.g. "/home/holger/myfile.rcc")
     */
    explicit KRcc(const QString &filename);

    /*!
     * If the rcc file is still opened, then it will be
     * closed automatically by the destructor.
     */
    ~KRcc() override;

protected:
    /*
     * Writing is not supported by this class, will always fail.
     * Returns always false
     */
    bool doPrepareWriting(const QString &name,
                          const QString &user,
                          const QString &group,
                          qint64 size,
                          mode_t perm,
                          const QDateTime &atime,
                          const QDateTime &mtime,
                          const QDateTime &ctime) override;

    /*
     * Writing is not supported by this class, will always fail.
     * Returns always false
     */
    bool doFinishWriting(qint64 size) override;

    /*
     * Writing is not supported by this class, will always fail.
     * Returns always false
     */
    bool doWriteDir(const QString &name,
                    const QString &user,
                    const QString &group,
                    mode_t perm,
                    const QDateTime &atime,
                    const QDateTime &mtime,
                    const QDateTime &ctime) override;

    /*
     * Writing is not supported by this class, will always fail.
     * Returns always false
     */
    bool doWriteSymLink(const QString &name,
                        const QString &target,
                        const QString &user,
                        const QString &group,
                        mode_t perm,
                        const QDateTime &atime,
                        const QDateTime &mtime,
                        const QDateTime &ctime) override;

    /*!
     * Registers the .rcc resource in the QResource system under a unique identifier,
     * then lists that, and creates the KArchiveFile/KArchiveDirectory entries.
     */
    bool openArchive(QIODevice::OpenMode mode) override;
    /*!
     * Unregisters the .rcc resource from the QResource system.
     */
    bool closeArchive() override;

protected:
    void virtual_hook(int id, void *data) override;

private:
    class KRccPrivate;
    KRccPrivate *const d;
};

#endif
