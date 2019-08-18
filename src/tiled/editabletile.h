/*
 * editabletile.h
 * Copyright 2019, Thorbjørn Lindeijer <bjorn@lindeijer.nl>
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
#include "tile.h"

#include <QJSValue>

namespace Tiled {

class EditableObjectGroup;
class EditableTerrain;
class EditableTileset;

class EditableTile : public EditableObject
{
    Q_OBJECT

    Q_PROPERTY(int id READ id)
    Q_PROPERTY(int width READ width)
    Q_PROPERTY(int height READ height)
    Q_PROPERTY(QSize size READ size)
    Q_PROPERTY(QString type READ type WRITE setType)
    Q_PROPERTY(QString imageFileName READ imageFileName WRITE setImageFileName)
    Q_PROPERTY(QJSValue terrain READ terrain WRITE setTerrain)
    Q_PROPERTY(qreal probability READ probability WRITE setProbability)
    Q_PROPERTY(Tiled::EditableObjectGroup *objectGroup READ objectGroup WRITE setObjectGroup)
    Q_PROPERTY(QJSValue frames READ frames WRITE setFrames)
    Q_PROPERTY(bool animated READ isAnimated)
    Q_PROPERTY(Tiled::EditableTileset *tileset READ tileset)

public:
    enum Flags {
        FlippedHorizontally     = 0x01,
        FlippedVertically       = 0x02,
        FlippedAntiDiagonally   = 0x04,
        RotatedHexagonal120     = 0x08
    };
    Q_ENUM(Flags)

    enum Corner {
        TopLeft,
        TopRight,
        BottomLeft,
        BottomRight
    };
    Q_ENUM(Corner)

    EditableTile(EditableTileset *tileset,
                 Tile *tile,
                 QObject *parent = nullptr);
    ~EditableTile() override;

    int id() const;
    int width() const;
    int height() const;
    QSize size() const;
    const QString &type() const;
    QString imageFileName() const;
    QJSValue terrain() const;
    qreal probability() const;
    EditableObjectGroup *objectGroup() const;
    QJSValue frames() const;
    bool isAnimated() const;
    EditableTileset *tileset() const;

    Q_INVOKABLE Tiled::EditableTerrain *terrainAtCorner(Corner corner) const;
    Q_INVOKABLE void setTerrainAtCorner(Corner corner, Tiled::EditableTerrain *editableTerrain);

    Tile *tile() const;

    void detach();
    void attach(EditableTileset *tileset);

    const ObjectGroup *attachedObjectGroup() const { return mAttachedObjectGroup; }
    void detachObjectGroup();

public slots:
    void setType(const QString &type);
    void setImageFileName(const QString &fileName);
    void setTerrain(QJSValue value);
    void setProbability(qreal probability);
    void setObjectGroup(EditableObjectGroup *editableObjectGroup);
    void setFrames(QJSValue value);

private:
    std::unique_ptr<Tile> mDetachedTile;
    mutable ObjectGroup *mAttachedObjectGroup = nullptr;
};


inline int EditableTile::id() const
{
    return tile()->id();
}

inline int EditableTile::width() const
{
    return tile()->width();
}

inline int EditableTile::height() const
{
    return tile()->height();
}

inline QSize EditableTile::size() const
{
    return tile()->size();
}

inline const QString &EditableTile::type() const
{
    return tile()->type();
}

inline QString EditableTile::imageFileName() const
{
    return tile()->imageSource().toString(QUrl::PreferLocalFile);
}

inline qreal EditableTile::probability() const
{
    return tile()->probability();
}

inline bool EditableTile::isAnimated() const
{
    return tile()->isAnimated();
}

inline Tile *EditableTile::tile() const
{
    return static_cast<Tile*>(object());
}

} // namespace Tiled

Q_DECLARE_METATYPE(Tiled::EditableTile*)
