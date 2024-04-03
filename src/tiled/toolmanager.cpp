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
#include "mapdocument.h"
#include "preferences.h"

#include <QAction>
#include <QShortcut>
#include <QWidget>

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

    if (mMapDocument)
        mMapDocument->disconnect(this);

    mMapDocument = mapDocument;

    if (mMapDocument) {
        connect(mMapDocument, &MapDocument::currentLayerChanged,
                this, &ToolManager::currentLayerChanged);
    }

    const auto actions = mActionGroup->actions();
    for (QAction *action : actions) {
        AbstractTool *tool = action->data().value<AbstractTool*>();
        tool->setMapDocument(mapDocument);
    }

    currentLayerChanged(mMapDocument ? mMapDocument->currentLayer()
                                     : nullptr);
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
    tool->updateEnabledState();

    QAction *toolAction = new QAction(tool->icon(), tool->name(), this);
    toolAction->setShortcut(tool->shortcut());
    toolAction->setData(QVariant::fromValue<AbstractTool*>(tool));
    toolAction->setCheckable(true);
    toolAction->setText(tool->name());
    toolAction->setEnabled(tool->isEnabled());
    toolAction->setVisible(tool->isVisible());

    mActionGroup->addAction(toolAction);

    connect(tool, &AbstractTool::changed,
            this, &ToolManager::toolChanged);

    connect(tool, &AbstractTool::enabledChanged,
            this, &ToolManager::toolEnabledChanged);
    connect(tool, &AbstractTool::visibleChanged,
            this, [=] (bool visible) { toolAction->setVisible(visible); });

    // Select the first added tool
    if (tool->isVisible()) {
        if (!mSelectedTool && tool->isEnabled()) {
            setSelectedTool(tool);
            toolAction->setChecked(true);
        }

        if (mRegisterActions)
            ActionManager::registerAction(toolAction, tool->id());
    }

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

    QMutableHashIterator<Layer::TypeFlag, AbstractTool *> it(mSelectedToolForLayerType);
    while (it.hasNext())
        if (it.next().value() == tool)
            it.remove();

    if (mSelectedTool == tool)
        setSelectedTool(nullptr);

    autoSwitchTool();
}

/**
 * Selects the given tool. It should be previously added using registerTool().
 *
 * Returns whether the tool was successfully selected.
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
        if (mSelectedTool)
            setSelectedTool(nullptr);

        scheduleAutoSwitchTool();
    }
}

/**
 * Automatically switches to another tool based on the current layer type, or
 * to the first enabled tool.
 *
 * This is done with a delayed call since we first want all the tools to update
 * their enabled state.
 */
void ToolManager::scheduleAutoSwitchTool()
{
    if (mAutoSwitchToolPending)
        return;

    mAutoSwitchToolPending = true;

    QMetaObject::invokeMethod(this, &ToolManager::autoSwitchTool,
                              Qt::QueuedConnection);
}

void ToolManager::autoSwitchTool()
{
    mAutoSwitchToolPending = false;

    // Select the previous tool that was selected for this layer type
    if (mLayerType) {
        const auto layerType = static_cast<Layer::TypeFlag>(mLayerType);
        if (auto tool = mSelectedToolForLayerType.value(layerType)) {
            if (tool->isEnabled()) {
                selectTool(tool);
                return;
            }
        }
    }

    // Avoid changing tools when it's not absolutely necessary
    if (mSelectedTool && mSelectedTool->isEnabled())
        return;

    selectTool(firstEnabledTool());
}

void ToolManager::currentLayerChanged(Layer *layer)
{
    const int layerType = layer ? layer->layerType() : 0;

    if (mLayerType != layerType) {
        // Remember the selected tool for the current layer type
        if (mLayerType && mSelectedTool && !mAutoSwitchToolPending) {
            mSelectedToolForLayerType.insert(static_cast<Layer::TypeFlag>(mLayerType),
                                             mSelectedTool);
        }

        mLayerType = layerType;
        scheduleAutoSwitchTool();
    }

    const auto actions = mActionGroup->actions();
    for (QAction *action : actions) {
        AbstractTool *tool = action->data().value<AbstractTool*>();
        tool->updateEnabledState();
    }
}

AbstractTool *ToolManager::firstEnabledTool() const
{
    const auto actions = mActionGroup->actions();
    for (QAction *action : actions)
        if (AbstractTool *tool = action->data().value<AbstractTool*>())
            if (tool->isEnabled() && tool->isVisible())
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

#include "moc_toolmanager.cpp"
