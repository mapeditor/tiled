/*
 * editablewangset.h
 * Copyright 2019, Your Name <your.name@domain>
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
#include "wangset.h"

#include <QJSValue>

namespace Tiled {

class EditableTile;
class EditableTileset;
class TilesetDocument;

class EditableWangSet : public EditableObject
{
    Q_OBJECT

    Q_PROPERTY(QString name READ name WRITE setName)
    Q_PROPERTY(Type type READ type WRITE setType)
    Q_PROPERTY(Tiled::EditableTile *imageTile READ imageTile WRITE setImageTile)
    Q_PROPERTY(int colorCount READ colorCount WRITE setColorCount)
    Q_PROPERTY(Tiled::EditableTileset *tileset READ tileset CONSTANT)

public:
    enum Type {
        Corner,
        Edge,
        Mixed
    };
    Q_ENUM(Type)

    EditableWangSet(EditableTileset *tileset,
                    WangSet *wangSet,
                    QObject *parent = nullptr);
    ~EditableWangSet() override;

    QString name() const;
    Type type() const;
    EditableTile *imageTile() const;
    int colorCount() const;
    EditableTileset *tileset() const;

    Q_INVOKABLE QJSValue wangId(Tiled::EditableTile *tile);
    Q_INVOKABLE void setWangId(Tiled::EditableTile *tile, QJSValue value);

    Q_INVOKABLE QString colorName(int colorIndex) const;
    Q_INVOKABLE void setColorName(int colorIndex, const QString &name);

    Q_INVOKABLE Type effectiveTypeForColor(int color) const;

    void setName(const QString &name);
    void setType(Type type);
    void setImageTile(Tiled::EditableTile *imageTile);
    void setColorCount(int n);

    WangSet *wangSet() const;

    void detach();
    void attach(EditableTileset *tileset);
    void hold(std::unique_ptr<WangSet> wangSet);

    static EditableWangSet *find(WangSet *wangSet);
    static EditableWangSet *get(WangSet *wangSet);
    static EditableWangSet *get(EditableTileset *tileset, WangSet *wangSet);
    static void release(std::unique_ptr<WangSet> wangSet);

private:
    TilesetDocument *tilesetDocument() const;

    std::unique_ptr<WangSet> mDetachedWangSet;
};


inline QString EditableWangSet::name() const
{
    return wangSet()->name();
}

inline EditableWangSet::Type EditableWangSet::type() const
{
    return static_cast<Type>(wangSet()->type());
}

inline int EditableWangSet::colorCount() const
{
    return wangSet()->colorCount();
}

inline EditableWangSet::Type EditableWangSet::effectiveTypeForColor(int color) const
{
    return static_cast<Type>(wangSet()->effectiveTypeForColor(color));
}

inline WangSet *EditableWangSet::wangSet() const
{
    return static_cast<WangSet*>(object());
}

inline EditableWangSet *EditableWangSet::find(WangSet *wangSet)
{
    return static_cast<EditableWangSet*>(EditableObject::find(wangSet));
}

} // namespace Tiled

Q_DECLARE_METATYPE(Tiled::EditableWangSet*)
