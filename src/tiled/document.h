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
#include <QSharedPointer>
#include <QString>
#include <QVariant>

#include <memory>

class QUndoStack;

namespace Tiled {

class FileFormat;
class Object;
class Tile;

class ChangeEvent;
class EditableAsset;

/**
 * Keeps track of a file and its undo history.
 */
class Document : public QObject,
                 public QEnableSharedFromThis<Document>
{
    Q_OBJECT

    Q_PROPERTY(QString fileName READ fileName NOTIFY fileNameChanged)
    Q_PROPERTY(bool modified READ isModified NOTIFY modifiedChanged)

public:
    enum DocumentType {
        MapDocumentType,
        TilesetDocumentType,
        WorldDocumentType,
        ProjectDocumentType
    };

    Document(DocumentType type,
             const QString &fileName = QString(),
             QObject *parent = nullptr);
    ~Document() override;

    DocumentType type() const { return mType; }

    QString fileName() const;
    QString canonicalFilePath() const;

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

    EditableAsset *editable();
    void setEditable(std::unique_ptr<EditableAsset> editable);

    Object *currentObject() const { return mCurrentObject; }
    void setCurrentObject(Object *object);
    void setCurrentObject(Object *object, Document *owningDocument);

    virtual QList<Object*> currentObjects() const;

    void setProperty(Object *object, const QString &name, const QVariant &value);
    void setPropertyMember(Object *object, const QStringList &path, const QVariant &value);
    void setProperties(Object *object, const Properties &properties);
    void removeProperty(Object *object, const QString &name);

    bool ignoreBrokenLinks() const;
    void setIgnoreBrokenLinks(bool ignoreBrokenLinks);

    bool changedOnDisk() const;
    void setChangedOnDisk(bool changedOnDisk);

    virtual QString lastExportFileName() const = 0;
    virtual void setLastExportFileName(const QString &fileName) = 0;

    virtual FileFormat *exportFormat() const = 0;
    virtual void setExportFormat(FileFormat *format) = 0;

    virtual void checkIssues() {}

    static const QHash<QString, Document *> &documentInstances();

signals:
    void changed(const ChangeEvent &change);
    void saved();

    void fileNameChanged(const QString &fileName,
                         const QString &oldFileName);
    void modifiedChanged();

    void currentObjectSet(Object *object);
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
    virtual std::unique_ptr<EditableAsset> createEditable() = 0;
    virtual bool isModifiedImpl() const;

    void updateIsModified();

    void setFileName(const QString &fileName);

    void checkFilePathProperties(const Object *object) const;

    QDateTime mLastSaved;

    Object *mCurrentObject = nullptr;   /**< Current properties object. */
    Document *mCurrentObjectDocument = nullptr;

    std::unique_ptr<EditableAsset> mEditable;

private:
    void currentObjectDocumentChanged(const ChangeEvent &change);
    void currentObjectDocumentDestroyed();

    const DocumentType mType;

    QString mFileName;
    QString mCanonicalFilePath;

    QUndoStack * const mUndoStack;

    bool mModified = false;
    bool mChangedOnDisk = false;
    bool mIgnoreBrokenLinks = false;

    static QHash<QString, Document*> sDocumentInstances;
};


inline QString Document::fileName() const
{
    return mFileName;
}

inline QString Document::canonicalFilePath() const
{
    return mCanonicalFilePath;
}

/**
 * Returns the undo stack of this document. Should be used to push any commands
 * on that modify the document.
 */
inline QUndoStack *Document::undoStack() const
{
    return mUndoStack;
}

/**
 * Returns whether the document has unsaved changes.
 */
inline bool Document::isModified() const
{
    return mModified;
}

/**
 * Sets the current object for this document (displayed in the Properties view).
 *
 * This version should only be used with objects owned by this document. See
 * setCurrentObject(Object*, Document*) for setting it to an object from
 * another document.
 */
inline void Document::setCurrentObject(Object *object)
{
    setCurrentObject(object, this);
}

inline bool Document::ignoreBrokenLinks() const
{
    return mIgnoreBrokenLinks;
}

inline bool Document::changedOnDisk() const
{
    return mChangedOnDisk;
}

inline const QHash<QString, Document *> &Document::documentInstances()
{
    return sDocumentInstances;
}

using DocumentPtr = QSharedPointer<Document>;

} // namespace Tiled
