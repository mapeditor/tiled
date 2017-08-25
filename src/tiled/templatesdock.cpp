/*
 * templatesdock.cpp
 * Copyright 2017, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
 * Copyright 2017, Mohamed Thabet <thabetx@gmail.com>
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

#include "templatesdock.h"

#include "editpolygontool.h"
#include "mapdocument.h"
#include "mapscene.h"
#include "mapview.h"
#include "newtemplatedialog.h"
#include "objectgroup.h"
#include "objecttemplatemodel.h"
#include "objectselectiontool.h"
#include "preferences.h"
#include "propertiesdock.h"
#include "templatemanager.h"
#include "tmxmapformat.h"
#include "toolmanager.h"
#include "utils.h"

#include <QBoxLayout>
#include <QSplitter>
#include <QFileDialog>
#include <QMessageBox>
#include <QToolBar>
#include <QUndoStack>

using namespace Tiled;
using namespace Tiled::Internal;

TemplatesDock::TemplatesDock(QWidget *parent):
    QDockWidget(parent),
    mTemplatesView(new TemplatesView),
    mNewTemplateGroup(new QAction(this)),
    mOpenTemplateGroup(new QAction(this)),
    mUndoAction(new QAction(this)),
    mRedoAction(new QAction(this)),
    mDummyMapDocument(nullptr),
    mMapScene(new MapScene(this)),
    mMapView(new MapView(this, MapView::NoStaticContents)),
    mToolManager(new ToolManager(this))
{
    setObjectName(QLatin1String("TemplatesDock"));

    QWidget *widget = new QWidget(this);

    // Prevent dropping a template into the editing view
    mMapView->setAcceptDrops(false);
    mMapView->setScene(mMapScene);

    mMapView->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    mMapView->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    QToolBar *toolBar = new QToolBar;
    toolBar->setFloatable(false);
    toolBar->setMovable(false);
    toolBar->setIconSize(Utils::smallIconSize());

    mNewTemplateGroup->setIcon(QIcon(QLatin1String(":/images/16x16/document-new.png")));
    Utils::setThemeIcon(mNewTemplateGroup, "document-new");
    connect(mNewTemplateGroup, &QAction::triggered, this, [](){ NewTemplateDialog::newTemplateGroup(); });

    mOpenTemplateGroup->setIcon(QIcon(QLatin1String(":/images/16x16/document-open.png")));
    Utils::setThemeIcon(mOpenTemplateGroup, "document-open");
    connect(mOpenTemplateGroup, &QAction::triggered, this, &TemplatesDock::openTemplateGroup);
    connect(this, &TemplatesDock::setTile, mToolManager, &ToolManager::setTile);

    toolBar->addAction(mNewTemplateGroup);
    toolBar->addAction(mOpenTemplateGroup);

    mUndoAction->setIcon(QIcon(QLatin1String(":/images/16x16/edit-undo.png")));
    Utils::setThemeIcon(mUndoAction, "edit-undo");
    connect(mUndoAction, &QAction::triggered, this, &TemplatesDock::undo);
    toolBar->addAction(mUndoAction);

    mRedoAction->setIcon(QIcon(QLatin1String(":/images/16x16/edit-redo.png")));
    Utils::setThemeIcon(mRedoAction, "edit-redo");
    connect(mRedoAction, &QAction::triggered, this, &TemplatesDock::redo);
    toolBar->addAction(mRedoAction);

    // Initially disabled until a change happens
    mUndoAction->setDisabled(true);
    mRedoAction->setDisabled(true);

    QToolBar *editingToolBar = new QToolBar;
    editingToolBar->setFloatable(false);
    editingToolBar->setMovable(false);
    editingToolBar->setIconSize(Utils::smallIconSize());

    auto objectSelectionTool = new ObjectSelectionTool(this);
    auto editPolygonTool = new EditPolygonTool(this);

    // Assign empty shortcuts to avoid collision with the map editor
    objectSelectionTool->setShortcut(QKeySequence());
    editPolygonTool->setShortcut(QKeySequence());

    editingToolBar->addAction(mToolManager->registerTool(objectSelectionTool));
    editingToolBar->addAction(mToolManager->registerTool(editPolygonTool));

    // Construct the UI
    QVBoxLayout *editorLayout = new QVBoxLayout;
    editorLayout->addWidget(editingToolBar);
    editorLayout->addWidget(mMapView);
    editorLayout->setMargin(0);
    editorLayout->setSpacing(0);

    QWidget *editorWidget = new QWidget;
    editorWidget->setLayout(editorLayout);

    QSplitter *splitter = new QSplitter;
    splitter->addWidget(mTemplatesView);
    splitter->addWidget(editorWidget);

    QVBoxLayout *layout = new QVBoxLayout(widget);
    layout->setMargin(0);
    layout->setSpacing(0);
    layout->addWidget(splitter);
    layout->addWidget(toolBar);

    setWidget(widget);
    retranslateUi();

    // Retrieve saved template groups
    Preferences *prefs = Preferences::instance();
    QString documentsFileName = prefs->templateDocumentsFile();

    TemplateDocuments templateDocuments;

    TemplateDocumentsSerializer templateDocumentsSerializer;
    templateDocumentsSerializer.readTemplateDocuments(documentsFileName, templateDocuments);

    auto model = ObjectTemplateModel::instance();
    model->setTemplateDocuments(templateDocuments);

    mTemplatesView->setModel(model);

    mMapScene->setSelectedTool(mToolManager->selectedTool());

    connect(mTemplatesView, &TemplatesView::currentTemplateChanged,
            this, &TemplatesDock::currentTemplateChanged);

    connect(mTemplatesView, &TemplatesView::currentTemplateChanged,
            this, &TemplatesDock::setTemplate);

    connect(mTemplatesView->model(), &ObjectTemplateModel::dataChanged,
            mTemplatesView, &TemplatesView::applyTemplateGroups);

    connect(mTemplatesView->model(), &ObjectTemplateModel::rowsInserted,
            mTemplatesView, &TemplatesView::applyTemplateGroups);

    connect(mTemplatesView, &TemplatesView::focusInEvent,
            this, &TemplatesDock::focusInEvent);

    connect(mTemplatesView, &TemplatesView::focusOutEvent,
            this, &TemplatesDock::focusOutEvent);

    connect(mTemplatesView->selectionModel(), &QItemSelectionModel::selectionChanged,
            mTemplatesView, &TemplatesView::updateSelection);

    connect(mToolManager, &ToolManager::selectedToolChanged,
            this, &TemplatesDock::setSelectedTool);

    setFocusPolicy(Qt::ClickFocus);
    mMapView->setFocusProxy(this);
}

TemplatesDock::~TemplatesDock()
{
    mMapScene->disableSelectedTool();

    if (mDummyMapDocument) {
        disconnect(mDummyMapDocument->undoStack(), &QUndoStack::indexChanged,
                   this, &TemplatesDock::applyChanges);
    }

    delete mDummyMapDocument;

    ObjectTemplateModel::deleteInstance();
}

void TemplatesDock::setSelectedTool(AbstractTool *tool)
{
    mMapScene->disableSelectedTool();
    mMapScene->setSelectedTool(tool);
    mMapScene->enableSelectedTool();
}

void TemplatesDock::openTemplateGroup()
{
    FormatHelper<TemplateGroupFormat> helper(FileFormat::ReadWrite);
    QString filter = helper.filter();

    Preferences *prefs = Preferences::instance();
    QString suggestedFileName = prefs->lastPath(Preferences::TemplateDocumentsFile);
    QString selectedFilter = TgxTemplateGroupFormat().nameFilter();

    QString fileName = QFileDialog::getOpenFileName(this, tr("Open Template Group"),
                                                    suggestedFileName,
                                                    filter,
                                                    &selectedFilter);

    if (fileName.isEmpty())
        return;

    if (TemplateManager::instance()->findTemplateGroup(fileName)) {
        QMessageBox::information(this, tr("Open Template Group"),
                                 tr("This template group has been already opened."));
        return;
    }

    auto document = TemplateGroupDocument::load(fileName);

    if (!document) {
        QMessageBox::warning(this, tr("Open Template Group"),
                             tr("Couldn't open this template group."));
        return;
    }

    ObjectTemplateModel::instance()->addDocument(document);

    prefs->setLastPath(Preferences::TemplateDocumentsFile,
                       QFileInfo(fileName).path());
}

void TemplatesDock::setTemplate(ObjectTemplate *objectTemplate)
{
    if (mObjectTemplate == objectTemplate)
        return;

    mObjectTemplate = objectTemplate;

    mMapScene->disableSelectedTool();
    MapDocument *previousDocument = mDummyMapDocument;

    mMapView->setEnabled(objectTemplate);

    if (objectTemplate) {
        Map::Orientation orientation = Map::Orthogonal;

        Map *map = new Map(orientation, 1, 1, 1, 1);

        mObject = objectTemplate->object()->clone();
        mObject->markAsTemplateBase();

        if (Tile *tile = mObject->cell().tile()) {
            map->addTileset(tile->sharedTileset());
            mObject->setPosition({-mObject->width() / 2, mObject->height() / 2});
        } else {
            mObject->setPosition({-mObject->width() / 2, -mObject->height()  /2});
        }

        ObjectGroup *objectGroup = new ObjectGroup;
        objectGroup->addObject(mObject);

        map->addLayer(objectGroup);

        mDummyMapDocument = new MapDocument(map);
        mDummyMapDocument->setCurrentLayer(objectGroup);

        mMapScene->setMapDocument(mDummyMapDocument);

        mMapScene->enableSelectedTool();
        mToolManager->setMapDocument(mDummyMapDocument);

        mPropertiesDock->setDocument(mDummyMapDocument);
        mDummyMapDocument->setCurrentObject(mObject);

        mUndoAction->setDisabled(true);
        mRedoAction->setDisabled(true);

        connect(mDummyMapDocument->undoStack(), &QUndoStack::indexChanged,
                this, &TemplatesDock::applyChanges);
    } else {
        mPropertiesDock->setDocument(nullptr);
        mDummyMapDocument = nullptr;
        mMapScene->setMapDocument(nullptr);
        mToolManager->setMapDocument(nullptr);
    }

    if (previousDocument) {
        disconnect(previousDocument->undoStack(), &QUndoStack::indexChanged,
                   this, &TemplatesDock::applyChanges);

        delete previousDocument;
    }
}

void TemplatesDock::undo()
{
    if (mDummyMapDocument) {
        mDummyMapDocument->undoStack()->undo();
        emit mDummyMapDocument->selectedObjectsChanged();
    }
}

void TemplatesDock::redo()
{
    if (mDummyMapDocument) {
        mDummyMapDocument->undoStack()->redo();
        emit mDummyMapDocument->selectedObjectsChanged();
    }
}

void TemplatesDock::applyChanges()
{
    TemplateGroup *templateGroup = mObjectTemplate->templateGroup();

    // Add the tileset of the new tile in case the operation was change tile
    // TODO: only save used tilesets
    if (auto tileset = mObject->cell().tileset())
        templateGroup->addTileset(tileset->sharedPointer());

    mObjectTemplate->setObject(mObject);
    ObjectTemplateModel::instance()->save(templateGroup);

    mUndoAction->setEnabled(mDummyMapDocument->undoStack()->canUndo());
    mRedoAction->setEnabled(mDummyMapDocument->undoStack()->canRedo());
    emit templateEdited(mObjectTemplate->object());
}

void TemplatesDock::focusInEvent(QFocusEvent *event)
{
    Q_UNUSED(event);
    mPropertiesDock->setDocument(mDummyMapDocument);
}

void TemplatesDock::focusOutEvent(QFocusEvent *event)
{
    Q_UNUSED(event);

    if (hasFocus() || !mDummyMapDocument)
        return;

    mDummyMapDocument->setSelectedObjects(QList<MapObject*>());
}

void TemplatesDock::retranslateUi()
{
    setWindowTitle(tr("Templates"));
    mNewTemplateGroup->setText(tr("New Template Group"));
    mOpenTemplateGroup->setText(tr("Open Template Group"));
}

TemplatesView::TemplatesView(QWidget *parent)
    : QTreeView(parent)
{
    setUniformRowHeights(true);
    setHeaderHidden(true);
    setSelectionBehavior(QAbstractItemView::SelectRows);
    setSelectionMode(QAbstractItemView::SingleSelection);
    setDragEnabled(true);
    setDragDropMode(QAbstractItemView::DragOnly);
}

void TemplatesView::applyTemplateGroups()
{
    auto model = ObjectTemplateModel::instance();
    auto templateDocuments = model->templateDocuments();

    Preferences *prefs = Preferences::instance();

    QString templateDocumentsFile = prefs->templateDocumentsFile();
    QDir templateDocumentsDir = QFileInfo(templateDocumentsFile).dir();

    if (!templateDocumentsDir.exists())
        templateDocumentsDir.mkpath(QLatin1String("."));

    TemplateDocumentsSerializer serializer;
    if (!serializer.writeTemplateDocuments(templateDocumentsFile, templateDocuments)) {
        QMessageBox::critical(this, tr("Error Writing Template Groups"),
                              tr("Error writing to %1:\n%2")
                              .arg(prefs->templateDocumentsFile(),
                                   serializer.errorString()));
    }
}

QSize TemplatesView::sizeHint() const
{
    return Utils::dpiScaled(QSize(130, 100));
}

void TemplatesView::updateSelection(const QItemSelection &selected, const QItemSelection &deselected)
{
    Q_UNUSED(deselected);
    auto model = ObjectTemplateModel::instance();

    QModelIndexList indexes = selected.indexes();
    if (indexes.isEmpty())
        return;

    ObjectTemplate *objectTemplate = model->toObjectTemplate(indexes.first());
    emit currentTemplateChanged(objectTemplate);
}
