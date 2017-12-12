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
#include "objectgroup.h"
#include "preferences.h"
#include "replacetemplate.h"
#include "replacetileset.h"
#include "templatesdock.h"
#include "templatemanager.h"
#include "tmxmapformat.h"
#include "tile.h"
#include "tilesetchanges.h"
#include "tilesetdocument.h"
#include "tilesetmanager.h"
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
        return _tileset->imageSource().toString(QUrl::PreferLocalFile);
    case MapTilesetReference:
        return _tileset->fileName();
    case TilesetTileImageSource:
        return _tile->imageSource().toString(QUrl::PreferLocalFile);
    case ObjectTemplateReference:
        return _objectTemplate->fileName();
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
    case ObjectTemplateReference:
        return nullptr;
    }

    return nullptr;
}

const ObjectTemplate *BrokenLink::objectTemplate() const
{
    return type == ObjectTemplateReference ? _objectTemplate : nullptr;
}

BrokenLinksModel::BrokenLinksModel(QObject *parent)
    : QAbstractListModel(parent)
    , mDocument(nullptr)
{
}

void BrokenLinksModel::setDocument(Document *document)
{
    if (auto mapDocument = qobject_cast<MapDocument*>(mDocument)) {
        mapDocument->disconnect(this);

        for (const SharedTileset &tileset : mapDocument->map()->tilesets())
            disconnectFromTileset(tileset);

    } else if (auto tilesetDocument = qobject_cast<TilesetDocument*>(mDocument)) {
        disconnectFromTileset(tilesetDocument->tileset());
    }

    mDocument = document;
    refresh();

    if (mDocument) {
        if (auto mapDocument = qobject_cast<MapDocument*>(mDocument)) {
            connect(mapDocument, &MapDocument::tilesetReplaced,
                    this, &BrokenLinksModel::tilesetReplaced);
            connect(mapDocument, &MapDocument::tilesetAdded,
                    this, &BrokenLinksModel::tilesetAdded);
            connect(mapDocument, &MapDocument::tilesetRemoved,
                    this, &BrokenLinksModel::tilesetRemoved);

            connect(mapDocument, &MapDocument::objectTemplateReplaced,
                    this, &BrokenLinksModel::objectTemplateReplaced);

            for (const SharedTileset &tileset : mapDocument->map()->tilesets())
                connectToTileset(tileset);

        } else if (auto tilesetDocument = qobject_cast<TilesetDocument*>(mDocument)) {
            connectToTileset(tilesetDocument->tileset());
        }

        connect(mDocument, &Document::ignoreBrokenLinksChanged,
                this, &BrokenLinksModel::refresh);
    }
}

void BrokenLinksModel::refresh()
{
    bool brokenLinksBefore = hasBrokenLinks();

    beginResetModel();

    mBrokenLinks.clear();

    if (mDocument && !mDocument->ignoreBrokenLinks()) {
        QSet<SharedTileset> processedTilesets;

        auto processTileset = [this,&processedTilesets](const SharedTileset &tileset) {
            if (processedTilesets.contains(tileset))
                return;

            processedTilesets.insert(tileset);

            if (tileset->isCollection()) {
                for (Tile *tile : tileset->tiles()) {
                    if (!tile->imageSource().isEmpty() && tile->imageStatus() == LoadingError) {
                        BrokenLink link;
                        link.type = TilesetTileImageSource;
                        link._tile = tile;
                        mBrokenLinks.append(link);
                    }
                }
            } else {
                if (tileset->imageStatus() == LoadingError) {
                    BrokenLink link;
                    link.type = TilesetImageSource;
                    link._tileset = tileset.data();
                    mBrokenLinks.append(link);
                }
            }
        };

        if (auto mapDocument = qobject_cast<MapDocument*>(mDocument)) {
            for (const SharedTileset &tileset : mapDocument->map()->tilesets()) {
                if (!tileset->fileName().isEmpty() && tileset->status() == LoadingError) {
                    BrokenLink link;
                    link.type = MapTilesetReference;
                    link._tileset = tileset.data();
                    mBrokenLinks.append(link);
                } else {
                    processTileset(tileset);
                }
            }

            QSet<const ObjectTemplate*> brokenTemplates;

            LayerIterator it(mapDocument->map());
            while (Layer *layer = it.next()) {
                if (ObjectGroup *objectGroup = layer->asObjectGroup()) {
                    for (MapObject *mapObject : *objectGroup) {
                        if (const ObjectTemplate *objectTemplate = mapObject->objectTemplate()) {
                            if (const MapObject *mapObject = objectTemplate->object()) {
                                if (Tileset *tileset = mapObject->cell().tileset())
                                    processTileset(tileset->sharedPointer());
                            } else {
                                brokenTemplates.insert(objectTemplate);
                            }
                        }
                    }
                }
            }

            for (const ObjectTemplate *objectTemplate : brokenTemplates) {
                BrokenLink link;
                link.type = ObjectTemplateReference;
                link._objectTemplate = objectTemplate;
                mBrokenLinks.append(link);
            }
        } else if (auto tilesetDocument = qobject_cast<TilesetDocument*>(mDocument)) {
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
            case ObjectTemplateReference:
                return tr("Template");
            }
            break;
        }
        break;

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
        return link.type == TilesetTileImageSource && link._tile == tile;
    };

    QVector<BrokenLink>::iterator it = std::find_if(mBrokenLinks.begin(),
                                                    mBrokenLinks.end(),
                                                    matchesTile);

    if (!tile->imageSource().isEmpty() && tile->imageStatus() == LoadingError) {
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

void BrokenLinksModel::objectTemplateReplaced()
{
    refresh();
}

void BrokenLinksModel::connectToTileset(const SharedTileset &tileset)
{
    auto tilesetDocument = DocumentManager::instance()->findTilesetDocument(tileset);
    if (tilesetDocument) {
        connect(tilesetDocument, &TilesetDocument::tileImageSourceChanged,
                this, &BrokenLinksModel::tileImageSourceChanged);
        connect(tilesetDocument, &TilesetDocument::tilesetChanged,
                this, &BrokenLinksModel::tilesetChanged);
    }
}

void BrokenLinksModel::disconnectFromTileset(const SharedTileset &tileset)
{
    auto tilesetDocument = DocumentManager::instance()->findTilesetDocument(tileset->sharedPointer());
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
    , mButtons(new QDialogButtonBox(QDialogButtonBox::Ignore,
                                    Qt::Horizontal,
                                    this))
{
    mTitleLabel->setText(tr("Some files could not be found"));
    mDescriptionLabel->setText(tr("One or more referenced files could not be found. You can help locate them below."));
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
    mView->setSelectionMode(QAbstractItemView::ExtendedSelection);

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

    connect(mView, &QTreeView::doubleClicked, this, [this](const QModelIndex &proxyIndex) {
        const auto index = mProxyModel->mapToSource(proxyIndex);
        const BrokenLink &link = mBrokenLinksModel->brokenLink(index.row());
        tryFixLink(link);
    });

    // For some reason a model reset doesn't trigger the selectionChanged signal,
    // so we need to handle that explicitly.
    connect(brokenLinksModel, &BrokenLinksModel::modelReset, this, [this](){
        selectionChanged();
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

        QVector<BrokenLink> links;
        links.reserve(proxySelection.size());

        for (const QModelIndex &proxyIndex : proxySelection) {
            const auto index = mProxyModel->mapToSource(proxyIndex);
            links.append(mBrokenLinksModel->brokenLink(index.row()));
        }

        tryFixLinks(links);
    }
}

void BrokenLinksWidget::selectionChanged()
{
    const auto selection = mView->selectionModel()->selectedRows();

    mLocateButton->setEnabled(!selection.isEmpty());

    bool isTileset = qobject_cast<TilesetDocument*>(mBrokenLinksModel->document()) != nullptr;

    if (!selection.isEmpty()) {
        const auto firstIndex = selection.first();
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
        case Tiled::Internal::ObjectTemplateReference:
            mLocateButton->setText(tr("Locate File..."));
        }
    }
}

void BrokenLinksWidget::tryFixLinks(const QVector<BrokenLink> &links)
{
    if (links.isEmpty())
        return;

    if (links.size() == 1)
        return tryFixLink(links.first());

    // If any of the links need to be fixed in a tileset, open the first such tileset and abort
    Document *document = mBrokenLinksModel->document();
    bool editingTileset = qobject_cast<TilesetDocument*>(document) != nullptr;
    for (const BrokenLink &link : links) {
        if (link.type == TilesetImageSource || link.type == TilesetTileImageSource) {
            if (!editingTileset) {
                // We need to open the tileset document in order to be able to make changes to it...
                const SharedTileset tileset = link.tileset()->sharedPointer();
                DocumentManager::instance()->openTileset(tileset);
                return;
            }
        }
    }

    // todo: fix caption after string freeze (and the text on the button)
    static QString startingLocation = QFileInfo(links.first().filePath()).path();
    const QString directory = QFileDialog::getExistingDirectory(window(),
                                                                tr("Locate File"),
                                                                startingLocation);

    if (directory.isEmpty())
        return;

    startingLocation = directory;

    const QDir dir(directory);
    const auto files = dir.entryList(QDir::Files |
                                     QDir::Readable |
                                     QDir::NoDotAndDotDot).toSet();

    // See if any of the links we're looking for is located in this directory
    for (const BrokenLink &link : links) {
        const QString fileName = QFileInfo(link.filePath()).fileName();
        if (files.contains(fileName))
            if (!tryFixLink(link, dir.filePath(fileName)))
                break;
    }

    // todo: provide better feedback (like maybe a dialog showing any errors
    // or the number of links fixed)
}

void BrokenLinksWidget::tryFixLink(const BrokenLink &link)
{
    Document *document = mBrokenLinksModel->document();
    Preferences *prefs = Preferences::instance();

    if (link.type == TilesetImageSource || link.type == TilesetTileImageSource) {
        auto tilesetDocument = qobject_cast<TilesetDocument*>(document);
        if (!tilesetDocument) {
            // We need to open the tileset document in order to be able to make changes to it...
            const SharedTileset tileset = link.tileset()->sharedPointer();
            DocumentManager::instance()->openTileset(tileset);
            return;
        }

        QString startLocation = QFileInfo(prefs->lastPath(Preferences::ImageFile)).absolutePath();
        startLocation += QLatin1Char('/');
        startLocation += QFileInfo(link.filePath()).fileName();

        QUrl newFileUrl = QFileDialog::getOpenFileUrl(window(),
                                                      tr("Locate File"),
                                                      QUrl::fromLocalFile(startLocation),
                                                      Utils::readableImageFormatsFilter());

        if (newFileUrl.isEmpty())
            return;

        // For local images, check if they can be loaded
        if (newFileUrl.isLocalFile()) {
            QString localFile = newFileUrl.toLocalFile();
            prefs->setLastPath(Preferences::ImageFile, localFile);
            tryFixLink(link, localFile);
            return;
        }

        if (link.type == TilesetImageSource) {
            TilesetParameters parameters(*link._tileset);
            parameters.imageSource = newFileUrl;

            auto command = new ChangeTilesetParameters(tilesetDocument,
                                                       parameters);

            tilesetDocument->undoStack()->push(command);
        } else {
            auto command = new ChangeTileImageSource(tilesetDocument,
                                                     link._tile,
                                                     newFileUrl);

            tilesetDocument->undoStack()->push(command);
        }

    } else if (link.type == MapTilesetReference) {
        FormatHelper<TilesetFormat> helper(FileFormat::Read, tr("All Files (*)"));

        QString start = prefs->lastPath(Preferences::ExternalTileset);
        QString fileName = QFileDialog::getOpenFileName(this, tr("Locate External Tileset"),
                                                        start,
                                                        helper.filter());

        if (!fileName.isEmpty()) {
            prefs->setLastPath(Preferences::ExternalTileset, QFileInfo(fileName).path());
            tryFixLink(link, fileName);
        }

    } else if (link.type == ObjectTemplateReference) {
        FormatHelper<ObjectTemplateFormat> helper(FileFormat::Read, tr("All Files (*)"));

        QString start = prefs->lastPath(Preferences::ObjectTemplateFile);
        QString fileName = QFileDialog::getOpenFileName(this, tr("Locate Object Template"),
                                                        start,
                                                        helper.filter());

        if (!fileName.isEmpty()) {
            prefs->setLastPath(Preferences::ObjectTemplateFile, QFileInfo(fileName).path());
            tryFixLink(link, fileName);
        }
    }
}

bool BrokenLinksWidget::tryFixLink(const BrokenLink &link, const QString &newFilePath)
{
    Q_ASSERT(!newFilePath.isEmpty());

    Document *document = mBrokenLinksModel->document();

    if (link.type == TilesetImageSource || link.type == TilesetTileImageSource) {
        auto tilesetDocument = qobject_cast<TilesetDocument*>(document);
        Q_ASSERT(tilesetDocument);

        QImageReader reader(newFilePath);
        QImage image = reader.read();

        if (image.isNull()) {
            QMessageBox::critical(this, tr("Error Loading Image"), reader.errorString());
            return false;
        }

        const QUrl newSource(QUrl::fromLocalFile(newFilePath));

        if (link.type == TilesetImageSource) {
            TilesetParameters parameters(*link._tileset);
            parameters.imageSource = newSource;

            auto command = new ChangeTilesetParameters(tilesetDocument,
                                                       parameters);

            tilesetDocument->undoStack()->push(command);
        } else {
            auto command = new ChangeTileImageSource(tilesetDocument,
                                                     link._tile,
                                                     newSource);

            tilesetDocument->undoStack()->push(command);
        }

    } else if (link.type == MapTilesetReference) {
        // It could be, that we have already loaded this tileset.
        SharedTileset newTileset = TilesetManager::instance()->findTileset(newFilePath);
        if (!newTileset || newTileset->status() == LoadingError) {
            QString error;
            newTileset = Tiled::readTileset(newFilePath, &error);

            if (!newTileset) {
                QMessageBox::critical(window(), tr("Error Reading Tileset"), error);
                return false;
            }
        }

        MapDocument *mapDocument = static_cast<MapDocument*>(document);
        int index = mapDocument->map()->tilesets().indexOf(link._tileset->sharedPointer());
        if (index != -1)
            document->undoStack()->push(new ReplaceTileset(mapDocument, index, newTileset));

    } else if (link.type == ObjectTemplateReference) {
        ObjectTemplate *newObjectTemplate = TemplateManager::instance()->findObjectTemplate(newFilePath);

        if (!newObjectTemplate || !newObjectTemplate->object()) {
            QString error;
            newObjectTemplate = TemplateManager::instance()->loadObjectTemplate(newFilePath, &error);

            if (!newObjectTemplate->object()) {
                QMessageBox::critical(window(), tr("Error Reading Object Template"), error);
                return false;
            }
        }

        MapDocument *mapDocument = static_cast<MapDocument*>(document);
        document->undoStack()->push(new ReplaceTemplate(mapDocument,
                                                        link.objectTemplate(),
                                                        newObjectTemplate));
    }

    return true;
}

} // namespace Internal
} // namespace Tiled
