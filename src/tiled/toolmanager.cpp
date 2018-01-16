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
#include <QShortcut>

using namespace Tiled;
using namespace Tiled::Internal;

ToolManager::ToolManager(QObject *parent)
    : QObject(parent)
    , mActionGroup(new QActionGroup(this))
    , mSelectedTool(nullptr)
    , mDisabledTool(nullptr)
    , mPreviouslyDisabledTool(nullptr)
    , mMapDocument(nullptr)
    , mTile(nullptr)
    , mObjectTemplate(nullptr)
    , mSelectEnabledToolPending(false)
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

    mMapDocument = mapDocument;

    const auto actions = mActionGroup->actions();
    for (QAction *action : actions) {
        AbstractTool *tool = action->data().value<AbstractTool*>();
        tool->setMapDocument(mapDocument);
    }
}

/**
 * Registers a new tool. The tool manager does not take ownership over the
 * tool.
 *
 * @return The action for activating the tool.
 */
QAction *ToolManager::registerTool(AbstractTool *tool)
{
    Q_ASSERT(!tool->mToolManager);
    tool->mToolManager = this;

    tool->setMapDocument(mMapDocument);

    QAction *toolAction = new QAction(tool->icon(), tool->name(), this);
    toolAction->setShortcut(tool->shortcut());
    toolAction->setData(QVariant::fromValue<AbstractTool*>(tool));
    toolAction->setCheckable(true);
    if (!tool->shortcut().isEmpty()) {
        toolAction->setToolTip(
                QString(QLatin1String("%1 (%2)")).arg(tool->name(),
                                                      tool->shortcut().toString()));
    } else {
        toolAction->setToolTip(tool->name());
    }

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

/**
 * Selects the given tool. It should be previously added using registerTool().
 *
 * Returns whether the tool was succesfully selected.
 */
bool ToolManager::selectTool(AbstractTool *tool)
{
    if (mSelectedTool == tool)
        return true;

    if (tool && !tool->isEnabled()) // Refuse to select disabled tools
        return false;

    const auto actions = mActionGroup->actions();
    for (QAction *action : actions) {
        if (action->data().value<AbstractTool*>() == tool) {
            action->trigger();
            return true;
        }
    }

    // The given tool was not found. Don't select any tool.
    for (QAction *action : actions)
        action->setChecked(false);
    setSelectedTool(nullptr);
    return tool == nullptr;
}

void ToolManager::actionTriggered(QAction *action)
{
    setSelectedTool(action->data().value<AbstractTool*>());
}

void ToolManager::retranslateTools()
{
    // Allow the tools to adapt to the new language
    const auto actions = mActionGroup->actions();
    for (QAction *action : actions) {
        AbstractTool *tool = action->data().value<AbstractTool*>();
        tool->languageChanged();

        // Update the text, shortcut and tooltip of the action
        action->setText(tool->name());
        action->setShortcut(tool->shortcut());
        action->setToolTip(QString(QLatin1String("%1 (%2)")).arg(
                tool->name(), tool->shortcut().toString()));
    }
}

/**
 * Replaces the shortcuts set on the actions with QShortcut instances, using
 * \a parent as their parent.
 *
 * This is done to make sure the shortcuts can still be used even when the
 * actions are only added to a tool bar and this tool bar is hidden.
 */
void ToolManager::createShortcuts(QWidget *parent)
{
    const auto actions = mActionGroup->actions();
    for (QAction *action : actions) {
        QKeySequence key = action->shortcut();

        if (!key.isEmpty()) {
            auto shortcut = new QShortcut(key, parent);

            // Make sure the shortcut is only enabled when the action is,
            // because different tools may use the same shortcut.
            shortcut->setEnabled(action->isEnabled());
            connect(action, &QAction::changed, shortcut, [=]() {
                shortcut->setEnabled(action->isEnabled());
            });

            connect(shortcut, &QShortcut::activated, action, &QAction::trigger);

            // Unset the shortcut from the action to avoid ambiguous overloads
            action->setShortcut(QKeySequence());
        }
    }
}

void ToolManager::setTile(Tile *tile)
{
    mTile = tile;
    if (mSelectedTool)
        mSelectedTool->setTile(mTile);
}

void ToolManager::setObjectTemplate(ObjectTemplate *objectTemplate)
{
    mObjectTemplate = objectTemplate;
    if (mSelectedTool)
        mSelectedTool->setObjectTemplate(mObjectTemplate);
}

void ToolManager::toolEnabledChanged(bool enabled)
{
    AbstractTool *tool = qobject_cast<AbstractTool*>(sender());

    const auto actions = mActionGroup->actions();
    for (QAction *action : actions) {
        if (action->data().value<AbstractTool*>() == tool) {
            action->setEnabled(enabled);
            break;
        }
    }

    if ((!enabled && tool == mSelectedTool) || (enabled && !mSelectedTool)) {
        if (mSelectedTool) {
            mDisabledTool = mSelectedTool;
            setSelectedTool(nullptr);
        }

        // Automatically switch to another enabled tool when the current tool
        // gets disabled. This is done with a delayed call since we first want
        // all the tools to update their enabled state.
        if (!mSelectEnabledToolPending) {
            mSelectEnabledToolPending = true;
            QMetaObject::invokeMethod(this, "selectEnabledTool",
                                      Qt::QueuedConnection);
        }
    }
}

void ToolManager::selectEnabledTool()
{
    mSelectEnabledToolPending = false;

    // Avoid changing tools when it's no longer necessary
    if (mSelectedTool && mSelectedTool->isEnabled())
        return;

    // Prefer the tool we switched away from last time
    if (mPreviouslyDisabledTool && mPreviouslyDisabledTool->isEnabled())
        selectTool(mPreviouslyDisabledTool);
    else
        selectTool(firstEnabledTool());

    mPreviouslyDisabledTool = mDisabledTool;
}

AbstractTool *ToolManager::firstEnabledTool() const
{
    const auto actions = mActionGroup->actions();
    for (QAction *action : actions)
        if (AbstractTool *tool = action->data().value<AbstractTool*>())
            if (tool->isEnabled())
                return tool;

    return nullptr;
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
        tool->setTile(mTile);
        tool->setObjectTemplate(mObjectTemplate);
    }
}
