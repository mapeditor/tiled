/*
 * Tiled Map Editor (Qt)
 * Copyright 2009 Tiled (Qt) developers (see AUTHORS file)
 *
 * This file is part of Tiled (Qt).
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
 * this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 * Place, Suite 330, Boston, MA 02111-1307, USA.
 */

#include "toolmanager.h"

#include "abstracttool.h"

#include <QAction>
#include <QActionGroup>
#include <QToolBar>

using namespace Tiled;
using namespace Tiled::Internal;

ToolManager *ToolManager::mInstance = 0;

ToolManager *ToolManager::instance()
{
    if (!mInstance)
        mInstance = new ToolManager;
    return mInstance;
}

void ToolManager::deleteInstance()
{
    delete mInstance;
    mInstance = 0;
}

ToolManager::ToolManager()
    : mToolBar(new QToolBar)
    , mActionGroup(new QActionGroup(this))
    , mSelectedTool(0)
{
    mToolBar->setObjectName(QLatin1String("toolsToolBar"));
    mToolBar->setWindowTitle(QObject::tr("Tools"));

    mActionGroup->setExclusive(true);
    connect(mActionGroup, SIGNAL(triggered(QAction*)),
            this, SLOT(actionTriggered(QAction*)));
}

ToolManager::~ToolManager()
{
    delete mToolBar;
}

void ToolManager::registerTool(AbstractTool *tool)
{
    QAction *toolAction = new QAction(tool->icon(), tool->name(), this);
    toolAction->setShortcut(tool->shortcut());
    toolAction->setData(QVariant::fromValue<AbstractTool*>(tool));
    toolAction->setCheckable(true);
    toolAction->setToolTip(
            QString(QLatin1String("%1 (%2)")).arg(tool->name(),
                                                  tool->shortcut().toString()));
    mActionGroup->addAction(toolAction);
    mToolBar->addAction(toolAction);

    // Select the first added tool
    if (!mSelectedTool) {
        setSelectedTool(tool);
        toolAction->setChecked(true);
    }
}

void ToolManager::selectTool(AbstractTool *tool)
{
    foreach (QAction *action, mActionGroup->actions()) {
        if (action->data().value<AbstractTool*>() == tool) {
            action->trigger();
            break;
        }
    }
}

void ToolManager::actionTriggered(QAction *action)
{
    setSelectedTool(action->data().value<AbstractTool*>());
}

void ToolManager::setSelectedTool(AbstractTool *tool)
{
    if (mSelectedTool == tool)
        return;

    if (mSelectedTool)
        mSelectedTool->disconnect(this);

    mSelectedTool = tool;
    emit selectedToolChanged(mSelectedTool);

    if (mSelectedTool) {
        emit statusInfoChanged(mSelectedTool->statusInfo());
        connect(mSelectedTool, SIGNAL(statusInfoChanged(QString)),
                this, SIGNAL(statusInfoChanged(QString)));
    }
}
