/*
 * addremovemapobject.cpp
 * Copyright 2009, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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

#include "addremovemapobject.h"

#include "map.h"
#include "mapdocument.h"
#include "mapobject.h"
#include "objectgroup.h"
#include "mapobjectmodel.h"

#include "rtbchangemapobjectproperties.h"

#include <QCoreApplication>

using namespace Tiled;
using namespace Tiled::Internal;

AddRemoveMapObject::AddRemoveMapObject(MapDocument *mapDocument,
                                       ObjectGroup *objectGroup,
                                       MapObject *mapObject,
                                       bool ownObject,
                                       QUndoCommand *parent)
    : QUndoCommand(parent)
    , mMapDocument(mapDocument)
    , mMapObject(mapObject)
    , mObjectGroup(objectGroup)
    , mIndex(-1)
    , mOwnsObject(ownObject)
    , mRelatedObjects(QList<MapObject*>())
{
}

AddRemoveMapObject::~AddRemoveMapObject()
{
    if (mOwnsObject)
        delete mMapObject;
}

void AddRemoveMapObject::addObject()
{
    mMapDocument->mapObjectModel()->insertObject(mObjectGroup, mIndex,
                                                 mMapObject);
    mOwnsObject = false;

    updateRelatedObjectsAdd();
}

void AddRemoveMapObject::removeObject()
{
    mIndex = mMapDocument->mapObjectModel()->removeObject(mObjectGroup,
                                                          mMapObject);
    mOwnsObject = true;

    updateRelatedObjectsRemove();
}

void AddRemoveMapObject::updateRelatedObjectsRemove()
{
    // update objects which points of this object as target
    if(mMapObject->rtbMapObject()->objectType() == RTBMapObject::LaserBeam)
    {
        for(MapObject *obj: mMapDocument->map()->objectGroups().first()->objects())
        {
            if(obj->rtbMapObject()->objectType() == RTBMapObject::Button)
            {
                RTBButtonObject *rtbMapObject = static_cast<RTBButtonObject*>(obj->rtbMapObject());

                // check if one if the targets points of the object to delete, if so delete entry
                if(mMapObject->id() == rtbMapObject->target(RTBChangeMapObjectProperties::RTBLaserBeamTarget1).toInt())
                {
                    rtbMapObject->insertTarget(RTBChangeMapObjectProperties::RTBLaserBeamTarget1, QString());
                    mRelatedObjects.append(obj);
                    break;
                }
                else if(mMapObject->id() == rtbMapObject->target(RTBChangeMapObjectProperties::RTBLaserBeamTarget2).toInt())
                {
                    rtbMapObject->insertTarget(RTBChangeMapObjectProperties::RTBLaserBeamTarget2, QString());
                    mRelatedObjects.append(obj);
                    break;
                }
                else if(mMapObject->id() == rtbMapObject->target(RTBChangeMapObjectProperties::RTBLaserBeamTarget3).toInt())
                {
                    rtbMapObject->insertTarget(RTBChangeMapObjectProperties::RTBLaserBeamTarget3, QString());
                    mRelatedObjects.append(obj);
                    break;
                }
                else if(mMapObject->id() == rtbMapObject->target(RTBChangeMapObjectProperties::RTBLaserBeamTarget4).toInt())
                {
                    rtbMapObject->insertTarget(RTBChangeMapObjectProperties::RTBLaserBeamTarget4, QString());
                    mRelatedObjects.append(obj);
                    break;
                }
                else if(mMapObject->id() == rtbMapObject->target(RTBChangeMapObjectProperties::RTBLaserBeamTarget5).toInt())
                {
                    rtbMapObject->insertTarget(RTBChangeMapObjectProperties::RTBLaserBeamTarget5, QString());
                    mRelatedObjects.append(obj);
                    break;
                }
            }
        }
    }
    else if(mMapObject->rtbMapObject()->objectType() == RTBMapObject::Target)
    {
        for(MapObject *obj: mMapDocument->map()->objectGroups().first()->objects())
        {
            switch (obj->rtbMapObject()->objectType()) {
            case RTBMapObject::Teleporter:
            {
                RTBTeleporter *rtbMapObject = static_cast<RTBTeleporter*>(obj->rtbMapObject());

                if(mMapObject->id() == rtbMapObject->teleporterTarget().toInt())
                {
                    rtbMapObject->setTeleporterTarget(QString());
                    mRelatedObjects.append(obj);
                }

                break;
            }
            case RTBMapObject::CameraTrigger:
            {
                RTBCameraTrigger *rtbMapObject = static_cast<RTBCameraTrigger*>(obj->rtbMapObject());

                if(mMapObject->id() == rtbMapObject->target().toInt())
                {
                    rtbMapObject->setTarget(QString());
                    mRelatedObjects.append(obj);
                }
                break;
            }
            default:
                break;
            }
        }
    }

    // update property view
    mMapDocument->mapObjectModel()->emitObjectsChanged(mRelatedObjects);
}

void AddRemoveMapObject::updateRelatedObjectsAdd()
{
    if(mRelatedObjects.size() > 0)
    {
        switch (mMapObject->rtbMapObject()->objectType()) {
        case RTBMapObject::LaserBeam:
        {
            RTBButtonObject *rtbMapObject = static_cast<RTBButtonObject*>(mRelatedObjects.first()->rtbMapObject());

            // check if one if the targets points of the object to delete, if so delete entry
            if(QString() == rtbMapObject->target(RTBChangeMapObjectProperties::RTBLaserBeamTarget1))
            {
                rtbMapObject->insertTarget(RTBChangeMapObjectProperties::RTBLaserBeamTarget1, QString::number(mMapObject->id()));
            }
            else if(QString() == rtbMapObject->target(RTBChangeMapObjectProperties::RTBLaserBeamTarget2))
            {
                rtbMapObject->insertTarget(RTBChangeMapObjectProperties::RTBLaserBeamTarget2, QString::number(mMapObject->id()));
            }
            else if(QString() == rtbMapObject->target(RTBChangeMapObjectProperties::RTBLaserBeamTarget3))
            {
                rtbMapObject->insertTarget(RTBChangeMapObjectProperties::RTBLaserBeamTarget3, QString::number(mMapObject->id()));
            }
            else if(QString() == rtbMapObject->target(RTBChangeMapObjectProperties::RTBLaserBeamTarget4))
            {
                rtbMapObject->insertTarget(RTBChangeMapObjectProperties::RTBLaserBeamTarget4, QString::number(mMapObject->id()));
            }
            else if(QString() == rtbMapObject->target(RTBChangeMapObjectProperties::RTBLaserBeamTarget5))
            {
                rtbMapObject->insertTarget(RTBChangeMapObjectProperties::RTBLaserBeamTarget5, QString::number(mMapObject->id()));
            }

            break;
        }
        case RTBMapObject::Target:
        {
            for(MapObject *obj: mRelatedObjects)
            {
                switch (obj->rtbMapObject()->objectType()) {
                case RTBMapObject::Teleporter:
                {
                    RTBTeleporter *rtbMapObject = static_cast<RTBTeleporter*>(obj->rtbMapObject());
                    rtbMapObject->setTeleporterTarget(QString::number(mMapObject->id()));

                    break;
                }
                case RTBMapObject::CameraTrigger:
                {
                    RTBCameraTrigger *rtbMapObject = static_cast<RTBCameraTrigger*>(obj->rtbMapObject());
                    rtbMapObject->setTarget(QString::number(mMapObject->id()));

                    break;
                }
                default:
                    break;
                }
            }
        }
        default:
            break;
        }

        // update property view
        mMapDocument->mapObjectModel()->emitObjectsChanged(mRelatedObjects);
    }
}

AddMapObject::AddMapObject(MapDocument *mapDocument, ObjectGroup *objectGroup,
                           MapObject *mapObject, QUndoCommand *parent)
    : AddRemoveMapObject(mapDocument,
                         objectGroup,
                         mapObject,
                         true,
                         parent)
{
    setText(QCoreApplication::translate("Undo Commands", "Add Object"));
}


RemoveMapObject::RemoveMapObject(MapDocument *mapDocument,
                                 MapObject *mapObject,
                                 QUndoCommand *parent)
    : AddRemoveMapObject(mapDocument,
                         mapObject->objectGroup(),
                         mapObject,
                         false,
                         parent)
{
    setText(QCoreApplication::translate("Undo Commands", "Remove Object"));
}
