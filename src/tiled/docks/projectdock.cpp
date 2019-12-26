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

#include "actionmanager.h"
#include "documentmanager.h"
#include "projectmodel.h"
#include "utils.h"

#include <QBoxLayout>
#include <QFileDialog>
#include <QFileInfo>
#include <QMenu>
#include <QMouseEvent>
#include <QTreeView>

namespace Tiled {

/**
 * Shows the list of files in a project.
 */
class ProjectView final : public QTreeView
{
    Q_OBJECT

public:
    ProjectView(QWidget *parent = nullptr);

    /**
     * Returns a sensible size hint.
     */
    QSize sizeHint() const override;

    void setModel(QAbstractItemModel *model) override;

    ProjectModel *model() const { return mProjectModel; }

protected:
    void contextMenuEvent(QContextMenuEvent *event) override;

private:
    void onActivated(const QModelIndex &index);

    ProjectModel *mProjectModel;
};


ProjectDock::ProjectDock(QWidget *parent)
    : QDockWidget(parent)
    , mProjectView(new ProjectView)
{
    setObjectName(QLatin1String("ProjectDock"));

    auto widget = new QWidget(this);
    auto layout = new QVBoxLayout(widget);
    layout->setMargin(0);
    layout->setSpacing(0);

    layout->addWidget(mProjectView);

    setWidget(widget);
    retranslateUi();
}

void ProjectDock::addFolderToProject()
{
    const QString folder = QFileDialog::getExistingDirectory(window(),
                                                             tr("Choose Folder"),
                                                             QFileInfo(project().fileName()).path());

    if (folder.isEmpty())
        return;

    mProjectView->model()->addFolder(folder);

    auto &p = project();
    if (!p.fileName().isEmpty())
        p.save(p.fileName());
}

void ProjectDock::refreshProjectFolders()
{
    mProjectView->model()->refreshFolders();
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

Project &ProjectDock::project() const
{
    return mProjectView->model()->project();
}

void ProjectDock::setProject(Project project)
{
    mProjectView->model()->setProject(std::move(project));
}

void ProjectDock::retranslateUi()
{
    setWindowTitle(tr("Project"));
}

///////////////////////////////////////////////////////////////////////////////

ProjectView::ProjectView(QWidget *parent)
    : QTreeView(parent)
{
    setHeaderHidden(true);
    setUniformRowHeights(true);
    setDragEnabled(true);
    setDefaultDropAction(Qt::MoveAction);

    setModel(new ProjectModel(this));

    connect(this, &QAbstractItemView::activated,
            this, &ProjectView::onActivated);
}

QSize ProjectView::sizeHint() const
{
    return Utils::dpiScaled(QSize(130, 200));
}

void ProjectView::setModel(QAbstractItemModel *model)
{
    mProjectModel = qobject_cast<ProjectModel*>(model);
    Q_ASSERT(mProjectModel);
    QTreeView::setModel(model);
}

void ProjectView::contextMenuEvent(QContextMenuEvent *event)
{
    const QModelIndex index = indexAt(event->pos());

    QMenu menu;

    if (index.isValid()) {
        if (!index.parent().isValid()) {
            auto removeFolder = menu.addAction(tr("&Remove Folder from Project"), [=] {
                model()->removeFolder(index.row());

                auto &p = model()->project();
                if (!p.fileName().isEmpty())
                    p.save(p.fileName());
            });
            Utils::setThemeIcon(removeFolder, "list-remove");
        }
    } else {
        menu.addAction(ActionManager::action("AddFolderToProject"));
        menu.addAction(ActionManager::action("RefreshProjectFolders"));
    }

    if (!menu.isEmpty())
        menu.exec(event->globalPos());
}

void ProjectView::onActivated(const QModelIndex &index)
{
    const QString path = model()->filePath(index);
    if (!QFileInfo(path).isDir())
        DocumentManager::instance()->openFile(path);
}

} // namespace Tiled

#include "projectdock.moc"
