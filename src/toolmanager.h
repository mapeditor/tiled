/*
 * Tiled Map Editor (Qt)
 * Copyright 2009 Tiled (Qt) developers (see AUTHORS file)
 *
 * This file is part of Tiled (Qt).
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
 * this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 * Place, Suite 330, Boston, MA 02111-1307, USA.
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

private:
    ToolManager();
    ~ToolManager();

    void setSelectedTool(AbstractTool *tool);

    static ToolManager *mInstance;

    QToolBar *mToolBar;
    QActionGroup *mActionGroup;
    AbstractTool *mSelectedTool;
};

} // namespace Internal
} // namespace Tiled

#endif // TOOLMANAGER_H
