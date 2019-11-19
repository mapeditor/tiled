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
    QMultiHash<Id, QAction*> mIdToActions;
    QHash<Id, QMenu*> mIdToMenu;

    QHash<Id, QKeySequence> mDefaultShortcuts;      // for resetting to default
    QHash<Id, QKeySequence> mCustomShortcuts;
    QHash<Id, QKeySequence> mLastKnownShortcuts;    // for detecting shortcut changes

    bool mApplyingShortcut = false;
    bool mResettingShortcut = false;
};

static ActionManager *m_instance;
static ActionManagerPrivate *d;

static void readCustomShortcuts()
{
    const auto settings = Preferences::instance()->settings();
    settings->beginGroup(QLatin1String("CustomShortcuts"));

    const auto keys = settings->childKeys();
    for (const auto &key : keys) {
        auto keySequence = QKeySequence::fromString(settings->value(key).toString());
        d->mCustomShortcuts.insert(Id(key.toUtf8()), keySequence);
    }

    settings->endGroup();
}

static void applyShortcut(QAction *action, const QKeySequence &shortcut)
{
    d->mApplyingShortcut = true;
    action->setShortcut(shortcut);
    d->mApplyingShortcut = false;
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
    Q_ASSERT_X(!d->mIdToActions.contains(id, action), "ActionManager::registerAction", "duplicate action");
    d->mIdToActions.insert(id, action);
    d->mLastKnownShortcuts.insert(id, action->shortcut());

    connect(action, &QAction::changed, m_instance, [id,action] {
        if (!d->mApplyingShortcut && d->mDefaultShortcuts.contains(id) && d->mLastKnownShortcuts.value(id) != action->shortcut()) {
            // Update remembered default shortcut
            d->mDefaultShortcuts.insert(id, action->shortcut());

            // Reset back to user-defined shortcut if set
            if (d->mCustomShortcuts.contains(id)) {
                applyShortcut(action, d->mCustomShortcuts.value(id));
                return;
            }
        }

        d->mLastKnownShortcuts.insert(id, action->shortcut());

        emit m_instance->actionChanged(id);
    });

    if (m_instance->hasCustomShortcut(id)) {
        d->mDefaultShortcuts.insert(id, action->shortcut());
        applyShortcut(action, d->mCustomShortcuts.value(id));
    }

    emit m_instance->actionsChanged();
}

void ActionManager::unregisterAction(QAction *action, Id id)
{
    Q_ASSERT_X(d->mIdToActions.contains(id, action), "ActionManager::unregisterAction", "unknown action");
    d->mIdToActions.remove(id, action);
    action->disconnect(m_instance);
    d->mDefaultShortcuts.remove(id);
    d->mLastKnownShortcuts.remove(id);
    emit m_instance->actionsChanged();
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
    auto action = d->mIdToActions.value(id);
    Q_ASSERT_X(action, "ActionManager::action", "unknown id");
    return action;
}

QAction *ActionManager::findAction(Id id)
{
    return d->mIdToActions.value(id);
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
    return d->mIdToActions.uniqueKeys();
}

QList<Id> ActionManager::menus()
{
    return d->mIdToMenu.keys();
}

void ActionManager::setCustomShortcut(Id id, const QKeySequence &keySequence)
{
    Q_ASSERT(!d->mResettingShortcut);

    const auto actions = d->mIdToActions.values(id);
    Q_ASSERT_X(!actions.isEmpty(), "ActionManager::setCustomShortcut", "unknown id");

    if (!hasCustomShortcut(id))
        d->mDefaultShortcuts.insert(id, actions.first()->shortcut());

    d->mCustomShortcuts.insert(id, keySequence);

    for (QAction *a : actions)
        applyShortcut(a, keySequence);

    auto settings = Preferences::instance()->settings();
    settings->setValue(QLatin1String("CustomShortcuts/") + id.toString(),
                       keySequence.toString());
}

bool ActionManager::hasCustomShortcut(Id id) const
{
    return d->mCustomShortcuts.contains(id);
}

void ActionManager::resetCustomShortcut(Id id)
{
    if (!hasCustomShortcut(id))
        return;

    const auto actions = d->mIdToActions.values(id);
    Q_ASSERT_X(!actions.isEmpty(), "ActionManager::resetCustomShortcut", "unknown id");

    d->mResettingShortcut = true;

    const QKeySequence defaultShortcut = d->mDefaultShortcuts.take(id);
    for (QAction *a : actions)
        applyShortcut(a, defaultShortcut);
    d->mCustomShortcuts.remove(id);

    d->mResettingShortcut = false;

    auto settings = Preferences::instance()->settings();
    settings->remove(QLatin1String("CustomShortcuts/") + id.toString());
}

void ActionManager::resetAllCustomShortcuts()
{
    QHashIterator<Id, QKeySequence> iterator(d->mDefaultShortcuts);
    while (iterator.hasNext()) {
        iterator.next();
        const auto actions = d->mIdToActions.values(iterator.key());
        for (QAction *a : actions)
            applyShortcut(a, iterator.value());
    }
    d->mDefaultShortcuts.clear();
    d->mCustomShortcuts.clear();

    auto settings = Preferences::instance()->settings();
    settings->remove(QLatin1String("CustomShortcuts"));
}

QKeySequence ActionManager::defaultShortcut(Id id)
{
    if (d->mDefaultShortcuts.contains(id))
        return d->mDefaultShortcuts.value(id);
    if (auto a = findAction(id))
        return a->shortcut();
    return QKeySequence();
}

/**
 * Sets the custom shortcuts.
 *
 * Shortcuts that are the same as the default ones will be reset.
 */
void ActionManager::setCustomShortcuts(const QHash<Id, QKeySequence> &shortcuts)
{
    QHashIterator<Id, QKeySequence> iterator(shortcuts);
    while (iterator.hasNext()) {
        iterator.next();

        const Id id = iterator.key();
        const QKeySequence &shortcut = iterator.value();

        if (auto a = findAction(id)) {
            if (d->mDefaultShortcuts.contains(id)
                    ? d->mDefaultShortcuts.value(id) == shortcut
                    : a->shortcut() == shortcut) {
                resetCustomShortcut(id);
            } else {
                setCustomShortcut(id, shortcut);
            }
        }
    }
}

} // namespace Tiled
