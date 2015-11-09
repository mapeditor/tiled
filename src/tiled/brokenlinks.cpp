/*
 * brokenlinks.cpp
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

#include "brokenlinks.h"

#include "changetileimagesource.h"
#include "map.h"
#include "mapdocument.h"
#include "preferences.h"
#include "replacetileset.h"
#include "tmxmapformat.h"
#include "tile.h"
#include "tilesetchanges.h"
#include "utils.h"

#include <QPushButton>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QFileInfo>
#include <QHeaderView>
#include <QImageReader>
#include <QLabel>
#include <QMessageBox>
#include <QSettings>
#include <QSortFilterProxyModel>
#include <QStackedLayout>
#include <QTreeView>

#include <algorithm>

namespace Tiled {
namespace Internal {

QString BrokenLink::filePath() const
{
    switch (type) {
    case TilesetImageSource:
        return tileset->imageSource();
    case TilesetFileName:
        return tileset->fileName();
    case TileImageSource:
        return tile->imageSource();
    }

    return QString();
}


BrokenLinksModel::BrokenLinksModel(MapDocument *mapDocument, QObject *parent)
    : QAbstractListModel(parent)
    , mMapDocument(mapDocument)
{
    refresh();

    connect(mMapDocument, &MapDocument::tileImageSourceChanged,
            this, &BrokenLinksModel::tileImageSourceChanged);
    connect(mMapDocument, &MapDocument::tilesetChanged,
            this, &BrokenLinksModel::tilesetChanged);
    connect(mMapDocument, &MapDocument::tilesetReplaced,
            this, &BrokenLinksModel::tilesetReplaced);
}

void BrokenLinksModel::refresh()
{
    bool brokenLinksBefore = hasBrokenLinks();

    beginResetModel();

    mBrokenLinks.clear();

    for (const SharedTileset &tileset : mMapDocument->map()->tilesets()) {
        if (!tileset->imageSource().isEmpty() && !tileset->imageLoaded()) {
            BrokenLink link;
            link.type = TilesetImageSource;
            link.tileset = tileset.data();
            mBrokenLinks.append(link);
        }

        if (!tileset->fileName().isEmpty() && !tileset->loaded()) {
            BrokenLink link;
            link.type = TilesetFileName;
            link.tileset = tileset.data();
            mBrokenLinks.append(link);
        }

        for (Tile *tile : tileset->tiles()) {
            if (!tile->imageSource().isEmpty() && !tile->imageLoaded()) {
                BrokenLink link;
                link.type = TileImageSource;
                link.tile = tile;
                mBrokenLinks.append(link);
            }
        }
    }

    endResetModel();

    bool brokenLinksAfter = hasBrokenLinks();
    if (brokenLinksBefore != brokenLinksAfter)
        emit hasBrokenLinksChanged(brokenLinksAfter);
}

int BrokenLinksModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : mBrokenLinks.count();
}

int BrokenLinksModel::columnCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : 3; // file name | path | type
}

QVariant BrokenLinksModel::data(const QModelIndex &index, int role) const
{
    const BrokenLink &link = mBrokenLinks.at(index.row());

    switch (role) {
    case Qt::DisplayRole:
        switch (index.column()) {
        case 0:
            return QFileInfo(link.filePath()).fileName();
        case 1:
            return QFileInfo(link.filePath()).path();
        case 2:
            switch (link.type) {
            case TilesetImageSource:
                return tr("Tileset image");
            case TilesetFileName:
                return tr("Tileset");
            case TileImageSource:
                return tr("Tile image");
            }
            break;
        }

    case Qt::DecorationRole:
        switch (index.column()) {
        case 0:
            // todo: status icon
            break;
        }
        break;
    }

    return QVariant();
}

QVariant BrokenLinksModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole && orientation == Qt::Horizontal) {
        switch (section) {
        case 0: return tr("File name");
        case 1: return tr("Location");
        case 2: return tr("Type");
        }
    }
    return QVariant();
}

void BrokenLinksModel::tileImageSourceChanged(Tile *tile)
{
    auto matchesTile = [tile](const BrokenLink &link) {
        return link.type == TileImageSource && link.tile == tile;
    };

    QVector<BrokenLink>::iterator it = std::find_if(mBrokenLinks.begin(),
                                                    mBrokenLinks.end(),
                                                    matchesTile);

    if (!tile->imageSource().isEmpty() && !tile->imageLoaded()) {
        if (it != mBrokenLinks.end()) {
            int linkIndex = it - mBrokenLinks.begin();
            emit dataChanged(index(linkIndex, 0), index(linkIndex, 1));
        } else {
            refresh(); // lazy way of adding an entry for this tile
        }
    } else if (it != mBrokenLinks.end()) {
        removeLink(it - mBrokenLinks.begin());
    }
}

void BrokenLinksModel::tilesetChanged(Tileset *tileset)
{
    auto matchesTileset = [tileset](const BrokenLink &link) {
        return link.type == TilesetImageSource && link.tileset == tileset;
    };

    QVector<BrokenLink>::iterator it = std::find_if(mBrokenLinks.begin(),
                                                    mBrokenLinks.end(),
                                                    matchesTileset);

    if (!tileset->imageSource().isEmpty() && !tileset->imageLoaded()) {
        if (it != mBrokenLinks.end()) {
            int linkIndex = it - mBrokenLinks.begin();
            emit dataChanged(index(linkIndex, 0), index(linkIndex, 1));
        } else {
            refresh(); // lazy way of adding an entry for this tileset
        }
    } else if (it != mBrokenLinks.end()) {
        removeLink(it - mBrokenLinks.begin());
    }
}

void BrokenLinksModel::tilesetReplaced(int index, Tileset *tileset)
{
    Q_UNUSED(index)
    Q_UNUSED(tileset)

    refresh();
}

void BrokenLinksModel::removeLink(int index)
{
    beginRemoveRows(QModelIndex(), index, index);
    mBrokenLinks.remove(index);
    endRemoveRows();
}


BrokenLinksWidget::BrokenLinksWidget(BrokenLinksModel *brokenLinksModel, QWidget *parent)
    : QWidget(parent)
    , mBrokenLinksModel(brokenLinksModel)
    , mTitleLabel(new QLabel(this))
    , mDescriptionLabel(new QLabel(this))
    , mView(new QTreeView(this))
    , mButtons(new QDialogButtonBox(QDialogButtonBox::Ignore,
                                    Qt::Horizontal,
                                    this))
{
    mTitleLabel->setText(tr("Some files could not be found"));
    mDescriptionLabel->setText(tr("One or more files referenced by the map could not be found. You can help locate them below."));
    mDescriptionLabel->setWordWrap(true);

    mLocateButton = mButtons->addButton(tr("Locate File..."), QDialogButtonBox::ActionRole);
    mLocateButton->setEnabled(false);

    QFont font = mTitleLabel->font();
    font.setBold(true);
    mTitleLabel->setFont(font);

    mProxyModel = new QSortFilterProxyModel(this);
    mProxyModel->setSortLocaleAware(true);
    mProxyModel->setSortCaseSensitivity(Qt::CaseInsensitive);
    mProxyModel->setSourceModel(mBrokenLinksModel);

    mView->setModel(mProxyModel);
    mView->setRootIsDecorated(false);
    mView->setItemsExpandable(false);
    mView->setUniformRowHeights(true);
    mView->setSortingEnabled(true);
    mView->sortByColumn(0, Qt::AscendingOrder);

    mView->header()->setStretchLastSection(false);
    mView->header()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    mView->header()->setSectionResizeMode(1, QHeaderView::Stretch);
    mView->header()->setSectionResizeMode(2, QHeaderView::ResizeToContents);

    QBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(mTitleLabel);
    layout->addWidget(mDescriptionLabel);
    layout->addWidget(mView);
    layout->addWidget(mButtons);
    setLayout(layout);

    connect(mButtons, &QDialogButtonBox::clicked, this, &BrokenLinksWidget::clicked);
    connect(mView->selectionModel(), &QItemSelectionModel::selectionChanged,
            this, &BrokenLinksWidget::selectionChanged);
}

static QModelIndexList mapToSource(const QSortFilterProxyModel *proxyModel,
                                   const QModelIndexList &proxyIndexes)
{
    QModelIndexList mapped;

    for (const QModelIndex &proxyIndex : proxyIndexes)
        mapped.append(proxyModel->mapToSource(proxyIndex));

    return mapped;
}

void BrokenLinksWidget::clicked(QAbstractButton *button)
{
    if (button == mButtons->button(QDialogButtonBox::Ignore)) {
        emit ignore();
    } else if (button == mLocateButton) {
        const auto proxySelection = mView->selectionModel()->selectedRows();
        const auto selection = mapToSource(mProxyModel, proxySelection);

        if (selection.isEmpty())
            return;

        const BrokenLink &link = mBrokenLinksModel->brokenLink(selection.first());
        tryFixLink(link);

        // todo: support multi-selection and do something smart
    }
}

void BrokenLinksWidget::selectionChanged(const QItemSelection &selected)
{
    mLocateButton->setEnabled(!selected.isEmpty());
}

void BrokenLinksWidget::tryFixLink(const BrokenLink &link)
{
    MapDocument *mapDocument = mBrokenLinksModel->mapDocument();
    Preferences *prefs = Preferences::instance();

    if (link.type == TilesetImageSource || link.type == TileImageSource) {
        QString startLocation = QFileInfo(prefs->lastPath(Preferences::ImageFile)).absolutePath();
        startLocation += QLatin1Char('/');
        startLocation += QFileInfo(link.filePath()).fileName();

        QString newFileName = QFileDialog::getOpenFileName(window(),
                                                           tr("Locate File"),
                                                           startLocation,
                                                           Utils::readableImageFormatsFilter());

        if (newFileName.isEmpty())
            return;

        QImageReader reader(newFileName);
        QImage image = reader.read();

        if (image.isNull()) {
            QMessageBox::critical(this, tr("Error Loading Image"), reader.errorString());
            return;
        }

        if (link.type == TilesetImageSource) {
            TilesetParameters parameters(*link.tileset);
            parameters.imageSource = newFileName;

            auto command = new ChangeTilesetParameters(mapDocument,
                                                       *link.tileset,
                                                       parameters);

            mapDocument->undoStack()->push(command);
        } else {
            auto command = new ChangeTileImageSource(mapDocument,
                                                     link.tile,
                                                     newFileName);

            mapDocument->undoStack()->push(command);
        }

        prefs->setLastPath(Preferences::ImageFile, newFileName);

    } else if (link.type == TilesetFileName) {
        const QString allFilesFilter = tr("All Files (*)");
        const QString tsxFilter = TsxTilesetFormat().nameFilter();

        QString selectedFilter = allFilesFilter;

        QString filter = allFilesFilter;
        filter += QLatin1String(";;");
        filter += tsxFilter;

        FormatHelper<TilesetFormat> helper(FileFormat::Read, filter);

        QString start = prefs->lastPath(Preferences::ExternalTileset);
        const QString fileName =
                QFileDialog::getOpenFileName(this, tr("Locate External Tileset"),
                                             start,
                                             helper.filter(),
                                             &selectedFilter);

        if (fileName.isEmpty())
            return;

        QString error;
        SharedTileset newTileset = Tiled::readTileset(fileName, &error);
        if (!newTileset) {
            QMessageBox::critical(window(), tr("Error Reading Tileset"), error);
            return;
        }

        int index = mapDocument->map()->tilesets().indexOf(link.tileset->sharedPointer());
        if (index != -1)
            mapDocument->undoStack()->push(new ReplaceTileset(mapDocument, index, newTileset));

        prefs->setLastPath(Preferences::ExternalTileset,
                           QFileInfo(fileName).path());
    }
}

} // namespace Internal
} // namespace Tiled
