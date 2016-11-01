/*
 * rtbtileselectionmanager.cpp
 * Copyright 2016, David Stammer
 *
 * This file is part of Road to Ballhalla Editor.
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

#include "rtbtileselectionmanager.h"

#include "tile.h"
#include "mapdocument.h"
#include "map.h"
#include "stampbrush.h"
#include "bucketfilltool.h"

#include "rtbtilebutton.h"
#include "rtbmapsettings.h"

#include <QAction>
#include <QPainter>

using namespace Tiled;
using namespace Tiled::Internal;

RTBTileSelectionManager::RTBTileSelectionManager(QObject *parent)
    : QObject(parent)
    , mActionGroup(new QActionGroup(this))
    , mSelectedTileButton(0)
    , mPreviouslyDisabledTileButton(0)
    , mMapDocument(0)
    , mStampBrush(0)
    , mBucketFillTool(0)
    , mSeparatorAction(0)
{
    mActionGroup->setExclusive(true);
    connect(mActionGroup, SIGNAL(triggered(QAction*)),
            this, SLOT(actionTriggered(QAction*)));
}

RTBTileSelectionManager::~RTBTileSelectionManager()
{
}

/**
 * Sets the MapDocument on which the registered tile buttons will operate.
 */
void RTBTileSelectionManager::setMapDocument(MapDocument *mapDocument)
{
    if (mMapDocument == mapDocument)
        return;

    mMapDocument = mapDocument;

    foreach (QAction *action, mActionGroup->actions()) {
        AbstractTool *tileButton = action->data().value<RTBTileButton*>();
        tileButton->setMapDocument(mapDocument);
    }

    if(mSelectedTileButton && mMapDocument)
    {
        // if a new map is created but the tileset does not exist yet
        if(mMapDocument->map()->tilesetCount() == 0)
        {
            // create the stamp delayed
            QMetaObject::invokeMethod(this, "createStamp",
                                      Qt::QueuedConnection, Q_ARG(int, mSelectedTileButton->type()));
        }
        else
        {
            createStamp(mSelectedTileButton);
        }
    }
}

/**
 * Registers a new tile button.
 *
 * @return The action for activating the tool.
 */
QAction *RTBTileSelectionManager::registerTile(RTBTileButton *tileButton)
{
    tileButton->setMapDocument(mMapDocument);

    QAction *tileButtonAction = new QAction(tileButton->icon(), tileButton->name(), this);
    tileButtonAction->setShortcut(tileButton->shortcut());
    tileButtonAction->setData(QVariant::fromValue<RTBTileButton*>(tileButton));
    tileButtonAction->setCheckable(true);

    // if no tooltip is set
    if(tileButton->shortcut().isEmpty())
    {
        tileButtonAction->setToolTip(
                QString(QLatin1String("%1")).arg(tileButton->name()));
    }
    else
    {
        tileButtonAction->setToolTip(
                QString(QLatin1String("%1 (%2)")).arg(tileButton->name(),
                                                      tileButton->shortcut().toString()));
    }

    tileButtonAction->setEnabled(tileButton->isEnabled());
    tileButtonAction->setVisible(tileButton->isEnabled());
    mActionGroup->addAction(tileButtonAction);

    connect(tileButton, SIGNAL(enabledChanged(bool)),
            this, SLOT(tileButtonEnabledChanged(bool)));
    connect(tileButton, SIGNAL(visibleChanged(bool)),
            this, SLOT(tileButtonVisibleChanged(bool)));

    return tileButtonAction;
}


void RTBTileSelectionManager::selectTileButton(RTBTileButton *tileButton)
{
    if (tileButton && !tileButton->isEnabled()) // Refuse to select disabled tools
        return;

    foreach (QAction *action, mActionGroup->actions()) {
        if (action->data().value<RTBTileButton*>() == tileButton) {
            action->trigger();
            return;
        }
    }

    // The given tool was not found. Don't select any tool.
    foreach (QAction *action, mActionGroup->actions())
        action->setChecked(false);
    setSelectedTileButton(0);
}

void RTBTileSelectionManager::actionTriggered(QAction *action)
{
    setSelectedTileButton(action->data().value<RTBTileButton*>());
}

void RTBTileSelectionManager::tileButtonVisibleChanged(bool visible)
{
    RTBTileButton *tileButton = qobject_cast<RTBTileButton*>(sender());

    foreach (QAction *action, mActionGroup->actions()) {
        if (action->data().value<RTBTileButton*>() == tileButton)
        {
            action->setVisible(visible);

            if(tileButton->type() == RTBMapSettings::WallBlock)
                updateIcon(action, tileButton);

            break;
        }
    }
}

void RTBTileSelectionManager::tileButtonEnabledChanged(bool enabled)
{
    RTBTileButton *tileButton = qobject_cast<RTBTileButton*>(sender());

    foreach (QAction *action, mActionGroup->actions()) {
        if (action->data().value<RTBTileButton*>() == tileButton)
        {
            action->setEnabled(enabled);

            // if enabled/disabled state have different tooltips change them
            if(action->toolTip() != tileButton->name())
                action->setToolTip(tileButton->name());

            // prevent changing active object to wall block tile
            // if the hasWalls prop was changed in the map properties
            if(!enabled && action->isVisible() && tileButton == mSelectedTileButton)
            {
                mSelectedTileButton = 0;
                action->setChecked(false);
            }

            if(tileButton->type() == RTBMapSettings::WallBlock)
                updateIcon(action, tileButton);
        }
    }

    // Switch to another tool when the current tool gets disabled. This is done
    // with a delayed call since we first want all the tools to update their
    // enabled state.
    if ((!enabled && tileButton == mSelectedTileButton) || (enabled && !mSelectedTileButton))
    {
        QMetaObject::invokeMethod(this, "selectEnabledTileButton",
                                  Qt::QueuedConnection);
    }
    // if the layer change to floor or orb switch to the previous selected tile of this layer
    else if(mPreviouslyDisabledTileButton == tileButton
                        && enabled && mSelectedTileButton != tileButton)
    {
        QMetaObject::invokeMethod(this, "selectEnabledTileButton",
                                  Qt::QueuedConnection);
    }
    else if(mSelectedTileButton == tileButton && enabled)
    {
        createStamp(mSelectedTileButton);
    }
}

void RTBTileSelectionManager::selectEnabledTileButton()
{

    // Avoid changing tools when it's no longer necessary
    if (mSelectedTileButton && mSelectedTileButton->isEnabled())
        return;

    RTBTileButton *currentTileButton = mSelectedTileButton;

    // Prefer the tool we switched away from last time
    if (mPreviouslyDisabledTileButton && mPreviouslyDisabledTileButton->isEnabled())
    {
        selectTileButton(mPreviouslyDisabledTileButton);
    }
    else
    {
        mStampBrush->setStamp(TileStamp());
        mBucketFillTool->setStamp(TileStamp());
    }

    if(mMapDocument)
    {
        if(mMapDocument->currentLayerIndex() == RTBMapSettings::FloorID)
        {
            mPreviouslyDisabledTileButton = currentTileButton;
        }
    }
}


void RTBTileSelectionManager::setSelectedTileButton(RTBTileButton *tileButton)
{
    if (mSelectedTileButton == tileButton && mStampBrush && mBucketFillTool)
        return;


    if (mSelectedTileButton) {
        disconnect(mSelectedTileButton, SIGNAL(statusInfoChanged(QString)),
                   this, SIGNAL(statusInfoChanged(QString)));
    }

    mSelectedTileButton = tileButton;
    // emit only if the selected tile is new, prevent emit if layer is changed
    if(mPreviouslyDisabledTileButton)
    {
        if(mPreviouslyDisabledTileButton != mSelectedTileButton)
            emit selectedTileChanged(mSelectedTileButton);
    }
    else
        emit selectedTileChanged(mSelectedTileButton);

    if (mSelectedTileButton) {
        emit statusInfoChanged(mSelectedTileButton->statusInfo());
        connect(mSelectedTileButton, SIGNAL(statusInfoChanged(QString)),
                this, SIGNAL(statusInfoChanged(QString)));
    }

    createStamp(tileButton);
}

void RTBTileSelectionManager::createStamp(RTBTileButton *tileButton)
{
    if(!mMapDocument || mMapDocument->map()->tilesetCount() == 0)
        return;

    // create tilelayer with the needed tile
    TileLayer *tileLayer = new TileLayer(QString(), 0, 0, 1, 1);
    Tile *tile = mMapDocument->map()->tilesetAt(0)->tileAt(tileButton->type());
    tileLayer->setCell(0, 0, Cell(tile));

    Map *map = mMapDocument->map();
    Map *stamp = new Map(map->orientation(),
                         32,
                         32,
                         map->tileWidth(),
                         map->tileHeight());
    stamp->addLayer(tileLayer->clone());
    stamp->addTilesets(tileLayer->usedTilesets());

    TileStamp tileStamp(stamp);
    mStampBrush->setStamp(tileStamp);
    mBucketFillTool->setStamp(tileStamp);

    // show tile properties
    mMapDocument->setCurrentObject(tile);

}

void RTBTileSelectionManager::createStamp(int type)
{
    if(!mMapDocument || mMapDocument->map()->tilesetCount() == 0)
        return;

    // create tilelayer with the needed tile
    TileLayer *tileLayer = new TileLayer(QString(), 0, 0, 1, 1);
    Tile *tile = mMapDocument->map()->tilesetAt(0)->tileAt(type);
    tileLayer->setCell(0, 0, Cell(tile));

    Map *map = mMapDocument->map();
    Map *stamp = new Map(map->orientation(),
                         32,
                         32,
                         map->tileWidth(),
                         map->tileHeight());
    stamp->addLayer(tileLayer->clone());
    stamp->addTilesets(tileLayer->usedTilesets());

    TileStamp tileStamp(stamp);
    mStampBrush->setStamp(tileStamp);
    mBucketFillTool->setStamp(tileStamp);

    // show tile properties
    mMapDocument->setCurrentObject(tile);

}


void RTBTileSelectionManager::setSeparatorAction(QAction *separatorAction)
{
    mSeparatorAction = separatorAction;
    mSeparatorAction->setVisible(false);
}

void RTBTileSelectionManager::setBucketFillTool(BucketFillTool *bucketFillTool)
{
    mBucketFillTool = bucketFillTool;
}

void RTBTileSelectionManager::setStampBrush(StampBrush *stampBrush)
{
    mStampBrush = stampBrush;
    connect(mStampBrush, SIGNAL(enabledChanged(bool)),
            this, SLOT(toggleSeparator(bool)));
}

void RTBTileSelectionManager::toggleSeparator(bool visible)
{
    if(mSeparatorAction)
        mSeparatorAction->setVisible(visible);
}

void RTBTileSelectionManager::updateIcon(QAction *action, RTBTileButton *tileButton)
{
    if(!action->isVisible())
        return;

    if(action->isVisible() && !action->isEnabled())
    {
        // change icon only if is necessary
        if(action->icon().pixmap(22, 22).toImage() == tileButton->icon().pixmap(22, 22).toImage())
        {
            QPixmap input = tileButton->icon().pixmap(22, 22);

            QImage image(input.size(), QImage::Format_ARGB32_Premultiplied);
            QPainter p(&image);
            p.setOpacity(0.4);
            p.drawPixmap(0, 0, input);
            p.end();

            QPixmap output = QPixmap::fromImage(image);
            action->setIcon(output);
        }
    }
    // change icon only if is necessary
    else if(action->icon().pixmap(22, 22).toImage() != tileButton->icon().pixmap(22, 22).toImage())
    {
        action->setIcon(tileButton->icon());
    }
}
