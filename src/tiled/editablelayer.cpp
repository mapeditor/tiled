/*
 * editablelayer.cpp
 * Copyright 2018, Thorbjørn Lindeijer <bjorn@lindeijer.nl>
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

#include "editablelayer.h"

#include "changelayer.h"
#include "editablemap.h"
#include "renamelayer.h"
#include "scriptmanager.h"
#include "scriptmodule.h"

#include <QJSEngine>

namespace Tiled {
namespace Internal {

EditableLayer::EditableLayer(EditableMap *map, Layer *layer, QObject *parent)
    : QObject(parent)
    , mMap(map)
    , mLayer(layer)
{
}

EditableLayer::~EditableLayer()
{
    if (mMap)
        mMap->editableLayerDeleted(this);
}

void EditableLayer::invalidate()
{
    mMap = nullptr;
}

void EditableLayer::setName(const QString &name)
{
    if (checkValid())
        mMap->push(new RenameLayer(mMap->mapDocument(), mLayer, name));
}

void EditableLayer::setOpacity(qreal opacity)
{
    if (checkValid())
        mMap->push(new SetLayerOpacity(mMap->mapDocument(), mLayer, opacity));
}

void EditableLayer::setVisible(bool visible)
{
    if (checkValid())
        mMap->push(new SetLayerVisible(mMap->mapDocument(), mLayer, visible));
}

void EditableLayer::setLocked(bool locked)
{
    if (checkValid())
        mMap->push(new SetLayerLocked(mMap->mapDocument(), mLayer, locked));
}

void EditableLayer::setOffset(QPointF offset)
{
    if (checkValid())
        mMap->push(new SetLayerOffset(mMap->mapDocument(), mLayer, offset));
}

bool EditableLayer::checkValid()
{
    if (!mMap) {
        const QString message = tr("Can't modify layer that was removed from the map");
#if QT_VERSION < QT_VERSION_CHECK(5, 12, 0)
        ScriptManager::instance().module()->error(message);
#else
        ScriptManager::instance().engine()->throwError(message);
#endif
        return false;
    }
    return true;
}

} // namespace Internal
} // namespace Tiled
