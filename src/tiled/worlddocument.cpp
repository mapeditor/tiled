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

#include "changeevents.h"
#include "editableworld.h"
#include "worldmanager.h"

#include <QFileInfo>
#include <QUndoStack>

namespace Tiled {

class ReloadWorld : public QUndoCommand
{
public:
    ReloadWorld(WorldDocument *worldDocument, std::unique_ptr<World> world)
        : mWorldDocument(worldDocument)
        , mWorld(std::move(world))
    {
        setText(QCoreApplication::translate("Undo Commands", "Reload World"));
    }

    void undo() override { mWorldDocument->swapWorld(mWorld); }
    void redo() override { mWorldDocument->swapWorld(mWorld); }

private:
    WorldDocument *mWorldDocument;
    std::unique_ptr<World> mWorld;
};


WorldDocument::WorldDocument(std::unique_ptr<World> world, QObject *parent)
    : Document(WorldDocumentType, world->fileName, parent)
    , mWorld(std::move(world))
{
    setCurrentObject(mWorld.get());
}

WorldDocument::~WorldDocument()
{
    // The Editable needs to be deleted before the World, otherwise ~World()
    // will delete it, whereas the editable is actually owned by the Document.
    mEditable.reset();
}

QString WorldDocument::displayName() const
{
    QString displayName = QFileInfo(fileName()).fileName();
    if (displayName.isEmpty())
        displayName = tr("untitled.world");

    return displayName;
}

/**
 * Saves the world. Does not support changing the file name!
 */
bool WorldDocument::save(const QString &/*fileName*/, QString *error)
{
    if (!World::save(*mWorld, error))
        return false;

    undoStack()->setClean();

    mLastSaved = QFileInfo(fileName()).lastModified();

    emit saved();
    return true;
}

bool WorldDocument::canReload() const
{
    return !fileName().isEmpty();
}

bool WorldDocument::reload(QString *error)
{
    if (!canReload())
        return false;

    auto world = World::load(fileName(), error);
    if (!world)
        return false;

    undoStack()->push(new ReloadWorld(this, std::move(world)));
    undoStack()->setClean();

    mLastSaved = QFileInfo(fileName()).lastModified();
    setChangedOnDisk(false);

    return true;
}

WorldDocumentPtr WorldDocument::load(const QString &fileName, QString *error)
{
    auto world = World::load(fileName, error);
    if (!world)
        return nullptr;

    return WorldDocumentPtr::create(std::move(world));
}

void WorldDocument::swapWorld(std::unique_ptr<World> &other)
{
    setCurrentObject(nullptr);

    emit changed(AboutToReloadEvent());

    mWorld->clearErrorsAndWarnings();
    mWorld.swap(other);
    updateIsModified();

    emit changed(ReloadEvent());

    setCurrentObject(mWorld.get());
    emit worldChanged();
}

std::unique_ptr<EditableAsset> WorldDocument::createEditable()
{
    return std::make_unique<EditableWorld>(this, this);
}

} // namespace Tiled

#include "moc_worlddocument.cpp"
