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
#include "addremovetileset.h"
#include "documentmanager.h"
#include "mapdocumentactionhandler.h"
#include "mapeditor.h"
#include "objecttemplate.h"
#include "preferences.h"
#include "projectmanager.h"
#include "projectmodel.h"
#include "session.h"
#include "templatemanager.h"
#include "tilesetdock.h"
#include "tilesetmanager.h"
#include "utils.h"

#include <QAction>
#include <QBoxLayout>
#include <QFileDialog>
#include <QFileInfo>
#include <QMenu>
#include <QMouseEvent>
#include <QScrollBar>
#include <QSet>
#include <QToolBar>
#include <QTreeView>
#include <QUndoStack>

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

    ProjectModel *projectModel() const { return mProjectModel; }

    QStringList expandedPaths() const { return mExpandedPaths.values(); }
    void setExpandedPaths(const QStringList &paths);
    void addExpandedPath(const QString &path);

    void setCollapseAllAction(QAction *action) { mCollapseAllAction = action; }
    void setExpandToCurrentActive(bool active) { mExpandToCurrentActive = active; }

    void selectPath(const QString &path);
    void expandToPath(const QString &filePath);
    void restoreExpandedPaths();

    QString filePath(const QModelIndex &index) const;

protected:
    void contextMenuEvent(QContextMenuEvent *event) override;

private:
    void onActivated(const QModelIndex &index);
    void onRowsInserted(const QModelIndex &parent);

    void restoreExpanded(const QModelIndex &parent);

    ProjectModel *mProjectModel;
    ProjectProxyModel *mProxyModel;
    QSet<QString> mExpandedPaths;
    QAction *mCollapseAllAction = nullptr;
    bool mExpandToCurrentActive = false;
    QString mSelectedPath;
    int mScrollBarValue = 0;
};


ProjectDock::ProjectDock(QWidget *parent)
    : QDockWidget(parent)
    , mProjectView(new ProjectView)
{
    setObjectName(QLatin1String("ProjectDock"));

    auto toolBar = new QToolBar;
    toolBar->setIconSize(Utils::smallIconSize());

    mCollapseAllAction = new QAction(this);
    mCollapseAllAction->setIcon(QIcon(QLatin1String(":/images/scalable/chevrons-down-up.svg")));
    connect(mCollapseAllAction, &QAction::triggered, this, [=] {
        mProjectView->collapseAll();
        mProjectView->setExpandedPaths({});
    });
    toolBar->addAction(mCollapseAllAction);

    mExpandToCurrentAction = new QAction(this);
    mExpandToCurrentAction->setCheckable(true);
    mExpandToCurrentAction->setIcon(QIcon(QLatin1String(":/images/scalable/focus.svg")));
    connect(mExpandToCurrentAction, &QAction::toggled, this, [=](bool checked) {
        mProjectView->setExpandToCurrentActive(checked);
        if (checked) {
            auto doc = DocumentManager::instance()->currentDocument();
            if (doc)
                mProjectView->expandToPath(doc->fileName());
        } else {
            mProjectView->restoreExpandedPaths();
        }
    });
    toolBar->addAction(mExpandToCurrentAction);

    connect(DocumentManager::instance(), &DocumentManager::currentDocumentChanged,
            this, [=](Document *doc) {
        if (!mExpandToCurrentAction->isChecked())
            return;
        if (doc)
            mProjectView->expandToPath(doc->fileName());
    });

    mProjectView->setCollapseAllAction(mCollapseAllAction);

    auto widget = new QWidget(this);
    auto layout = new QVBoxLayout(widget);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    layout->addWidget(mProjectView);
    layout->addWidget(toolBar);

    setWidget(widget);
    retranslateUi();

    connect(Preferences::instance(), &Preferences::aboutToSwitchSession,
            this, [=] {
        Session::current().expandedProjectPaths = mProjectView->expandedPaths();
    });

    connect(mProjectView->selectionModel(), &QItemSelectionModel::currentRowChanged,
            this, &ProjectDock::onCurrentRowChanged);

    // Forwarding signals
    auto projectModel = mProjectView->projectModel();
    connect(projectModel, &ProjectModel::folderAdded, this, &ProjectDock::folderAdded);
    connect(projectModel, &ProjectModel::folderRemoved, this, &ProjectDock::folderRemoved);
}

void ProjectDock::addFolderToProject()
{
    Project &project = ProjectManager::instance()->project();

    QString folder = QFileInfo(project.fileName()).path();
    if (folder.isEmpty()) {
        if (!project.folders().isEmpty())
            folder = QFileInfo(project.folders().last()).path();
        else
            folder = Preferences::homeLocation();
    }

    folder = QFileDialog::getExistingDirectory(window(),
                                               tr("Choose Folder"),
                                               folder);

    if (folder.isEmpty())
        return;

    mProjectView->projectModel()->addFolder(folder);
    mProjectView->addExpandedPath(folder);

    project.save();
}

void ProjectDock::refreshProjectFolders()
{
    mProjectView->projectModel()->refreshFolders();
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

    const auto filePath = mProjectView->filePath(current);
    if (QFileInfo { filePath }.isFile())
        emit fileSelected(filePath);
}

void ProjectDock::selectFile(const QString &filePath)
{
    mProjectView->selectPath(filePath);
}

void ProjectDock::retranslateUi()
{
    setWindowTitle(tr("Project"));
    mCollapseAllAction->setText(tr("Collapse All"));
    mCollapseAllAction->setToolTip(tr("Collapse all folders in the project view."));
    mExpandToCurrentAction->setText(tr("Only Expand to Current"));
    mExpandToCurrentAction->setToolTip(tr("Shows only the folder path of the active file. Restores the previous expansion state when disabled."));
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

    mProjectModel = ProjectManager::instance()->projectModel();
    mProxyModel = new ProjectProxyModel(this);
    mProxyModel->setSourceModel(mProjectModel);
    setModel(mProxyModel);

    connect(this, &QAbstractItemView::activated,
            this, &ProjectView::onActivated);

    connect(mProxyModel, &QAbstractItemModel::rowsInserted,
            this, &ProjectView::onRowsInserted);

    connect(this, &QTreeView::expanded,
            this, [=] (const QModelIndex &index) {
        if (!mExpandToCurrentActive)
            mExpandedPaths.insert(filePath(index));
    });
    connect(this, &QTreeView::collapsed,
            this, [=] (const QModelIndex &index) {
        if (!mExpandToCurrentActive)
            mExpandedPaths.remove(filePath(index));
    });

    // Reselect a previously selected path and restore scrollbar after refresh
    connect(mProjectModel, &ProjectModel::aboutToRefresh,
            this, [=] {
        mSelectedPath = filePath(currentIndex());
        mScrollBarValue = verticalScrollBar()->value();
    });
    connect(mProjectModel, &ProjectModel::refreshed,
            this, [=] {
        selectPath(mSelectedPath);
        verticalScrollBar()->setValue(mScrollBarValue);
    });
}

QSize ProjectView::sizeHint() const
{
    return Utils::dpiScaled(QSize(250, 200));
}

void ProjectView::setExpandedPaths(const QStringList &paths)
{
    mExpandedPaths = QSet<QString>(paths.begin(), paths.end());
}

void ProjectView::addExpandedPath(const QString &path)
{
    mExpandedPaths.insert(path);
}

void ProjectView::selectPath(const QString &path)
{
    const auto sourceIndex = mProjectModel->index(path);
    const auto proxyIndex = mProxyModel->mapFromSource(sourceIndex);
    if (proxyIndex.isValid())
        setCurrentIndex(proxyIndex);
}

void ProjectView::expandToPath(const QString &filePath)
{
    const QModelIndex sourceIndex = mProjectModel->index(filePath);
    const QModelIndex proxyIndex = mProxyModel->mapFromSource(sourceIndex);
    if (!proxyIndex.isValid())
        return;

    collapseAll();

    if (currentIndex() == proxyIndex)
        scrollTo(proxyIndex);
    else
        setCurrentIndex(proxyIndex);
}

void ProjectView::restoreExpandedPaths()
{
    collapseAll();

    const int topLevel = mProxyModel->rowCount();
    for (int i = 0; i < topLevel; ++i)
        restoreExpanded(mProxyModel->index(i, 0));
}

QString ProjectView::filePath(const QModelIndex &index) const
{
    return mProjectModel->filePath(mProxyModel->mapToSource(index));
}

void ProjectView::contextMenuEvent(QContextMenuEvent *event)
{
    const auto index = indexAt(event->pos());

    QMenu menu;

    if (index.isValid()) {
        const auto path = filePath(index);

        Utils::addFileManagerActions(menu, path);

        if (QFileInfo { path }.isFile()) {
            Utils::addOpenWithSystemEditorAction(menu, path);

            auto mapDocumentActionHandler = MapDocumentActionHandler::instance();
            auto mapDocument = mapDocumentActionHandler->mapDocument();

            // Add template-specific actions
            auto objectTemplate = TemplateManager::instance()->loadObjectTemplate(path);
            if (objectTemplate->object()) {
                menu.addSeparator();
                menu.addAction(tr("Select Template Instances"), [=] {
                    mapDocumentActionHandler->selectAllInstances(objectTemplate);
                })->setEnabled(mapDocument != nullptr);
            }
            // Add tileset-specific actions
            else if (auto tileset = TilesetManager::instance()->loadTileset(path)) {
                if (mapDocument) {
                    auto documentManager = DocumentManager::instance();
                    auto mapEditor = static_cast<MapEditor*>(documentManager->editor(Document::MapDocumentType));
                    auto tilesetDock = mapEditor->tilesetDock();

                    const bool mapHasTileset = mapDocument->map()->tilesets().contains(tileset);
                    const bool tilesetVisibleInDock = tilesetDock->hasTileset(tileset);

                    menu.addSeparator();

                    menu.addAction(tr("Select in Tilesets View"), [=] {
                        tilesetDock->setCurrentTileset(tileset);
                    })->setEnabled(tilesetVisibleInDock);

                    menu.addAction(tr("Add Tileset to Map"), [=] {
                        mapDocument->undoStack()->push(new AddTileset(mapDocument, tileset));
                        tilesetDock->setCurrentTileset(tileset);
                    })->setEnabled(!mapHasTileset);
                }
            }
        }

        if (!index.parent().isValid()) {
            menu.addSeparator();
            auto removeFolder = menu.addAction(tr("&Remove Folder from Project"), [=] {
                projectModel()->removeFolder(index.row());
                projectModel()->project().save();
            });
            Utils::setThemeIcon(removeFolder, "list-remove");
        }
    } else {
        menu.addAction(mCollapseAllAction);

        menu.addSeparator();
        menu.addAction(ActionManager::action("AddFolderToProject"));
        menu.addAction(ActionManager::action("RefreshProjectFolders"));
    }

    ActionManager::applyMenuExtensions(&menu, MenuIds::projectViewFiles);

    if (!menu.isEmpty())
        menu.exec(event->globalPos());
}

void ProjectView::onActivated(const QModelIndex &index)
{
    const QString path = filePath(index);
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
    const QString path = filePath(parent);

    if (mExpandedPaths.contains(path)) {
        setExpanded(parent, true);

        for (int row = 0, count = model()->rowCount(parent); row < count; ++row)
            restoreExpanded(model()->index(row, 0, parent));
    }
}

} // namespace Tiled

#include "projectdock.moc"
#include "moc_projectdock.cpp"
