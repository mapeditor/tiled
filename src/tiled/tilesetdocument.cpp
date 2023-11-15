/*
 * tilesetdocument.cpp
 * Copyright 2015-2016, Thorbjørn Lindeijer <bjorn@lindeijer.nl>
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

#include "tilesetdocument.h"

#include "changeevents.h"
#include "editabletileset.h"
#include "issuesmodel.h"
#include "map.h"
#include "mapdocument.h"
#include "tile.h"
#include "tilesetformat.h"
#include "tilesetwangsetmodel.h"
#include "wangcolormodel.h"
#include "wangset.h"

#include <QCoreApplication>
#include <QFileInfo>
#include <QQmlEngine>
#include <QUndoStack>

namespace Tiled {

class ReloadTileset : public QUndoCommand
{
public:
    ReloadTileset(TilesetDocument *tilesetDocument, const SharedTileset &tileset)
        : mTilesetDocument(tilesetDocument)
        , mTileset(tileset)
    {
        setText(QCoreApplication::translate("Undo Commands", "Reload Tileset"));
    }

    void undo() override { mTilesetDocument->swapTileset(mTileset); }
    void redo() override { mTilesetDocument->swapTileset(mTileset); }

private:
    TilesetDocument *mTilesetDocument;
    SharedTileset mTileset;
};


QMap<SharedTileset, TilesetDocument*> TilesetDocument::sTilesetToDocument;

TilesetDocument::TilesetDocument(const SharedTileset &tileset)
    : Document(TilesetDocumentType, tileset->fileName())
    , mTileset(tileset)
    , mWangSetModel(new TilesetWangSetModel(this, this))
{
    Q_ASSERT(!sTilesetToDocument.contains(tileset));
    sTilesetToDocument.insert(tileset, this);

    // If there already happens to be an editable for this tileset, take
    // ownership of it.
    if (auto editable = EditableTileset::find(tileset.data())) {
        setEditable(std::unique_ptr<EditableAsset>(editable));
        QQmlEngine::setObjectOwnership(editable, QQmlEngine::CppOwnership);
    }

    mCurrentObject = tileset.data();

    connect(this, &TilesetDocument::propertyAdded,
            this, &TilesetDocument::onPropertyAdded);
    connect(this, &TilesetDocument::propertyRemoved,
            this, &TilesetDocument::onPropertyRemoved);
    connect(this, &TilesetDocument::propertyChanged,
            this, &TilesetDocument::onPropertyChanged);
    connect(this, &TilesetDocument::propertiesChanged,
            this, &TilesetDocument::onPropertiesChanged);

    connect(mWangSetModel, &TilesetWangSetModel::wangSetRemoved,
            this, &TilesetDocument::onWangSetRemoved);
}

TilesetDocument::~TilesetDocument()
{
    // Clear any previously found issues in this document
    IssuesModel::instance().removeIssuesWithContext(this);

    sTilesetToDocument.remove(mTileset);

    // Needs to be deleted before the Tileset instance is deleted, because it
    // may cause script values to detach from the map, in which case they'll
    // need to be able to copy the data.
    mEditable.reset();
}

bool TilesetDocument::save(const QString &fileName, QString *error)
{
    auto tilesetFormat = findFileFormat<TilesetFormat>(mTileset->format(), FileFormat::Write);;
    if (!tilesetFormat) {
        if (error)
            *error = tr("Tileset format '%1' not found").arg(mTileset->format());
        return false;
    }

    if (!tilesetFormat->write(*tileset(), fileName)) {
        if (error)
            *error = tilesetFormat->errorString();
        return false;
    }

    undoStack()->setClean();

    if (mTileset->fileName() != fileName) {
        mTileset->setFileName(fileName);
        mTileset->exportFileName.clear();
    }

    setFileName(fileName);

    mLastSaved = QFileInfo(fileName).lastModified();

    emit saved();
    return true;
}

bool TilesetDocument::canReload() const
{
    return !fileName().isEmpty() && !mTileset->format().isEmpty();
}

bool TilesetDocument::reload(QString *error)
{
    if (!canReload())
        return false;

    auto format = findFileFormat<TilesetFormat>(mTileset->format(), FileFormat::Read);
    if (!format) {
        if (error)
            *error = tr("Tileset format '%s' not found").arg(mTileset->format());
        return false;
    }

    SharedTileset tileset = format->read(fileName());

    if (tileset.isNull()) {
        if (error)
            *error = format->errorString();
        return false;
    }

    tileset->setFileName(fileName());
    tileset->setFormat(format->shortName());

    undoStack()->push(new ReloadTileset(this, tileset));
    undoStack()->setClean();
    mLastSaved = QFileInfo(fileName()).lastModified();

    return true;
}

TilesetDocumentPtr TilesetDocument::load(const QString &fileName,
                                         TilesetFormat *format,
                                         QString *error)
{
    SharedTileset tileset = format->read(fileName);

    if (tileset.isNull()) {
        if (error)
            *error = format->errorString();
        return TilesetDocumentPtr();
    }

    tileset->setFileName(fileName);
    tileset->setFormat(format->shortName());

    return TilesetDocumentPtr::create(tileset);
}

TilesetFormat *TilesetDocument::writerFormat() const
{
    return findFileFormat<TilesetFormat>(mTileset->format(), FileFormat::Write);
}

void TilesetDocument::setWriterFormat(TilesetFormat *format)
{
    Q_ASSERT(format->hasCapabilities(FileFormat::Write));
    mTileset->setFormat(format->shortName());
}

QString TilesetDocument::lastExportFileName() const
{
    return tileset()->exportFileName;
}

void TilesetDocument::setLastExportFileName(const QString &fileName)
{
    tileset()->exportFileName = fileName;
}

TilesetFormat* TilesetDocument::exportFormat() const
{
    return findFileFormat<TilesetFormat>(tileset()->exportFormat);
}

void TilesetDocument::setExportFormat(FileFormat *format)
{
    Q_ASSERT(qobject_cast<TilesetFormat*>(format));
    tileset()->exportFormat = format->shortName();
}

QString TilesetDocument::displayName() const
{
    QString displayName;

    if (isEmbedded()) {
        displayName = mMapDocuments.first()->displayName();
        displayName += QLatin1Char('#');
        displayName += mTileset->name();
    } else {
        displayName = QFileInfo(fileName()).fileName();
        if (displayName.isEmpty())
            displayName = tr("untitled.tsx");
    }

    return displayName;
}

QString TilesetDocument::externalOrEmbeddedFileName() const
{
    QString result;

    if (isEmbedded()) {
        result = mMapDocuments.first()->fileName();
        result += QLatin1Char('#');
        result += mTileset->name();
    } else {
        result = fileName();
    }

    return result;
}

/**
 * Exchanges the tileset data of the tileset wrapped by this document with the
 * data in the given \a tileset, and vice-versa.
 */
void TilesetDocument::swapTileset(SharedTileset &tileset)
{
    // Bring pointers to safety
    setSelectedTiles(QList<Tile*>());
    setCurrentObject(mTileset.data());
    mEditable.reset();

    sTilesetToDocument.remove(mTileset);
    mTileset->swap(*tileset);
    sTilesetToDocument.insert(mTileset, this);

    emit tilesetChanged(mTileset.data());
}

std::unique_ptr<EditableAsset> TilesetDocument::createEditable()
{
    return std::make_unique<EditableTileset>(this, this);
}

/**
 * Used when a map that has this tileset embedded is saved.
 */
void TilesetDocument::setClean()
{
    undoStack()->setClean();
}

void TilesetDocument::addMapDocument(MapDocument *mapDocument)
{
    Q_ASSERT(!mMapDocuments.contains(mapDocument));
    mMapDocuments.append(mapDocument);
}

void TilesetDocument::removeMapDocument(MapDocument *mapDocument)
{
    Q_ASSERT(mMapDocuments.contains(mapDocument));
    mMapDocuments.removeOne(mapDocument);
}

void TilesetDocument::setTilesetName(const QString &name)
{
    mTileset->setName(name);
    emit tilesetNameChanged(mTileset.data());

    for (MapDocument *mapDocument : mapDocuments())
        emit mapDocument->tilesetNameChanged(mTileset.data());
}

void TilesetDocument::setTilesetTileOffset(QPoint tileOffset)
{
    mTileset->setTileOffset(tileOffset);

    // Invalidate the draw margins of the maps using this tileset
    for (MapDocument *mapDocument : mapDocuments())
        mapDocument->map()->invalidateDrawMargins();

    emit tilesetTileOffsetChanged(mTileset.data());

    for (MapDocument *mapDocument : mapDocuments())
        emit mapDocument->tilesetTilePositioningChanged(mTileset.data());
}

void TilesetDocument::setTilesetObjectAlignment(Alignment objectAlignment)
{
    mTileset->setObjectAlignment(objectAlignment);

    emit tilesetObjectAlignmentChanged(mTileset.data());

    for (MapDocument *mapDocument : mapDocuments())
        emit mapDocument->tilesetTilePositioningChanged(mTileset.data());
}

void TilesetDocument::setTilesetTransformationFlags(Tileset::TransformationFlags flags)
{
    tileset()->setTransformationFlags(flags);
    emit tilesetChanged(mTileset.data());
}

void TilesetDocument::addTiles(const QList<Tile *> &tiles)
{
    mTileset->addTiles(tiles);
    emit tilesAdded(tiles);
    emit tilesetChanged(mTileset.data());
}

void TilesetDocument::removeTiles(const QList<Tile *> &tiles)
{
    // Switch current object to the tileset when it is one of the removed tiles
    for (Tile *tile : tiles) {
        if (tile == currentObject()) {
            setCurrentObject(mTileset.data());
            break;
        }
    }

    emit changed(TilesEvent(ChangeEvent::TilesAboutToBeRemoved, tiles));
    mTileset->removeTiles(tiles);
    emit tilesRemoved(tiles);
    emit tilesetChanged(mTileset.data());
}

/**
 * \sa Tileset::relocateTiles
 */
QList<int> TilesetDocument::relocateTiles(const QList<Tile *> &tiles, int location)
{
    const auto prevLocations = mTileset->relocateTiles(tiles, location);
    emit tilesetChanged(mTileset.data());
    return prevLocations;
}

void TilesetDocument::setSelectedTiles(const QList<Tile*> &selectedTiles)
{
    mSelectedTiles = selectedTiles;
    emit selectedTilesChanged();
}

QList<Object *> TilesetDocument::currentObjects() const
{
    if (mCurrentObject->typeId() == Object::TileType && !mSelectedTiles.isEmpty()) {
        QList<Object*> objects;
        objects.reserve(mSelectedTiles.size());
        for (Tile *tile : mSelectedTiles)
            objects.append(tile);
        return objects;
    }

    return Document::currentObjects();
}

/**
 * Returns the WangColorModel instance for the given \a wangSet.
 * The model instances are created on-demand and owned by the document.
 */
WangColorModel *TilesetDocument::wangColorModel(WangSet *wangSet)
{
    Q_ASSERT(wangSet->tileset() == mTileset.data());

    std::unique_ptr<WangColorModel> &model = mWangColorModels[wangSet];
    if (!model)
        model = std::make_unique<WangColorModel>(this, wangSet);
    return model.get();
}

void TilesetDocument::setTileImage(Tile *tile,
                                   const QPixmap &image,
                                   const QUrl &source)
{
    Q_ASSERT(tile->tileset() == mTileset.data());

    mTileset->setTileImage(tile, image, source);
    emit tileImageSourceChanged(tile);

    for (MapDocument *mapDocument : mapDocuments())
        emit mapDocument->tileImageSourceChanged(tile);
}

void TilesetDocument::setTileProbability(Tile *tile, qreal probability)
{
    Q_ASSERT(tile->tileset() == mTileset.data());

    tile->setProbability(probability);
    emit tileProbabilityChanged(tile);

    for (MapDocument *mapDocument : mapDocuments())
        emit mapDocument->tileProbabilityChanged(tile);
}

void TilesetDocument::swapTileObjectGroup(Tile *tile, std::unique_ptr<ObjectGroup> &objectGroup)
{
    tile->swapObjectGroup(objectGroup);
    emit tileObjectGroupChanged(tile);

    for (MapDocument *mapDocument : mapDocuments())
        emit mapDocument->tileObjectGroupChanged(tile);
}

void TilesetDocument::checkIssues()
{
    // Clear any previously found issues in this document
    IssuesModel::instance().removeIssuesWithContext(this);

    if (tileset()->imageStatus() == LoadingError) {
        auto fileName = tileset()->imageSource().toString(QUrl::PreferLocalFile);
        ERROR(tr("Failed to load tileset image '%1'").arg(fileName),
              std::function<void()>(), this);       // todo: hook to file dialog
    }

    checkFilePathProperties(tileset().data());

    for (Tile *tile : tileset()->tiles()) {
        checkFilePathProperties(tile);
        // todo: check properties on collision objects

        if (!tile->imageSource().isEmpty() && tile->imageStatus() == LoadingError) {
            auto fileName = tile->imageSource().toString(QUrl::PreferLocalFile);
            ERROR(tr("Failed to load tile image '%1'").arg(fileName),
                  std::function<void()>(), this);   // todo: hook to file dialog
        }
    }
    for (WangSet *wangSet : tileset()->wangSets()) {
        checkFilePathProperties(wangSet);
        // todo: check properties on wang colors
    }
}

TilesetDocument *TilesetDocument::findDocumentForTileset(const SharedTileset &tileset)
{
    return sTilesetToDocument.value(tileset);
}

void TilesetDocument::onPropertyAdded(Object *object, const QString &name)
{
    for (MapDocument *mapDocument : mapDocuments())
        emit mapDocument->propertyAdded(object, name);
}

void TilesetDocument::onPropertyRemoved(Object *object, const QString &name)
{
    for (MapDocument *mapDocument : mapDocuments())
        emit mapDocument->propertyRemoved(object, name);
}

void TilesetDocument::onPropertyChanged(Object *object, const QString &name)
{
    for (MapDocument *mapDocument : mapDocuments())
        emit mapDocument->propertyChanged(object, name);
}

void TilesetDocument::onPropertiesChanged(Object *object)
{
    for (MapDocument *mapDocument : mapDocuments())
        emit mapDocument->propertiesChanged(object);
}

void TilesetDocument::onWangSetRemoved(WangSet *wangSet)
{
    mWangColorModels.erase(wangSet);
}

} // namespace Tiled

#include "moc_tilesetdocument.cpp"
