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
#include "actionmanager.h"
#include "preferences.h"

#include <QAction>
#include <QShortcut>

using namespace Tiled;

ToolManager::ToolManager(QObject *parent)
    : QObject(parent)
    , mActionGroup(new QActionGroup(this))
{
    mActionGroup->setExclusive(true);
    connect(mActionGroup, &QActionGroup::triggered,
            this, &ToolManager::actionTriggered);

    connect(Preferences::instance(), &Preferences::languageChanged,
            this, &ToolManager::retranslateTools);
}

ToolManager::~ToolManager()
{
}

/**
 * Can be used to disable the registration of actions with the ActionManager.
 * Should be called before any tools are registered.
 */
void ToolManager::setRegisterActions(bool enabled)
{
    mRegisterActions = enabled;
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

    QString toolTip = tool->name();
    QKeySequence shortcut = tool->shortcut();
    if (!shortcut.isEmpty()) {
        toolTip = QString(QLatin1String("%1 (%2)")).arg(toolTip,
                                                        shortcut.toString());
    }

    QAction *toolAction = new QAction(tool->icon(), tool->name(), this);
    toolAction->setShortcut(shortcut);
    toolAction->setData(QVariant::fromValue<AbstractTool*>(tool));
    toolAction->setCheckable(true);
    toolAction->setToolTip(toolTip);
    toolAction->setEnabled(tool->isEnabled());

    mActionGroup->addAction(toolAction);

    connect(tool, &AbstractTool::changed,
            this, &ToolManager::toolChanged);

    connect(toolAction, &QAction::changed,
            this, &ToolManager::toolActionChanged);

    connect(tool, &AbstractTool::enabledChanged,
            this, &ToolManager::toolEnabledChanged);

    // Select the first added tool
    if (!mSelectedTool && tool->isEnabled()) {
        setSelectedTool(tool);
        toolAction->setChecked(true);
    }

    if (mRegisterActions)
        ActionManager::registerAction(toolAction, tool->id());

    return toolAction;
}

void ToolManager::unregisterTool(AbstractTool *tool)
{
    auto action = findAction(tool);
    Q_ASSERT(action);

    if (mRegisterActions)
        ActionManager::unregisterAction(action, tool->id());

    delete action;

    tool->disconnect(this);

    if (mDisabledTool == tool)
        mDisabledTool = nullptr;
    if (mPreviouslyDisabledTool == tool)
        mPreviouslyDisabledTool = nullptr;
    if (mSelectedTool == tool)
        mSelectedTool = nullptr;

    selectEnabledTool();
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

QAction *ToolManager::findAction(AbstractTool *tool) const
{
    const auto actions = mActionGroup->actions();
    for (QAction *action : actions) {
        if (action->data().value<AbstractTool*>() == tool)
            return action;
    }
    return nullptr;
}

void ToolManager::actionTriggered(QAction *action)
{
    setSelectedTool(action->data().value<AbstractTool*>());
}

void ToolManager::toolChanged()
{
    auto tool = static_cast<AbstractTool*>(sender());

    if (auto action = findAction(tool)) {
        action->setText(tool->name());
        action->setIcon(tool->icon());
        action->setShortcut(tool->shortcut());
    }
}

void ToolManager::toolActionChanged()
{
    if (mUpdatingActionToolTip)
        return;

    auto action = static_cast<QAction*>(sender());

    QString toolTip = action->text();
    QKeySequence shortcut = action->shortcut();

    if (!shortcut.isEmpty()) {
        toolTip = QString(QLatin1String("%1 (%2)")).arg(toolTip,
                                                        shortcut.toString());
    }

    mUpdatingActionToolTip = true;
    action->setToolTip(toolTip);
    mUpdatingActionToolTip = false;
}

void ToolManager::retranslateTools()
{
    // Allow the tools to adapt to the new language
    const auto actions = mActionGroup->actions();
    for (QAction *action : actions) {
        AbstractTool *tool = action->data().value<AbstractTool*>();
        tool->languageChanged();

        action->setText(tool->name());
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
            connect(action, &QAction::changed, shortcut, [=] {
                shortcut->setKey(action->shortcut());
                shortcut->setEnabled(action->isEnabled());
            });

            connect(shortcut, &QShortcut::activated, action, &QAction::trigger);

            // Limit the context of the shortcut to avoid ambiguous overloads
            action->setShortcutContext(Qt::WidgetShortcut);
        }
    }
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
#if QT_VERSION < QT_VERSION_CHECK(5, 10, 0)
            QMetaObject::invokeMethod(this, "selectEnabledTool",
                                      Qt::QueuedConnection);
#else
            QMetaObject::invokeMethod(this, &ToolManager::selectEnabledTool,
                                      Qt::QueuedConnection);
#endif
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
        disconnect(mSelectedTool, &AbstractTool::statusInfoChanged,
                   this, &ToolManager::statusInfoChanged);
    }

    mSelectedTool = tool;
    emit selectedToolChanged(mSelectedTool);

    if (mSelectedTool) {
        emit statusInfoChanged(mSelectedTool->statusInfo());
        connect(mSelectedTool, &AbstractTool::statusInfoChanged,
                this, &ToolManager::statusInfoChanged);
    }
}
