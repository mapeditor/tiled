/*
 * tilestampmodel.h
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

#ifndef TILED_INTERNAL_TILESTAMPMODEL_H
#define TILED_INTERNAL_TILESTAMPMODEL_H

#include <QAbstractItemModel>

namespace Tiled {

class Map;

namespace Internal {

class TileStamp;
struct TileStampVariation;

class TileStampModel : public QAbstractItemModel
{
public:
    TileStampModel(QObject *parent = 0);

    QModelIndex index(int row, int column,
                      const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &index) const override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;

    bool setData(const QModelIndex &index, const QVariant &value,
                 int role = Qt::EditRole) override;

    QVariant data(const QModelIndex &index,
                  int role = Qt::DisplayRole) const override;

    Qt::ItemFlags flags(const QModelIndex &index) const override;

    bool removeRows(int row, int count, const QModelIndex &parent) override;

    /**
     * Returns the stamp at the given \a index.
     */
    const TileStamp &stampAt(const QModelIndex &index) const;
    bool isStamp(const QModelIndex &index) const;

    const TileStampVariation *variationAt(const QModelIndex &index) const;

    const QList<TileStamp> &stamps() const;

    void addStamp(const TileStamp &stamp);
    void removeStamp(const TileStamp &stamp);

    void addVariation(const TileStamp &stamp,
                      const TileStampVariation &variation);

private:
    QList<TileStamp> mStamps;

    mutable QHash<Map *, QPixmap> mThumbnailCache;
};


inline const QList<TileStamp> &TileStampModel::stamps() const
{
    return mStamps;
}

} // namespace Internal
} // namespace Tiled

#endif // TILED_INTERNAL_TILESTAMPMODEL_H
