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

    void setMapDocument(MapDocument *mapDocument);

    QAction *registerTool(AbstractTool *tool);

    void selectTool(AbstractTool *tool);
    AbstractTool *selectedTool() const;

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


/**
 * Returns the selected tool.
 */
inline AbstractTool *ToolManager::selectedTool() const
{
    return mSelectedTool;
}

} // namespace Internal
} // namespace Tiled

#endif // TOOLMANAGER_H
