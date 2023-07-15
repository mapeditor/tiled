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
#include "mapscene.h"
#include "mapview.h"
#include "objectgroup.h"
#include "objectselectiontool.h"
#include "objecttemplate.h"
#include "propertiesdock.h"
#include "replacetileset.h"
#include "session.h"
#include "templatemanager.h"
#include "tilesetdocument.h"
#include "tilesetmanager.h"
#include "toolmanager.h"
#include "utils.h"

#include <QAction>
#include <QBoxLayout>
#include <QFileDialog>
#include <QHeaderView>
#include <QLabel>
#include <QMenu>
#include <QMessageBox>
#include <QMimeData>
#include <QPushButton>
#include <QScopedValueRollback>
#include <QSplitter>
#include <QToolBar>
#include <QUndoStack>

using namespace Tiled;

// This references created dummy documents, to make sure they are shared if the
// same template is open in the MapEditor and the TilesetEditor.
QHash<ObjectTemplate*, QWeakPointer<MapDocument>> TemplatesDock::ourDummyDocuments;
bool TemplatesDock::ourEmittingChanged;

TemplatesDock::TemplatesDock(QWidget *parent)
    : QDockWidget(parent)
    , mUndoAction(new QAction(this))
    , mRedoAction(new QAction(this))
    , mMapScene(new MapScene(this))
    , mMapView(new MapView(this))
    , mToolManager(new ToolManager(this))
{
    setObjectName(QLatin1String("TemplatesDock"));

    // Prevent dropping a template into the editing view
    mMapView->setAcceptDrops(false);
    mMapView->setScene(mMapScene);

    // But accept drops on the dock
    setAcceptDrops(true);

    mMapView->setResizeAnchor(QGraphicsView::AnchorViewCenter);
    mMapView->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    mMapView->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    mUndoAction->setIcon(QIcon(QLatin1String(":/images/16/edit-undo.png")));
    Utils::setThemeIcon(mUndoAction, "edit-undo");
    connect(mUndoAction, &QAction::triggered, this, &TemplatesDock::undo);

    mRedoAction->setIcon(QIcon(QLatin1String(":/images/16/edit-redo.png")));
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

    // Assign empty shortcuts and don't register actions for these tools, to
    // avoid collisions with the map editor and tile collision editor.
    objectSelectionTool->setShortcut(QKeySequence());
    editPolygonTool->setShortcut(QKeySequence());
    mToolManager->setRegisterActions(false);

    editingToolBar->addAction(mUndoAction);
    editingToolBar->addAction(mRedoAction);
    editingToolBar->addSeparator();
    editingToolBar->addAction(mToolManager->registerTool(objectSelectionTool));
    editingToolBar->addAction(mToolManager->registerTool(editPolygonTool));

    mFixTilesetButton = new QPushButton(this);
    connect(mFixTilesetButton, &QPushButton::clicked, this, &TemplatesDock::fixTileset);
    mFixTilesetButton->setVisible(false);

    mDescriptionLabel = new QLabel;
    mDescriptionLabel->setWordWrap(true);
    mDescriptionLabel->setVisible(false);

    auto toolsLayout = new QHBoxLayout;
    toolsLayout->addWidget(editingToolBar);
    toolsLayout->addWidget(mFixTilesetButton);

    auto *editorLayout = new QVBoxLayout;
    editorLayout->addLayout(toolsLayout);
    editorLayout->addWidget(mDescriptionLabel);
    editorLayout->addWidget(mMapView);
    editorLayout->setContentsMargins(0, 0, 0, 0);
    editorLayout->setSpacing(0);

    auto *widget = new QWidget;
    widget->setLayout(editorLayout);

    setWidget(widget);
    retranslateUi();

    connect(mToolManager, &ToolManager::selectedToolChanged,
            mMapScene, &MapScene::setSelectedTool);

    connect(TemplateManager::instance(), &TemplateManager::objectTemplateChanged,
            this, &TemplatesDock::objectTemplateChanged);

    setFocusPolicy(Qt::ClickFocus);
    mMapView->setFocusProxy(this);
}

TemplatesDock::~TemplatesDock()
{
    mMapScene->setSelectedTool(nullptr);

    if (mDummyMapDocument)
        mDummyMapDocument->undoStack()->disconnect(this);
}

void TemplatesDock::setTile(Tile *tile)
{
    mToolManager->setTile(tile);
}

void TemplatesDock::openTemplate(const QString &path)
{
    bringToFront();
    setTemplate(TemplateManager::instance()->loadObjectTemplate(path));
}

bool TemplatesDock::tryOpenTemplate(const QString &filePath)
{
    auto objectTemplate = TemplateManager::instance()->loadObjectTemplate(filePath);
    if (objectTemplate->object()) {
        setTemplate(objectTemplate);
        return true;
    }
    return false;
}

void TemplatesDock::bringToFront()
{
    show();
    raise();
    setFocus();
}

static ObjectTemplate *readObjectTemplate(const QMimeData *mimeData)
{
    const auto urls = mimeData->urls();
    if (urls.size() != 1)
        return nullptr;

    const QString fileName = urls.first().toLocalFile();
    if (fileName.isEmpty())
        return nullptr;

    const QFileInfo info(fileName);
    if (info.isDir())
        return nullptr;

    auto objectTemplate = TemplateManager::instance()->loadObjectTemplate(info.absoluteFilePath());
    return objectTemplate->object() ? objectTemplate : nullptr;
}

void TemplatesDock::dragEnterEvent(QDragEnterEvent *event)
{
    if (!readObjectTemplate(event->mimeData()))
        return;

    event->acceptProposedAction();
}

void TemplatesDock::dropEvent(QDropEvent *event)
{
    if (auto objectTemplate = readObjectTemplate(event->mimeData()))
        setTemplate(objectTemplate);
}

void TemplatesDock::setTemplate(ObjectTemplate *objectTemplate)
{
    if (mObjectTemplate == objectTemplate)
        return;

    mObjectTemplate = objectTemplate;
    refreshDummyObject();

    emit currentTemplateChanged(mObjectTemplate);
}

void TemplatesDock::refreshDummyObject()
{
    mMapScene->setSelectedTool(nullptr);
    MapDocumentPtr previousDocument = mDummyMapDocument;

    mMapView->setEnabled(mObjectTemplate);

    if (mObjectTemplate && mObjectTemplate->object()) {
        mDummyMapDocument = ourDummyDocuments.value(mObjectTemplate);

        if (!mDummyMapDocument) {
            // TODO: Isometric template objects are currently not supported
            Map::Parameters mapParameters;

            // Setting sizes to 1 makes it render a one-pixel square (the map
            // border), which serves as a somewhat hacky origin indicator.
            mapParameters.width = 1;
            mapParameters.height = 1;
            mapParameters.tileWidth = 1;
            mapParameters.tileHeight = 1;

            auto map = std::make_unique<Map>(mapParameters);

            MapObject *dummyObject = mObjectTemplate->object()->clone();
            dummyObject->markAsTemplateBase();

            if (Tileset *tileset = dummyObject->cell().tileset()) {
                map->addTileset(tileset->sharedFromThis());
                dummyObject->setPosition({-dummyObject->width() / 2, dummyObject->height() / 2});
            } else {
                dummyObject->setPosition({-dummyObject->width() / 2, -dummyObject->height()  /2});
            }

            ObjectGroup *objectGroup = new ObjectGroup;
            objectGroup->addObject(dummyObject);

            map->addLayer(objectGroup);

            mDummyMapDocument = MapDocumentPtr::create(std::move(map));
            mDummyMapDocument->setAllowHidingObjects(false);
            mDummyMapDocument->switchCurrentLayer(objectGroup);

            ourDummyDocuments.insert(mObjectTemplate, mDummyMapDocument);
        }

        mDummyMapDocument->setCurrentObject(dummyObject());

        mUndoAction->setEnabled(mDummyMapDocument->undoStack()->canUndo());
        mRedoAction->setEnabled(mDummyMapDocument->undoStack()->canRedo());

        connect(mDummyMapDocument->undoStack(), &QUndoStack::indexChanged,
                this, &TemplatesDock::applyChanges);

        checkTileset();
    } else {
        mDummyMapDocument.reset();
    }

    mMapScene->setMapDocument(mDummyMapDocument.data());
    mToolManager->setMapDocument(mDummyMapDocument.data());
    mPropertiesDock->setDocument(mDummyMapDocument.data());

    mMapScene->setSelectedTool(mToolManager->selectedTool());

    if (previousDocument)
        previousDocument->undoStack()->disconnect(this);
}

void TemplatesDock::checkTileset()
{
    if (!mObjectTemplate || !mObjectTemplate->tileset()) {
        mFixTilesetButton->setVisible(false);
        mDescriptionLabel->setVisible(false);
        return;
    }

    auto templateName = QFileInfo(mObjectTemplate->fileName()).fileName();
    auto tileset = mObjectTemplate->tileset();

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

void TemplatesDock::objectTemplateChanged(ObjectTemplate *objectTemplate)
{
    if (ourEmittingChanged)
        return;

    // Apparently the template was changed externally
    ourDummyDocuments.remove(objectTemplate);

    if (mObjectTemplate == objectTemplate)
        refreshDummyObject();
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
    mObjectTemplate->setObject(dummyObject());

    // Write out the template file
    mObjectTemplate->save();

    mUndoAction->setEnabled(mDummyMapDocument->undoStack()->canUndo());
    mRedoAction->setEnabled(mDummyMapDocument->undoStack()->canRedo());

    checkTileset();

    QScopedValueRollback<bool> emittingChanged(ourEmittingChanged, true);
    emit TemplateManager::instance()->objectTemplateChanged(mObjectTemplate);
}

void TemplatesDock::focusInEvent(QFocusEvent *event)
{
    mPropertiesDock->setDocument(mDummyMapDocument.data());
    QDockWidget::focusInEvent(event);
}

void TemplatesDock::retranslateUi()
{
    setWindowTitle(tr("Template Editor"));
}

void TemplatesDock::fixTileset()
{
    if (!mObjectTemplate)
        return;

    auto tileset = mObjectTemplate->tileset();
    if (!tileset)
        return;

    if (tileset->imageStatus() == LoadingError) {
        auto tilesetDocument = DocumentManager::instance()->openTileset(tileset);
        connect(tilesetDocument, &TilesetDocument::tilesetChanged,
                this, &TemplatesDock::checkTileset, Qt::UniqueConnection);
    } else if (!tileset->fileName().isEmpty() && tileset->status() == LoadingError) {
        FormatHelper<TilesetFormat> helper(FileFormat::Read, tr("All Files (*)"));

        Session &session = Session::current();
        QString start = session.lastPath(Session::ExternalTileset);
        QString fileName = QFileDialog::getOpenFileName(this, tr("Locate External Tileset"),
                                                        start,
                                                        helper.filter());

        if (!fileName.isEmpty()) {
            session.setLastPath(Session::ExternalTileset, QFileInfo(fileName).path());

            QString error;
            auto newTileset = TilesetManager::instance()->loadTileset(fileName, &error);
            if (!newTileset || newTileset->status() == LoadingError) {
                QMessageBox::critical(window(), tr("Error Reading Tileset"), error);
                return;
            }
            // Replace with the first (and only) tileset.
            mDummyMapDocument->undoStack()->push(new ReplaceTileset(mDummyMapDocument.data(), 0, newTileset));

            emit templateTilesetReplaced();
        }
    }
}

MapObject *TemplatesDock::dummyObject() const
{
    if (!mDummyMapDocument)
        return nullptr;

    return mDummyMapDocument->map()->layerAt(0)->asObjectGroup()->objectAt(0);
}

#include "moc_templatesdock.cpp"
