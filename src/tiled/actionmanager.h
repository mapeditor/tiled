/*
 * actionmanager.h
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

#pragma once

#include "id.h"

#include <QObject>

class QAction;
class QMenu;

namespace Tiled {

class MainWindow;

/**
 * Manager of global actions.
 */
class ActionManager : public QObject
{
    Q_OBJECT

public:
    static ActionManager *instance();

    static void registerAction(QAction *action, Id id);
    static void unregisterAction(QAction *action, Id id);

    static void registerMenu(QMenu *menu, Id id);
    static void unregisterMenu(Id id);

    static QAction *action(Id id);
    static QAction *findAction(Id id);

    static QMenu *menu(Id id);
    static QMenu *findMenu(Id id);

    static QList<Id> actions();
    static QList<Id> menus();

    void setCustomShortcut(Id id, const QKeySequence &keySequence);
    bool hasCustomShortcut(Id id) const;
    void resetCustomShortcut(Id id);
    void resetAllCustomShortcuts();
    QKeySequence defaultShortcut(Id id);

    void setCustomShortcuts(const QHash<Id, QKeySequence> &shortcuts);

signals:
    void actionChanged(Id id);
    void actionsChanged();

private:
    explicit ActionManager(QObject *parent = nullptr);
    ~ActionManager();

    friend class Tiled::MainWindow;   // creation
};

} // namespace Tiled
