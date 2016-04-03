/*
 * tilesetparametersedit.h
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

#ifndef TILESETPARAMETERSEDIT_H
#define TILESETPARAMETERSEDIT_H

#include "mapdocument.h"

#include <QWidget>

class QLabel;

namespace Tiled {
namespace Internal {

/**
 * A widget that allows opening the EditTilesetDialog for changing the tileset
 * parameters.
 */
class TilesetParametersEdit : public QWidget
{
    Q_OBJECT

public:
    explicit TilesetParametersEdit(QWidget *parent = nullptr);

    void setTileset(const EmbeddedTileset &tileset);
    const EmbeddedTileset &tileset() const;

private slots:
    void buttonClicked();

private:
    QLabel *mLabel;
    EmbeddedTileset mTileset;
};

inline const EmbeddedTileset &TilesetParametersEdit::tileset() const
{
    return mTileset;
}

} // namespace Internal
} // namespace Tiled

#endif // TILESETPARAMETERSEDIT_H
