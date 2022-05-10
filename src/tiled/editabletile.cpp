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
#include "editablemanager.h"
#include "editableobjectgroup.h"
#include "editabletileset.h"
#include "imagecache.h"
#include "objectgroup.h"
#include "scriptimage.h"
#include "scriptmanager.h"

#include <QCoreApplication>
#include <QJSEngine>

namespace Tiled {

EditableTile::EditableTile(EditableTileset *tileset, Tile *tile, QObject *parent)
    : EditableObject(tileset, tile, parent)
{
}

EditableTile::~EditableTile()
{
    EditableManager::instance().remove(this);
}

EditableObjectGroup *EditableTile::objectGroup() const
{
    if (!mAttachedObjectGroup) {
        mAttachedObjectGroup = tile()->objectGroup();
    } else {
        Q_ASSERT(mAttachedObjectGroup == tile()->objectGroup());
    }

    return EditableManager::instance().editableObjectGroup(asset(), mAttachedObjectGroup);
}

QJSValue EditableTile::frames() const
{
    QJSEngine *engine = ScriptManager::instance().engine();

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

void EditableTile::setImage(ScriptImage *image)
{
    if (!image) {
        ScriptManager::instance().throwNullArgError(0);
        return;
    }

    // WARNING: This function has no undo!
    tile()->setImage(QPixmap::fromImage(image->image()));
}

void EditableTile::detach()
{
    Q_ASSERT(tileset());

    auto &editableManager = EditableManager::instance();

    editableManager.remove(this);
    setAsset(nullptr);

    mDetachedTile.reset(tile()->clone(nullptr));
    setObject(mDetachedTile.get());
    editableManager.mEditables.insert(tile(), this);

    // Move over any attached editable object group
    if (auto editable = editableManager.find(mAttachedObjectGroup)) {
        editableManager.remove(editable);
        editable->setAsset(nullptr);
        editable->setObject(tile()->objectGroup());
        editableManager.mEditables.insert(tile()->objectGroup(), editable);
        mAttachedObjectGroup = tile()->objectGroup();
    } else {
        mAttachedObjectGroup = nullptr;
    }
}

void EditableTile::attach(EditableTileset *tileset)
{
    Q_ASSERT(!asset() && tileset);

    setAsset(tileset);
    mDetachedTile.release();
}

void EditableTile::detachObjectGroup()
{
    if (auto editable = EditableManager::instance().find(mAttachedObjectGroup))
        editable->detach();
    mAttachedObjectGroup = nullptr;
}

void EditableTile::setType(const QString &type)
{
    if (TilesetDocument *doc = tilesetDocument())
        asset()->push(new ChangeTileType(doc, { tile() }, type));
    else if (!checkReadOnly())
        tile()->setType(type);
}

void EditableTile::setImageFileName(const QString &fileName)
{
    if (TilesetDocument *doc = tilesetDocument()) {
        if (!tileset()->tileset()->isCollection()) {
            ScriptManager::instance().throwError(QCoreApplication::translate("Script Errors", "Tileset needs to be an image collection"));
            return;
        }

        asset()->push(new ChangeTileImageSource(doc, tile(),
                                                QUrl::fromLocalFile(fileName),
                                                imageRect()));
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

        asset()->push(new ChangeTileImageSource(doc, tile(),
                                                QUrl::fromLocalFile(imageFileName()),
                                                rect));
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
    if (!editableObjectGroup) {
        ScriptManager::instance().throwNullArgError(0);
        return;
    }

    if (!editableObjectGroup->isOwning()) {
        ScriptManager::instance().throwError(QCoreApplication::translate("Script Errors", "ObjectGroup is in use"));
        return;
    }

    if (checkReadOnly())
        return;

    std::unique_ptr<ObjectGroup> og(static_cast<ObjectGroup*>(editableObjectGroup->release()));

    if (TilesetDocument *doc = tilesetDocument()) {
        asset()->push(new ChangeTileObjectGroup(doc, tile(), std::move(og)));
    } else {
        detachObjectGroup();
        tile()->setObjectGroup(std::move(og));
    }

    Q_ASSERT(editableObjectGroup->objectGroup() == tile()->objectGroup());
    Q_ASSERT(!editableObjectGroup->isOwning());
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
