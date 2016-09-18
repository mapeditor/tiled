/*
 * toolmanager.cpp
 * Copyright 2009-2010, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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

#include "toolmanager.h"

#include "abstracttool.h"
#include "objectselectiontool.h"
#include "stampbrush.h"
#include "bucketfilltool.h"

#include "rtbmapsettings.h"
#include "rtbcreateobjecttool.h"
#include "rtbselectareatool.h"
#include "rtbinserttool.h"

#include <QAction>
#include <QActionGroup>

using namespace Tiled;
using namespace Tiled::Internal;

ToolManager::ToolManager(QObject *parent)
    : QObject(parent)
    , mActionGroup(new QActionGroup(this))
    , mSelectedTool(0)
    , mMapDocument(0)
    , mPreviouslyDisabledFloorTool(0)
    , mPreviouslyDisabledObjectTool(0)
    , mPreviouslyDisabledOrbObjectTool(0)
    , mSeparatorAction(0)
{
    mActionGroup->setExclusive(true);
    connect(mActionGroup, SIGNAL(triggered(QAction*)),
            this, SLOT(actionTriggered(QAction*)));
}

ToolManager::~ToolManager()
{
}

/**
 * Sets the MapDocument on which the registered tools will operate.
 */
void ToolManager::setMapDocument(MapDocument *mapDocument)
{
    if (mMapDocument == mapDocument)
        return;

    if(mMapDocument)
        disconnect(mMapDocument, SIGNAL(currentLayerIndexChanged(int)),
                this, SLOT(updateSelectedTool()));

    mMapDocument = mapDocument;

    foreach (QAction *action, mActionGroup->actions()) {
        AbstractTool *tool = action->data().value<AbstractTool*>();
        tool->setMapDocument(mapDocument);
    }

    if(mMapDocument)
        connect(mapDocument, SIGNAL(currentLayerIndexChanged(int)),
                this, SLOT(updateSelectedTool()));

    resetToolbarActionIcons();
}

/**
 * Registers a new tool. The tool manager does not take ownership over the
 * tool.
 *
 * @return The action for activating the tool.
 */
QAction *ToolManager::registerTool(AbstractTool *tool)
{
    tool->setMapDocument(mMapDocument);

    QAction *toolAction = new QAction(tool->icon(), tool->name(), this);
    toolAction->setShortcut(tool->shortcut());
    toolAction->setData(QVariant::fromValue<AbstractTool*>(tool));
    toolAction->setCheckable(true);

    // if no tooltip is set
    if(tool->shortcut().isEmpty())
    {
        toolAction->setToolTip(
                QString(QLatin1String("%1")).arg(tool->name()));
    }
    else
    {
        toolAction->setToolTip(
                QString(QLatin1String("%1 (%2)")).arg(tool->name(),
                                                      tool->shortcut().toString()));
    }

    toolAction->setEnabled(tool->isEnabled());

    if(dynamic_cast<RTBSelectAreaTool*>(tool))
        toolAction->setVisible(true);
    else if(dynamic_cast<RTBInsertTool*>(tool))
        toolAction->setVisible(false);
    else
        toolAction->setVisible(tool->isEnabled());

    mActionGroup->addAction(toolAction);

    connect(tool, SIGNAL(enabledChanged(bool)),
            this, SLOT(toolEnabledChanged(bool)));

    // Select the first added tool
    if (!mSelectedTool && tool->isEnabled()) {
        setSelectedTool(tool);
        toolAction->setChecked(true);
    }

    if(dynamic_cast<ObjectSelectionTool*>(tool))
        connect(tool, SIGNAL(enabledChanged(bool)),
                this, SLOT(toggleSeparator(bool)));

    return toolAction;
}

/**
 * Selects the given tool. It should be previously added using registerTool().
 */
void ToolManager::selectTool(AbstractTool *tool)
{
    if (tool && !tool->isEnabled()) // Refuse to select disabled tools
        return;

    foreach (QAction *action, mActionGroup->actions()) {
        if (action->data().value<AbstractTool*>() == tool) {
            action->trigger();
            return;
        }
    }

    // The given tool was not found. Don't select any tool.
    foreach (QAction *action, mActionGroup->actions())
        action->setChecked(false);
    setSelectedTool(0);
}

void ToolManager::actionTriggered(QAction *action)
{
    setSelectedTool(action->data().value<AbstractTool*>());
}

void ToolManager::retranslateTools()
{
    // Allow the tools to adapt to the new language
    foreach (QAction *action, mActionGroup->actions()) {
        AbstractTool *tool = action->data().value<AbstractTool*>();
        tool->languageChanged();

        // Update the text, shortcut and tooltip of the action
        action->setText(tool->name());
        action->setShortcut(tool->shortcut());
        action->setToolTip(QString(QLatin1String("%1 (%2)")).arg(
                tool->name(), tool->shortcut().toString()));
    }
}

void ToolManager::toolEnabledChanged(bool enabled)
{
    AbstractTool *tool = qobject_cast<AbstractTool*>(sender());

    foreach (QAction *action, mActionGroup->actions()) {
        if (action->data().value<AbstractTool*>() == tool) {
            action->setEnabled(enabled);

            if(dynamic_cast<RTBSelectAreaTool*>(tool))
                action->setVisible(true);
            else if(dynamic_cast<RTBInsertTool*>(tool))
                action->setVisible(false);
            else
                action->setVisible(tool->isEnabled());

            break;
        }
    }

    // Switch to another tool when the current tool gets disabled. This is done
    // with a delayed call since we first want all the tools to update their
    // enabled state.
    if ((!enabled && tool == mSelectedTool) || (enabled && !mSelectedTool)) {
        QMetaObject::invokeMethod(this, "selectEnabledTool",
                                  Qt::QueuedConnection);
    }
}

void ToolManager::selectEnabledTool()
{
    // Avoid changing tools when it's no longer necessary
    if (mSelectedTool && mSelectedTool->isEnabled())
        return;

    // if no mapdocument stop here
    if(!mMapDocument)
        return;

    // select the previous tool for this layer
    switch (mMapDocument->currentLayerIndex()) {
    case RTBMapSettings::FloorID:
    {
        if (mPreviouslyDisabledFloorTool && mPreviouslyDisabledFloorTool->isEnabled())
            selectTool(mPreviouslyDisabledFloorTool);
        else
            selectTool(0);

        break;
    }
    case RTBMapSettings::ObjectID:
    {
        if (mPreviouslyDisabledObjectTool && mPreviouslyDisabledObjectTool->isEnabled())
            selectTool(mPreviouslyDisabledObjectTool);
        // if no previously disabled tool exists
        else
            selectDefaultObjectTool();

        break;
    }
    case RTBMapSettings::OrbObjectID:
    {
        if (mPreviouslyDisabledOrbObjectTool && mPreviouslyDisabledOrbObjectTool->isEnabled())
            selectTool(mPreviouslyDisabledOrbObjectTool);
        // if no previously disabled tool exists
        else
            selectDefaultObjectTool();

        break;
    }
    default:
        selectTool(0);
        break;
    }

}

void ToolManager::setSelectedTool(AbstractTool *tool)
{
    if (mSelectedTool == tool)
        return;

    if (mSelectedTool) {
        disconnect(mSelectedTool, SIGNAL(statusInfoChanged(QString)),
                   this, SIGNAL(statusInfoChanged(QString)));
    }

    mSelectedTool = tool;
    emit selectedToolChanged(mSelectedTool);

    if (mSelectedTool) {
        emit statusInfoChanged(mSelectedTool->statusInfo());
        connect(mSelectedTool, SIGNAL(statusInfoChanged(QString)),
                this, SIGNAL(statusInfoChanged(QString)));
    }

    // if no mapdocument stop here
    if(!mMapDocument)
        return;

    // store the previous tool for this layer
    switch (mMapDocument->currentLayerIndex()) {
    case RTBMapSettings::FloorID:
    {
        mPreviouslyDisabledFloorTool = mSelectedTool;

        break;
    }
    case RTBMapSettings::ObjectID:
    {
        mPreviouslyDisabledObjectTool = mSelectedTool;
        break;
    }
    case RTBMapSettings::OrbObjectID:
    {
        mPreviouslyDisabledOrbObjectTool = mSelectedTool;
        break;
    }
    }
}

void ToolManager::selectDefaultTool()
{
    if(mMapDocument->currentLayerIndex() == RTBMapSettings::FloorID)
        selectDefaultTileTool();
    else
        selectDefaultObjectTool();
}

void ToolManager::selectDefaultTileTool()
{
    if(dynamic_cast<RTBInsertTool*>(mSelectedTool) ||
            (!dynamic_cast<StampBrush*>(mSelectedTool) && !dynamic_cast<BucketFillTool*>(mSelectedTool)))
    {
        QAction *stampBrushAction = mActionGroup->actions().at(RTBMapSettings::StampBrush);
        AbstractTool *stampBrushTool = stampBrushAction->data().value<AbstractTool*>();

        // if possible select stamp brush tool
        if(stampBrushTool->isEnabled())
           selectTool(stampBrushTool);
    }
}

void ToolManager::selectDefaultObjectTool()
{
    if(!dynamic_cast<ObjectSelectionTool*>(mSelectedTool))
    {
        QAction *selectionAction = mActionGroup->actions().at(RTBMapSettings::ObjectSelection);
        AbstractTool *selectionTool = selectionAction->data().value<AbstractTool*>();

        // if possible select stamp brush tool
        if(selectionTool->isEnabled())
           selectTool(selectionTool);
        else
            selectTool(0);
    }
}

void ToolManager::highlightToolbarAction(int id)
{
    QAction *createObjectAction = mActionGroup->actions().at(id - 35);
    AbstractTool *createObjectTool = createObjectAction->data().value<AbstractTool*>();

    if(RTBCreateObjectTool *tool = dynamic_cast<RTBCreateObjectTool*>(createObjectTool))
    {
        if(!tool->errorIcon().isNull())
            createObjectAction->setIcon(tool->errorIcon());
    }

}

void ToolManager::activateToolbarAction(int id)
{
    QAction *createObjectAction = mActionGroup->actions().at(id - 35);
    AbstractTool *createObjectTool = createObjectAction->data().value<AbstractTool*>();

    if(createObjectTool->isEnabled())
        selectTool(createObjectTool);
}

void ToolManager::resetToolbarActionIcons()
{
    foreach (QAction *action, mActionGroup->actions()) {
        AbstractTool *tool = action->data().value<AbstractTool*>();

        QIcon actionIcon = action->icon();
        QIcon toolIcon = tool->icon();
        // change icon only if is necessary
        if(actionIcon.pixmap(22, 22).toImage() != toolIcon.pixmap(22, 22).toImage())
        {
            action->setIcon(tool->icon());
        }
    }

}

void ToolManager::setSeparatorAction(QAction *separatorAction)
{
    mSeparatorAction = separatorAction;
    mSeparatorAction->setVisible(false);
}

void ToolManager::toggleSeparator(bool visible)
{
    if(mSeparatorAction)
        mSeparatorAction->setVisible(visible);
}

void ToolManager::updateSelectedTool()
{
    // select the previous tool for this layer
    switch (mMapDocument->currentLayerIndex()) {
    case RTBMapSettings::FloorID:
    {
        return;
    }
    case RTBMapSettings::ObjectID:
    {
        if (mPreviouslyDisabledObjectTool && mPreviouslyDisabledObjectTool->isEnabled())
            selectTool(mPreviouslyDisabledObjectTool);
        // if no previously disabled tool exists
        else
            selectDefaultObjectTool();

        break;
    }
    case RTBMapSettings::OrbObjectID:
    {
        if (mPreviouslyDisabledOrbObjectTool && mPreviouslyDisabledOrbObjectTool->isEnabled())
            selectTool(mPreviouslyDisabledOrbObjectTool);
        // if no previously disabled tool exists
        else
            selectDefaultObjectTool();

        break;
    }
    default:
        selectTool(0);
        break;
    }
}
