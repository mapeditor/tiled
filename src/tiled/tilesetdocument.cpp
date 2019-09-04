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

#include "editabletileset.h"
#include "issuesmodel.h"
#include "map.h"
#include "mapdocument.h"
#include "terrain.h"
#include "tile.h"
#include "tilesetformat.h"
#include "tilesetterrainmodel.h"
#include "tilesetwangsetmodel.h"
#include "wangcolormodel.h"
#include "wangset.h"

#include <QCoreApplication>
#include <QFileInfo>
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
    , mTerrainModel(new TilesetTerrainModel(this, this))
    , mWangSetModel(new TilesetWangSetModel(this, this))
{
    Q_ASSERT(!sTilesetToDocument.contains(tileset));
    sTilesetToDocument.insert(tileset, this);

    mCurrentObject = tileset.data();

    connect(this, &TilesetDocument::propertyAdded,
            this, &TilesetDocument::onPropertyAdded);
    connect(this, &TilesetDocument::propertyRemoved,
            this, &TilesetDocument::onPropertyRemoved);
    connect(this, &TilesetDocument::propertyChanged,
            this, &TilesetDocument::onPropertyChanged);
    connect(this, &TilesetDocument::propertiesChanged,
            this, &TilesetDocument::onPropertiesChanged);

    connect(mTerrainModel, &TilesetTerrainModel::terrainRemoved,
            this, &TilesetDocument::onTerrainRemoved);

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
    TilesetFormat *tilesetFormat = mTileset->format();

    if (!tilesetFormat || !(tilesetFormat->capabilities() & FileFormat::Write))
        return false;

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
    return !fileName().isEmpty() && mTileset->format();
}

bool TilesetDocument::reload(QString *error)
{
    if (!canReload())
        return false;

    auto format = mTileset->format();

    SharedTileset tileset = format->read(fileName());

    if (tileset.isNull()) {
        if (error)
            *error = format->errorString();
        return false;
    }

    tileset->setFileName(fileName());
    tileset->setFormat(format);

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
    tileset->setFormat(format);

    return TilesetDocumentPtr::create(tileset);
}

FileFormat *TilesetDocument::writerFormat() const
{
    return mTileset->format();
}

void TilesetDocument::setWriterFormat(TilesetFormat *format)
{
    mTileset->setFormat(format);
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
    if (tileset()->exportFormat.isEmpty())
        return nullptr;
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
        MapDocument *mapDocument = mMapDocuments.first();
        displayName = mapDocument->displayName();
        displayName += QLatin1String("#");
        displayName += mTileset->name();
    } else {
        displayName = QFileInfo(fileName()).fileName();
        if (displayName.isEmpty())
            displayName = tr("untitled.tsx");
    }

    return displayName;
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

EditableTileset *TilesetDocument::editable()
{
    if (!mEditable)
        mEditable.reset(new EditableTileset(this, this));

    return static_cast<EditableTileset*>(mEditable.get());
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
        emit mapDocument->tilesetTileOffsetChanged(mTileset.data());
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

    mTileset->removeTiles(tiles);
    emit tilesRemoved(tiles);
    emit tilesetChanged(mTileset.data());
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

void TilesetDocument::setTileType(Tile *tile, const QString &type)
{
    Q_ASSERT(tile->tileset() == mTileset.data());

    tile->setType(type);
    emit tileTypeChanged(tile);

    for (MapDocument *mapDocument : mapDocuments())
        emit mapDocument->tileTypeChanged(tile);
}

void TilesetDocument::setTileImage(Tile *tile, const QPixmap &image, const QUrl &source)
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
    for (Terrain *terrain : tileset()->terrains())
        checkFilePathProperties(terrain);
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

void TilesetDocument::onTerrainRemoved(Terrain *terrain)
{
    if (terrain == mCurrentObject)
        setCurrentObject(nullptr);
}

void TilesetDocument::onWangSetRemoved(WangSet *wangSet)
{
    if (wangSet == mCurrentObject)
        setCurrentObject(nullptr);

    mWangColorModels.erase(wangSet);
}

} // namespace Tiled
