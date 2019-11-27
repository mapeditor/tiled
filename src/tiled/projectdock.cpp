/*
 * projectdock.cpp
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

#include "projectdock.h"

#include "documentmanager.h"
#include "preferences.h"
#include "projectmodel.h"
#include "utils.h"

#include <QBoxLayout>
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <QMouseEvent>
#include <QSettings>

using namespace Tiled;

static const char * const LAST_PROJECT_KEY = "Project/LastProject";

ProjectDock::ProjectDock(QWidget *parent)
    : QDockWidget(parent)
    , mProjectView(new ProjectView)
{
    setObjectName(QLatin1String("ProjectDock"));

    auto widget = new QWidget(this);
    auto layout = new QVBoxLayout(widget);
    layout->setMargin(0);

    // Reopen last used project
    const auto prefs = Preferences::instance();
    const auto settings = prefs->settings();
    const auto lastProjectFileName = settings->value(QLatin1String(LAST_PROJECT_KEY)).toString();
    if (prefs->openLastFilesOnStartup() && !lastProjectFileName.isEmpty())
        mProject.load(lastProjectFileName);

    auto projectModel = new ProjectModel(this);
    projectModel->setFolders(mProject.folders());

    mProjectView->setModel(projectModel);

    layout->addWidget(mProjectView);

    setWidget(widget);
    retranslateUi();

    connect(this, &ProjectDock::projectFileNameChanged, [this] {
        Preferences::instance()->settings()->setValue(QLatin1String(LAST_PROJECT_KEY), projectFileName());
    });
}

void ProjectDock::openProject()
{
    const QString projectFilesFilter = tr("Tiled Projects (*.tiled-project)");
    const QString fileName = QFileDialog::getOpenFileName(window(),
                                                          tr("Open Project"),
                                                          mProject.fileName(),
                                                          projectFilesFilter,
                                                          nullptr);
    if (fileName.isEmpty())
        return;

    Project project;

    if (!project.load(fileName)) {
        QMessageBox::critical(window(),
                              tr("Error Opening Project"),
                              tr("An error occurred while opening the project."));
        return;
    }

    mProjectView->model()->setFolders(nullptr);
    std::swap(mProject, project);
    mProjectView->model()->setFolders(mProject.folders());

    emit projectFileNameChanged();
}

void ProjectDock::saveProjectAs()
{
    const QString projectFilesFilter = tr("Tiled Projects (*.tiled-project)");
    const QString fileName = QFileDialog::getSaveFileName(window(),
                                                          tr("Save Project As"),
                                                          mProject.fileName(),
                                                          projectFilesFilter,
                                                          nullptr);
    if (fileName.isEmpty())
        return;

    if (!mProject.save(fileName)) {
        QMessageBox::critical(window(),
                              tr("Error Saving Project"),
                              tr("An error occurred while saving the project."));
    }

    emit projectFileNameChanged();
}

void ProjectDock::closeProject()
{
    mProjectView->model()->setFolders(nullptr);
    mProject.clear();

    emit projectFileNameChanged();
}

void ProjectDock::addFolderToProject()
{
    const QString folder = QFileDialog::getExistingDirectory(window(),
                                                             tr("Choose Folder"),
                                                             QFileInfo(mProject.fileName()).path());

    if (folder.isEmpty())
        return;

    mProject.addFolder(folder);
    // FIXME: Should just add the new top-level row, not trigger complete reset
    mProjectView->model()->setFolders(mProject.folders());

    if (!mProject.fileName().isEmpty())
        mProject.save(mProject.fileName());
}

void ProjectDock::refreshProjectFolders()
{
    mProjectView->model()->setFolders(nullptr);
    mProject.refreshFolders();
    mProjectView->model()->setFolders(mProject.folders());
}

void ProjectDock::changeEvent(QEvent *e)
{
    QDockWidget::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        retranslateUi();
        break;
    default:
        break;
    }
}

void ProjectDock::retranslateUi()
{
    setWindowTitle(tr("Project"));
}

///// ///// ///// ///// /////

ProjectView::ProjectView(QWidget *parent)
    : QTreeView(parent)
{
    setHeaderHidden(true);
    setUniformRowHeights(true);
    setDragEnabled(true);
    setDefaultDropAction(Qt::MoveAction);

    connect(this, &QAbstractItemView::activated,
            this, &ProjectView::onActivated);
}

QSize ProjectView::sizeHint() const
{
    return Utils::dpiScaled(QSize(130, 100));
}

void ProjectView::setModel(QAbstractItemModel *model)
{
    mProjectModel = qobject_cast<ProjectModel*>(model);
    Q_ASSERT(mProjectModel);
    QTreeView::setModel(model);
}

void ProjectView::mousePressEvent(QMouseEvent *event)
{
    QModelIndex index = indexAt(event->pos());
    if (index.isValid()) {
        // Prevent drag-and-drop starting when clicking on an unselected item.
        setDragEnabled(selectionModel()->isSelected(index));
    }

    QTreeView::mousePressEvent(event);
}

void ProjectView::onActivated(const QModelIndex &index)
{
    const QString path = model()->filePath(index);
    if (!QFileInfo(path).isDir())
        DocumentManager::instance()->openFile(path);
}
