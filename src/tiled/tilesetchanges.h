/*
 * tilesetchanges.h
 * Copyright 2011, Maus <Zeitmaus@github>
 * Copyright 2011-2013, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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

#include "changevalue.h"
#include "tileset.h"

#include <QColor>
#include <QPoint>
#include <QSize>

namespace Tiled {

class Tileset;

class TilesetDocument;

class RenameTileset : public QUndoCommand
{
public:
    RenameTileset(TilesetDocument *tilesetDocument,
                  const QString &newName);

    void undo() override;
    void redo() override;

    int id() const override { return Cmd_ChangeTilesetName; }
    bool mergeWith(const QUndoCommand *other) override;

private:
    TilesetDocument *mTilesetDocument;
    QString mOldName;
    QString mNewName;
};


class ChangeTilesetTileOffset : public QUndoCommand
{
public:
    ChangeTilesetTileOffset(TilesetDocument *tilesetDocument,
                            QPoint tileOffset);

    void undo() override;
    void redo() override;

    int id() const override { return Cmd_ChangeTilesetTileOffset; }
    bool mergeWith(const QUndoCommand *other) override;

private:
    TilesetDocument *mTilesetDocument;
    QPoint mOldTileOffset;
    QPoint mNewTileOffset;
};


struct TilesetParameters
{
    TilesetParameters() = default;

    explicit TilesetParameters(const Tileset &tileset);

    bool operator != (const TilesetParameters &other) const;

    QUrl imageSource;
    QColor transparentColor;
    QSize tileSize;
    int tileSpacing = 0;
    int margin = 0;
};

class ChangeTilesetParameters : public QUndoCommand
{
public:
    ChangeTilesetParameters(TilesetDocument *tilesetDocument,
                            const TilesetParameters &parameters);

    void undo() override;
    void redo() override;

private:
    void apply(const TilesetParameters &parameters);

    TilesetDocument *mTilesetDocument;
    TilesetParameters mOldParameters;
    TilesetParameters mNewParameters;
};


class ChangeTilesetColumnCount : public QUndoCommand
{
public:
    ChangeTilesetColumnCount(TilesetDocument *tilesetDocument,
                             int columnCount);

    void undo() override { swap(); }
    void redo() override { swap(); }

private:
    void swap();

    TilesetDocument *mTilesetDocument;
    int mColumnCount;
};


class ChangeTilesetBackgroundColor : public QUndoCommand
{
public:
    ChangeTilesetBackgroundColor(TilesetDocument *tilesetDocument,
                                 const QColor &color);

    void undo() override { swap(); }
    void redo() override { swap(); }

private:
    void swap();

    TilesetDocument *mTilesetDocument;
    QColor mColor;
};


class ChangeTilesetOrientation : public QUndoCommand
{
public:
    ChangeTilesetOrientation(TilesetDocument *tilesetDocument,
                             Tileset::Orientation orientation);

    void undo() override { swap(); }
    void redo() override { swap(); }

private:
    void swap();

    TilesetDocument *mTilesetDocument;
    Tileset::Orientation mOrientation;
};


class ChangeTilesetObjectAlignment : public QUndoCommand
{
public:
    ChangeTilesetObjectAlignment(TilesetDocument *tilesetDocument,
                                 Alignment objectAlignment);

    void undo() override { swap(); }
    void redo() override { swap(); }

private:
    void swap();

    TilesetDocument *mTilesetDocument;
    Alignment mObjectAlignment;
};

class ChangeTilesetTileRenderSize : public ChangeValue<Tileset, Tileset::TileRenderSize>
{
public:
    ChangeTilesetTileRenderSize(TilesetDocument *tilesetDocument,
                                Tileset::TileRenderSize tileRenderSize);

protected:
    Tileset::TileRenderSize getValue(const Tileset *tileset) const override;
    void setValue(Tileset *tileset, const Tileset::TileRenderSize &tileRenderSize) const override;
};

class ChangeTilesetFillMode : public ChangeValue<Tileset, Tileset::FillMode>
{
public:
    ChangeTilesetFillMode(TilesetDocument *tilesetDocument,
                          Tileset::FillMode fillMode);

protected:
    Tileset::FillMode getValue(const Tileset *tileset) const override;
    void setValue(Tileset *tileset, const Tileset::FillMode &fillMode) const override;
};

class ChangeTilesetGridSize : public QUndoCommand
{
public:
    ChangeTilesetGridSize(TilesetDocument *tilesetDocument,
                          QSize gridSize);

    void undo() override { swap(); }
    void redo() override { swap(); }

private:
    void swap();

    TilesetDocument *mTilesetDocument;
    QSize mGridSize;
};

class ChangeTilesetTransformationFlags : public QUndoCommand
{
public:
    ChangeTilesetTransformationFlags(TilesetDocument *TilesetDocument,
                                     Tileset::TransformationFlags newValue);

    void undo() override;
    void redo() override;

private:
    TilesetDocument *mTilesetDocument;
    const Tileset::TransformationFlags mOldValue;
    const Tileset::TransformationFlags mNewValue;
};

} // namespace Tiled
