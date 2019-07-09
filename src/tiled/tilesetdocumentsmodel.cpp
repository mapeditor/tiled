/*
 * tilesetdocumentsmodel.cpp
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

#include "tilesetdocumentsmodel.h"

#include "documentmanager.h"
#include "mapdocument.h"
#include "tilesetdocument.h"
#include "tileset.h"

#include <algorithm>

namespace Tiled {

TilesetDocumentsModel::TilesetDocumentsModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

int TilesetDocumentsModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : mTilesetDocuments.size();
}

QVariant TilesetDocumentsModel::data(const QModelIndex &index, int role) const
{
    const auto &document = mTilesetDocuments.at(index.row());

    switch (role) {
    case TilesetDocumentRole:
        return QVariant::fromValue(document.data());
    case TilesetRole:
        return QVariant::fromValue(document->tileset());
    case Qt::DisplayRole:
        return document->tileset()->name();
    case Qt::ToolTipRole:
        return document->fileName();
    }

    return QVariant();
}

void TilesetDocumentsModel::insert(int index, TilesetDocument *tilesetDocument)
{
    beginInsertRows(QModelIndex(), index, index);
    mTilesetDocuments.insert(index, tilesetDocument->sharedFromThis());
    endInsertRows();

    connect(tilesetDocument, &TilesetDocument::tilesetNameChanged,
            this, &TilesetDocumentsModel::tilesetNameChanged);
    connect(tilesetDocument, &TilesetDocument::fileNameChanged,
            this, &TilesetDocumentsModel::tilesetFileNameChanged);
}

void TilesetDocumentsModel::remove(int index)
{
    beginRemoveRows(QModelIndex(), index, index);
    auto tilesetDocument = mTilesetDocuments.takeAt(index);
    endRemoveRows();

    tilesetDocument->disconnect(this);
}

void TilesetDocumentsModel::tilesetNameChanged(Tileset *tileset)
{
    for (int i = 0; i < mTilesetDocuments.size(); ++i) {
        const auto &doc = mTilesetDocuments.at(i);
        if (doc->tileset() == tileset) {
            const QModelIndex dataIndex = index(i, 0, QModelIndex());
            emit dataChanged(dataIndex, dataIndex, { Qt::DisplayRole });
            break;
        }
    }
}

void TilesetDocumentsModel::tilesetFileNameChanged()
{
    TilesetDocument *tilesetDocument = static_cast<TilesetDocument*>(sender());
    for (int i = 0; i < mTilesetDocuments.size(); ++i) {
        if (mTilesetDocuments.at(i) == tilesetDocument) {
            const QModelIndex dataIndex = index(i, 0, QModelIndex());
            emit dataChanged(dataIndex, dataIndex, { Qt::ToolTipRole });
            break;
        }
    }
}


TilesetDocumentsFilterModel::TilesetDocumentsFilterModel(QObject *parent)
    : QSortFilterProxyModel(parent)
    , mMapDocument(nullptr)
{
    setSortLocaleAware(true);
    setSourceModel(DocumentManager::instance()->tilesetDocumentsModel());
    sort(0);
}

void TilesetDocumentsFilterModel::setMapDocument(MapDocument *mapDocument)
{
    if (mMapDocument == mapDocument)
        return;

    mMapDocument = mapDocument;
    invalidateFilter();
}

bool TilesetDocumentsFilterModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    const auto sm = sourceModel();
    const auto index = sm->index(sourceRow, 0, sourceParent);
    const QVariant variant = sm->data(index, TilesetDocumentsModel::TilesetDocumentRole);
    const TilesetDocument *tilesetDocument = variant.value<TilesetDocument *>();
    Q_ASSERT(tilesetDocument);

    const bool accepted = !tilesetDocument->isEmbedded()
            || tilesetDocument->mapDocuments().first() == mMapDocument;
    return accepted;
}

} // namespace Tiled
