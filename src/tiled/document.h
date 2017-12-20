/*
 * document.h
 * Copyright 2015, Thorbjørn Lindeijer <bjorn@lindeijer.nl>
 *
 * This file is part of Tiled.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include "properties.h"

#include <QDateTime>
#include <QObject>
#include <QString>
#include <QVariant>
#include <QPointer>

class QUndoStack;

namespace Tiled {

class FileFormat;
class Object;
class Tile;

namespace Internal {

/**
 * Keeps track of a file and its undo history.
 */
class Document : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString fileName READ fileName NOTIFY fileNameChanged)
    Q_PROPERTY(bool modified READ isModified NOTIFY modifiedChanged)

public:
    enum DocumentType {
        MapDocumentType,
        TilesetDocumentType
    };

    Document(DocumentType type,
             const QString &fileName = QString(),
             QObject *parent = nullptr);

    DocumentType type() const { return mType; }

    QString fileName() const;

    /**
     * Returns the name with which to display this document. It is the file name
     * without its path, or 'untitled' when the document has no file name.
     */
    virtual QString displayName() const = 0;

    /**
     * Saves the document to the file at \a fileName. Returns whether or not
     * the file was saved successfully. If not, <i>error</i> will be set to the
     * error message if it is not 0.
     *
     * If the save was successful, the file name of this document will be set
     * to \a fileName.
     *
     * The file format will be the same as this map was opened with.
     */
    virtual bool save(const QString &fileName, QString *error = nullptr) = 0;

    virtual FileFormat *writerFormat() const = 0;

    QDateTime lastSaved() const { return mLastSaved; }

    QUndoStack *undoStack() const;
    bool isModified() const;

    Object *currentObject() const { return mCurrentObject; }
    void setCurrentObject(Object *object);

    virtual QList<Object*> currentObjects() const;

    void setProperty(Object *object, const QString &name, const QVariant &value);
    void setProperties(Object *object, const Properties &properties);
    void removeProperty(Object *object, const QString &name);

    bool ignoreBrokenLinks() const;
    void setIgnoreBrokenLinks(bool ignoreBrokenLinks);

    QString lastExportFileName() const;
    void setLastExportFileName(const QString &fileName);

    virtual FileFormat *exportFormat() const = 0;
    virtual void setExportFormat(FileFormat *format) = 0;

signals:
    void saved();

    void fileNameChanged(const QString &fileName,
                         const QString &oldFileName);
    void modifiedChanged();

    void currentObjectChanged(Object *object);

    /**
     * Makes the Properties window visible and take focus.
     */
    void editCurrentObject();

    void propertyAdded(Object *object, const QString &name);
    void propertyRemoved(Object *object, const QString &name);
    void propertyChanged(Object *object, const QString &name);
    void propertiesChanged(Object *object);

    void ignoreBrokenLinksChanged(bool ignoreBrokenLinks);

protected:
    void setFileName(const QString &fileName);

    DocumentType mType;
    QString mFileName;
    QUndoStack *mUndoStack;
    QDateTime mLastSaved;

    Object *mCurrentObject;             /**< Current properties object. */

    bool mIgnoreBrokenLinks;

    QString mLastExportFileName;
};


inline QString Document::fileName() const
{
    return mFileName;
}

/**
 * Returns the undo stack of this document. Should be used to push any commands
 * on that modify the document.
 */
inline QUndoStack *Document::undoStack() const
{
    return mUndoStack;
}

inline bool Document::ignoreBrokenLinks() const
{
    return mIgnoreBrokenLinks;
}

inline QString Document::lastExportFileName() const
{
    return mLastExportFileName;
}

inline void Document::setLastExportFileName(const QString &fileName)
{
    mLastExportFileName = fileName;
}


} // namespace Internal
} // namespace Tiled
