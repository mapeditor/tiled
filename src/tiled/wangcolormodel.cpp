/*
 * wangcolormodel.cpp
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

#include "wangcolormodel.h"

#include "changeevents.h"
#include "changewangcolordata.h"
#include "mapdocument.h"
#include "tileset.h"
#include "tilesetdocument.h"
#include "wangset.h"

#include <QApplication>
#include <QFont>
#include <QPalette>

using namespace Tiled;

WangColorModel::WangColorModel(TilesetDocument *tilesetDocument,
                               WangSet *wangSet,
                               QObject *parent)
    : QAbstractListModel(parent)
    , mTilesetDocument(tilesetDocument)
    , mWangSet(wangSet)
    , mEraserIcon(QLatin1String(":images/22/stock-tool-eraser.png"))
{
}

QModelIndex WangColorModel::colorModelIndex(int color) const
{
    if (!mWangSet || color < 0 || color > mWangSet->colorCount())
        return QModelIndex();

    return createIndex(color, 0);
}

int WangColorModel::rowCount(const QModelIndex &parent) const
{
    if (!mWangSet || parent.isValid())
        return 0;

    return mWangSet->colorCount() + 1;
}

int WangColorModel::columnCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : 1;
}

QVariant WangColorModel::data(const QModelIndex &index, int role) const
{
    if (!mWangSet)
        return QVariant();

    const int colorIndex = index.row();

    switch (role) {
    case WangColorIndexRole:
        return colorIndex;
    case Qt::DisplayRole:
    case Qt::EditRole:
        if (colorIndex == 0)
            return QCoreApplication::translate("Tiled::WangDock", "Erase Terrain");
        return wangColorAt(index)->name();
    case Qt::DecorationRole:
        if (colorIndex == 0)
            return mEraserIcon;
        if (Tile *tile = mWangSet->tileset()->findTile(wangColorAt(index)->imageId()))
            return tile->image().copy(tile->imageRect());
        break;
    case ColorRole:
        if (colorIndex == 0)
            return QVariant();
        return wangColorAt(index)->color();
    }

    return QVariant();
}

bool WangColorModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (role == Qt::EditRole) {
        const auto wangColor = wangColorAt(index);
        if (!wangColor)
            return false;

        const QString newName = value.toString();
        if (wangColor->name() != newName) {
            auto command = new ChangeWangColorName(mTilesetDocument, wangColor.data(), newName);
            mTilesetDocument->undoStack()->push(command);
        }

        return true;
    }

    return false;
}

Qt::ItemFlags WangColorModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags flags = QAbstractItemModel::flags(index);
    if (index.row() == 0)
        return flags;

    return flags | Qt::ItemIsEditable;
}

void WangColorModel::resetModel()
{
    beginResetModel();
    endResetModel();
}

QSharedPointer<WangColor> WangColorModel::wangColorAt(const QModelIndex &index) const
{
    if (!index.isValid() || index.row() == 0)
        return QSharedPointer<WangColor>();

    return mWangSet->colorAt(index.row());
}

void WangColorModel::setName(WangColor *wangColor, const QString &name)
{
    wangColor->setName(name);
    emitDataChanged(wangColor);
    emitToTilesetAndMaps(WangColorChangeEvent(wangColor, WangColorChangeEvent::NameProperty));
}

void WangColorModel::setImage(WangColor *wangColor, int imageId)
{
    wangColor->setImageId(imageId);
    emitDataChanged(wangColor);
    emitToTilesetAndMaps(WangColorChangeEvent(wangColor, WangColorChangeEvent::ImageProperty));
}

void WangColorModel::setColor(WangColor *wangColor, const QColor &color)
{
    wangColor->setColor(color);
    emitDataChanged(wangColor);
    emitToTilesetAndMaps(WangColorChangeEvent(wangColor, WangColorChangeEvent::ColorProperty));
}

void WangColorModel::setProbability(WangColor *wangColor, qreal probability)
{
    wangColor->setProbability(probability);
    // no data changed signal because probability not exposed by model
    emitToTilesetAndMaps(WangColorChangeEvent(wangColor, WangColorChangeEvent::ProbabilityProperty));
}

void WangColorModel::emitDataChanged(WangColor *wangColor)
{
    const QModelIndex i = colorModelIndex(wangColor->colorIndex());
    emit dataChanged(i, i);
}

void WangColorModel::emitToTilesetAndMaps(const ChangeEvent &event)
{
    emit mTilesetDocument->changed(event);

    // todo: this doesn't work reliably because it only reaches maps that use
    // the tileset, whereas the Properties view can be showing stuff from any
    // tileset.
    for (MapDocument *mapDocument : mTilesetDocument->mapDocuments())
        emit mapDocument->changed(event);
}

#include "moc_wangcolormodel.cpp"
