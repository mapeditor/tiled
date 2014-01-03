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

#ifndef TOOLMANAGER_H
#define TOOLMANAGER_H

#include <QObject>

class QAction;
class QActionGroup;

namespace Tiled {
namespace Internal {

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

public:
    ToolManager(QObject *parent = 0);
    ~ToolManager();

    /**
     * Sets the MapDocument on which the registered tools will operate.
     */
    void setMapDocument(MapDocument *mapDocument);

    /**
     * Registers a new tool. The tool manager does not take ownership over the
     * tool.
     *
     * @return The action for activating the tool.
     */
    QAction *registerTool(AbstractTool *tool);

    /**
     * Selects the given tool. It should be previously added using
     * registerTool().
     */
    void selectTool(AbstractTool *tool);

    /**
     * Returns the selected tool.
     */
    AbstractTool *selectedTool() const { return mSelectedTool; }

    void retranslateTools();

signals:
    void selectedToolChanged(AbstractTool *tool);

    /**
     * Emitted when the status information of the current tool changed.
     * @see AbstractTool::setStatusInfo()
     */
    void statusInfoChanged(const QString &info);

private slots:
    void actionTriggered(QAction *action);
    void toolEnabledChanged(bool enabled);
    void selectEnabledTool();

private:
    Q_DISABLE_COPY(ToolManager)

    AbstractTool *firstEnabledTool() const;
    void setSelectedTool(AbstractTool *tool);

    QActionGroup *mActionGroup;
    AbstractTool *mSelectedTool;
    AbstractTool *mPreviouslyDisabledTool;
    MapDocument *mMapDocument;
};

} // namespace Internal
} // namespace Tiled

#endif // TOOLMANAGER_H
