/*
 * terraintype.h
 * Copyright 2008-2009, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
 * Copyrigth 2009, Edward Hutchins <eah1@yahoo.com>
 *
 * This file is part of libtiled.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *    1. Redistributions of source code must retain the above copyright notice,
 *       this list of conditions and the following disclaimer.
 *
 *    2. Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE CONTRIBUTORS ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
 * EVENT SHALL THE CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef TERRAINTYPE_H
#define TERRAINTYPE_H

#include "object.h"

#include <QColor>
#include <QVector>
#include <QString>

namespace Tiled {

class Tile;
class Tileset;

/**
 * A terraintype, representing a terrain type.
 */
class TILEDSHARED_EXPORT TerrainType : public Object
{
public:
    TerrainType(int id, Tileset *tileset, QString name, int imageTile):
      mId(id),
      mTileset(tileset),
      mName(name),
      mImageTile(imageTile)
      {
//          mColor;
      }

      int id() const { return mId; }
      Tileset *tileset() const { return mTileset; }
      QString name() const { return mName; }
//      QColor color() const { return mColor; }
      int paletteImageTile() const { return mImageTile; }
//      Tile *paletteImage() const { return mTileset->tileAt(mImageTile); }
      bool hasTransitionDistances() const { return !mTransitionDistance.isEmpty(); }
      int transitionDistance(int targetTerrainType) const { return mTransitionDistance[targetTerrainType + 1]; }
      void setTransitionDistance(int targetTerrainType, int distance) { mTransitionDistance[targetTerrainType + 1] = distance; }

      void setTransitionDistances(QVector<int> &transitionDistances) { mTransitionDistance = transitionDistances; }

private:
    int mId;
    Tileset *mTileset;
    QString mName;
//    QColor mColor;
    int mImageTile;
    QVector<int> mTransitionDistance;
};

} // namespace Tiled

#endif // TERRAINTYPE_H
