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
class QToolBar;

namespace Tiled {
namespace Internal {

class AbstractTool;

/**
 * The tool manager provides a central place to register editing tools. In
 * return, it provides the tool bar that shows you all the tools and allows you
 * to select them.
 */
class ToolManager : public QObject
{
    Q_OBJECT

public:
    /**
     * Returns the tool manager instance. Creates the instance when it doesn't
     * exist yet.
     */
    static ToolManager *instance();

    /**
     * Deletes the tool manager instance. Should only be called on application
     * exit.
     */
    static void deleteInstance();

    /**
     * Registers a new tool. It will be added to the tools tool bar. The tool
     * manager does not take ownership over the tool.
     */
    void registerTool(AbstractTool *tool);

    /**
     * Adds a separator to the tool bar between the last registered tool and
     * the next tool that is registered.
     */
    void addSeparator();

    /**
     * Selects the given tool. It should be previously added using
     * registerTool().
     */
    void selectTool(AbstractTool *tool);

    /**
     * Returns the selected tool.
     */
    AbstractTool *selectedTool() const { return mSelectedTool; }

    /**
     * Returns a tool bar with all tools added to it.
     */
    QToolBar *toolBar() const { return mToolBar; }

signals:
    void selectedToolChanged(AbstractTool *tool);

    /**
     * Emitted when the status information of the current tool changed.
     * @see AbstractTool::setStatusInfo()
     */
    void statusInfoChanged(const QString &info);

private slots:
    void actionTriggered(QAction *action);
    void languageChanged();
    void toolEnabledChanged(bool enabled);
    void selectEnabledTool();

private:
    Q_DISABLE_COPY(ToolManager)

    ToolManager();
    ~ToolManager();

    AbstractTool *firstEnabledTool() const;
    void setSelectedTool(AbstractTool *tool);

    static ToolManager *mInstance;

    QToolBar *mToolBar;
    QActionGroup *mActionGroup;
    AbstractTool *mSelectedTool;
    AbstractTool *mPreviouslyDisabledTool;
};

} // namespace Internal
} // namespace Tiled

#endif // TOOLMANAGER_H
