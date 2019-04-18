/*
 * grouplayer.h
 * Copyright 2017, Thorbjørn Lindeijer <bjorn@lindeijer.nl>
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

#include "layer.h"

#include <QList>

#include <memory>

namespace Tiled {

class TILEDSHARED_EXPORT GroupLayer : public Layer
{
    Q_OBJECT

public:
    GroupLayer(const QString &name, int x, int y);
    ~GroupLayer() override;

    int layerCount() const;
    Layer *layerAt(int index) const;
    const QList<Layer*> &layers() const { return mLayers; }

    void addLayer(std::unique_ptr<Layer> layer);
    void insertLayer(int index, Layer *layer);
    Layer *takeLayerAt(int index);

    bool isEmpty() const override;
    QSet<SharedTileset> usedTilesets() const override;
    bool referencesTileset(const Tileset *tileset) const override;
    void replaceReferencesToTileset(Tileset *oldTileset, Tileset *newTileset) override;
    bool canMergeWith(const Layer *other) const override;
    Layer *mergedWith(const Layer *other) const override;
    GroupLayer *clone() const override;

    // Enable easy iteration over children with range-based for
    QList<Layer*>::iterator begin() { return mLayers.begin(); }
    QList<Layer*>::iterator end() { return mLayers.end(); }
    QList<Layer*>::const_iterator begin() const { return mLayers.begin(); }
    QList<Layer*>::const_iterator end() const { return mLayers.end(); }

protected:
    void setMap(Map *map) override;
    GroupLayer *initializeClone(GroupLayer *clone) const;

private:
    void adoptLayer(Layer &layer);

    QList<Layer*> mLayers;
};


inline int GroupLayer::layerCount() const
{
    return mLayers.size();
}

inline Layer *GroupLayer::layerAt(int index) const
{
    return mLayers.at(index);
}

} // namespace Tiled
