/*
 * toolmanager.h
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

#pragma once

#include <QActionGroup>
#include <QObject>

class QAction;

namespace Tiled {

class Tile;
class ObjectTemplate;

class AbstractTool;
class MapDocument;

/**
 * The tool manager provides a central place to register editing tools.
 *
 * It creates actions for the tools that can be placed on a tool bar. All the
 * actions are put into a QActionGroup to make sure only one tool can be
 * selected at a time.
 */
class ToolManager : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(ToolManager)

public:
    ToolManager(QObject *parent = nullptr);
    ~ToolManager() override;

    void setRegisterActions(bool enabled);

    void setMapDocument(MapDocument *mapDocument);

    QAction *registerTool(AbstractTool *tool);
    void unregisterTool(AbstractTool *tool);

    bool selectTool(AbstractTool *tool);
    AbstractTool *selectedTool() const;

    template<typename Tool>
    Tool *findTool();

    QAction *findAction(AbstractTool *tool) const;

    void retranslateTools();

    void createShortcuts(QWidget *parent);

    Tile *tile() const;
    ObjectTemplate *objectTemplate() const;

public slots:
    /**
     * Sets the tile that will be used when the creation mode is
     * CreateTileObjects or when replacing a tile of a tile object.
     */
    void setTile(Tile *tile);
    void setObjectTemplate(ObjectTemplate *objectTemplate);

signals:
    void selectedToolChanged(AbstractTool *tool);

    /**
     * Emitted when the status information of the current tool changed.
     * @see AbstractTool::setStatusInfo()
     */
    void statusInfoChanged(const QString &info);

private slots:
    void actionTriggered(QAction *action);
    void toolChanged();
    void toolActionChanged();
    void toolEnabledChanged(bool enabled);
    void selectEnabledTool();

private:
    AbstractTool *firstEnabledTool() const;
    void setSelectedTool(AbstractTool *tool);

    QActionGroup *mActionGroup;
    AbstractTool *mSelectedTool = nullptr;
    AbstractTool *mDisabledTool = nullptr;
    AbstractTool *mPreviouslyDisabledTool = nullptr;
    MapDocument *mMapDocument = nullptr;
    Tile *mTile = nullptr;
    ObjectTemplate *mObjectTemplate = nullptr;

    bool mRegisterActions = true;
    bool mSelectEnabledToolPending = false;
    bool mUpdatingActionToolTip = false;
};

/**
 * Selects the tool that matches the specified type.
 */
template<class Tool>
Tool *ToolManager::findTool()
{
    const auto actions = mActionGroup->actions();
    for (QAction *action : actions) {
        AbstractTool *abstractTool = action->data().value<AbstractTool*>();
        if (Tool *tool = qobject_cast<Tool*>(abstractTool))
            return tool;
    }
    return nullptr;
}

/**
 * Returns the selected tool.
 */
inline AbstractTool *ToolManager::selectedTool() const
{
    return mSelectedTool;
}

inline Tile *ToolManager::tile() const
{
    return mTile;
}

inline ObjectTemplate *ToolManager::objectTemplate() const
{
    return mObjectTemplate;
}

inline void ToolManager::setTile(Tile *tile)
{
    mTile = tile;
}

inline void ToolManager::setObjectTemplate(ObjectTemplate *objectTemplate)
{
    mObjectTemplate = objectTemplate;
}

} // namespace Tiled
