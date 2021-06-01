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
#include "changewangsetdata.h"
#include "editablemanager.h"
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
    EditableManager::instance().mEditableWangSets.remove(wangSet());
}

EditableTile *EditableWangSet::imageTile() const
{
    return EditableManager::instance().editableTile(tileset(), wangSet()->imageTile());
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

    QJSEngine *engine = ScriptManager::instance().engine();
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

    auto &editableManager = EditableManager::instance();

    editableManager.mEditableWangSets.remove(wangSet());
    setAsset(nullptr);

    mDetachedWangSet.reset(wangSet()->clone(nullptr));
    setObject(mDetachedWangSet.get());
    editableManager.mEditableWangSets.insert(wangSet(), this);
}

void EditableWangSet::attach(EditableTileset *tileset)
{
    Q_ASSERT(!asset() && tileset);

    setAsset(tileset);
    mDetachedWangSet.release();
}

void EditableWangSet::hold()
{
    Q_ASSERT(!asset());             // if asset exists, it holds the object (possibly indirectly)
    Q_ASSERT(!mDetachedWangSet);    // can't already be holding the object

    mDetachedWangSet.reset(wangSet());
}

void EditableWangSet::release()
{
    Q_ASSERT(mDetachedWangSet.get() == wangSet());

    mDetachedWangSet.release();
}

TilesetDocument *EditableWangSet::tilesetDocument() const
{
    return tileset() ? tileset()->tilesetDocument() : nullptr;
}

} // namespace Tiled

#include "moc_editablewangset.cpp"
