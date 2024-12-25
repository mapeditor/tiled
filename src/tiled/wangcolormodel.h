/*
 * wangcolormodel.h
 * Copyright 2017, Benjamin Trotter <bdtrotte@ucsc.edu>
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

#include "wangset.h"

#include <QAbstractListModel>

namespace Tiled {

class Tileset;

class TilesetDocument;

class WangColorModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum UserRoles {
        ColorRole = Qt::UserRole,
    };

    WangColorModel(TilesetDocument *tilesetDocument,
                   WangSet *wangSet,
                   QObject *parent = nullptr);

    QModelIndex colorIndex(int color) const;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index,
                  int role = Qt::DisplayRole) const override;

    bool setData(const QModelIndex &index, const QVariant &value, int role) override;

    Qt::ItemFlags flags(const QModelIndex &index) const override;

    WangSet *wangSet() const { return mWangSet; }
    TilesetDocument *tilesetDocument() const { return mTilesetDocument; }

    void resetModel();

    int colorAt(const QModelIndex &index) const;

    QSharedPointer<WangColor> wangColorAt(const QModelIndex &index) const;

    void setName(WangColor *wangColor, const QString &name);
    void setImage(WangColor *wangColor, int imageId);
    void setColor(WangColor *wangColor, const QColor &color);
    void setProbability(WangColor *wangColor, qreal probability);

private:
    void emitDataChanged(WangColor *wangColor);

    TilesetDocument *mTilesetDocument;
    WangSet *mWangSet;
};

} // namespace Tiled
