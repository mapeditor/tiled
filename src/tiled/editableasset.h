/*
 * editableasset.h
 * Copyright 2018, Thorbjørn Lindeijer <bjorn@lindeijer.nl>
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

#include "editableobject.h"

#include <QJSValue>
#include <QSharedPointer>

#include <memory>

class QUndoCommand;
class QUndoStack;

namespace Tiled {

class Document;

namespace AssetType {
    Q_NAMESPACE

    enum Value {
        TileMap = 1,
        Tileset,
        Project,
        World,
    };
    Q_ENUM_NS(Value)
} // namespace AssetType

class EditableAsset : public EditableObject
{
    Q_OBJECT

    Q_PROPERTY(QString fileName READ fileName NOTIFY fileNameChanged)
    Q_PROPERTY(bool modified READ isModified NOTIFY modifiedChanged)
    Q_PROPERTY(bool isTileMap READ isMap CONSTANT)
    Q_PROPERTY(bool isTileset READ isTileset CONSTANT)
    Q_PROPERTY(AssetType::Value assetType READ assetType CONSTANT)

public:
    EditableAsset(Object *object, QObject *parent = nullptr);

    QString fileName() const;
    bool isReadOnly() const override = 0;
    bool isMap() const { return assetType() == AssetType::TileMap; }
    bool isTileset() const { return assetType() == AssetType::Tileset; }
    virtual AssetType::Value assetType() const = 0;

    QUndoStack *undoStack() const;
    bool isModified() const;
    bool push(QUndoCommand *command);
    bool push(std::unique_ptr<QUndoCommand> command);

    Q_INVOKABLE bool save();
    Q_INVOKABLE QJSValue macro(const QString &text, QJSValue callback);

    Document *document() const;

    /**
     * Creates a document for this asset.
     */
    virtual QSharedPointer<Document> createDocument() = 0;

public slots:
    void undo();
    void redo();

signals:
    void modifiedChanged();
    void fileNameChanged(const QString &fileName, const QString &oldFileName);

protected:
    virtual void setDocument(Document *document);

private:
    friend class Document;

    Document *mDocument = nullptr;
};


inline Document *EditableAsset::document() const
{
    return mDocument;
}

} // namespace Tiled

Q_DECLARE_METATYPE(Tiled::EditableAsset*)
