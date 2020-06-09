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
#include "mapdocumentactionhandler.h"
#include "objecttemplate.h"
#include "preferences.h"
#include "projectmodel.h"
#include "session.h"
#include "templatemanager.h"
#include "utils.h"

#include <QBoxLayout>
#include <QFileDialog>
#include <QFileInfo>
#include <QMenu>
#include <QMouseEvent>
#include <QScrollBar>
#include <QSet>
#include <QStandardPaths>
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

    // TODO: Add 'select by file name'

    QStringList expandedPaths() const { return mExpandedPaths.values(); }
    void setExpandedPaths(const QStringList &paths);
    void addExpandedPath(const QString &path);

    void selectPath(const QString &path);

protected:
    void contextMenuEvent(QContextMenuEvent *event) override;

private:
    void onActivated(const QModelIndex &index);
    void onRowsInserted(const QModelIndex &parent);

    void restoreExpanded(const QModelIndex &parent);

    ProjectModel *mProjectModel;
    QSet<QString> mExpandedPaths;
    QString mSelectedPath;
    int mScrollBarValue = 0;
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

    connect(Preferences::instance(), &Preferences::aboutToSwitchSession,
            this, [this] { Session::current().expandedProjectPaths = mProjectView->expandedPaths(); });

    connect(mProjectView->selectionModel(), &QItemSelectionModel::currentRowChanged,
            this, &ProjectDock::onCurrentRowChanged);

    connect(mProjectView->model(), &ProjectModel::folderAdded, this, &ProjectDock::folderAdded);
    connect(mProjectView->model(), &ProjectModel::folderRemoved, this, &ProjectDock::folderRemoved);
}

void ProjectDock::addFolderToProject()
{
    QString folder = QFileInfo(project().fileName()).path();
    if (folder.isEmpty()) {
        if (!project().folders().isEmpty())
            folder = QFileInfo(project().folders().last()).path();
        else
            folder = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
    }

    folder = QFileDialog::getExistingDirectory(window(),
                                               tr("Choose Folder"),
                                               folder);

    if (folder.isEmpty())
        return;

    mProjectView->model()->addFolder(folder);
    mProjectView->addExpandedPath(folder);

    project().save();
}

void ProjectDock::refreshProjectFolders()
{
    mProjectView->model()->refreshFolders();
}

void ProjectDock::setExpandedPaths(const QStringList &expandedPaths)
{
    mProjectView->setExpandedPaths(expandedPaths);
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

void ProjectDock::onCurrentRowChanged(const QModelIndex &current)
{
    if (!current.isValid())
        return;

    const auto filePath = mProjectView->model()->filePath(current);
    if (QFileInfo { filePath }.isFile())
        emit fileSelected(filePath);
}

Project &ProjectDock::project() const
{
    return mProjectView->model()->project();
}

void ProjectDock::setProject(Project project)
{
    mProjectView->model()->setProject(std::move(project));
}

ProjectModel *ProjectDock::projectModel() const
{
    return mProjectView->model();
}

void ProjectDock::selectFile(const QString &filePath)
{
    mProjectView->selectPath(filePath);
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
    setDragDropMode(QAbstractItemView::DragOnly);

    auto model = new ProjectModel(this);
    setModel(model);

    connect(this, &QAbstractItemView::activated,
            this, &ProjectView::onActivated);

    connect(model, &QAbstractItemModel::rowsInserted,
            this, &ProjectView::onRowsInserted);

    connect(this, &QTreeView::expanded,
            this, [=] (const QModelIndex &index) { mExpandedPaths.insert(model->filePath(index)); });
    connect(this, &QTreeView::collapsed,
            this, [=] (const QModelIndex &index) { mExpandedPaths.remove(model->filePath(index)); });

    // Reselect a previously selected path and restore scrollbar after refresh
    connect(model, &ProjectModel::aboutToRefresh,
            this, [=] {
        mSelectedPath = model->filePath(currentIndex());
        mScrollBarValue = verticalScrollBar()->value();
    });
    connect(model, &ProjectModel::refreshed,
            this, [=] {
        selectPath(mSelectedPath);
        verticalScrollBar()->setValue(mScrollBarValue);
    });
}

QSize ProjectView::sizeHint() const
{
    return Utils::dpiScaled(QSize(250, 200));
}

void ProjectView::setModel(QAbstractItemModel *model)
{
    mProjectModel = qobject_cast<ProjectModel*>(model);
    Q_ASSERT(mProjectModel);
    QTreeView::setModel(model);
}

void ProjectView::setExpandedPaths(const QStringList &paths)
{
#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
    mExpandedPaths = paths.toSet();
#else
    mExpandedPaths = QSet<QString>(paths.begin(), paths.end());
#endif
}

void ProjectView::addExpandedPath(const QString &path)
{
    mExpandedPaths.insert(path);
}

void ProjectView::selectPath(const QString &path)
{
    auto index = model()->index(path);
    if (index.isValid())
        setCurrentIndex(index);
}

void ProjectView::contextMenuEvent(QContextMenuEvent *event)
{
    const QModelIndex index = indexAt(event->pos());

    QMenu menu;

    if (index.isValid()) {
        const auto filePath = model()->filePath(index);

        Utils::addFileManagerActions(menu, filePath);

        if (QFileInfo { filePath }.isFile()) {
            Utils::addOpenWithSystemEditorAction(menu, filePath);

            auto objectTemplate = TemplateManager::instance()->loadObjectTemplate(filePath);
            if (objectTemplate->object()) {
                menu.addSeparator();
                auto action = menu.addAction(tr("Select Template Instances"), [objectTemplate] {
                    MapDocumentActionHandler::instance()->selectAllInstances(objectTemplate);
                });
                action->setEnabled(MapDocumentActionHandler::instance()->mapDocument() != nullptr);
            }
        }

        if (!index.parent().isValid()) {
            menu.addSeparator();
            auto removeFolder = menu.addAction(tr("&Remove Folder from Project"), [=] {
                model()->removeFolder(index.row());
                model()->project().save();
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
    if (QFileInfo(path).isFile())
        DocumentManager::instance()->openFile(path);
}

void ProjectView::onRowsInserted(const QModelIndex &parent)
{
    if (parent.isValid())
        restoreExpanded(parent);
}

void ProjectView::restoreExpanded(const QModelIndex &parent)
{
    const QString path = model()->filePath(parent);

    if (mExpandedPaths.contains(path)) {
        setExpanded(parent, true);

        for (int row = 0, count = model()->rowCount(parent); row < count; ++row)
            restoreExpanded(model()->index(row, 0, parent));
    }
}

} // namespace Tiled

#include "projectdock.moc"
