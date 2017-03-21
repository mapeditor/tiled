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
#include "documentmanager.h"
#include "map.h"
#include "mapdocument.h"
#include "preferences.h"
#include "replacetileset.h"
#include "tile.h"
#include "tilesetchanges.h"
#include "tilesetdocument.h"
#include "tilesetmanager.h"
#include "tmxmapformat.h"
#include "utils.h"

#include <QDialogButtonBox>
#include <QFileDialog>
#include <QFileInfo>
#include <QHeaderView>
#include <QImageReader>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
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
        return _tileset->imageSource();
    case MapTilesetReference:
        return _tileset->fileName();
    case TilesetTileImageSource:
        return _tile->imageSource();
    }

    return QString();
}

Tileset *BrokenLink::tileset() const
{
    switch (type) {
    case TilesetImageSource:
    case MapTilesetReference:
        return _tileset;
    case TilesetTileImageSource:
        return _tile->tileset();
    }

    return nullptr;
}


BrokenLinksModel::BrokenLinksModel(QObject *parent)
    : QAbstractListModel(parent)
    , mDocument(nullptr)
{
}

void BrokenLinksModel::setDocument(Document *document)
{
    if (auto mapDocument = qobject_cast<MapDocument *>(mDocument)) {
        mapDocument->disconnect(this);

        for (const SharedTileset &tileset : mapDocument->map()->tilesets())
            disconnectFromTileset(tileset);

    } else if (auto tilesetDocument = qobject_cast<TilesetDocument *>(mDocument)) {
        disconnectFromTileset(tilesetDocument->tileset());
    }

    mDocument = document;
    refresh();

    if (mDocument) {
        if (auto mapDocument = qobject_cast<MapDocument *>(mDocument)) {
            connect(mapDocument,
                    &MapDocument::tilesetReplaced,
                    this,
                    &BrokenLinksModel::tilesetReplaced);
            connect(mapDocument, &MapDocument::tilesetAdded, this, &BrokenLinksModel::tilesetAdded);
            connect(
                mapDocument, &MapDocument::tilesetRemoved, this, &BrokenLinksModel::tilesetRemoved);

            for (const SharedTileset &tileset : mapDocument->map()->tilesets())
                connectToTileset(tileset);

        } else if (auto tilesetDocument = qobject_cast<TilesetDocument *>(mDocument)) {
            connectToTileset(tilesetDocument->tileset());
        }

        connect(mDocument, &Document::ignoreBrokenLinksChanged, this, &BrokenLinksModel::refresh);
    }
}

void BrokenLinksModel::refresh()
{
    bool brokenLinksBefore = hasBrokenLinks();

    beginResetModel();

    mBrokenLinks.clear();

    if (mDocument && !mDocument->ignoreBrokenLinks()) {
        auto processTileset = [this](const SharedTileset &tileset) {
            if (tileset->isCollection()) {
                for (Tile *tile : tileset->tiles()) {
                    if (!tile->imageSource().isEmpty() && !tile->imageLoaded()) {
                        BrokenLink link;
                        link.type = TilesetTileImageSource;
                        link._tile = tile;
                        mBrokenLinks.append(link);
                    }
                }
            } else {
                if (!tileset->imageLoaded()) {
                    BrokenLink link;
                    link.type = TilesetImageSource;
                    link._tileset = tileset.data();
                    mBrokenLinks.append(link);
                }
            }
        };

        if (auto mapDocument = qobject_cast<MapDocument *>(mDocument)) {
            for (const SharedTileset &tileset : mapDocument->map()->tilesets()) {
                if (!tileset->fileName().isEmpty() && !tileset->loaded()) {
                    BrokenLink link;
                    link.type = MapTilesetReference;
                    link._tileset = tileset.data();
                    mBrokenLinks.append(link);
                } else {
                    processTileset(tileset);
                }
            }
        } else if (auto tilesetDocument = qobject_cast<TilesetDocument *>(mDocument)) {
            processTileset(tilesetDocument->tileset());
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
            case MapTilesetReference:
                return tr("Tileset");
            case TilesetImageSource:
                return tr("Tileset image");
            case TilesetTileImageSource:
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
        case 0:
            return tr("File name");
        case 1:
            return tr("Location");
        case 2:
            return tr("Type");
        }
    }
    return QVariant();
}

void BrokenLinksModel::tileImageSourceChanged(Tile *tile)
{
    auto matchesTile = [tile](const BrokenLink &link) {
        return link.type == TilesetTileImageSource && link._tile == tile;
    };

    QVector<BrokenLink>::iterator it =
        std::find_if(mBrokenLinks.begin(), mBrokenLinks.end(), matchesTile);

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
    Q_UNUSED(tileset)

    // This may mean either the tileset properties changed or tiles were
    // added/removed from the tileset. Easiest to just refresh entirely.
    refresh();
}

void BrokenLinksModel::tilesetAdded(int index, Tileset *tileset)
{
    Q_UNUSED(index)
    connectToTileset(tileset->sharedPointer());
}

void BrokenLinksModel::tilesetRemoved(Tileset *tileset)
{
    disconnectFromTileset(tileset->sharedPointer());
}

void BrokenLinksModel::tilesetReplaced(int index, Tileset *newTileset, Tileset *oldTileset)
{
    Q_UNUSED(index)

    disconnectFromTileset(oldTileset->sharedPointer());
    connectToTileset(newTileset->sharedPointer());

    refresh();
}

void BrokenLinksModel::connectToTileset(const SharedTileset &tileset)
{
    auto tilesetDocument = DocumentManager::instance()->findTilesetDocument(tileset);
    if (tilesetDocument) {
        connect(tilesetDocument,
                &TilesetDocument::tileImageSourceChanged,
                this,
                &BrokenLinksModel::tileImageSourceChanged);
        connect(tilesetDocument,
                &TilesetDocument::tilesetChanged,
                this,
                &BrokenLinksModel::tilesetChanged);
    }
}

void BrokenLinksModel::disconnectFromTileset(const SharedTileset &tileset)
{
    auto tilesetDocument =
        DocumentManager::instance()->findTilesetDocument(tileset->sharedPointer());
    if (tilesetDocument)
        tilesetDocument->disconnect(this);
}

void BrokenLinksModel::removeLink(int index)
{
    beginRemoveRows(QModelIndex(), index, index);
    mBrokenLinks.remove(index);
    endRemoveRows();

    if (!hasBrokenLinks())
        emit hasBrokenLinksChanged(false);
}


BrokenLinksWidget::BrokenLinksWidget(BrokenLinksModel *brokenLinksModel, QWidget *parent)
    : QWidget(parent)
    , mBrokenLinksModel(brokenLinksModel)
    , mTitleLabel(new QLabel(this))
    , mDescriptionLabel(new QLabel(this))
    , mView(new QTreeView(this))
    , mButtons(new QDialogButtonBox(QDialogButtonBox::Ignore, Qt::Horizontal, this))
{
    mTitleLabel->setText(tr("Some files could not be found"));
    mDescriptionLabel->setText(
        tr("One or more referenced files could not be found. You can help locate them below."));
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

    connect(mView->selectionModel(),
            &QItemSelectionModel::selectionChanged,
            this,
            &BrokenLinksWidget::selectionChanged);

    connect(mView, &QTreeView::doubleClicked, this, [this](const QModelIndex &proxyIndex) {
        const auto index = mProxyModel->mapToSource(proxyIndex);
        const BrokenLink &link = mBrokenLinksModel->brokenLink(index.row());
        tryFixLink(link);
    });

    // For some reason a model reset doesn't trigger the selectionChanged signal,
    // so we need to handle that explicitly.
    connect(brokenLinksModel, &BrokenLinksModel::modelReset, this, [this]() {
        selectionChanged(mView->selectionModel()->selection());
    });
}

void BrokenLinksWidget::clicked(QAbstractButton *button)
{
    if (button == mButtons->button(QDialogButtonBox::Ignore)) {
        mBrokenLinksModel->document()->setIgnoreBrokenLinks(true);
    } else if (button == mLocateButton) {
        const auto proxySelection = mView->selectionModel()->selectedRows();
        if (proxySelection.isEmpty())
            return;

        const auto firstIndex = mProxyModel->mapToSource(proxySelection.first());
        const BrokenLink &link = mBrokenLinksModel->brokenLink(firstIndex.row());

        tryFixLink(link);

        // todo: support multi-selection and do something smart
    }
}

void BrokenLinksWidget::selectionChanged(const QItemSelection &selected)
{
    mLocateButton->setEnabled(!selected.isEmpty());

    bool isTileset = qobject_cast<TilesetDocument *>(mBrokenLinksModel->document()) != nullptr;

    if (!selected.isEmpty()) {
        const auto firstIndex = selected.first().topLeft();
        const BrokenLink &link = mBrokenLinksModel->brokenLink(firstIndex.row());

        switch (link.type) {
        case Tiled::Internal::MapTilesetReference:
            mLocateButton->setText(tr("Locate File..."));
            break;
        case Tiled::Internal::TilesetTileImageSource:
        case Tiled::Internal::TilesetImageSource:
            if (isTileset)
                mLocateButton->setText(tr("Locate File..."));
            else
                mLocateButton->setText(tr("Open Tileset..."));
            break;
        }
    }
}

void BrokenLinksWidget::tryFixLink(const BrokenLink &link)
{
    Document *document = mBrokenLinksModel->document();
    Preferences *prefs = Preferences::instance();

    if (link.type == TilesetImageSource || link.type == TilesetTileImageSource) {
        auto tilesetDocument = qobject_cast<TilesetDocument *>(document);
        if (!tilesetDocument) {
            // We need to open the tileset document in order to be able to make changes to it...
            const SharedTileset tileset = link.tileset()->sharedPointer();
            DocumentManager::instance()->openTileset(tileset);
            return;
        }

        QString startLocation = QFileInfo(prefs->lastPath(Preferences::ImageFile)).absolutePath();
        startLocation += QLatin1Char('/');
        startLocation += QFileInfo(link.filePath()).fileName();

        QString newFileName = QFileDialog::getOpenFileName(
            window(), tr("Locate File"), startLocation, Utils::readableImageFormatsFilter());

        if (newFileName.isEmpty())
            return;

        QImageReader reader(newFileName);
        QImage image = reader.read();

        if (image.isNull()) {
            QMessageBox::critical(this, tr("Error Loading Image"), reader.errorString());
            return;
        }

        if (link.type == TilesetImageSource) {
            TilesetParameters parameters(*link._tileset);
            parameters.imageSource = newFileName;

            auto command = new ChangeTilesetParameters(tilesetDocument, parameters);

            tilesetDocument->undoStack()->push(command);
        } else {
            auto command = new ChangeTileImageSource(tilesetDocument, link._tile, newFileName);

            tilesetDocument->undoStack()->push(command);
        }

        prefs->setLastPath(Preferences::ImageFile, newFileName);

    } else if (link.type == MapTilesetReference) {
        const QString allFilesFilter = tr("All Files (*)");

        QString selectedFilter = allFilesFilter;
        QString filter = allFilesFilter;
        FormatHelper<TilesetFormat> helper(FileFormat::Read, filter);

        QString start = prefs->lastPath(Preferences::ExternalTileset);
        const QString fileName = QFileDialog::getOpenFileName(
            this, tr("Locate External Tileset"), start, helper.filter(), &selectedFilter);

        if (fileName.isEmpty())
            return;

        QString error;

        // It could be, that we have already loaded this tileset.
        SharedTileset newTileset = TilesetManager::instance()->findTileset(fileName);
        if (!newTileset) {
            newTileset = Tiled::readTileset(fileName, &error);

            if (!newTileset) {
                QMessageBox::critical(window(), tr("Error Reading Tileset"), error);
                return;
            }
        }

        MapDocument *mapDocument = static_cast<MapDocument *>(document);
        int index = mapDocument->map()->tilesets().indexOf(link._tileset->sharedPointer());
        if (index != -1)
            document->undoStack()->push(new ReplaceTileset(mapDocument, index, newTileset));

        prefs->setLastPath(Preferences::ExternalTileset, QFileInfo(fileName).path());
    }
}

} // namespace Internal
} // namespace Tiled
