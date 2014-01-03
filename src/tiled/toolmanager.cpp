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

#include <QAction>
#include <QActionGroup>

using namespace Tiled;
using namespace Tiled::Internal;

ToolManager::ToolManager(QObject *parent)
    : QObject(parent)
    , mActionGroup(new QActionGroup(this))
    , mSelectedTool(0)
    , mPreviouslyDisabledTool(0)
    , mMapDocument(0)
{
    mActionGroup->setExclusive(true);
    connect(mActionGroup, SIGNAL(triggered(QAction*)),
            this, SLOT(actionTriggered(QAction*)));
}

ToolManager::~ToolManager()
{
}

void ToolManager::setMapDocument(MapDocument *mapDocument)
{
    if (mMapDocument == mapDocument)
        return;

    mMapDocument = mapDocument;

    foreach (QAction *action, mActionGroup->actions()) {
        AbstractTool *tool = action->data().value<AbstractTool*>();
        tool->setMapDocument(mapDocument);
    }
}

QAction *ToolManager::registerTool(AbstractTool *tool)
{
    tool->setMapDocument(mMapDocument);

    QAction *toolAction = new QAction(tool->icon(), tool->name(), this);
    toolAction->setShortcut(tool->shortcut());
    toolAction->setData(QVariant::fromValue<AbstractTool*>(tool));
    toolAction->setCheckable(true);
    toolAction->setToolTip(
            QString(QLatin1String("%1 (%2)")).arg(tool->name(),
                                                  tool->shortcut().toString()));
    toolAction->setEnabled(tool->isEnabled());
    mActionGroup->addAction(toolAction);

    connect(tool, SIGNAL(enabledChanged(bool)),
            this, SLOT(toolEnabledChanged(bool)));

    // Select the first added tool
    if (!mSelectedTool && tool->isEnabled()) {
        setSelectedTool(tool);
        toolAction->setChecked(true);
    }

    return toolAction;
}

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

    AbstractTool *currentTool = mSelectedTool;

    // Prefer the tool we switched away from last time
    if (mPreviouslyDisabledTool && mPreviouslyDisabledTool->isEnabled())
        selectTool(mPreviouslyDisabledTool);
    else
        selectTool(firstEnabledTool());

    mPreviouslyDisabledTool = currentTool;
}

AbstractTool *ToolManager::firstEnabledTool() const
{
    foreach (QAction *action, mActionGroup->actions())
        if (AbstractTool *tool = action->data().value<AbstractTool*>())
            if (tool->isEnabled())
                return tool;

    return 0;
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
}
