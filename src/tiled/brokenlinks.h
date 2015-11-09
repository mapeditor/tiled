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

#ifndef TILED_INTERNAL_BROKENLINKS_H
#define TILED_INTERNAL_BROKENLINKS_H

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

namespace Internal {

class MapDocument;

enum BrokenLinkType {
    TileImageSource,
    TilesetFileName,
    TilesetImageSource,
};

struct BrokenLink {
    BrokenLinkType type;

    union {
        Tileset *tileset;
        Tile *tile;
    };

    QString filePath() const;
};


class BrokenLinksModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(bool hasBrokenLinks READ hasBrokenLinks NOTIFY hasBrokenLinksChanged)

public:
    BrokenLinksModel(MapDocument *mapDocument, QObject *parent = nullptr);

    MapDocument *mapDocument() const;

    void refresh();
    bool hasBrokenLinks() const;

    const BrokenLink &brokenLink(const QModelIndex &index) const;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

signals:
    void hasBrokenLinksChanged(bool hasBrokenLinks);

private slots:
    void tileImageSourceChanged(Tile *tile);
    void tilesetChanged(Tileset *tileset);
    void tilesetReplaced(int index, Tileset *tileset);

private:
    void removeLink(int index);

    MapDocument *mMapDocument;
    QVector<BrokenLink> mBrokenLinks;
};


inline MapDocument *BrokenLinksModel::mapDocument() const
{
    return mMapDocument;
}

inline bool BrokenLinksModel::hasBrokenLinks() const
{
    return !mBrokenLinks.isEmpty();
}

inline const BrokenLink &BrokenLinksModel::brokenLink(const QModelIndex &index) const
{
    return mBrokenLinks.at(index.row());
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
    void selectionChanged(const QItemSelection &selected);

private:
    void tryFixLink(const BrokenLink &link);

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

#endif // TILED_INTERNAL_BROKENLINKS_H
