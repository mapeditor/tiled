/*
 * actionmanager.cpp
 * Copyright 2016, Thorbjørn Lindeijer <bjorn@lindeijer.nl>
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

#include "actionmanager.h"

#include <QHash>
#include <QMenu>

namespace Tiled {

class ActionManagerPrivate
{
public:
    QHash<Id, QAction*> mIdToAction;
    QHash<Id, QMenu*> mIdToMenu;
};

static ActionManager *m_instance = nullptr;
static ActionManagerPrivate *d;


ActionManager::ActionManager(QObject *parent)
    : QObject(parent)
{
    m_instance = this;
    d = new ActionManagerPrivate;
}

ActionManager::~ActionManager()
{
    delete d;
}

void ActionManager::registerAction(QAction *action, Id id)
{
    Q_ASSERT_X(!d->mIdToAction.contains(id), "ActionManager::registerAction", "duplicate id");
    d->mIdToAction.insert(id, action);
}

void ActionManager::unregisterAction(Id id)
{
    Q_ASSERT_X(d->mIdToAction.contains(id), "ActionManager::unregisterAction", "unknown id");
    d->mIdToAction.remove(id);
}

void ActionManager::registerMenu(QMenu *menu, Id id)
{
    Q_ASSERT_X(!d->mIdToMenu.contains(id), "ActionManager::registerMenu", "duplicate id");
    d->mIdToMenu.insert(id, menu);
}

void ActionManager::unregisterMenu(Id id)
{
    Q_ASSERT_X(d->mIdToMenu.contains(id), "ActionManager::unregisterMenu", "unknown id");
    d->mIdToMenu.remove(id);
}

QAction *ActionManager::action(Id id)
{
    auto action = d->mIdToAction.value(id);
    Q_ASSERT_X(action, "ActionManager::action", "unknown id");
    return action;
}

QAction *ActionManager::findAction(Id id)
{
    return d->mIdToAction.value(id);
}

QMenu *ActionManager::menu(Id id)
{
    auto menu = d->mIdToMenu.value(id);
    Q_ASSERT_X(menu, "ActionManager::menu", "unknown id");
    return menu;
}

QMenu *ActionManager::findMenu(Id id)
{
    return d->mIdToMenu.value(id);
}

QList<Id> ActionManager::actions()
{
    return d->mIdToAction.keys();
}

QList<Id> ActionManager::menus()
{
    return d->mIdToMenu.keys();
}
} // namespace Tiled
