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

#include <QHash>
#include <QObject>
#include <QVector>

#include <memory>

class QAction;
class QMenu;

namespace Tiled {

class MainWindow;

namespace MenuIds {

constexpr char layerViewLayers[] = "LayerView.Layers";
constexpr char mapViewObjects[] = "MapView.Objects";
constexpr char projectViewFiles[] = "ProjectView.Files";
constexpr char propertiesViewProperties[] = "PropertiesView.Properties";
constexpr char tilesetViewTiles[] = "TilesetView.Tiles";

} // namespace MenuId

/**
 * Manager of global actions.
 */
class ActionManager : public QObject
{
    Q_OBJECT

    explicit ActionManager(QObject *parent = nullptr);
    ~ActionManager() override;

public:
    struct MenuItem {
        Id action;
        Id beforeAction;
        bool isSeparator;
    };

    struct MenuExtension {
        QVector<MenuItem> items;
    };

    static ActionManager *instance();

    static void registerAction(QAction *action, Id id);
    static void unregisterAction(QAction *action, Id id);

    static void registerMenu(QMenu *menu, Id id);
    static void unregisterMenu(Id id);

    static void registerMenuExtension(Id id, MenuExtension extension);
    static void applyMenuExtensions(QMenu *menu, Id id);
    static void clearMenuExtensions();

    static QAction *action(Id id);
    static QAction *findAction(Id id);
    static QAction *findEnabledAction(Id id);

    static bool hasMenu(Id id);

    static QList<Id> actions();
    static QList<Id> menus();

    void setCustomShortcut(Id id, const QKeySequence &keySequence);
    bool hasCustomShortcut(Id id) const;
    void resetCustomShortcut(Id id);
    void resetAllCustomShortcuts();
    QList<QKeySequence> defaultShortcuts(Id id) const;

    void setCustomShortcuts(const QHash<Id, QList<QKeySequence>> &shortcuts);

signals:
    void actionChanged(Id id);
    void actionsChanged();

private:
    void readCustomShortcuts();
    void applyShortcut(QAction *action, const QKeySequence &shortcut);
    void applyShortcuts(QAction *action, const QList<QKeySequence> &shortcuts);
    void updateToolTipWithShortcut(QAction *action);
    void applyMenuExtension(QMenu *menu, const MenuExtension &extension);

    QMultiHash<Id, QAction*> mIdToActions;
    QHash<Id, QMenu*> mIdToMenu;
    QHash<Id, QVector<MenuExtension>> mIdToMenuExtensions;
    std::unique_ptr<QObject> mMenuSeparatorsParent;

    QHash<Id, QList<QKeySequence>> mDefaultShortcuts;   // for resetting to default
    QHash<Id, QKeySequence> mCustomShortcuts;
    QHash<Id, QList<QKeySequence>> mLastKnownShortcuts; // for detecting shortcut changes

    bool mApplyingShortcut = false;
    bool mApplyingToolTipWithShortcut = false;
    bool mResettingShortcut = false;
};

} // namespace Tiled
