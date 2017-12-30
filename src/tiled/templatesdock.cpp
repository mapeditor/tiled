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

#include "documentmanager.h"
#include "editpolygontool.h"
#include "mapdocument.h"
#include "mapdocumentactionhandler.h"
#include "mapscene.h"
#include "mapview.h"
#include "objectgroup.h"
#include "objecttemplatemodel.h"
#include "objectselectiontool.h"
#include "preferences.h"
#include "propertiesdock.h"
#include "replacetileset.h"
#include "templatemanager.h"
#include "tilesetmanager.h"
#include "tilesetdocument.h"
#include "tmxmapformat.h"
#include "toolmanager.h"
#include "utils.h"

#include <QBoxLayout>
#include <QFileDialog>
#include <QHeaderView>
#include <QLabel>
#include <QMenu>
#include <QMessageBox>
#include <QPushButton>
#include <QSplitter>
#include <QToolBar>
#include <QUndoStack>

using namespace Tiled;
using namespace Tiled::Internal;

TemplatesDock::TemplatesDock(QWidget *parent):
    QDockWidget(parent),
    mTemplatesView(new TemplatesView),
    mChooseDirectory(new QAction(this)),
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

    mChooseDirectory->setIcon(QIcon(QLatin1String(":/images/16x16/document-open.png")));
    Utils::setThemeIcon(mChooseDirectory, "document-open");
    connect(mChooseDirectory, &QAction::triggered, this, &TemplatesDock::chooseDirectory);

    connect(this, &TemplatesDock::setTile, mToolManager, &ToolManager::setTile);

    toolBar->addAction(mChooseDirectory);

    mUndoAction->setIcon(QIcon(QLatin1String(":/images/16x16/edit-undo.png")));
    Utils::setThemeIcon(mUndoAction, "edit-undo");
    connect(mUndoAction, &QAction::triggered, this, &TemplatesDock::undo);

    mRedoAction->setIcon(QIcon(QLatin1String(":/images/16x16/edit-redo.png")));
    Utils::setThemeIcon(mRedoAction, "edit-redo");
    connect(mRedoAction, &QAction::triggered, this, &TemplatesDock::redo);

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

    editingToolBar->addAction(mUndoAction);
    editingToolBar->addAction(mRedoAction);
    editingToolBar->addSeparator();
    editingToolBar->addAction(mToolManager->registerTool(objectSelectionTool));
    editingToolBar->addAction(mToolManager->registerTool(editPolygonTool));

    mFixTilesetButton = new QPushButton(QString(), this);
    connect(mFixTilesetButton, &QPushButton::clicked, this, &TemplatesDock::fixTileset);
    mFixTilesetButton->setVisible(false);

    mDescriptionLabel = new QLabel;
    mDescriptionLabel->setWordWrap(true);

    // Construct the UI
    auto toolsLayout = new QHBoxLayout;
    toolsLayout->addWidget(editingToolBar);
    toolsLayout->addWidget(mFixTilesetButton);

    auto *editorLayout = new QVBoxLayout;
    editorLayout->addLayout(toolsLayout);
    editorLayout->addWidget(mDescriptionLabel);
    editorLayout->addWidget(mMapView);
    editorLayout->setMargin(0);
    editorLayout->setSpacing(0);


    auto *editorWidget = new QWidget;
    editorWidget->setLayout(editorLayout);

    auto *splitter = new QSplitter;
    splitter->addWidget(mTemplatesView);
    splitter->addWidget(editorWidget);

    auto *layout = new QVBoxLayout(widget);
    layout->setMargin(0);
    layout->setSpacing(0);
    layout->addWidget(splitter);
    layout->addWidget(toolBar);

    setWidget(widget);
    retranslateUi();

    mMapScene->setSelectedTool(mToolManager->selectedTool());

    connect(mTemplatesView, &TemplatesView::currentTemplateChanged,
            this, &TemplatesDock::currentTemplateChanged);

    connect(mTemplatesView, &TemplatesView::currentTemplateChanged,
            this, &TemplatesDock::setTemplate);

    connect(mTemplatesView, &TemplatesView::focusInEvent,
            this, &TemplatesDock::focusInEvent);

    connect(mTemplatesView, &TemplatesView::focusOutEvent,
            this, &TemplatesDock::focusOutEvent);

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
}

void TemplatesDock::openTemplate(const QString &path)
{
    bringToFront();
    setTemplate(TemplateManager::instance()->loadObjectTemplate(path));
    mTemplatesView->setSelectedTemplate(path);
}

void TemplatesDock::bringToFront()
{
    show();
    raise();
    setFocus();
}

void TemplatesDock::setSelectedTool(AbstractTool *tool)
{
    mMapScene->disableSelectedTool();
    mMapScene->setSelectedTool(tool);
    mMapScene->enableSelectedTool();
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
        Q_ASSERT(objectTemplate->object());

        Map::Orientation orientation = Map::Orthogonal;

        Map *map = new Map(orientation, 1, 1, 1, 1);

        mObject = objectTemplate->object()->clone();
        mObject->markAsTemplateBase();

        checkTileset();

        if (Tileset *tileset = mObject->cell().tileset()) {
            map->addTileset(tileset->sharedPointer());
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
                   this, &TemplatesDock::checkTileset);

        delete previousDocument;
    }
}

void TemplatesDock::checkTileset()
{
    Q_ASSERT(mObject);

    Cell cell = mObject->cell();

    if (cell.isEmpty()) {
        mFixTilesetButton->setVisible(false);
        mDescriptionLabel->setVisible(false);
        return;
    }

    auto tileset = cell.tileset();

    QString templateName = QFileInfo(mObjectTemplate->fileName()).fileName();

    if (tileset->imageStatus() == LoadingError) {
        mFixTilesetButton->setVisible(true);
        mFixTilesetButton->setText(tr("Open Tileset"));
        mFixTilesetButton->setToolTip(tileset->imageSource().fileName());

        mDescriptionLabel->setVisible(true);
        mDescriptionLabel->setText(tr("%1: Couldn't find \"%2\"").arg(templateName,
                                                                      tileset->imageSource().fileName()));
        mDescriptionLabel->setToolTip(tileset->imageSource().fileName());
    } else if (!tileset->fileName().isEmpty() && tileset->status() == LoadingError) {
        mFixTilesetButton->setVisible(true);
        mFixTilesetButton->setText(tr("Locate Tileset"));
        mFixTilesetButton->setToolTip(tileset->fileName());

        mDescriptionLabel->setVisible(true);
        mDescriptionLabel->setText(tr("%1: Couldn't find \"%2\"").arg(templateName,
                                                                      tileset->fileName()));
        mDescriptionLabel->setToolTip(tileset->fileName());
    } else {
        mFixTilesetButton->setVisible(false);
        mDescriptionLabel->setVisible(false);
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
    mObjectTemplate->setObject(mObject);

    // Write out the template file
    mObjectTemplate->format()->write(mObjectTemplate,
                                     mObjectTemplate->fileName());

    mUndoAction->setEnabled(mDummyMapDocument->undoStack()->canUndo());
    mRedoAction->setEnabled(mDummyMapDocument->undoStack()->canRedo());

    checkTileset();

    emit templateEdited(mObjectTemplate);
}

void TemplatesDock::chooseDirectory()
{
    Preferences *prefs = Preferences::instance();
    QString f = QFileDialog::getExistingDirectory(window(),
                                                  tr("Choose the Templates Folder"),
                                                  prefs->templatesDirectory());
    if (!f.isEmpty())
        prefs->setTemplatesDirectory(f);
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
    mChooseDirectory->setText(tr("Choose Templates Directory"));
}

void TemplatesDock::fixTileset()
{
    Q_ASSERT(mObject);
    Q_ASSERT(!mObject->cell().isEmpty());

    auto tileset = mObject->cell().tileset()->sharedPointer();

    if (tileset->imageStatus() == LoadingError) {
        // This code opens a new document even if there is a tileset document
        auto tilesetDocument = DocumentManager::instance()->findTilesetDocument(tileset);

        if (!tilesetDocument) {
            tilesetDocument = new TilesetDocument(tileset, tileset->fileName());
            DocumentManager::instance()->addDocument(tilesetDocument);
        } else {
            DocumentManager::instance()->openTileset(tileset);
        }

        connect(tilesetDocument, &TilesetDocument::tilesetChanged,
                this, &TemplatesDock::checkTileset, Qt::UniqueConnection);
    } else if (!tileset->fileName().isEmpty() && tileset->status() == LoadingError) {
        FormatHelper<TilesetFormat> helper(FileFormat::Read, tr("All Files (*)"));

        Preferences *prefs = Preferences::instance();
        QString start = prefs->lastPath(Preferences::ExternalTileset);
        QString fileName = QFileDialog::getOpenFileName(this, tr("Locate External Tileset"),
                                                        start,
                                                        helper.filter());

        if (!fileName.isEmpty()) {
            prefs->setLastPath(Preferences::ExternalTileset, QFileInfo(fileName).path());

            QString error;
            auto newTileset = TilesetManager::instance()->loadTileset(fileName, &error);
            if (!newTileset || newTileset->status() == LoadingError) {
                QMessageBox::critical(window(), tr("Error Reading Tileset"), error);
                return;
            }
            // Replace with the first (and only) tileset.
            mDummyMapDocument->undoStack()->push(new ReplaceTileset(mDummyMapDocument, 0, newTileset));

            emit templateTilesetReplaced();
        }
    }
}

TemplatesView::TemplatesView(QWidget *parent)
    : QTreeView(parent)
{
    setUniformRowHeights(true);
    setHeaderHidden(true);
    setDragEnabled(true);
    setDragDropMode(QAbstractItemView::DragOnly);

    Preferences *prefs = Preferences::instance();
    connect(prefs, &Preferences::templatesDirectoryChanged,
            this, &TemplatesView::onTemplatesDirectoryChanged);

    QDir templatesDir(prefs->templatesDirectory());
    if (!templatesDir.exists())
        templatesDir.setPath(QDir::currentPath());

    mModel = new ObjectTemplateModel(this);
    mModel->setRootPath(templatesDir.absolutePath());

    setModel(mModel);
    setRootIndex(mModel->index(templatesDir.absolutePath()));

    QHeaderView *headerView = header();
    headerView->setStretchLastSection(false);
    headerView->setSectionResizeMode(0, QHeaderView::Stretch);

    connect(selectionModel(), &QItemSelectionModel::currentChanged,
            this, &TemplatesView::onCurrentChanged);
}

void TemplatesView::setSelectedTemplate(const QString &path)
{
    auto index = mModel->index(path);
    if (index.isValid())
        setCurrentIndex(index);
}

void TemplatesView::contextMenuEvent(QContextMenuEvent *event)
{
    const QModelIndex index = indexAt(event->pos());
    if (!index.isValid())
        return;

    QMenu menu;

    Utils::addFileManagerActions(menu, mModel->filePath(index));

    if (ObjectTemplate *objectTemplate = mModel->toObjectTemplate(index)) {
        menu.addSeparator();
        menu.addAction(tr("Select All Instances"), [objectTemplate] {
            MapDocumentActionHandler *handler = MapDocumentActionHandler::instance();
            handler->selectAllInstances(objectTemplate);
        });
    }

    menu.exec(event->globalPos());
}

QSize TemplatesView::sizeHint() const
{
    return Utils::dpiScaled(QSize(130, 100));
}

void TemplatesView::onCurrentChanged(const QModelIndex &index)
{
    if (!index.isValid())
        return;

    ObjectTemplate *objectTemplate = mModel->toObjectTemplate(index);
    emit currentTemplateChanged(objectTemplate);
}

void TemplatesView::onTemplatesDirectoryChanged(const QString &templatesDirectory)
{
    mModel->setRootPath(templatesDirectory);
    setRootIndex(mModel->index(QDir(templatesDirectory).absolutePath()));
}
