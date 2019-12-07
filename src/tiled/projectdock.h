/*
 * projectdock.h
 * Copyright 2019, Thorbjørn Lindeijer <bjorn@lindeijer.nl>
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

#include "project.h"

#include <QDockWidget>

namespace Tiled {

class ProjectView;

class ProjectDock final : public QDockWidget
{
    Q_OBJECT

public:
    ProjectDock(QWidget *parent = nullptr);

    QString projectFileName() const;

    void openLastProject();
    void openProject();
    void openProjectFile(const QString &fileName);
    void saveProjectAs();
    void closeProject();
    void addFolderToProject();
    void refreshProjectFolders();

signals:
    void projectFileNameChanged();

protected:
    void changeEvent(QEvent *e) override;

private:
    Project &project() const;
    void retranslateUi();

    ProjectView *mProjectView;
};


inline QString ProjectDock::projectFileName() const
{
    return project().fileName();
}

} // namespace Tiled
