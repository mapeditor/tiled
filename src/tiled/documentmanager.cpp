/*
 * documentmanager.cpp
 * Copyright 2010, Stefan Beller <stefanbeller@googlemail.com>
 * Copyright 2010-2016, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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

#include "documentmanager.h"

#include "editormanager.h"
#include "abstracttool.h"
#include "adjusttileindexes.h"
#include "brokenlinks.h"
#include "containerhelpers.h"
#include "editableasset.h"
#include "editor.h"
#include "filechangedwarning.h"
#include "filesystemwatcher.h"
#include "logginginterface.h"
#include "map.h"
#include "mapdocument.h"
#include "mapeditor.h"
#include "mapformat.h"
#include "maprenderer.h"
#include "mapview.h"
#include "noeditorwidget.h"
#include "preferences.h"
#include "projectmanager.h"
#include "session.h"
#include "tabbar.h"
#include "tilesetdocument.h"
#include "tilesetdocumentsmodel.h"
#include "tilesetmanager.h"
#include "tmxmapformat.h"
#include "utils.h"
#include "world.h"
#include "worlddocument.h"
#include "worldmanager.h"
#include "zoomable.h"

#include <QCoreApplication>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QLabel>
#include <QMenu>
#include <QMessageBox>
#include <QScrollBar>
#include <QStackedLayout>
#include <QTabBar>
#include <QUndoGroup>
#include <QUndoStack>
#include <QVBoxLayout>

using namespace Tiled;


DocumentManager *DocumentManager::mInstance;

DocumentManager *DocumentManager::instance()
{
    Q_ASSERT(mInstance);
    return mInstance;
}

DocumentManager *DocumentManager::maybeInstance()
{
    return mInstance;
}

DocumentManager::DocumentManager(QObject *parent)
    : QObject(parent)
    , mLockedIcon(QLatin1String(":images/16/locked.png"))
    , mTilesetDocumentsModel(new TilesetDocumentsModel(this))
    , mWidget(new QWidget)
    , mNoEditorWidget(new NoEditorWidget(mWidget))
    , mTabBar(new TabBar(mWidget))
    , mFileChangedWarning(new FileChangedWarning(mWidget))
    , mBrokenLinksModel(new BrokenLinksModel(this))
    , mBrokenLinksWidget(new BrokenLinksWidget(mBrokenLinksModel, mWidget))
    , mMapEditor(nullptr) // todo: look into removing this
    , mMultiDocumentClose(false)
{
    Q_ASSERT(!mInstance);
    mInstance = this;

    mLockedIcon.addFile(QStringLiteral(":images/24/locked.png"));

    mBrokenLinksWidget->setVisible(false);

    mTabBar->setExpanding(false);
    mTabBar->setDocumentMode(true);
    mTabBar->setUsesScrollButtons(true);    // defaults to false on macOS
    mTabBar->setTabsClosable(true);
    mTabBar->setMovable(true);
    mTabBar->setContextMenuPolicy(Qt::CustomContextMenu);

    mFileChangedWarning->setVisible(false);

    connect(mFileChangedWarning, &FileChangedWarning::reload, this, &DocumentManager::reloadCurrentDocument);
    connect(mFileChangedWarning, &FileChangedWarning::ignore, this, &DocumentManager::hideChangedWarning);

    connect(this, &DocumentManager::templateTilesetReplaced,
            mBrokenLinksModel, &BrokenLinksModel::refresh);

    QVBoxLayout *vertical = new QVBoxLayout(mWidget);
    vertical->addWidget(mTabBar);
    vertical->addWidget(mFileChangedWarning);
    vertical->addWidget(mBrokenLinksWidget);
    vertical->setContentsMargins(0, 0, 0, 0);
    vertical->setSpacing(0);

    mEditorStack = new QStackedLayout;
    mEditorStack->addWidget(mNoEditorWidget);
    vertical->addLayout(mEditorStack);

    connect(mTabBar, &QTabBar::currentChanged,
            this, &DocumentManager::currentIndexChanged);
    connect(mTabBar, &QTabBar::tabCloseRequested,
            this, &DocumentManager::documentCloseRequested);
    connect(mTabBar, &QTabBar::tabMoved,
            this, &DocumentManager::documentTabMoved);
    connect(mTabBar, &QWidget::customContextMenuRequested,
            this, &DocumentManager::tabContextMenuRequested);

    connect(EditorManager::instance()->fileSystemWatcher(), &FileSystemWatcher::pathsChanged,
            this, &DocumentManager::filesChanged);

    connect(EditorManager::instance(), &EditorManager::documentOpened,
            this, &DocumentManager::onEditorManagerDocumentOpened);
    connect(EditorManager::instance(), &EditorManager::currentDocumentChanged,
            this, &DocumentManager::onEditorManagerCurrentDocumentChanged);
    connect(EditorManager::instance(), &EditorManager::documentAboutToClose,
            this, &DocumentManager::onEditorManagerDocumentAboutToClose);
    connect(EditorManager::instance(), &EditorManager::tilesetDocumentAdded,
            this, [this](TilesetDocument *tilesetDocument) {
        if (!mTilesetDocumentsModel->contains(tilesetDocument)) {
            mTilesetDocumentsModel->append(tilesetDocument);
            emit tilesetDocumentAdded(tilesetDocument);
        }
    });

    connect(mBrokenLinksModel, &BrokenLinksModel::hasBrokenLinksChanged,
            mBrokenLinksWidget, &BrokenLinksWidget::setVisible);

    connect(TilesetManager::instance(), &TilesetManager::tilesetImagesChanged,
            this, &DocumentManager::tilesetImagesChanged);

    connect(Preferences::instance(), &Preferences::aboutToSwitchSession,
            this, &DocumentManager::updateSession);

    OpenFile::activated = [this] (const OpenFile &open) {
        openFile(open.file);
    };

    JumpToTile::activated = [this] (const JumpToTile &jump) {
        if (auto mapDocument = openMapFile(jump.mapFile)) {
            auto renderer = mapDocument->renderer();
            auto mapView = viewForDocument(mapDocument);
            auto pos = renderer->tileToScreenCoords(jump.tilePos);

            if (auto layer = mapDocument->map()->findLayerById(jump.layerId)) {
                mapDocument->switchSelectedLayers({ layer });
                mapView->forceCenterOn(pos, *layer);
            } else {
                mapView->forceCenterOn(pos);
            }
        }
    };

    JumpToObject::activated = [this] (const JumpToObject &jump) {
        if (auto mapDocument = openMapFile(jump.mapFile)) {
            if (auto object = mapDocument->map()->findObjectById(jump.objectId)) {
                emit mapDocument->focusMapObjectRequested(object);
                mapDocument->setSelectedObjects({ object });
            }
        }
    };

    SelectLayer::activated = [this] (const SelectLayer &select) {
        if (auto mapDocument = openMapFile(select.mapFile)) {
            if (auto layer = mapDocument->map()->findLayerById(select.layerId)) {
                mapDocument->switchSelectedLayers({ layer });
                mapDocument->setCurrentObject(layer);
            }
        }
    };

    SelectCustomProperty::activated = [this] (const SelectCustomProperty &select) {
        openFile(select.fileName);
        const int i = EditorManager::instance()->findDocument(select.fileName);
        if (i == -1)
            return;

        auto doc = EditorManager::instance()->documents().at(i).data();
        Object *obj = nullptr;

        switch (doc->type()) {
        case Document::MapDocumentType: {
            auto mapDocument = static_cast<MapDocument*>(doc);
            switch (select.objectType) {
            case Object::LayerType:
                if (auto layer = mapDocument->map()->findLayerById(select.id)) {
                    mapDocument->switchSelectedLayers({ layer });
                    obj = layer;
                }
                break;
            case Object::MapObjectType:
                if (auto object = mapDocument->map()->findObjectById(select.id)) {
                    emit mapDocument->focusMapObjectRequested(object);
                    mapDocument->setSelectedObjects({ object });
                    obj = object;
                }
                break;
            case Object::MapType:
                obj = mapDocument->map();
                break;
            }
            break;
        }
        case Document::TilesetDocumentType: {
            auto tilesetDocument = static_cast<TilesetDocument*>(doc);
            switch (select.objectType) {
            case Object::MapObjectType:
                // todo: no way to know to which tile this object belongs
                break;
            case Object::TilesetType:
                obj = tilesetDocument->tileset().data();
                break;
            case Object::TileType:
                if (auto tile = tilesetDocument->tileset()->findTile(select.id)) {
                    tilesetDocument->setSelectedTiles({ tile });
                    obj = tile;
                }
                break;
            case Object::WangSetType: {
                // todo: select the wang set
                if (select.id < tilesetDocument->tileset()->wangSetCount())
                    obj = tilesetDocument->tileset()->wangSet(select.id);
                break;
            }
            case Object::WangColorType:
                // todo: can't select just by color index
                break;
            }
            break;
        }
        case Document::WorldDocumentType:
        case Document::ProjectDocumentType:
            break;
        }

        if (obj) {
            doc->setCurrentObject(obj);
            emit selectCustomPropertyRequested(select.propertyName);
        }
    };

    SelectTile::activated = [this] (const SelectTile &select) {
        TilesetDocument* tilesetDocument = nullptr;

        if (SharedTileset tileset { select.tileset }) {
            tilesetDocument = findTilesetDocument(tileset);
            if (tilesetDocument) {
                if (!switchToDocument(tilesetDocument))
                    addDocument(tilesetDocument->sharedFromThis());
            }
        }

        if (!tilesetDocument && !select.tilesetFile.isEmpty())
            tilesetDocument = openTilesetFile(select.tilesetFile);

        if (tilesetDocument) {
            if (auto tile = tilesetDocument->tileset()->findTile(select.tileId)) {
                tilesetDocument->setSelectedTiles({ tile });
                tilesetDocument->setCurrentObject(tile);
            }
        }
    };

    WorldManager &worldManager = WorldManager::instance();
    connect(&worldManager, &WorldManager::worldLoaded,
            this, &DocumentManager::onWorldLoaded);
    connect(&worldManager, &WorldManager::worldUnloaded,
            this, &DocumentManager::onWorldUnloaded);
}

DocumentManager::~DocumentManager()
{
    // All documents should be closed gracefully beforehand
    Q_ASSERT(EditorManager::instance()->documents().isEmpty());
    Q_ASSERT(mTilesetDocumentsModel->rowCount() == 0);
    delete mWidget;

    mInstance = nullptr;
}

/**
 * Returns the document manager widget. It contains the different map views
 * and a tab bar to switch between them.
 */
QWidget *DocumentManager::widget() const
{
    return mWidget;
}

void DocumentManager::setEditor(Document::DocumentType documentType, Editor *editor)
{
    Q_ASSERT(!mEditorForType.contains(documentType));
    mEditorForType.insert(documentType, editor);
    mEditorStack->addWidget(editor->editorWidget());

    if (MapEditor *mapEditor = qobject_cast<MapEditor*>(editor))
        mMapEditor = mapEditor;
}

Editor *DocumentManager::editor(Document::DocumentType documentType) const
{
    return mEditorForType.value(documentType);
}

void DocumentManager::deleteEditors()
{
    qDeleteAll(mEditorForType);
    mEditorForType.clear();
    mMapEditor = nullptr;
}

QList<Editor *> DocumentManager::editors() const
{
    return mEditorForType.values();
}

Editor *DocumentManager::currentEditor() const
{
    if (const auto document = currentDocument())
        return editor(document->type());

    return nullptr;
}

void DocumentManager::saveState()
{
    QHashIterator<Document::DocumentType, Editor*> iterator(mEditorForType);
    while (iterator.hasNext())
        iterator.next().value()->saveState();
}

void DocumentManager::restoreState()
{
    QHashIterator<Document::DocumentType, Editor*> iterator(mEditorForType);
    while (iterator.hasNext())
        iterator.next().value()->restoreState();
}

/**
 * Returns the current document, or nullptr when there is none.
 */
Document *DocumentManager::currentDocument() const
{
    const int index = mTabBar->currentIndex();
    if (index == -1)
        return nullptr;

    const auto &docs = EditorManager::instance()->documents();
    if (index >= docs.size())
        return nullptr;
    return docs.at(index).data();
}

/**
 * Returns the map view of the current document, or 0 when there is none.
 */
MapView *DocumentManager::currentMapView() const
{
    return mMapEditor->currentMapView();
}

/**
 * Returns the map view that displays the given document, or null when there
 * is none.
 */
MapView *DocumentManager::viewForDocument(MapDocument *mapDocument) const
{
    return mMapEditor->viewForDocument(mapDocument);
}

/**
 * Searches for an open document with the given \a fileName and returns its
 * index. Returns -1 when the document isn't open.
 */
int DocumentManager::findDocument(const QString &fileName) const
{
    return EditorManager::instance()->findDocument(fileName);
}

int DocumentManager::findDocument(Document *document) const
{
    return EditorManager::instance()->findDocument(document);
}

/**
 * Switches to the map document at the given \a index.
 */
void DocumentManager::switchToDocument(int index)
{
    mTabBar->setCurrentIndex(index);
}

bool DocumentManager::switchToDocument(const QString &fileName)
{
    const int index = findDocument(fileName);
    if (index != -1) {
        switchToDocument(index);
        return true;
    }
    return false;
}

/**
 * Switches to the given \a document, if there is already a tab open for it.
 * \return whether the switch was successful
 */
bool DocumentManager::switchToDocument(Document *document)
{
    const int index = findDocument(document);
    if (index != -1) {
        switchToDocument(index);
        return true;
    }
    return false;
}

/**
 * Switches to the given \a mapDocument, centering the view on \a viewCenter
 * (scene coordinates) at the given \a scale.
 *
 * If the given map document is not open yet, a tab will be created for it.
 */
void DocumentManager::switchToDocument(MapDocument *mapDocument, QPointF viewCenter, qreal scale)
{
    if (!switchToDocument(mapDocument))
        addDocument(mapDocument->sharedFromThis());

    MapView *view = currentMapView();
    view->zoomable()->setScale(scale);
    view->forceCenterOn(viewCenter);
}

/**
 * Switches to the given \a mapDocument, taking tilesets into account
 */
void DocumentManager::switchToDocumentAndHandleSimiliarTileset(MapDocument *mapDocument, QPointF viewCenter, qreal scale)
{
    // Try selecting similar layers and tileset by name to the previously active mapitem
    SharedTileset newSimilarTileset;

    if (auto currentMapDocument = qobject_cast<MapDocument*>(currentDocument())) {
        const Layer *currentLayer = currentMapDocument->currentLayer();
        const QList<Layer*> selectedLayers = currentMapDocument->selectedLayers();

        if (currentLayer) {
            Layer *newCurrentLayer = mapDocument->map()->findLayer(currentLayer->name(),
                                                                   currentLayer->layerType());
            if (newCurrentLayer)
                mapDocument->setCurrentLayer(newCurrentLayer);
        }

        QList<Layer*> newSelectedLayers;
        for (Layer *selectedLayer : selectedLayers) {
            Layer *newSelectedLayer = mapDocument->map()->findLayer(selectedLayer->name(),
                                                                    selectedLayer->layerType());
            if (newSelectedLayer)
                newSelectedLayers << newSelectedLayer;
        }
        if (!newSelectedLayers.isEmpty())
            mapDocument->setSelectedLayers(newSelectedLayers);

        Editor *currentEditor = DocumentManager::instance()->currentEditor();
        if (auto currentMapEditor = qobject_cast<MapEditor*>(currentEditor)) {
            if (SharedTileset currentTileset = currentMapEditor->currentTileset()) {
                if (!mapDocument->map()->tilesets().contains(currentTileset))
                    newSimilarTileset = currentTileset->findSimilarTileset(mapDocument->map()->tilesets());
            }
        }
    }

    DocumentManager::instance()->switchToDocument(mapDocument, viewCenter, scale);

    Editor *newEditor = DocumentManager::instance()->currentEditor();
    if (auto newMapEditor = qobject_cast<MapEditor*>(newEditor))
        if (newSimilarTileset)
            newMapEditor->setCurrentTileset(newSimilarTileset);
}

void DocumentManager::switchToLeftDocument()
{
    const int tabCount = mTabBar->count();
    if (tabCount < 2)
        return;

    const int currentIndex = mTabBar->currentIndex();
    switchToDocument((currentIndex > 0 ? currentIndex : tabCount) - 1);
}

void DocumentManager::switchToRightDocument()
{
    const int tabCount = mTabBar->count();
    if (tabCount < 2)
        return;

    const int currentIndex = mTabBar->currentIndex();
    switchToDocument((currentIndex + 1) % tabCount);
}

void DocumentManager::openFileDialog()
{
    emit fileOpenDialogRequested();
}

void DocumentManager::openFile(const QString &path)
{
    emit fileOpenRequested(path);
}

void DocumentManager::saveFile()
{
    emit fileSaveRequested();
}

/**
 * Adds the new or opened \a document to the document manager and makes sure
 * it is the current document.
 */
void DocumentManager::addDocument(const DocumentPtr &document)
{
    EditorManager::instance()->addDocument(document);

    if (mBrokenLinksModel->hasBrokenLinks())
        mBrokenLinksWidget->show();
}

int DocumentManager::insertDocument(int index, const DocumentPtr &document)
{
    EditorManager::instance()->insertDocument(index, document);
    return index;
}

void DocumentManager::onEditorManagerDocumentOpened(Document *document)
{
    const int index = EditorManager::instance()->findDocument(document);
    if (index < 0)
        return;


    if (Editor *editor = mEditorForType.value(document->type()))
        editor->addDocument(document);

    connect(document, &Document::fileNameChanged, this, &DocumentManager::fileNameChanged);
    connect(document, &Document::modifiedChanged, this, [=] { updateDocumentTab(document); });
    connect(document, &Document::isReadOnlyChanged, this, [=] { updateDocumentTab(document); });
    connect(document, &Document::changed, this, &DocumentManager::onDocumentChanged);
    connect(document, &Document::saved, this, &DocumentManager::onDocumentSaved);

    mTabBar->insertTab(index, QString());
    updateDocumentTab(document);

    if (auto *mapDocument = qobject_cast<MapDocument*>(document)) {
        connect(mapDocument, &MapDocument::tilesetAdded, this, &DocumentManager::tilesetAdded);
        connect(mapDocument, &MapDocument::tilesetRemoved, this, &DocumentManager::tilesetRemoved);
    }

    if (auto *tilesetDocument = qobject_cast<TilesetDocument*>(document))
        connect(tilesetDocument, &TilesetDocument::tilesetNameChanged, this, &DocumentManager::tilesetNameChanged);

    if (EditorManager::instance()->currentDocument() == document)
        mTabBar->setCurrentIndex(index);
}

void DocumentManager::onEditorManagerCurrentDocumentChanged(Document *document)
{
    if (!document) {
        mTabBar->setCurrentIndex(-1);
        return;
    }
    const int index = EditorManager::instance()->findDocument(document);
    if (index >= 0 && mTabBar->currentIndex() != index)
        mTabBar->setCurrentIndex(index);
}

void DocumentManager::onEditorManagerDocumentAboutToClose(Document *document)
{
    const int index = EditorManager::instance()->findDocument(document);
    if (index < 0)
        return;

    document->disconnect(this);

    if (Editor *editor = mEditorForType.value(document->type()))
        editor->removeDocument(document);

    if (auto tilesetDocument = qobject_cast<TilesetDocument*>(document)) {
        if (tilesetDocument->mapDocuments().isEmpty()) {
            mTilesetDocumentsModel->remove(tilesetDocument);
            emit tilesetDocumentRemoved(tilesetDocument);
        }
    }

    mTabBar->removeTab(index);

    if (!document->fileName().isEmpty())
        Preferences::instance()->addRecentFile(document->fileName());
}

/**
 * Returns whether the given document has unsaved modifications. For map files
 * with embedded tilesets, that includes checking whether any of the embedded
 * tilesets have unsaved modifications.
 */
bool DocumentManager::isDocumentModified(Document *document) const
{
    if (auto mapDocument = qobject_cast<MapDocument*>(document)) {
        for (const SharedTileset &tileset : mapDocument->map()->tilesets()) {
            if (const auto tilesetDocument = findTilesetDocument(tileset))
                if (tilesetDocument->isEmbedded() && tilesetDocument->isModified())
                    return true;
        }
    }

    return document->isModified();
}

/**
 * Returns whether the given document was changed on disk. Taking into account
 * the case where the given document is an embedded tileset document.
 */
static bool isDocumentChangedOnDisk(Document *document)
{
    if (auto tilesetDocument = qobject_cast<TilesetDocument*>(document)) {
        if (tilesetDocument->isEmbedded())
            document = tilesetDocument->mapDocuments().first();
    }

    return document->changedOnDisk();
}

DocumentPtr DocumentManager::loadDocument(const QString &fileName,
                                          FileFormat *fileFormat,
                                          QString *error)
{
    return EditorManager::instance()->loadDocument(fileName, fileFormat, error);
}

/**
 * Save the given document with the given file name.
 *
 * @return <code>true</code> on success, <code>false</code> on failure
 */
bool DocumentManager::saveDocument(Document *document, const QString &fileName)
{
    QString error;
    if (!EditorManager::instance()->saveDocument(document, fileName, &error)) {
        switchToDocument(document);
        QMessageBox::critical(mWidget->window(), QCoreApplication::translate("Tiled::MainWindow", "Error Saving File"), error);
        return false;
    }
    return true;
}

/**
 * Save the given document with a file name chosen by the user. When saved
 * successfully, the file is added to the list of recent files.
 *
 * @return <code>true</code> on success, <code>false</code> on failure
 */
bool DocumentManager::saveDocumentAs(Document *document)
{
    QString selectedFilter;
    QString fileName = document->fileName();

    if (FileFormat *format = document->writerFormat())
        selectedFilter = format->nameFilter();

    auto getSaveFileName = [&](const QString &filter, const QString &defaultFileName) {
        if (fileName.isEmpty()) {
            fileName = fileDialogStartLocation();
            fileName += QLatin1Char('/');
            fileName += defaultFileName;
            fileName += Utils::firstExtension(selectedFilter);
        }

        while (true) {
            fileName = QFileDialog::getSaveFileName(mWidget->window(), tr("Save File As"),
                                                    fileName,
                                                    filter,
                                                    &selectedFilter);

            if (!fileName.isEmpty() &&
                !Utils::fileNameMatchesNameFilter(fileName, selectedFilter))
            {
                QMessageBox messageBox(QMessageBox::Warning,
                                       QCoreApplication::translate("Tiled::MainWindow", "Extension Mismatch"),
                                       QCoreApplication::translate("Tiled::MainWindow", "The file extension does not match the chosen file type."),
                                       QMessageBox::Yes | QMessageBox::No,
                                       mWidget->window());

                messageBox.setInformativeText(QCoreApplication::translate("Tiled::MainWindow",
                                                                          "Tiled may not automatically recognize your file when loading. "
                                                                          "Are you sure you want to save with this extension?"));

                int answer = messageBox.exec();
                if (answer != QMessageBox::Yes)
                    continue;
            }
            return fileName;
        }
    };

    if (auto mapDocument = qobject_cast<MapDocument*>(document)) {
        FormatHelper<MapFormat> helper(FileFormat::ReadWrite);
        SessionOption<QString> lastUsedMapFormat { "map.lastUsedFormat" };

        if (selectedFilter.isEmpty()) {
            if (auto format = helper.findFormat(lastUsedMapFormat))
                selectedFilter = format->nameFilter();
        }

        if (selectedFilter.isEmpty())
            selectedFilter = TmxMapFormat().nameFilter();

        auto suggestedFileName = QCoreApplication::translate("Tiled::MainWindow", "untitled");

        fileName = getSaveFileName(helper.filter(), suggestedFileName);
        if (fileName.isEmpty())
            return false;

        MapFormat *format = helper.formatByNameFilter(selectedFilter);
        mapDocument->setWriterFormat(format);
        mapDocument->setReaderFormat(format);

        lastUsedMapFormat = format->shortName();

    } else if (auto tilesetDocument = qobject_cast<TilesetDocument*>(document)) {
        FormatHelper<TilesetFormat> helper(FileFormat::ReadWrite);
        SessionOption<QString> lastUsedTilesetFormat { "tileset.lastUsedFormat" };

        if (selectedFilter.isEmpty()) {
            if (auto format = helper.findFormat(lastUsedTilesetFormat))
                selectedFilter = format->nameFilter();
        }

        if (selectedFilter.isEmpty())
            selectedFilter = TsxTilesetFormat().nameFilter();

        auto suggestedFileName = tilesetDocument->tileset()->name().trimmed();
        if (suggestedFileName.isEmpty())
            suggestedFileName = QCoreApplication::translate("Tiled::MainWindow", "untitled");

        fileName = getSaveFileName(helper.filter(), suggestedFileName);
        if (fileName.isEmpty())
            return false;

        TilesetFormat *format = helper.formatByNameFilter(selectedFilter);
        tilesetDocument->setWriterFormat(format);

        lastUsedTilesetFormat = format->shortName();
    }

    return saveDocument(document, fileName);
}

/**
 * Closes the current map document. Will not ask the user whether to save
 * any changes!
 */
void DocumentManager::closeCurrentDocument()
{
    const int index = mTabBar->currentIndex();
    if (index == -1)
        return;

    closeDocumentAt(index);
}

/**
 * Close all documents. Will not ask the user whether to save any changes!
 */
void DocumentManager::closeAllDocuments()
{
    EditorManager::instance()->closeAllDocuments();
}

/**
 * Closes all documents except the one pointed to by index.
 */
void DocumentManager::closeOtherDocuments(int index)
{
    if (index == -1)
        return;

    mMultiDocumentClose = true;

    for (int i = mTabBar->count() - 1; i >= 0; --i) {
        if (i != index)
            emit documentCloseRequested(i);

        if (!mMultiDocumentClose)
            return;
    }
}

/**
 * Closes all documents whose tabs are to the right of the index.
 */
void DocumentManager::closeDocumentsToRight(int index)
{
    if (index == -1)
        return;

    mMultiDocumentClose = true;

    for (int i = mTabBar->count() - 1; i > index; --i) {
        emit documentCloseRequested(i);

        if (!mMultiDocumentClose)
            return;
    }
}

/**
 * Closes the document at the given \a index. Will not ask the user whether
 * to save any changes!
 *
 * The file is added to the list of recent files (in onEditorManagerDocumentAboutToClose).
 */
void DocumentManager::closeDocumentAt(int index)
{
    EditorManager::instance()->closeDocumentAt(index);
}

/**
 * Reloads the current document. Will not ask the user whether to save any
 * changes!
 *
 * \sa reloadDocumentAt()
 */
bool DocumentManager::reloadCurrentDocument()
{
    const int index = mTabBar->currentIndex();
    if (index == -1)
        return false;

    return reloadDocumentAt(index);
}

/**
 * Reloads the document at the given \a index. Will not ask the user whether to
 * save any changes!
 *
 * Returns whether the document reloaded successfully.
 */
bool DocumentManager::reloadDocumentAt(int index)
{
    const auto &docs = EditorManager::instance()->documents();
    if (index < 0 || index >= docs.size())
        return false;
    return reloadDocument(docs.at(index).data());
}

/**
 * Reloads the given \a document.
 *
 * The document may not actually be open in any editor. It might be a map that
 * is loaded as part of a world, or a tileset that is loaded as part of a map.
 *
 * Returns whether the document reloaded successfully.
 */
bool DocumentManager::reloadDocument(Document *document)
{
    if (!EditorManager::instance()->reloadDocument(document))
        return false;

    if (auto mapDocument = qobject_cast<MapDocument*>(document)) {
        if (document == currentDocument() && mBrokenLinksModel->hasBrokenLinks())
            mBrokenLinksWidget->show();
        if (findDocument(document) != -1)
            checkTilesetColumns(mapDocument);
    }
    if (auto current = currentDocument())
        if (!isDocumentChangedOnDisk(current))
            mFileChangedWarning->setVisible(false);

    return true;
}

void DocumentManager::currentIndexChanged()
{
    auto document = currentDocument();
    EditorManager::instance()->setCurrentDocument(document);
    Editor *editor = nullptr;
    bool changed = false;

    if (document) {
        editor = mEditorForType.value(document->type());
        changed = isDocumentChangedOnDisk(document);
    }

    QWidget *editorWidget = mNoEditorWidget;

    if (editor) {
        editor->setCurrentDocument(document);
        editorWidget = editor->editorWidget();
    }

    if (mEditorStack->currentWidget() != editorWidget) {
        mEditorStack->setCurrentWidget(editorWidget);
        emit currentEditorChanged(editor);
    }

    mFileChangedWarning->setVisible(changed);

    mBrokenLinksModel->setDocument(document);

    emit currentDocumentChanged(document);
}

void DocumentManager::fileNameChanged(const QString &/* fileName */,
                                      const QString &/* oldFileName */)
{
    Document *document = static_cast<Document*>(sender());

    // Update the tabs for all opened embedded tilesets
    if (MapDocument *mapDocument = qobject_cast<MapDocument*>(document)) {
        for (const SharedTileset &tileset : mapDocument->map()->tilesets()) {
            if (auto tilesetDocument = findTilesetDocument(tileset))
                updateDocumentTab(tilesetDocument);
        }
    }

    updateDocumentTab(document);
}

void DocumentManager::updateDocumentTab(Document *document)
{
    const int index = findDocument(document);
    if (index == -1)
        return;

    QIcon tabIcon = document->isReadOnly() ? mLockedIcon : QIcon();
    QString tabText = document->displayName();
    QString tabToolTip = document->fileName();

    if (document->isModified())
        tabText.prepend(QLatin1Char('*'));
    if (document->isReadOnly())
        tabToolTip = tr("%1 [read-only]").arg(tabToolTip);

    mTabBar->setTabIcon(index, tabIcon);
    mTabBar->setTabText(index, tabText);
    mTabBar->setTabToolTip(index, tabToolTip);
}

void DocumentManager::onDocumentChanged(const ChangeEvent &event)
{
    auto mapDocument = qobject_cast<MapDocument*>(sender());
    if (!mapDocument)
        return;

    // In case a map is reloaded, the set of used tilesets might have changed
    switch (event.type) {
    case ChangeEvent::DocumentAboutToReload:
        for (const SharedTileset &tileset : mapDocument->map()->tilesets())
            removeFromTilesetDocument(tileset, mapDocument);
        break;
    case ChangeEvent::DocumentReloaded:
        for (const SharedTileset &tileset : mapDocument->map()->tilesets())
            addToTilesetDocument(tileset, mapDocument);
        break;
    default:
        break;
    }
}

void DocumentManager::onDocumentSaved()
{
    Document *document = static_cast<Document*>(sender());

    if (document->changedOnDisk()) {
        document->setChangedOnDisk(false);
        if (!isDocumentModified(currentDocument()))
            mFileChangedWarning->setVisible(false);
    }
}

void DocumentManager::documentTabMoved(int from, int to)
{

    EditorManager::instance()->moveDocument(from, to);
}

void DocumentManager::tabContextMenuRequested(const QPoint &pos)
{
    int index = mTabBar->tabAt(pos);
    if (index == -1)
        return;

    QMenu menu(mTabBar->window());

    const Document *fileDocument = EditorManager::instance()->documents().at(index).data();
    if (fileDocument->type() == Document::TilesetDocumentType) {
        auto tilesetDocument = static_cast<const TilesetDocument*>(fileDocument);
        if (tilesetDocument->isEmbedded())
            fileDocument = tilesetDocument->mapDocuments().first();
    }

    Utils::addFileManagerActions(menu, fileDocument->fileName());

    menu.addSeparator();

    QAction *closeTab = menu.addAction(tr("Close"), [this, index] {
        documentCloseRequested(index);
    });
    closeTab->setIcon(QIcon(QStringLiteral(":/images/16/window-close.png")));
    Utils::setThemeIcon(closeTab, "window-close");

    menu.addAction(tr("Close Other Tabs"), [this, index] {
        closeOtherDocuments(index);
    });

    menu.addAction(tr("Close Tabs to the Right"), [this, index] {
        closeDocumentsToRight(index);
    });

    menu.exec(mTabBar->mapToGlobal(pos));
}

void DocumentManager::tilesetAdded(int index, Tileset *tileset)
{
    Q_UNUSED(index)
    MapDocument *mapDocument = static_cast<MapDocument*>(QObject::sender());
    addToTilesetDocument(tileset->sharedFromThis(), mapDocument);
}

void DocumentManager::tilesetRemoved(Tileset *tileset)
{
    MapDocument *mapDocument = static_cast<MapDocument*>(QObject::sender());
    removeFromTilesetDocument(tileset->sharedFromThis(), mapDocument);
}

void DocumentManager::tilesetNameChanged(Tileset *tileset)
{
    auto *tilesetDocument = findTilesetDocument(tileset->sharedFromThis());
    if (tilesetDocument->isEmbedded())
        updateDocumentTab(tilesetDocument);
}

void DocumentManager::filesChanged(const QStringList &fileNames)
{
    for (const QString &fileName : fileNames)
        fileChanged(fileName);
}

void DocumentManager::fileChanged(const QString &fileName)
{
    const auto document = EditorManager::instance()->documentForFileName(fileName);
    if (!document) {
        qWarning() << "Document not found for changed file:" << fileName;
        return;
    }

    const QFileInfo fileInfo { fileName };

    // Always update potentially changed read-only state
    document->setReadOnly(fileInfo.exists() && !fileInfo.isWritable());

    // Ignore change event when it seems to be our own save
    if (fileInfo.lastModified() == document->lastSaved())
        return;

    // Automatically reload when there are no unsaved changes
    if (!isDocumentModified(document)) {
        reloadDocument(document);
        return;
    }

    document->setChangedOnDisk(true);

    // We may need to show the file changed warning
    if (auto current = currentDocument())
        if (isDocumentChangedOnDisk(current))
            mFileChangedWarning->setVisible(true);
}

void DocumentManager::hideChangedWarning()
{
    Document *document = currentDocument();
    if (auto tilesetDocument = qobject_cast<TilesetDocument*>(document)) {
        if (tilesetDocument->isEmbedded())
            document = tilesetDocument->mapDocuments().first();
    }

    document->setChangedOnDisk(false);
    mFileChangedWarning->setVisible(false);
}

TilesetDocument* DocumentManager::findTilesetDocument(const SharedTileset &tileset) const
{
    return EditorManager::instance()->findTilesetDocument(tileset);
}

TilesetDocument* DocumentManager::findTilesetDocument(const QString &fileName) const
{
    return EditorManager::instance()->findTilesetDocument(fileName);
}

/**
 * Opens the document for the given \a tileset. Returns the tileset's document.
 */
TilesetDocument *DocumentManager::openTileset(const SharedTileset &tileset)
{
    return EditorManager::instance()->openTileset(tileset);
}

void DocumentManager::addToTilesetDocument(const SharedTileset &tileset, MapDocument *mapDocument)
{
    EditorManager::instance()->addToTilesetDocument(tileset, mapDocument);
}

void DocumentManager::removeFromTilesetDocument(const SharedTileset &tileset, MapDocument *mapDocument)
{
    EditorManager::instance()->removeFromTilesetDocument(tileset, mapDocument);
}

void DocumentManager::updateSession() const
{
    QStringList fileList;
    for (const auto &document : EditorManager::instance()->documents()) {
        if (!document->fileName().isEmpty())
            fileList.append(document->fileName());
    }

    auto doc = currentDocument();

    auto &session = Session::current();
    session.setOpenFiles(fileList);
    session.setActiveFile(doc ? doc->fileName() : QString());
}

MapDocument *DocumentManager::openMapFile(const QString &path)
{
    openFile(path);
    const int i = EditorManager::instance()->findDocument(path);
    const auto &docs = EditorManager::instance()->documents();
    return i == -1 || i >= docs.size() ? nullptr : qobject_cast<MapDocument*>(docs.at(i).data());
}

TilesetDocument *DocumentManager::openTilesetFile(const QString &path)
{
    openFile(path);
    const int i = EditorManager::instance()->findDocument(path);
    const auto &docs = EditorManager::instance()->documents();
    return i == -1 || i >= docs.size() ? nullptr : qobject_cast<TilesetDocument*>(docs.at(i).data());
}


bool DocumentManager::isAnyWorldModified() const
{
    for (auto &worldDocument : WorldManager::instance().worlds())
        if (worldDocument->isModified())
            return true;

    return false;
}

/**
 * Returns a logical start location for a file dialog to open a file, based on
 * the currently selected file, a recent file, the project path or finally, the
 * home location.
 */
QString DocumentManager::fileDialogStartLocation() const
{
    if (auto doc = currentDocument()) {
        QString path = QFileInfo(doc->fileName()).path();
        if (!path.isEmpty())
            return path;
    }

    const auto &session = Session::current();
    if (!session.recentFiles.isEmpty())
        return QFileInfo(session.recentFiles.first()).path();

    const auto &project = ProjectManager::instance()->project();
    if (!project.fileName().isEmpty())
        return QFileInfo(project.fileName()).path();

    return Preferences::homeLocation();
}

void DocumentManager::onWorldLoaded(WorldDocument *worldDocument)
{
    EditorManager::instance()->undoGroup()->addStack(worldDocument->undoStack());
    emit documentOpened(worldDocument);
}

void DocumentManager::onWorldUnloaded(WorldDocument *worldDocument)
{
    EditorManager::instance()->undoGroup()->removeStack(worldDocument->undoStack());
}

static bool mayNeedColumnCountAdjustment(const Tileset &tileset)
{
    if (tileset.isCollection())
        return false;
    if (tileset.imageStatus() != LoadingReady)
        return false;
    if (tileset.columnCount() == tileset.expectedColumnCount())
        return false;
    if (tileset.columnCount() == 0 || tileset.expectedColumnCount() == 0)
        return false;
    if (tileset.expectedRowCount() < 2 || tileset.rowCount() < 2)
        return false;

    return true;
}

void DocumentManager::tilesetImagesChanged(Tileset *tileset)
{
    if (!mayNeedColumnCountAdjustment(*tileset))
        return;

    SharedTileset sharedTileset = tileset->sharedFromThis();
    QList<Document*> affectedDocuments;

    for (const auto &document : std::as_const(EditorManager::instance()->documents())) {
        if (auto mapDocument = qobject_cast<MapDocument*>(document.data())) {
            if (mapDocument->map()->tilesets().contains(sharedTileset))
                affectedDocuments.append(document.data());
        }
    }

    if (TilesetDocument *tilesetDocument = findTilesetDocument(sharedTileset))
        affectedDocuments.append(tilesetDocument);

    if (!affectedDocuments.isEmpty() && askForAdjustment(*tileset)) {
        for (Document *document : std::as_const(affectedDocuments)) {
            if (auto mapDocument = qobject_cast<MapDocument*>(document)) {
                auto command = new AdjustTileIndexes(mapDocument, *tileset);
                document->undoStack()->push(command);
            } else if (auto tilesetDocument = qobject_cast<TilesetDocument*>(document)) {
                auto command = new AdjustTileMetaData(tilesetDocument);
                document->undoStack()->push(command);
            }
        }
    }

    tileset->syncExpectedColumnsAndRows();
}

/**
 * Checks whether the number of columns in tileset image based tilesets matches
 * with the expected amount. Offers to adjust tile indexes if not.
 */
void DocumentManager::checkTilesetColumns(MapDocument *mapDocument)
{
    for (const SharedTileset &tileset : mapDocument->map()->tilesets()) {
        TilesetDocument *tilesetDocument = findTilesetDocument(tileset);
        Q_ASSERT(tilesetDocument);

        if (checkTilesetColumns(tilesetDocument)) {
            auto command = new AdjustTileIndexes(mapDocument, *tileset);
            mapDocument->undoStack()->push(command);
        }

        tileset->syncExpectedColumnsAndRows();
    }
}

bool DocumentManager::checkTilesetColumns(TilesetDocument *tilesetDocument)
{
    if (!mayNeedColumnCountAdjustment(*tilesetDocument->tileset()))
        return false;

    if (askForAdjustment(*tilesetDocument->tileset())) {
        auto command = new AdjustTileMetaData(tilesetDocument);
        tilesetDocument->undoStack()->push(command);
        return true;
    }

    return false;
}

bool DocumentManager::askForAdjustment(const Tileset &tileset)
{
    int r = QMessageBox::question(mWidget->window(),
                                  tr("Tileset Columns Changed"),
                                  tr("The number of tile columns in the tileset '%1' appears to have changed from %2 to %3. "
                                     "Do you want to adjust tile references?")
                                  .arg(tileset.name())
                                  .arg(tileset.expectedColumnCount())
                                  .arg(tileset.columnCount()),
                                  QMessageBox::Yes | QMessageBox::No,
                                  QMessageBox::Yes);

    return r == QMessageBox::Yes;
}

/**
 * Unsets a flag to stop closeOtherDocuments() and closeDocumentsToRight()
 * when Cancel is pressed
 */
void DocumentManager::abortMultiDocumentClose()
{
    mMultiDocumentClose = false;
}

#include "moc_documentmanager.cpp"
