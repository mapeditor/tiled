/*
 * worlddocument.cpp
 * Copyright 2019, Nils Kübler <nils-kuebler@web.de>
 * Copyright 2020, Thorbjørn Lindeijer <bjorn@lindeijer.nl>
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

#include "worlddocument.h"

#include "worldmanager.h"

#include <QFileInfo>
#include <QUndoStack>

namespace Tiled {

WorldDocument::WorldDocument(const QString &fileName, QObject *parent)
    : Document(WorldDocumentType, fileName, parent)
{
    WorldManager &worldManager = WorldManager::instance();
    connect(&worldManager, &WorldManager::worldReloaded,
            this, &WorldDocument::onWorldReloaded);
}

QString WorldDocument::displayName() const
{
    QString displayName = QFileInfo(fileName()).fileName();
    if (displayName.isEmpty())
        displayName = tr("untitled.world");

    return displayName;
}

bool WorldDocument::save(const QString &fileName, QString *error)
{
    return WorldManager::instance().saveWorld(fileName, error);
}

void WorldDocument::onWorldReloaded(const QString &fileName)
{
    if (this->fileName() == fileName)
        undoStack()->clear();
}

} // namespace Tiled
