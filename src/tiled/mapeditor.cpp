/*
 * mapeditor.cpp
 * Copyright 2016, Thorbjørn Lindeijer <bjorn@lindijer.nl>
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

#include "mapeditor.h"

#include "actionmanager.h"
#include "addremovelayer.h"
#include "addremovetileset.h"
#include "brokenlinks.h"
#include "bucketfilltool.h"
#include "changeselectedarea.h"
#include "createellipseobjecttool.h"
#include "createobjecttool.h"
#include "createpointobjecttool.h"
#include "createpolygonobjecttool.h"
#include "createrectangleobjecttool.h"
#include "createtemplatetool.h"
#include "createtextobjecttool.h"
#include "createtileobjecttool.h"
#include "documentmanager.h"
#include "editablemanager.h"
#include "editablemap.h"
#include "editablewangset.h"
#include "editpolygontool.h"
#include "eraser.h"
#include "filechangedwarning.h"
#include "layerdock.h"
#include "layermodel.h"
#include "layeroffsettool.h"
#include "magicwandtool.h"
#include "maintoolbar.h"
#include "mapdocumentactionhandler.h"
#include "mapscene.h"
#include "mapview.h"
#include "minimapdock.h"
#include "newtilesetdialog.h"
#include "objectgroup.h"
#include "objectreferencetool.h"
#include "objectsdock.h"
#include "objectselectiontool.h"
#include "objecttemplate.h"
#include "painttilelayer.h"
#include "preferences.h"
#include "propertiesdock.h"
#include "reversingproxymodel.h"
#include "scriptmanager.h"
#include "selectsametiletool.h"
#include "shapefilltool.h"
#include "stampbrush.h"
#include "templatesdock.h"
#include "tile.h"
#include "tileselectiontool.h"
#include "tilesetdock.h"
#include "tilesetdocument.h"
#include "tilesetformat.h"
#include "tilesetmanager.h"
#include "tilestamp.h"
#include "tilestampmanager.h"
#include "tilestampsdock.h"
#include "toolmanager.h"
#include "treeviewcombobox.h"
#include "undodock.h"
#include "wangbrush.h"
#include "wangdock.h"
#include "wangset.h"
#include "zoomable.h"
#include "worldmovemaptool.h"
#include "worldmanager.h"

#include <QComboBox>
#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QIdentityProxyModel>
#include <QLabel>
#include <QMainWindow>
#include <QMessageBox>
#include <QQmlEngine>
#include <QShortcut>
#include <QStackedWidget>
#include <QToolBar>
#include <QUndoGroup>

#include <memory>

namespace Tiled {

namespace preferences {
static Preference<QSize> mapEditorSize { "MapEditor/Size" };
static Preference<QByteArray> mapEditorState { "MapEditor/State" };
} // namespace preferences

/**
 * A proxy model that makes sure no items are checked or checkable and that
 * there is only one column.
 *
 * Used in the layer combo box, since the checkboxes can't be used in that
 * context but are otherwise anyway rendered there on Windows.
 */
class ComboBoxProxyModel : public QIdentityProxyModel
{
public:
    explicit ComboBoxProxyModel(QObject *parent = nullptr)
        : QIdentityProxyModel(parent)
    {}

    int columnCount(const QModelIndex &) const override { return 1; }
    QVariant data(const QModelIndex &index, int role) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
};

QVariant ComboBoxProxyModel::data(const QModelIndex &index, int role) const
{
    if (role == Qt::CheckStateRole)
        return QVariant();

    return QIdentityProxyModel::data(index, role);
}

Qt::ItemFlags ComboBoxProxyModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags rc = QIdentityProxyModel::flags(index);
    return rc & ~Qt::ItemIsUserCheckable;
}



MapEditor::MapEditor(QObject *parent)
    : Editor(parent)
    , mMainWindow(new QMainWindow)
    , mLayerDock(new LayerDock(mMainWindow))
    , mWidgetStack(new QStackedWidget(mMainWindow))
    , mCurrentMapDocument(nullptr)
    , mUndoDock(new UndoDock(mMainWindow))
    , mObjectsDock(new ObjectsDock(mMainWindow))
    , mTemplatesDock(new TemplatesDock(mMainWindow))
    , mTilesetDock(new TilesetDock(mMainWindow))
    , mWangDock(new WangDock(mMainWindow))
    , mMiniMapDock(new MiniMapDock(mMainWindow))
    , mLayerComboBox(new TreeViewComboBox)
    , mComboBoxProxyModel(new ComboBoxProxyModel(this))
    , mReversingProxyModel(new ReversingProxyModel(this))
    , mZoomable(nullptr)
    , mZoomComboBox(new QComboBox)
    , mStatusInfoLabel(new QLabel)
    , mMainToolBar(new MainToolBar(mMainWindow))
    , mToolManager(new ToolManager(this))
    , mSelectedTool(nullptr)
    , mViewWithTool(nullptr)
    , mTileStampManager(new TileStampManager(*mToolManager, this))
{
    mMainWindow->setDockOptions(mMainWindow->dockOptions() | QMainWindow::GroupedDragging);
    mMainWindow->setDockNestingEnabled(true);
    mMainWindow->setCentralWidget(mWidgetStack);

    mToolsToolBar = new QToolBar(mMainWindow);
    mToolsToolBar->setObjectName(QStringLiteral("toolsToolBar"));

    mToolSpecificToolBar = new QToolBar(mMainWindow);
    mToolSpecificToolBar->setObjectName(QStringLiteral("toolSpecificToolBar"));

    mStampBrush = new StampBrush(this);
    mWangBrush = new WangBrush(this);
    mBucketFillTool = new BucketFillTool(this);
    mEditPolygonTool = new EditPolygonTool(this);
    mShapeFillTool = new ShapeFillTool(this);
    CreateObjectTool *tileObjectsTool = new CreateTileObjectTool(this);
    CreateTemplateTool *templatesTool = new CreateTemplateTool(this);
    CreateObjectTool *rectangleObjectsTool = new CreateRectangleObjectTool(this);
    CreateObjectTool *pointObjectsTool = new CreatePointObjectTool(this);
    CreateObjectTool *ellipseObjectsTool = new CreateEllipseObjectTool(this);
    CreateObjectTool *polygonObjectsTool = new CreatePolygonObjectTool(this);
    CreateObjectTool *textObjectsTool = new CreateTextObjectTool(this);

    mToolsToolBar->addAction(mToolManager->registerTool(mStampBrush));
    mToolsToolBar->addAction(mToolManager->registerTool(mWangBrush));
    mToolsToolBar->addAction(mToolManager->registerTool(mBucketFillTool));
    mToolsToolBar->addAction(mToolManager->registerTool(mShapeFillTool));
    mToolsToolBar->addAction(mToolManager->registerTool(new Eraser(this)));
    mToolsToolBar->addAction(mToolManager->registerTool(new TileSelectionTool(this)));
    mToolsToolBar->addAction(mToolManager->registerTool(new MagicWandTool(this)));
    mToolsToolBar->addAction(mToolManager->registerTool(new SelectSameTileTool(this)));
    mToolsToolBar->addSeparator();
    mToolsToolBar->addAction(mToolManager->registerTool(new ObjectSelectionTool(this)));
    mToolsToolBar->addAction(mToolManager->registerTool(new ObjectReferenceTool(this)));
    mToolsToolBar->addAction(mToolManager->registerTool(mEditPolygonTool));
    mToolsToolBar->addAction(mToolManager->registerTool(rectangleObjectsTool));
    mToolsToolBar->addAction(mToolManager->registerTool(pointObjectsTool));
    mToolsToolBar->addAction(mToolManager->registerTool(ellipseObjectsTool));
    mToolsToolBar->addAction(mToolManager->registerTool(polygonObjectsTool));
    mToolsToolBar->addAction(mToolManager->registerTool(tileObjectsTool));
    mToolsToolBar->addAction(mToolManager->registerTool(templatesTool));
    mToolsToolBar->addAction(mToolManager->registerTool(textObjectsTool));
    mToolsToolBar->addSeparator();
    mToolsToolBar->addAction(mToolManager->registerTool(new WorldMoveMapTool(this)));
    mToolsToolBar->addAction(mToolManager->registerTool(new LayerOffsetTool(this)));
    mToolsToolBar->addSeparator();  // todo: hide when there are no tool extensions

    const auto tools = PluginManager::instance()->objects<AbstractTool>();
    for (auto tool : tools)
        mToolsToolBar->addAction(mToolManager->registerTool(tool));

    connect(PluginManager::instance(), &PluginManager::objectAdded,
            this, [this] (QObject *object) {
        if (auto tool = qobject_cast<AbstractTool*>(object))
            mToolsToolBar->addAction(mToolManager->registerTool(tool));
    });
    connect(PluginManager::instance(), &PluginManager::objectRemoved,
            this, [this] (QObject *object) {
        if (auto tool = qobject_cast<AbstractTool*>(object)) {
            auto action = mToolManager->findAction(tool);
            mToolsToolBar->removeAction(action);
            mToolManager->unregisterTool(tool);
        }
    });

    mToolManager->createShortcuts(mMainWindow);

    mPropertiesDock = new PropertiesDock(mMainWindow);
    mTemplatesDock->setPropertiesDock(mPropertiesDock);
    mTileStampsDock = new TileStampsDock(mTileStampManager, mMainWindow);

    resetLayout();

    mComboBoxProxyModel->setSourceModel(mReversingProxyModel);
    mLayerComboBox->setModel(mComboBoxProxyModel);
    mLayerComboBox->setMinimumContentsLength(10);
    mLayerComboBox->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    connect(mLayerComboBox.get(), static_cast<void (QComboBox::*)(int)>(&QComboBox::activated),
            this, &MapEditor::layerComboActivated);

    connect(mWidgetStack, &QStackedWidget::currentChanged, this, &MapEditor::currentWidgetChanged);
    connect(mToolManager, &ToolManager::statusInfoChanged, this, &MapEditor::updateStatusInfoLabel);
    connect(mTilesetDock, &TilesetDock::currentTileChanged, mToolManager, &ToolManager::setTile);
    connect(mTilesetDock, &TilesetDock::currentTileChanged, mTemplatesDock, &TemplatesDock::setTile);
    connect(mTilesetDock, &TilesetDock::stampCaptured, this, &MapEditor::setStamp);
    connect(mTilesetDock, &TilesetDock::localFilesDropped, this, &MapEditor::filesDroppedOnTilesetDock);
    connect(mTemplatesDock, &TemplatesDock::currentTemplateChanged, mToolManager, &ToolManager::setObjectTemplate);
    connect(DocumentManager::instance(), &DocumentManager::templateOpenRequested,
            mTemplatesDock, &TemplatesDock::openTemplate);
    connect(DocumentManager::instance(), &DocumentManager::selectCustomPropertyRequested,
            mPropertiesDock, &PropertiesDock::selectCustomProperty);

    connect(mTemplatesDock, &TemplatesDock::templateTilesetReplaced,
            DocumentManager::instance(), &DocumentManager::templateTilesetReplaced);

    connect(mStampBrush, &StampBrush::stampChanged, this, &MapEditor::setStamp);
    connect(mBucketFillTool, &BucketFillTool::stampChanged, this, &MapEditor::setStamp);
    connect(mShapeFillTool, &ShapeFillTool::stampChanged, this, &MapEditor::setStamp);
    connect(mStampBrush, &StampBrush::randomChanged, this, &MapEditor::setRandom);
    connect(mBucketFillTool, &BucketFillTool::randomChanged, this, &MapEditor::setRandom);
    connect(mShapeFillTool, &ShapeFillTool::randomChanged, this, &MapEditor::setRandom);
    connect(mStampBrush, &StampBrush::wangFillChanged, this, &MapEditor::setWangFill);
    connect(mBucketFillTool, &BucketFillTool::wangFillChanged, this, &MapEditor::setWangFill);
    connect(mShapeFillTool, &ShapeFillTool::wangFillChanged, this, &MapEditor::setWangFill);

    connect(mWangDock, &WangDock::currentWangSetChanged,
            mBucketFillTool, &BucketFillTool::setWangSet);
    connect(mWangDock, &WangDock::currentWangSetChanged,
            mShapeFillTool, &ShapeFillTool::setWangSet);
    connect(mWangDock, &WangDock::currentWangSetChanged,
            mStampBrush, &StampBrush::setWangSet);
    connect(mWangDock, &WangDock::currentWangSetChanged,
            mWangBrush, &WangBrush::wangSetChanged);
    connect(mWangDock, &WangDock::currentWangSetChanged,
            this, &MapEditor::currentWangSetChanged);
    connect(mWangDock, &WangDock::wangColorChanged,
            this, &MapEditor::currentWangColorIndexChanged);
    connect(mWangDock, &WangDock::selectWangBrush,
            this, &MapEditor::selectWangBrush);
    connect(mWangDock, &WangDock::wangColorChanged,
            mWangBrush, &WangBrush::setColor);
    connect(mWangBrush, &WangBrush::colorCaptured,
            mWangDock, &WangDock::onColorCaptured);

    connect(mTileStampsDock, &TileStampsDock::setStamp,
            this, &MapEditor::setStamp);

    setSelectedTool(mToolManager->selectedTool());
    connect(mToolManager, &ToolManager::selectedToolChanged,
            this, &MapEditor::setSelectedTool);

    setupQuickStamps();
    retranslateUi();

    Preferences *prefs = Preferences::instance();
    connect(prefs, &Preferences::languageChanged, this, &MapEditor::retranslateUi);
    connect(prefs, &Preferences::showTileCollisionShapesChanged,
            this, &MapEditor::showTileCollisionShapesChanged);
    connect(prefs, &Preferences::parallaxEnabledChanged,
            this, &MapEditor::parallaxEnabledChanged);
    connect(prefs, &Preferences::aboutToSwitchSession,
            this, [this] { if (mCurrentMapDocument) saveDocumentState(mCurrentMapDocument); });

    connect(&WorldManager::instance(), &WorldManager::worldsChanged,
            this, &MapEditor::updateActiveUndoStack);
}

MapEditor::~MapEditor()
{
}

void MapEditor::saveState()
{
    preferences::mapEditorSize = mMainWindow->size();
    preferences::mapEditorState = mMainWindow->saveState();
}

void MapEditor::restoreState()
{
    QSize size = preferences::mapEditorSize;
    if (!size.isEmpty()) {
        mMainWindow->resize(size);
        mMainWindow->restoreState(preferences::mapEditorState);
    }
}

void MapEditor::addDocument(Document *document)
{
    MapDocument *mapDocument = qobject_cast<MapDocument*>(document);
    Q_ASSERT(mapDocument);

    // Some file state settings need to be restored before the map becomes current
    const QVariantMap fileState = Session::current().fileState(mapDocument->fileName());
    if (!fileState.isEmpty()) {
        mapDocument->expandedGroupLayers = fromSettingsValue<QSet<int>>(fileState.value(QStringLiteral("expandedGroupLayers")));
        mapDocument->expandedObjectLayers = fromSettingsValue<QSet<int>>(fileState.value(QStringLiteral("expandedObjectLayers")));
    }

    MapView *view = new MapView(mWidgetStack);
    MapScene *scene = new MapScene(view); // scene is owned by the view

    auto prefs = Preferences::instance();
    scene->setShowTileCollisionShapes(prefs->showTileCollisionShapes());
    scene->setParallaxEnabled(prefs->parallaxEnabled());
    scene->setMapDocument(mapDocument);
    view->setScene(scene);

    mWidgetForMap.insert(mapDocument, view);
    mWidgetStack->addWidget(view);

    restoreDocumentState(mapDocument);
}

void MapEditor::removeDocument(Document *document)
{
    MapDocument *mapDocument = qobject_cast<MapDocument*>(document);
    Q_ASSERT(mapDocument);
    Q_ASSERT(mWidgetForMap.contains(mapDocument));

    if (mapDocument == mCurrentMapDocument)
        setCurrentDocument(nullptr);

    MapView *mapView = mWidgetForMap.take(mapDocument);
    // remove first, to keep it valid while the current widget changes
    mWidgetStack->removeWidget(mapView);
    delete mapView;
}

void MapEditor::setCurrentDocument(Document *document)
{
    MapDocument *mapDocument = qobject_cast<MapDocument*>(document);
    Q_ASSERT(mapDocument || !document);

    if (mCurrentMapDocument == mapDocument) {
        updateActiveUndoStack();
        return;
    }

    if (mCurrentMapDocument) {
        saveDocumentState(mCurrentMapDocument);
        mCurrentMapDocument->disconnect(this);
    }

    mCurrentMapDocument = mapDocument;

    MapView *mapView = mWidgetForMap.value(mapDocument);
    if (mapView)
        mWidgetStack->setCurrentWidget(mapView);

    mLayerDock->setMapDocument(mapDocument);

    if (mZoomable) {
        mZoomable->setComboBox(nullptr);
        mZoomable = nullptr;
    }

    mPropertiesDock->setDocument(mapDocument);
    mUndoDock->setStack(document ? document->undoStack() : nullptr);
    mObjectsDock->setMapDocument(mapDocument);
    mTilesetDock->setMapDocument(mapDocument);
    mWangDock->setDocument(mapDocument);
    mMiniMapDock->setMapDocument(mapDocument);

    if (mapDocument) {
        connect(mapDocument, &MapDocument::currentLayerChanged,
                this, &MapEditor::updateLayerComboIndex);
//        connect(mapDocument, &MapDocument::selectedAreaChanged,
//                this, &MapEditor::updateActions);
//        connect(mapDocument, &MapDocument::selectedObjectsChanged,
//                this, &MapEditor::updateActions);

        if (mapView) {
            mZoomable = mapView->zoomable();
            mZoomable->setComboBox(mZoomComboBox.get());
        }

        connect(mCurrentMapDocument, &MapDocument::currentObjectSet,
                this, [this, mapDocument] { mPropertiesDock->setDocument(mapDocument); });

        mReversingProxyModel->setSourceModel(mapDocument->layerModel());
    } else {
        mReversingProxyModel->setSourceModel(nullptr);
    }

    mLayerComboBox->setEnabled(mapDocument);
    updateLayerComboIndex();

    // Take the currently active tool to the new map view
    if (mViewWithTool) {
        MapScene *mapScene = mViewWithTool->mapScene();
        mapScene->setSelectedTool(nullptr);
        mViewWithTool = nullptr;
    }

    mToolManager->setMapDocument(mapDocument);

    if (mapView) {
        MapScene *mapScene = mapView->mapScene();
        mapScene->setSelectedTool(mSelectedTool);

        if (mSelectedTool)
            mapView->viewport()->setCursor(mSelectedTool->cursor());
        else
            mapView->viewport()->unsetCursor();

        mViewWithTool = mapView;
    }

    updateActiveUndoStack();
}

Document *MapEditor::currentDocument() const
{
    return mCurrentMapDocument;
}

QWidget *MapEditor::editorWidget() const
{
    return mMainWindow;
}

QList<QToolBar *> MapEditor::toolBars() const
{
    return {
        mMainToolBar,
        mToolsToolBar,
        mToolSpecificToolBar
    };
}

QList<QDockWidget *> MapEditor::dockWidgets() const
{
    return {
        mPropertiesDock,
        mLayerDock,
        mUndoDock,
        mObjectsDock,
        mTemplatesDock,
        mTilesetDock,
        mWangDock,
        mMiniMapDock,
        mTileStampsDock
    };
}

QList<QWidget *> MapEditor::statusBarWidgets() const
{
    return {
        mStatusInfoLabel.get()
    };
}

QList<QWidget *> MapEditor::permanentStatusBarWidgets() const
{
    return {
        mLayerComboBox.get(),
        mZoomComboBox.get()
    };
}

Editor::StandardActions MapEditor::enabledStandardActions() const
{
    StandardActions standardActions;

    if (mCurrentMapDocument) {
        Layer *currentLayer = mCurrentMapDocument->currentLayer();
        bool objectsSelected = !mCurrentMapDocument->selectedObjects().isEmpty();
        bool areaSelected = !mCurrentMapDocument->selectedArea().isEmpty();

        if ((currentLayer && areaSelected) || objectsSelected)
            standardActions |= CutAction | CopyAction | DeleteAction;

        if (ClipboardManager::instance()->hasMap())
            standardActions |= PasteAction | PasteInPlaceAction;
    }

    return standardActions;
}

void MapEditor::performStandardAction(StandardAction action)
{
    switch (action) {
    case CutAction:
        MapDocumentActionHandler::instance()->cut();
        break;
    case CopyAction:
        MapDocumentActionHandler::instance()->copy();
        break;
    case PasteAction:
        paste(ClipboardManager::PasteDefault);
        break;
    case PasteInPlaceAction:
        paste(ClipboardManager::PasteInPlace);
        break;
    case DeleteAction:
        if (mEditPolygonTool->hasSelectedHandles())
            mEditPolygonTool->deleteNodes();
        else
            MapDocumentActionHandler::instance()->delete_();
        break;
    }
}

/**
 * Arranges views and toolbars to default layout.
 */
void MapEditor::resetLayout()
{
    // Remove dock widgets and set them to visible
    const QList<QDockWidget*> dockWidgets = this->dockWidgets();
    for (auto dockWidget : dockWidgets) {
        mMainWindow->removeDockWidget(dockWidget);
        dockWidget->setVisible(true);
    }

    // Make sure all toolbars are visible
    const QList<QToolBar*> toolBars = this->toolBars();
    for (auto toolBar : toolBars)
        toolBar->setVisible(true);

    // Adding dock widgets and toolbars in their default position
    mMainWindow->addToolBar(mMainToolBar);
    mMainWindow->addToolBar(mToolsToolBar);
    mMainWindow->addToolBar(mToolSpecificToolBar);

    mMainWindow->addDockWidget(Qt::LeftDockWidgetArea, mPropertiesDock);
    mMainWindow->addDockWidget(Qt::LeftDockWidgetArea, mUndoDock);

    mMainWindow->addDockWidget(Qt::LeftDockWidgetArea, mTemplatesDock);
    mMainWindow->addDockWidget(Qt::LeftDockWidgetArea, mTileStampsDock);
    mMainWindow->tabifyDockWidget(mTemplatesDock, mTileStampsDock);

    mMainWindow->addDockWidget(Qt::RightDockWidgetArea, mLayerDock);
    mMainWindow->addDockWidget(Qt::RightDockWidgetArea, mObjectsDock);
    mMainWindow->addDockWidget(Qt::RightDockWidgetArea, mMiniMapDock);
    mMainWindow->tabifyDockWidget(mMiniMapDock, mObjectsDock);
    mMainWindow->tabifyDockWidget(mObjectsDock, mLayerDock);

    mMainWindow->addDockWidget(Qt::RightDockWidgetArea, mWangDock);
    mMainWindow->addDockWidget(Qt::RightDockWidgetArea, mTilesetDock);
    mMainWindow->tabifyDockWidget(mWangDock, mTilesetDock);

    // These dock widgets may not be immediately useful to many people, so
    // they are hidden by default.
    mUndoDock->setVisible(false);
    mTemplatesDock->setVisible(false);
    mTileStampsDock->setVisible(false);
}

Zoomable *MapEditor::zoomable() const
{
    if (auto view = currentMapView())
        return view->zoomable();
    return nullptr;
}

void MapEditor::saveDocumentState(MapDocument *mapDocument) const
{
    MapView *mapView = mWidgetForMap.value(mapDocument);
    if (!mapView)
        return;

    if (mapDocument->fileName().isEmpty())
        return;

    const QRect viewportRect = mapView->viewport()->rect();
    const QPointF viewCenter = mapView->mapToScene(viewportRect).boundingRect().center();

    QVariantMap fileState;
    fileState.insert(QLatin1String("scale"), mapView->zoomable()->scale());
    fileState.insert(QStringLiteral("viewCenter"), toSettingsValue(viewCenter));
    fileState.insert(QStringLiteral("selectedLayer"), globalIndex(mapDocument->currentLayer()));
    if (!mapDocument->expandedGroupLayers.isEmpty())
        fileState.insert(QStringLiteral("expandedGroupLayers"), toSettingsValue(mapDocument->expandedGroupLayers));
    if (!mapDocument->expandedObjectLayers.isEmpty())
        fileState.insert(QStringLiteral("expandedObjectLayers"), toSettingsValue(mapDocument->expandedObjectLayers));

    Session::current().setFileState(mapDocument->fileName(), fileState);
}

void MapEditor::restoreDocumentState(MapDocument *mapDocument) const
{
    MapView *mapView = mWidgetForMap.value(mapDocument);
    if (!mapView)
        return;

    const QVariantMap fileState = Session::current().fileState(mapDocument->fileName());
    if (fileState.isEmpty())
        return;

    const qreal scale = fileState.value(QStringLiteral("scale")).toReal();
    if (scale > 0)
        mapView->zoomable()->setScale(scale);

    const QPointF viewCenter = fromSettingsValue<QPointF>(fileState.value(QStringLiteral("viewCenter")));
    mapView->forceCenterOn(viewCenter);

    const int layerIndex = fileState.value(QStringLiteral("selectedLayer")).toInt();
    if (Layer *layer = layerAtGlobalIndex(mapDocument->map(), layerIndex))
        mapDocument->switchCurrentLayer(layer);
}

void MapEditor::setSelectedTool(AbstractTool *tool)
{
    if (mSelectedTool == tool)
        return;

    if (mSelectedTool) {
        disconnect(mSelectedTool, &AbstractTool::cursorChanged,
                   this, &MapEditor::cursorChanged);
    }

    mSelectedTool = tool;
    mToolSpecificToolBar->clear();

    if (mViewWithTool) {
        MapScene *mapScene = mViewWithTool->mapScene();
        mapScene->setSelectedTool(tool);

        if (tool)
            mViewWithTool->viewport()->setCursor(tool->cursor());
        else
            mViewWithTool->viewport()->unsetCursor();
    }

    if (tool) {
        connect(tool, &AbstractTool::cursorChanged,
                this, &MapEditor::cursorChanged);

        tool->populateToolBar(mToolSpecificToolBar);
    }

    updateActiveUndoStack();
}

void MapEditor::updateActiveUndoStack()
{
    QUndoStack *undoStack = DocumentManager::instance()->undoGroup()->activeStack();
    if (mSelectedTool) {
        undoStack = mSelectedTool->undoStack();
        if (!undoStack && mCurrentMapDocument) {
            undoStack = mCurrentMapDocument->undoStack();
        }
    }
    else if (mCurrentMapDocument) {
        undoStack = mCurrentMapDocument->undoStack();
    }
    mUndoDock->setStack(undoStack);
    if (DocumentManager::instance()->currentEditor() == this) {
        DocumentManager::instance()->undoGroup()->setActiveStack(undoStack);
    }
}

void MapEditor::paste(ClipboardManager::PasteFlags flags)
{
    if (!mCurrentMapDocument)
        return;

    ClipboardManager *clipboardManager = ClipboardManager::instance();
    std::unique_ptr<Map> map = clipboardManager->map();
    if (!map)
        return;

    bool tilesetsUnified = false;

    if (flags & ClipboardManager::PasteInPlace)
        mCurrentMapDocument->undoStack()->beginMacro(tr("Paste in Place"));

    const bool hasTileLayers = LayerIterator(map.get(), Layer::TileLayerType).next();
    if (hasTileLayers && (flags & ClipboardManager::PasteInPlace)) {
        QVector<SharedTileset> missingTilesets;
        mCurrentMapDocument->unifyTilesets(*map, missingTilesets);
        mCurrentMapDocument->paintTileLayers(*map, false, &missingTilesets);
        tilesetsUnified = missingTilesets.isEmpty();
    }

    LayerIterator objectGroupIterator(map.get(), Layer::ObjectGroupType);
    if (ObjectGroup *objectGroup = static_cast<ObjectGroup*>(objectGroupIterator.next())) {
        if (!tilesetsUnified)
            mCurrentMapDocument->unifyTilesets(*map);

        // todo: Handle multiple object groups
        const MapView *view = currentMapView();
        clipboardManager->pasteObjectGroup(objectGroup, mCurrentMapDocument, view, flags);
    }

    if (hasTileLayers && !(flags & ClipboardManager::PasteInPlace)) {
        // Reset tile selection and paste into the stamp brush
        if (!mCurrentMapDocument->selectedArea().isEmpty()) {
            QUndoCommand *command = new ChangeSelectedArea(mCurrentMapDocument, QRegion());
            mCurrentMapDocument->undoStack()->push(command);
        }

        map->normalizeTileLayerPositionsAndMapSize();
        setStamp(TileStamp(std::move(map)));
        mToolManager->selectTool(mStampBrush);
    }

    if (flags & ClipboardManager::PasteInPlace)
        mCurrentMapDocument->undoStack()->endMacro();
}

void MapEditor::setRandom(bool value)
{
    mStampBrush->setRandom(value);

    auto fillMethod = value ? AbstractTileFillTool::RandomFill :
                              AbstractTileFillTool::TileFill;

    mBucketFillTool->setFillMethod(fillMethod);
    mShapeFillTool->setFillMethod(fillMethod);
}

void MapEditor::setWangFill(bool value)
{
    mStampBrush->setWangFill(value);

    auto fillMethod = value ? AbstractTileFillTool::WangFill :
                              AbstractTileFillTool::TileFill;

    mBucketFillTool->setFillMethod(fillMethod);
    mShapeFillTool->setFillMethod(fillMethod);
}

/**
 * Sets the current stamp, which is used by both the stamp brush and the bucket
 * fill tool.
 */
void MapEditor::setStamp(const TileStamp &stamp)
{
    if (stamp.isEmpty())
        return;

    mStampBrush->setStamp(stamp);
    mBucketFillTool->setStamp(stamp);
    mShapeFillTool->setStamp(stamp);

    // When selecting a new stamp, it makes sense to switch to a stamp tool
    AbstractTool *selectedTool = mToolManager->selectedTool();
    if (!selectedTool || !selectedTool->usesSelectedTiles())
        mToolManager->selectTool(mStampBrush);

    mTilesetDock->selectTilesInStamp(stamp);
}

void MapEditor::selectWangBrush()
{
    // Don't switch tools when the current tool also uses Wang sets
    AbstractTool *selectedTool = mToolManager->selectedTool();
    if (selectedTool && selectedTool->usesWangSets())
        return;

    mToolManager->selectTool(mWangBrush);
}

void MapEditor::currentWidgetChanged()
{
    if (!mWidgetStack->currentWidget())
        setCurrentDocument(nullptr);
}

void MapEditor::cursorChanged(const QCursor &cursor)
{
    if (mViewWithTool)
        mViewWithTool->viewport()->setCursor(cursor);
}

void MapEditor::updateStatusInfoLabel(const QString &statusInfo)
{
    mStatusInfoLabel->setText(statusInfo);
}

void MapEditor::layerComboActivated()
{
    if (!mCurrentMapDocument)
        return;

    const QModelIndex comboIndex = mLayerComboBox->currentModelIndex();
    const QModelIndex reversedIndex = mComboBoxProxyModel->mapToSource(comboIndex);
    const QModelIndex sourceIndex = mReversingProxyModel->mapToSource(reversedIndex);
    Layer *layer = mCurrentMapDocument->layerModel()->toLayer(sourceIndex);
    if (!layer)
        return;

    mCurrentMapDocument->switchCurrentLayer(layer);
}

void MapEditor::updateLayerComboIndex()
{
    QModelIndex index;

    if (mCurrentMapDocument) {
        const auto currentLayer = mCurrentMapDocument->currentLayer();
        const QModelIndex sourceIndex = mCurrentMapDocument->layerModel()->index(currentLayer);
        const QModelIndex reversedIndex = mReversingProxyModel->mapFromSource(sourceIndex);
        index = mComboBoxProxyModel->mapFromSource(reversedIndex);
    }

    mLayerComboBox->setCurrentModelIndex(index);
}

void MapEditor::addExternalTilesets(const QStringList &fileNames)
{
    handleExternalTilesetsAndImages(fileNames, false);
}

QAction *MapEditor::actionSelectNextTileset() const
{
    return mTilesetDock->actionSelectNextTileset();
}

QAction *MapEditor::actionSelectPreviousTileset() const
{
    return mTilesetDock->actionSelectPreviousTileset();
}

void MapEditor::filesDroppedOnTilesetDock(const QStringList &fileNames)
{
    handleExternalTilesetsAndImages(fileNames, true);
}

void MapEditor::handleExternalTilesetsAndImages(const QStringList &fileNames,
                                                bool handleImages)
{
    // These files could be either external tilesets, in which case we'll add
    // them to the current map, or images, in which case we'll offer to create
    // tilesets based on them (unless handleImages is false).

    QVector<SharedTileset> tilesets;

    for (const QString &fileName : fileNames) {
        QString error;

        // Check if the file is an already loaded tileset
        SharedTileset tileset = TilesetManager::instance()->findTileset(fileName);
        if (tileset) {
            tilesets.append(tileset);
            continue;
        }

        // Check if the file is has a supported tileset format
        TilesetFormat *tilesetFormat = findSupportingTilesetFormat(fileName);
        if (tilesetFormat) {
            tileset = tilesetFormat->read(fileName);
            if (tileset) {
                tileset->setFileName(fileName);
                tileset->setFormat(tilesetFormat->shortName());
                tilesets.append(tileset);
                continue;
            } else {
                error = tilesetFormat->errorString();
            }
        }

        if (handleImages) {
            // Check if the file is a supported image format
            const QImage image(fileName);
            if (!image.isNull()) {
                tileset = newTileset(fileName, image);
                if (tileset)
                    tilesets.append(tileset);
                continue;
            }
        }

        if (!tilesetFormat)
            error = tr("Unrecognized tileset format.");

        if (fileNames.size() == 1) {
            QMessageBox::critical(mMainWindow, tr("Error Reading Tileset"), error);
            return;
        } else {
            int result;

            result = QMessageBox::warning(mMainWindow, tr("Error Reading Tileset"),
                                          tr("%1: %2").arg(fileName, error),
                                          QMessageBox::Abort | QMessageBox::Ignore,
                                          QMessageBox::Ignore);

            if (result == QMessageBox::Abort)
                return;
        }
    }

    // Filter out any tilesets that are already referenced by the map
    auto it = tilesets.begin();
    auto end = std::remove_if(it, tilesets.end(), [this](SharedTileset &tileset) {
        return mCurrentMapDocument->map()->tilesets().contains(tileset);
    });

    if (it != end) {
        QUndoStack *undoStack = mCurrentMapDocument->undoStack();
        undoStack->beginMacro(tr("Add %n Tileset(s)", "", tilesets.size()));
        for (; it != end; ++it)
            undoStack->push(new AddTileset(mCurrentMapDocument, *it));
        undoStack->endMacro();

        mTilesetDock->setCurrentTileset(tilesets.last());
    }
}

SharedTileset MapEditor::newTileset(const QString &path, const QImage &image)
{
    NewTilesetDialog newTileset(mMainWindow->window());
    newTileset.setImagePath(path);

    SharedTileset tileset = newTileset.createTileset();
    if (!tileset)
        return tileset;

    // Try to do something sensible when the user chooses to make a collection
    if (tileset->isCollection())
        tileset->addTile(QPixmap::fromImage(image), QUrl::fromLocalFile(path));

    if (!newTileset.isEmbedded()) {
        // Save new external tileset
        const auto tilesetDocument = std::make_unique<TilesetDocument>(tileset);
        if (!DocumentManager::instance()->saveDocumentAs(tilesetDocument.get()))
            return SharedTileset();
    }

    return tileset;
}

void MapEditor::setupQuickStamps()
{
    QList<Qt::Key> keys = TileStampManager::quickStampKeys();

    for (int i = 0; i < keys.length(); i++) {
        Qt::Key key = keys.at(i);

        // Set up shortcut for selecting this quick stamp
        QShortcut *selectStamp = new QShortcut(key, mMainWindow);
        connect(selectStamp, &QShortcut::activated, [=] { mTileStampManager->selectQuickStamp(i); });

        // Set up shortcut for creating this quick stamp
        QShortcut *createStamp = new QShortcut(Qt::CTRL + key, mMainWindow);
        connect(createStamp, &QShortcut::activated, [=] { mTileStampManager->createQuickStamp(i); });

        // Set up shortcut for extending this quick stamp
        QShortcut *extendStamp = new QShortcut((Qt::CTRL | Qt::SHIFT) + key, mMainWindow);
        connect(extendStamp, &QShortcut::activated, [=] { mTileStampManager->extendQuickStamp(i); });
    }

    connect(mTileStampManager, &TileStampManager::setStamp,
            this, &MapEditor::setStamp);
}

void MapEditor::retranslateUi()
{
    mToolsToolBar->setWindowTitle(tr("Tools"));
    mToolSpecificToolBar->setWindowTitle(tr("Tool Options"));
}

void MapEditor::showTileCollisionShapesChanged(bool enabled)
{
    for (MapView *mapView : qAsConst(mWidgetForMap))
        mapView->mapScene()->setShowTileCollisionShapes(enabled);
}

void MapEditor::parallaxEnabledChanged(bool enabled)
{
    for (MapView *mapView : qAsConst(mWidgetForMap))
        mapView->mapScene()->setParallaxEnabled(enabled);
}

void MapEditor::setCurrentTileset(const SharedTileset &tileset)
{
    mTilesetDock->setCurrentTileset(tileset);
}

SharedTileset MapEditor::currentTileset() const
{
    return mTilesetDock->currentTileset();
}

EditableMap *MapEditor::currentBrush() const
{
    const TileStamp &stamp = mStampBrush->stamp();
    if (stamp.isEmpty())
        return nullptr;

    auto map = stamp.variations().first().map->clone();
    auto editableMap = new EditableMap(std::move(map));
    QQmlEngine::setObjectOwnership(editableMap, QQmlEngine::JavaScriptOwnership);
    return editableMap;
}

void MapEditor::setCurrentBrush(EditableMap *editableMap)
{
    if (!editableMap) {
        ScriptManager::instance().throwNullArgError(0);
        return;
    }
    // todo: filter any non-tilelayers out of the map?
    setStamp(TileStamp(editableMap->map()->clone()));
}

EditableWangSet *MapEditor::currentWangSet() const
{
    auto currentWangSet = mWangDock->currentWangSet();
    return EditableManager::instance().editableWangSet(currentWangSet);
}

int MapEditor::currentWangColorIndex() const
{
    return mWangDock->currentWangColor();
}

AbstractTool *MapEditor::selectedTool() const
{
    return mSelectedTool;
}

} // namespace Tiled

#include "moc_mapeditor.cpp"
