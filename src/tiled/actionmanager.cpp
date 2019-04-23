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

#include "preferences.h"

#include <QHash>
#include <QMenu>
#include <QSettings>

namespace Tiled {

class ActionManagerPrivate
{
public:
    QHash<Id, QAction*> mIdToAction;
    QHash<Id, QMenu*> mIdToMenu;

    QHash<Id, QKeySequence> mDefaultShortcuts;
    QHash<Id, QKeySequence> mCustomShortcuts;
};

static ActionManager *m_instance;
static ActionManagerPrivate *d;

static void saveCustomShortcuts()
{
    auto settings = Preferences::instance()->settings();
    settings->beginGroup(QLatin1String("CustomShortcuts"));

    QHashIterator<Id, QKeySequence> iterator(d->mCustomShortcuts);
    while (iterator.hasNext()) {
        iterator.next();
        settings->setValue(QString::fromLatin1(iterator.key().name()),
                           iterator.value().toString());
    }

    settings->endGroup();
}

static void readCustomShortcuts()
{
    const auto settings = Preferences::instance()->settings();
    settings->beginGroup(QLatin1String("CustomShortcuts"));

    const auto keys = settings->childKeys();
    for (const auto &key : keys) {
        auto keySequence = QKeySequence(settings->value(key).toString(),
                                        QKeySequence::PortableText);

        d->mCustomShortcuts.insert(Id(key.toUtf8()), keySequence);
    }

    settings->endGroup();
}


ActionManager::ActionManager(QObject *parent)
    : QObject(parent)
{
    Q_ASSERT(!m_instance);
    m_instance = this;
    d = new ActionManagerPrivate;

    readCustomShortcuts();
}

ActionManager::~ActionManager()
{
    m_instance = nullptr;
    delete d;
    d = nullptr;
}

ActionManager *ActionManager::instance()
{
    Q_ASSERT(m_instance);
    return m_instance;
}

void ActionManager::registerAction(QAction *action, Id id)
{
    Q_ASSERT_X(!d->mIdToAction.contains(id), "ActionManager::registerAction", "duplicate id");
    d->mIdToAction.insert(id, action);

    if (m_instance->hasCustomShortcut(id)) {
        d->mDefaultShortcuts.insert(id, action->shortcut());
        action->setShortcut(d->mCustomShortcuts.value(id));
    }
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

void ActionManager::setCustomShortcut(Id id, const QKeySequence &keySequence)
{
    auto a = action(id);

    if (!hasCustomShortcut(id))
        d->mDefaultShortcuts.insert(id, a->shortcut());

    d->mCustomShortcuts.insert(id, keySequence);
    a->setShortcut(keySequence);

    saveCustomShortcuts();
}

bool ActionManager::hasCustomShortcut(Id id) const
{
    return d->mCustomShortcuts.contains(id);
}

void ActionManager::resetCustomShortcut(Id id)
{
    if (hasCustomShortcut(id)) {
        auto a = action(id);
        a->setShortcut(d->mDefaultShortcuts.take(id));
        d->mCustomShortcuts.remove(id);
    }

    saveCustomShortcuts();
}

void ActionManager::resetAllCustomShortcuts()
{
    QHashIterator<Id, QKeySequence> iterator(d->mDefaultShortcuts);
    while (iterator.hasNext()) {
        iterator.next();
        if (auto a = findAction(iterator.key()))
            a->setShortcut(iterator.value());
    }
    d->mDefaultShortcuts.clear();
    d->mCustomShortcuts.clear();

    saveCustomShortcuts();
}

} // namespace Tiled
