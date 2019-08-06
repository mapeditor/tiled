/*
 * editableterrain.h
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
#include "terrain.h"

namespace Tiled {

class EditableTile;
class EditableTileset;

class EditableTerrain : public EditableObject
{
    Q_OBJECT

    Q_PROPERTY(int id READ id)
    Q_PROPERTY(QString name READ name WRITE setName)
    Q_PROPERTY(Tiled::EditableTile *imageTile READ imageTile WRITE setImageTile)
    Q_PROPERTY(Tiled::EditableTileset *tileset READ tileset)

public:
    EditableTerrain(EditableTileset *tileset,
                    Terrain *terrain,
                    QObject *parent = nullptr);
    ~EditableTerrain() override;

    int id() const;
    QString name() const;
    EditableTile *imageTile() const;
    EditableTileset *tileset() const;

    Terrain *terrain() const;

    void detach();
    void attach(EditableTileset *tileset);

public slots:
    void setName(const QString &type);
    void setImageTile(Tiled::EditableTile *imageTile);

private:
    std::unique_ptr<Terrain> mDetachedTerrain;
};


inline int EditableTerrain::id() const
{
    return terrain()->id();
}

inline QString EditableTerrain::name() const
{
    return terrain()->name();
}

inline Terrain *EditableTerrain::terrain() const
{
    return static_cast<Terrain*>(object());
}

} // namespace Tiled

Q_DECLARE_METATYPE(Tiled::EditableTerrain*)
