/*
 * editabletile.cpp
 * Copyright 2019, Thorbjørn Lindeijer <bjorn@lindeijer.nl>
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

#include "editabletile.h"

#include "changetile.h"
#include "changetileanimation.h"
#include "changetileimagesource.h"
#include "changetileobjectgroup.h"
#include "editableobjectgroup.h"
#include "editabletileset.h"
#include "imagecache.h"
#include "objectgroup.h"
#include "scriptimage.h"
#include "scriptmanager.h"
#include "tilesetdocument.h"

#include <QCoreApplication>
#include <QJSEngine>

namespace Tiled {

EditableTile::EditableTile(EditableTileset *tileset, Tile *tile, QObject *parent)
    : EditableObject(tileset, tile, parent)
{
}

EditableTile::~EditableTile()
{
    // Prevent owned object from trying to delete us again
    if (mDetachedTile)
        setObject(nullptr);
}

ScriptImage *EditableTile::image() const
{
    return new ScriptImage(tile()->image().toImage());
}

EditableObjectGroup *EditableTile::objectGroup() const
{
    if (!mAttachedObjectGroup) {
        mAttachedObjectGroup = tile()->objectGroup();
    } else {
        Q_ASSERT(mAttachedObjectGroup == tile()->objectGroup());
    }

    return EditableObjectGroup::get(asset(), mAttachedObjectGroup);
}

QJSValue EditableTile::frames() const
{
    QJSEngine *engine = qjsEngine(this);
    if (!engine)
        return QJSValue();

    const auto &frames = tile()->frames();
    QJSValue array = engine->newArray(frames.size());

    for (int i = 0; i < frames.size(); ++i) {
        QJSValue frameObject = engine->newObject();
        frameObject.setProperty(QStringLiteral("tileId"), frames.at(i).tileId);
        frameObject.setProperty(QStringLiteral("duration"), frames.at(i).duration);
        array.setProperty(i, frameObject);
    }

    return array;
}

EditableTileset *EditableTile::tileset() const
{
    return static_cast<EditableTileset*>(asset());
}

void EditableTile::setImage(ScriptImage *image, const QString &fileName)
{
    if (!image) {
        ScriptManager::instance().throwNullArgError(0);
        return;
    }

    const auto pixmap = QPixmap::fromImage(image->image());

    // WARNING: This function has no undo!
    if (auto doc = tilesetDocument())
        doc->setTileImage(tile(), pixmap, QUrl::fromLocalFile(fileName));
    else
        tile()->setImage(pixmap);
}

void EditableTile::detach()
{
    Q_ASSERT(tileset());
    setAsset(nullptr);

    if (!moveOwnershipToJavaScript())
        return;

    mDetachedTile.reset(tile()->clone(nullptr));
    setObject(mDetachedTile.get());

    // Move over any attached editable object group
    if (auto editable = EditableLayer::find(mAttachedObjectGroup)) {
        editable->setAsset(nullptr);
        editable->setObject(tile()->objectGroup());
        mAttachedObjectGroup = tile()->objectGroup();
    } else {
        mAttachedObjectGroup = nullptr;
    }
}

void EditableTile::attach(EditableTileset *tileset)
{
    Q_ASSERT(!asset() && tileset);

    moveOwnershipToCpp();
    setAsset(tileset);
    mDetachedTile.release();
}

void EditableTile::detachObjectGroup()
{
    if (auto editable = EditableLayer::find(mAttachedObjectGroup))
        editable->detach();
    mAttachedObjectGroup = nullptr;
}

EditableTile *EditableTile::get(Tile *tile)
{
    if (!tile)
        return nullptr;

    auto tileset = EditableTileset::get(tile->tileset());
    return get(tileset, tile);
}

EditableTile *EditableTile::get(EditableTileset *tileset, Tile *tile)
{
    Q_ASSERT(tile);
    Q_ASSERT(tile->tileset() == tileset->tileset());

    auto editable = EditableTile::find(tile);
    if (editable)
        return editable;

    editable = new EditableTile(tileset, tile);
    editable->moveOwnershipToCpp();
    return editable;
}

void EditableTile::setImageFileName(const QString &fileName)
{
    if (TilesetDocument *doc = tilesetDocument()) {
        if (!tileset()->tileset()->isCollection()) {
            ScriptManager::instance().throwError(QCoreApplication::translate("Script Errors", "Tileset needs to be an image collection"));
            return;
        }

        asset()->push(new ChangeTileImageSource(doc, tile(),
                                                QUrl::fromLocalFile(fileName)));
    } else if (!checkReadOnly()) {
        tile()->setImage(ImageCache::loadPixmap(fileName));
        tile()->setImageSource(QUrl::fromLocalFile(fileName));
    }
}

void EditableTile::setImageRect(const QRect &rect)
{
    if (TilesetDocument *doc = tilesetDocument()) {
        if (!tileset()->tileset()->isCollection()) {
            ScriptManager::instance().throwError(QCoreApplication::translate("Script Errors", "Tileset needs to be an image collection"));
            return;
        }

        asset()->push(new ChangeTileImageRect(doc, { tile() }, { rect }));
    } else if (!checkReadOnly()) {
        tile()->setImageRect(rect);
    }
}

void EditableTile::setProbability(qreal probability)
{
    if (TilesetDocument *doc = tilesetDocument())
        asset()->push(new ChangeTileProbability(doc, { tile() }, probability));
    else if (!checkReadOnly())
        tile()->setProbability(probability);
}

void EditableTile::setObjectGroup(EditableObjectGroup *editableObjectGroup)
{
    if (checkReadOnly())
        return;

    std::unique_ptr<ObjectGroup> og;

    if (editableObjectGroup) {
        if (!editableObjectGroup->isOwning()) {
            ScriptManager::instance().throwError(QCoreApplication::translate("Script Errors", "ObjectGroup is in use"));
            return;
        }

        og.reset(static_cast<ObjectGroup*>(editableObjectGroup->attach(asset())));
    }

    if (TilesetDocument *doc = tilesetDocument()) {
        asset()->push(new ChangeTileObjectGroup(doc, tile(), std::move(og)));
    } else {
        detachObjectGroup();
        tile()->setObjectGroup(std::move(og));
    }

    if (editableObjectGroup) {
        Q_ASSERT(editableObjectGroup->objectGroup() == tile()->objectGroup());
        Q_ASSERT(!editableObjectGroup->isOwning());
    } else {
        Q_ASSERT(tile()->objectGroup() == nullptr);
    }
}

void EditableTile::setFrames(QJSValue value)
{
    if (!value.isArray()) {
        ScriptManager::instance().throwError(QCoreApplication::translate("Script Errors", "Array expected"));
        return;
    }

    if (checkReadOnly())
        return;

    QVector<Frame> frames;
    const int length = value.property(QStringLiteral("length")).toInt();

    for (int i = 0; i < length; ++i) {
        const auto frameValue = value.property(i);
        const Frame frame {
            frameValue.property(QStringLiteral("tileId")).toInt(),
            frameValue.property(QStringLiteral("duration")).toInt()
        };

        if (frame.tileId < 0 || frame.duration < 0) {
            ScriptManager::instance().throwError(QCoreApplication::translate("Script Errors", "Invalid value (negative)"));
            return;
        }

        frames.append(frame);
    }

    if (TilesetDocument *doc = tilesetDocument())
        asset()->push(new ChangeTileAnimation(doc, tile(), frames));
    else
        tile()->setFrames(frames);
}

TilesetDocument *EditableTile::tilesetDocument() const
{
    return tileset() ? tileset()->tilesetDocument() : nullptr;
}

} // namespace Tiled

#include "moc_editabletile.cpp"
