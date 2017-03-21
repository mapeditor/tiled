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

namespace Tiled {
namespace Internal {

class ActionManagerPrivate
{
public:
    QHash<Id, QAction *> mIdToAction;
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

QAction *ActionManager::action(Id id)
{
    auto act = d->mIdToAction.value(id);
    Q_ASSERT_X(act, "ActionManager::action", "unknown id");
    return act;
}

} // namespace Internal
} // namespace Tiled
