#include <QUndoStack>

#include "worlddocument.h"
#include "worldmanager.h"

namespace Tiled {

WorldDocument::WorldDocument(const QString& fileName)
    : mUndoStack( new QUndoStack(this) ),
      mFileName( fileName )
{
    mUndoStack->setClean();
    WorldManager& worldManager = WorldManager::instance();
    connect(&worldManager, &WorldManager::worldReloaded,
            this, &WorldDocument::onWorldReloaded);
}

WorldDocument::~WorldDocument()
{
}

void WorldDocument::onWorldReloaded( const QString& fileName )
{
    if(fileName == mFileName) {
        mUndoStack->clear();
    }
}

QUndoStack* WorldDocument::undoStack() const
{
    return mUndoStack;
}

}
