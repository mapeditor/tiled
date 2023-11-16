/*
 * editablewangset.cpp
 * Copyright 2019, Your Name <your.name@domain>
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

#include "editablewangset.h"

#include "changetilewangid.h"
#include "changewangcolordata.h"
#include "changewangsetdata.h"
#include "editabletile.h"
#include "editabletileset.h"
#include "scriptmanager.h"

#include <QCoreApplication>
#include <QJSEngine>

namespace Tiled {

EditableWangSet::EditableWangSet(EditableTileset *tileset,
                                 WangSet *wangSet,
                                 QObject *parent)
    : EditableObject(tileset, wangSet, parent)
{
}

EditableWangSet::~EditableWangSet()
{
    // Prevent owned object from trying to delete us again
    if (mDetachedWangSet)
        setObject(nullptr);
}

EditableTile *EditableWangSet::imageTile() const
{
    if (Tile *tile = wangSet()->imageTile())
        return EditableTile::get(tileset(), tile);

    return nullptr;
}

EditableTileset *EditableWangSet::tileset() const
{
    return static_cast<EditableTileset*>(asset());
}

QJSValue EditableWangSet::wangId(EditableTile *editableTile)
{
    if (!editableTile) {
        ScriptManager::instance().throwNullArgError(0);
        return {};
    }

    QJSEngine *engine = qjsEngine(this);
    if (!engine)
        return QJSValue();

    WangId wangId = wangSet()->wangIdOfTile(editableTile->tile());

    QJSValue wangIdArray = engine->newArray(WangId::NumIndexes);
    for (quint32 i = 0; i < WangId::NumIndexes; ++i)
        wangIdArray.setProperty(i, wangId.indexColor(i));

    return wangIdArray;
}

void EditableWangSet::setWangId(EditableTile *editableTile, QJSValue value)
{
    if (!editableTile) {
        ScriptManager::instance().throwNullArgError(0);
        return;
    }

    const int length = value.property(QStringLiteral("length")).toInt();
    if (!value.isArray() || length != WangId::NumIndexes) {
        ScriptManager::instance().throwError(QCoreApplication::translate("Script Errors", "Wang ID must be an array of length 8"));
        return;
    }

    WangId wangId;

    for (quint32 i = 0; i < WangId::NumIndexes; ++i) {
        const unsigned color = value.property(i).toUInt();
        wangId.setIndexColor(i, color);
    }

    if (!wangSet()->wangIdIsValid(wangId)) {
        ScriptManager::instance().throwError(QCoreApplication::translate("Script Errors", "Invalid Wang ID"));
        return;
    }

    if (auto doc = tilesetDocument())
        asset()->push(new ChangeTileWangId(doc, wangSet(), editableTile->tile(), wangId));
    else if (!checkReadOnly())
        wangSet()->setWangId(editableTile->id(), wangId);
}

QString EditableWangSet::colorName(int colorIndex) const
{
    if (colorIndex <= 0 || colorIndex > colorCount()) {
        ScriptManager::instance().throwError(QCoreApplication::translate("Script Errors", "Index out of range"));
        return QString();
    }
    return wangSet()->colorAt(colorIndex)->name();
}

void EditableWangSet::setColorName(int colorIndex, const QString &name)
{
    if (colorIndex <= 0 || colorIndex > colorCount()) {
        ScriptManager::instance().throwError(QCoreApplication::translate("Script Errors", "Index out of range"));
        return;
    }

    if (auto doc = tilesetDocument())
        asset()->push(new ChangeWangColorName(doc, wangSet()->colorAt(colorIndex).data(), name));
    else if (!checkReadOnly())
        wangSet()->colorAt(colorIndex)->setName(name);
}

void EditableWangSet::setName(const QString &name)
{
    if (auto document = tilesetDocument())
        asset()->push(new RenameWangSet(document, wangSet(), name));
    else if (!checkReadOnly())
        wangSet()->setName(name);
}

void EditableWangSet::setType(EditableWangSet::Type type)
{
    if (auto document = tilesetDocument()) {
        asset()->push(new ChangeWangSetType(document, wangSet(),
                                            static_cast<WangSet::Type>(type)));
    } else if (!checkReadOnly()) {
        wangSet()->setType(static_cast<WangSet::Type>(type));
    }
}

void EditableWangSet::setImageTile(EditableTile *imageTile)
{
    if (imageTile && imageTile->tileset() != tileset()) {
        ScriptManager::instance().throwError(QCoreApplication::translate("Script Errors", "Tile not from the same tileset"));
        return;
    }

    int tileId = imageTile ? imageTile->id() : -1;

    if (auto document = tilesetDocument())
        asset()->push(new SetWangSetImage(document, wangSet(), tileId));
    else if (!checkReadOnly())
        wangSet()->setImageTileId(tileId);
}

void EditableWangSet::setColorCount(int n)
{
    if (auto document = tilesetDocument()) {
        asset()->push(new ChangeWangSetColorCount(document, wangSet(), n));
    } else if (!checkReadOnly()) {
        ChangeTileWangId::applyChanges(wangSet(),
                                       ChangeTileWangId::changesOnSetColorCount(wangSet(), n));
        wangSet()->setColorCount(n);
    }
}

void EditableWangSet::detach()
{
    Q_ASSERT(tileset());
    setAsset(nullptr);

    if (!moveOwnershipToJavaScript())
        return;

    mDetachedWangSet.reset(wangSet()->clone(nullptr));
    setObject(mDetachedWangSet.get());
}

void EditableWangSet::attach(EditableTileset *tileset)
{
    Q_ASSERT(!asset() && tileset);

    moveOwnershipToCpp();
    setAsset(tileset);
    mDetachedWangSet.release();
}

/**
 * Take ownership of the referenced wang set or delete it.
 */
void EditableWangSet::hold(std::unique_ptr<WangSet> wangSet)
{
    Q_ASSERT(!mDetachedWangSet);    // can't already be holding the object
    Q_ASSERT(this->wangSet() == wangSet.get());

    if (!moveOwnershipToJavaScript())
        return;

    setAsset(nullptr);
    mDetachedWangSet = std::move(wangSet);
}

EditableWangSet *EditableWangSet::get(WangSet *wangSet)
{
    if (!wangSet)
        return nullptr;

    auto tileset = EditableTileset::get(wangSet->tileset());
    return get(tileset, wangSet);
}

EditableWangSet *EditableWangSet::get(EditableTileset *tileset, WangSet *wangSet)
{
    Q_ASSERT(wangSet);
    Q_ASSERT(wangSet->tileset() == tileset->tileset());

    auto editable = EditableWangSet::find(wangSet);
    if (editable)
        return editable;

    editable = new EditableWangSet(tileset, wangSet);
    editable->moveOwnershipToCpp();
    return editable;
}

/**
 * Releases the WangSet by either finding an EditableWangSet instance to take
 * ownership of it or deleting it.
 */
void EditableWangSet::release(std::unique_ptr<WangSet> wangSet)
{
    if (auto editable = EditableWangSet::find(wangSet.get()))
        editable->hold(std::move(wangSet));
}

TilesetDocument *EditableWangSet::tilesetDocument() const
{
    return tileset() ? tileset()->tilesetDocument() : nullptr;
}

} // namespace Tiled

#include "moc_editablewangset.cpp"
