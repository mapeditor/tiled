/*
 * brokenlinks.h
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

#pragma once

#include "tileset.h"

#include <QAbstractListModel>
#include <QWidget>

class QAbstractButton;
class QDialogButtonBox;
class QItemSelection;
class QLabel;
class QSortFilterProxyModel;
class QTreeView;

namespace Tiled {

class Tile;
class Tileset;
class ObjectTemplate;

namespace Internal {

class Document;
class TilesetDocument;

enum BrokenLinkType {
    MapTilesetReference,
    TilesetTileImageSource,
    TilesetImageSource,
    ObjectTemplateReference,
};

struct BrokenLink {
    BrokenLinkType type;

    union {
        Tileset *_tileset;
        Tile *_tile;
        const ObjectTemplate *_objectTemplate;
    };

    QString filePath() const;
    Tileset *tileset() const;
    const ObjectTemplate *objectTemplate() const;
};


class BrokenLinksModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(bool hasBrokenLinks READ hasBrokenLinks NOTIFY hasBrokenLinksChanged)

public:
    BrokenLinksModel(QObject *parent = nullptr);

    void setDocument(Document *document);
    Document *document() const;

    void refresh();
    bool hasBrokenLinks() const;

    const BrokenLink &brokenLink(int index) const;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

signals:
    void hasBrokenLinksChanged(bool hasBrokenLinks);

private slots:
    void tileImageSourceChanged(Tile *tile);
    void tilesetChanged(Tileset *tileset);

    void tilesetAdded(int index, Tileset *tileset);
    void tilesetRemoved(Tileset *tileset);
    void tilesetReplaced(int index, Tileset *newTileset, Tileset *oldTileset);

    void objectTemplateReplaced();

private:
    void connectToTileset(const SharedTileset &tileset);
    void disconnectFromTileset(const SharedTileset &tileset);

    void removeLink(int index);

    Document *mDocument;
    QVector<BrokenLink> mBrokenLinks;
};


inline Document *BrokenLinksModel::document() const
{
    return mDocument;
}

inline bool BrokenLinksModel::hasBrokenLinks() const
{
    return !mBrokenLinks.isEmpty();
}

inline const BrokenLink &BrokenLinksModel::brokenLink(int index) const
{
    return mBrokenLinks.at(index);
}


class BrokenLinksWidget : public QWidget
{
    Q_OBJECT

public:
    BrokenLinksWidget(BrokenLinksModel *brokenLinksModel, QWidget *parent = nullptr);

signals:
    void ignore();

private slots:
    void clicked(QAbstractButton *button);
    void selectionChanged();

private:
    void tryFixLinks(const QVector<BrokenLink> &links);
    void tryFixLink(const BrokenLink &link);
    bool tryFixLink(const BrokenLink &link, const QString &newFilePath);

    BrokenLinksModel *mBrokenLinksModel;
    QSortFilterProxyModel *mProxyModel;
    QLabel *mTitleLabel;
    QLabel *mDescriptionLabel;
    QTreeView *mView;
    QDialogButtonBox *mButtons;
    QAbstractButton *mLocateButton;
};

} // namespace Internal
} // namespace Tiled
