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
#include "terrain.h"
#include "tilesetdocument.h"
#include "tilesetdocumentsmodel.h"
#include "tilesetmanager.h"
#include "tmxmapformat.h"
#include "utils.h"
#include "wangset.h"
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
#include <QTabWidget>
#include <QUndoGroup>
#include <QUndoStack>
#include <QVBoxLayout>

#include "qtcompat_p.h"

using namespace Tiled;


DocumentManager *DocumentManager::mInstance;

DocumentManager *DocumentManager::instance()
{
    if (!mInstance)
        mInstance = new DocumentManager;
    return mInstance;
}

void DocumentManager::deleteInstance()
{
    delete mInstance;
    mInstance = nullptr;
}

DocumentManager::DocumentManager(QObject *parent)
    : QObject(parent)
    , mTilesetDocumentsModel(new TilesetDocumentsModel(this))
    , mWidget(new QWidget)
    , mNoEditorWidget(new NoEditorWidget(mWidget))
    , mTabBar(new QTabBar(mWidget))
    , mFileChangedWarning(new FileChangedWarning(mWidget))
    , mBrokenLinksModel(new BrokenLinksModel(this))
    , mBrokenLinksWidget(new BrokenLinksWidget(mBrokenLinksModel, mWidget))
    , mMapEditor(nullptr) // todo: look into removing this
    , mUndoGroup(new QUndoGroup(this))
    , mFileSystemWatcher(new FileSystemWatcher(this))
    , mMultiDocumentClose(false)
{
    mBrokenLinksWidget->setVisible(false);

    mTabBar->setExpanding(false);
    mTabBar->setDocumentMode(true);
    mTabBar->setTabsClosable(true);
    mTabBar->setMovable(true);
    mTabBar->setContextMenuPolicy(Qt::CustomContextMenu);

    mFileChangedWarning->setVisible(false);

    connect(mFileChangedWarning, &FileChangedWarning::reload, this, &DocumentManager::reloadCurrentDocument);
    connect(mFileChangedWarning, &FileChangedWarning::ignore, this, &DocumentManager::hideChangedWarning);

    QVBoxLayout *vertical = new QVBoxLayout(mWidget);
    vertical->addWidget(mTabBar);
    vertical->addWidget(mFileChangedWarning);
    vertical->addWidget(mBrokenLinksWidget);
    vertical->setMargin(0);
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

    connect(mFileSystemWatcher, &FileSystemWatcher::fileChanged,
            this, &DocumentManager::fileChanged);

    connect(mBrokenLinksModel, &BrokenLinksModel::hasBrokenLinksChanged,
            mBrokenLinksWidget, &BrokenLinksWidget::setVisible);

    connect(TilesetManager::instance(), &TilesetManager::tilesetImagesChanged,
            this, &DocumentManager::tilesetImagesChanged);

    OpenFile::activated = [this] (const OpenFile &open) {
        openFile(open.file);
    };

    JumpToTile::activated = [this] (const JumpToTile &jump) {
        if (auto mapDocument = openMapFile(jump.mapFile)) {
            auto renderer = mapDocument->renderer();
            auto mapView = viewForDocument(mapDocument);
            auto pos = renderer->tileToScreenCoords(jump.tilePos);
            mapView->forceCenterOn(pos);

            if (auto layer = mapDocument->map()->findLayerById(jump.layerId))
                mapDocument->switchSelectedLayers({ layer });
        }
    };

    JumpToObject::activated = [this] (const JumpToObject &jump) {
        if (auto mapDocument = openMapFile(jump.mapFile)) {
            if (auto object = mapDocument->map()->findObjectById(jump.objectId)) {
                mapDocument->focusMapObjectRequested(object);
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
        const int i = findDocument(select.fileName);
        if (i == -1)
            return;

        auto doc = mDocuments.at(i).data();
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
                    mapDocument->focusMapObjectRequested(object);
                    mapDocument->setSelectedObjects({ object });
                    obj = object;
                }
                break;
            case Object::MapType:
                obj = mapDocument->map();
                break;
            case Object::ObjectTemplateType:
                emit templateOpenRequested(select.fileName);
                // todo: can't access Object pointer
                break;
            }
            break;
        }
        case Document::TilesetDocumentType:
            auto tilesetDocument = static_cast<TilesetDocument*>(doc);
            switch (select.objectType) {
            case Object::MapObjectType:
                // todo: no way to know to which tile this object belongs
                break;
            case Object::TerrainType:
                // todo: select the terrain
                if (select.id < tilesetDocument->tileset()->terrainCount())
                    obj = tilesetDocument->tileset()->terrain(select.id);
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
            case Object::ObjectTemplateType:
                emit templateOpenRequested(select.fileName);
                // todo: can't access Object pointer
                break;
            }
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

    mTabBar->installEventFilter(this);
}

DocumentManager::~DocumentManager()
{
    mTabBar->removeEventFilter(this);

    // All documents should be closed gracefully beforehand
    Q_ASSERT(mDocuments.isEmpty());
    Q_ASSERT(mTilesetDocumentsModel->rowCount() == 0);
    delete mWidget;
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

void DocumentManager::deleteEditor(Document::DocumentType documentType)
{
    Q_ASSERT(mEditorForType.contains(documentType));
    Editor *editor = mEditorForType.take(documentType);
    if (editor == mMapEditor)
        mMapEditor = nullptr;
    delete editor;
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
 * Returns the current map document, or 0 when there is none.
 */
Document *DocumentManager::currentDocument() const
{
    const int index = mTabBar->currentIndex();
    if (index == -1)
        return nullptr;

    return mDocuments.at(index).data();
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
 * Searches for a document with the given \a fileName and returns its
 * index. Returns -1 when the document isn't open.
 */
int DocumentManager::findDocument(const QString &fileName) const
{
    const QString canonicalFilePath = QFileInfo(fileName).canonicalFilePath();
    if (canonicalFilePath.isEmpty()) // file doesn't exist
        return -1;

    for (int i = 0; i < mDocuments.size(); ++i) {
        if (mDocuments.at(i)->canonicalFilePath() == canonicalFilePath)
            return i;
    }

    return -1;
}

int DocumentManager::findDocument(Document *document) const
{
    return indexOf(mDocuments, document);
}

/**
 * Switches to the map document at the given \a index.
 */
void DocumentManager::switchToDocument(int index)
{
    mTabBar->setCurrentIndex(index);
}

/**
 * Switches to the given \a document, if there is already a tab open for it.
 * \return whether the switch was succesful
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
 * Adds the new or opened \a document to the document manager.
 */
void DocumentManager::addDocument(const DocumentPtr &document)
{
    insertDocument(mDocuments.size(), document);
}

void DocumentManager::insertDocument(int index, const DocumentPtr &document)
{
    Q_ASSERT(document);
    Q_ASSERT(!mDocuments.contains(document));

    mDocuments.insert(index, document);
    mUndoGroup->addStack(document->undoStack());

    Document *documentPtr = document.data();

    if (auto mapDocument = qobject_cast<MapDocument*>(documentPtr)) {
        for (const SharedTileset &tileset : mapDocument->map()->tilesets())
            addToTilesetDocument(tileset, mapDocument);
    } else if (auto tilesetDocument = qobject_cast<TilesetDocument*>(documentPtr)) {
        // We may have opened a bare tileset that wasn't seen before
        if (!mTilesetDocumentsModel->contains(tilesetDocument)) {
            mTilesetDocumentsModel->append(tilesetDocument);
            emit tilesetDocumentAdded(tilesetDocument);
        }
    }

    if (!document->fileName().isEmpty())
        mFileSystemWatcher->addPath(document->fileName());

    if (Editor *editor = mEditorForType.value(document->type()))
        editor->addDocument(documentPtr);

    QString tabText = document->displayName();
    if (document->isModified())
        tabText.prepend(QLatin1Char('*'));

    const int documentIndex = mTabBar->insertTab(index, tabText);
    mTabBar->setTabToolTip(documentIndex, document->fileName());

    connect(documentPtr, &Document::fileNameChanged, this, &DocumentManager::fileNameChanged);
    connect(documentPtr, &Document::modifiedChanged, this, [=] { updateDocumentTab(documentPtr); });
    connect(documentPtr, &Document::saved, this, &DocumentManager::onDocumentSaved);

    if (auto *mapDocument = qobject_cast<MapDocument*>(documentPtr)) {
        connect(mapDocument, &MapDocument::tilesetAdded, this, &DocumentManager::tilesetAdded);
        connect(mapDocument, &MapDocument::tilesetRemoved, this, &DocumentManager::tilesetRemoved);
    }

    if (auto *tilesetDocument = qobject_cast<TilesetDocument*>(documentPtr))
        connect(tilesetDocument, &TilesetDocument::tilesetNameChanged, this, &DocumentManager::tilesetNameChanged);

    switchToDocument(documentIndex);

    if (mBrokenLinksModel->hasBrokenLinks())
        mBrokenLinksWidget->show();

    emit documentOpened(documentPtr);
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
    // Try to find it in already loaded documents
    QString canonicalFilePath = QFileInfo(fileName).canonicalFilePath();
    if (Document *doc = Document::documentInstances().value(canonicalFilePath))
        return doc->sharedFromThis();

    if (!fileFormat) {
        // Try to find a plugin that implements support for this format
        const auto formats = PluginManager::objects<FileFormat>();
        for (FileFormat *format : formats) {
            if (format->supportsFile(fileName)) {
                fileFormat = format;
                break;
            }
        }
    }

    if (!fileFormat) {
        if (error)
            *error = tr("Unrecognized file format.");
        return DocumentPtr();
    }

    DocumentPtr document;

    if (MapFormat *mapFormat = qobject_cast<MapFormat*>(fileFormat)) {
        document = MapDocument::load(fileName, mapFormat, error);
    } else if (TilesetFormat *tilesetFormat = qobject_cast<TilesetFormat*>(fileFormat)) {
        // It could be, that we have already loaded this tileset while loading some map.
        if (auto tilesetDocument = findTilesetDocument(fileName)) {
            document = tilesetDocument->sharedFromThis();
        } else {
            document = TilesetDocument::load(fileName, tilesetFormat, error);
        }
    }

    return document;
}

/**
 * Save the given document with the given file name. When saved
 * successfully, the file is added to the list of recent files.
 *
 * @return <code>true</code> on success, <code>false</code> on failure
 */
bool DocumentManager::saveDocument(Document *document, const QString &fileName)
{
    if (fileName.isEmpty())
        return false;

    emit documentAboutToBeSaved(document);

    QString error;
    if (!document->save(fileName, &error)) {
        QMessageBox::critical(mWidget->window(), QCoreApplication::translate("Tiled::MainWindow", "Error Saving File"), error);
        return false;
    }

    Preferences::instance()->addRecentFile(fileName);

    emit documentSaved(document);

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
    QString filter;
    QString selectedFilter;
    QString fileName = document->fileName();

    if (FileFormat *format = document->writerFormat())
        selectedFilter = format->nameFilter();

    auto getSaveFileName = [&,this](const QString &defaultFileName) {
        if (fileName.isEmpty()) {
            fileName = Preferences::instance()->fileDialogStartLocation();
            fileName += QLatin1Char('/');
            fileName += defaultFileName;
        }

        while (true) {
            fileName = QFileDialog::getSaveFileName(mWidget->window(), tr("Save File As"),
                                                    fileName,
                                                    filter,
                                                    &selectedFilter);

            if (!fileName.isEmpty() &&
                !Utils::fileNameMatchesNameFilter(QFileInfo(fileName).fileName(), selectedFilter))
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
        if (selectedFilter.isEmpty())
            selectedFilter = TmxMapFormat().nameFilter();

        FormatHelper<MapFormat> helper(FileFormat::ReadWrite);
        filter = helper.filter();

        auto suggestedFileName = QCoreApplication::translate("Tiled::MainWindow", "untitled");
        suggestedFileName.append(QLatin1String(".tmx"));

        fileName = getSaveFileName(suggestedFileName);
        if (fileName.isEmpty())
            return false;

        MapFormat *format = helper.formatByNameFilter(selectedFilter);
        mapDocument->setWriterFormat(format);
        mapDocument->setReaderFormat(format);

    } else if (auto tilesetDocument = qobject_cast<TilesetDocument*>(document)) {
        if (selectedFilter.isEmpty())
            selectedFilter = TsxTilesetFormat().nameFilter();

        FormatHelper<TilesetFormat> helper(FileFormat::ReadWrite);
        filter = helper.filter();

        auto suggestedFileName = tilesetDocument->tileset()->name().trimmed();
        if (suggestedFileName.isEmpty())
            suggestedFileName = QCoreApplication::translate("Tiled::MainWindow", "untitled");
        suggestedFileName.append(QLatin1String(".tsx"));

        fileName = getSaveFileName(suggestedFileName);
        if (fileName.isEmpty())
            return false;

        TilesetFormat *format = helper.formatByNameFilter(selectedFilter);
        tilesetDocument->setWriterFormat(format);
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
    while (!mDocuments.isEmpty())
        closeCurrentDocument();
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
            documentCloseRequested(i);

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
        documentCloseRequested(i);

        if (!mMultiDocumentClose)
            return;
    }
}

/**
 * Closes the document at the given \a index. Will not ask the user whether
 * to save any changes!
 */
void DocumentManager::closeDocumentAt(int index)
{
    auto document = mDocuments.at(index);       // keeps alive and may delete

    emit documentAboutToClose(document.data());

    mDocuments.removeAt(index);
    mTabBar->removeTab(index);

    if (Editor *editor = mEditorForType.value(document->type()))
        editor->removeDocument(document.data());

    if (!document->fileName().isEmpty()) {
        mFileSystemWatcher->removePath(document->fileName());
        document->setChangedOnDisk(false);
    }

    if (auto mapDocument = qobject_cast<MapDocument*>(document.data())) {
        for (const SharedTileset &tileset : mapDocument->map()->tilesets())
            removeFromTilesetDocument(tileset, mapDocument);
    } else if (auto tilesetDocument = qobject_cast<TilesetDocument*>(document.data())) {
        if (tilesetDocument->mapDocuments().isEmpty()) {
            mTilesetDocumentsModel->remove(tilesetDocument);
            emit tilesetDocumentRemoved(tilesetDocument);
        } else {
            tilesetDocument->disconnect(this);
        }
    }
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
 * Reloads the document at the given \a index. It will lose any undo
 * history and current selections. Will not ask the user whether to save
 * any changes!
 *
 * Returns whether the document loaded successfully.
 */
bool DocumentManager::reloadDocumentAt(int index)
{
    const auto oldDocument = mDocuments.at(index);
    QString error;

    if (auto mapDocument = oldDocument.objectCast<MapDocument>()) {
        // TODO: Consider fixing the reload to avoid recreating the MapDocument
        auto newDocument = MapDocument::load(oldDocument->fileName(),
                                             mapDocument->readerFormat(),
                                             &error);
        if (!newDocument) {
            emit reloadError(tr("%1:\n\n%2").arg(oldDocument->fileName(), error));
            return false;
        }

        // Save the document state, to ensure the new document will match it
        static_cast<MapEditor*>(editor(Document::MapDocumentType))->saveDocumentState(mapDocument.data());

        // Replace old tab
        insertDocument(index, newDocument); // also selects the new document
        closeDocumentAt(index + 1);

        checkTilesetColumns(newDocument.data());

    } else if (auto tilesetDocument = qobject_cast<TilesetDocument*>(oldDocument)) {
        if (tilesetDocument->isEmbedded()) {
            // For embedded tilesets, we need to reload the map
            index = findDocument(tilesetDocument->mapDocuments().first());
            if (!reloadDocumentAt(index))
                return false;
        } else if (!tilesetDocument->reload(&error)) {
            emit reloadError(tr("%1:\n\n%2").arg(oldDocument->fileName(), error));
            return false;
        }

        tilesetDocument->setChangedOnDisk(false);
    }

    if (!isDocumentChangedOnDisk(currentDocument()))
        mFileChangedWarning->setVisible(false);

    return true;
}

void DocumentManager::currentIndexChanged()
{
    auto document = currentDocument();
    Editor *editor = nullptr;
    bool changed = false;

    if (document) {
        editor = mEditorForType.value(document->type());
        mUndoGroup->setActiveStack(document->undoStack());

        changed = isDocumentChangedOnDisk(document);
    }

    if (editor) {
        editor->setCurrentDocument(document);
        mEditorStack->setCurrentWidget(editor->editorWidget());
    } else {
        mEditorStack->setCurrentWidget(mNoEditorWidget);
    }

    mFileChangedWarning->setVisible(changed);

    mBrokenLinksModel->setDocument(document);

    emit currentDocumentChanged(document);
}

void DocumentManager::fileNameChanged(const QString &fileName,
                                      const QString &oldFileName)
{
    if (!fileName.isEmpty())
        mFileSystemWatcher->addPath(fileName);
    if (!oldFileName.isEmpty())
        mFileSystemWatcher->removePath(oldFileName);

    // Update the tabs for all opened embedded tilesets
    Document *document = static_cast<Document*>(sender());
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

    QString tabText = document->displayName();
    if (document->isModified())
        tabText.prepend(QLatin1Char('*'));

    mTabBar->setTabText(index, tabText);
    mTabBar->setTabToolTip(index, document->fileName());
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
    mDocuments.move(from, to);
}

void DocumentManager::tabContextMenuRequested(const QPoint &pos)
{
    int index = mTabBar->tabAt(pos);
    if (index == -1)
        return;

    QMenu menu(mTabBar->window());

    const Document *fileDocument = mDocuments.at(index).data();
    if (fileDocument->type() == Document::TilesetDocumentType) {
        auto tilesetDocument = static_cast<const TilesetDocument*>(fileDocument);
        if (tilesetDocument->isEmbedded())
            fileDocument = tilesetDocument->mapDocuments().first();
    }

    Utils::addFileManagerActions(menu, fileDocument->fileName());

    menu.addSeparator();

    QAction *closeTab = menu.addAction(tr("Close"));
    closeTab->setIcon(QIcon(QStringLiteral(":/images/16/window-close.png")));
    Utils::setThemeIcon(closeTab, "window-close");
    connect(closeTab, &QAction::triggered, [this, index] {
        documentCloseRequested(index);
    });

    QAction *closeOtherTabs = menu.addAction(tr("Close Other Tabs"));
    connect(closeOtherTabs, &QAction::triggered, [this, index] {
        closeOtherDocuments(index);
    });

    QAction *closeTabsToRight = menu.addAction(tr("Close Tabs to the Right"));
    connect(closeTabsToRight, &QAction::triggered, [this, index] {
        closeDocumentsToRight(index);
    });

    menu.exec(mTabBar->mapToGlobal(pos));
}

void DocumentManager::tilesetAdded(int index, Tileset *tileset)
{
    Q_UNUSED(index)
    MapDocument *mapDocument = static_cast<MapDocument*>(QObject::sender());
    addToTilesetDocument(tileset->sharedPointer(), mapDocument);
}

void DocumentManager::tilesetRemoved(Tileset *tileset)
{
    MapDocument *mapDocument = static_cast<MapDocument*>(QObject::sender());
    removeFromTilesetDocument(tileset->sharedPointer(), mapDocument);
}

void DocumentManager::tilesetNameChanged(Tileset *tileset)
{
    auto *tilesetDocument = findTilesetDocument(tileset->sharedPointer());
    if (tilesetDocument->isEmbedded())
        updateDocumentTab(tilesetDocument);
}

void DocumentManager::fileChanged(const QString &fileName)
{
    const int index = findDocument(fileName);

    // Most likely the file was removed
    if (index == -1)
        return;

    const auto &document = mDocuments.at(index);

    // Ignore change event when it seems to be our own save
    if (QFileInfo(fileName).lastModified() == document->lastSaved())
        return;

    // Automatically reload when there are no unsaved changes
    if (!isDocumentModified(document.data())) {
        reloadDocumentAt(index);
        return;
    }

    document->setChangedOnDisk(true);

    if (isDocumentChangedOnDisk(currentDocument()))
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
    return TilesetDocument::findDocumentForTileset(tileset);
}

TilesetDocument* DocumentManager::findTilesetDocument(const QString &fileName) const
{
    const QString canonicalFilePath = QFileInfo(fileName).canonicalFilePath();
    if (canonicalFilePath.isEmpty()) // file doesn't exist
        return nullptr;

    for (const auto &tilesetDocument : mTilesetDocumentsModel->tilesetDocuments()) {
        QString name = tilesetDocument->fileName();
        if (!name.isEmpty() && QFileInfo(name).canonicalFilePath() == canonicalFilePath)
            return tilesetDocument.data();
    }

    return nullptr;
}

/**
 * Opens the document for the given \a tileset.
 */
void DocumentManager::openTileset(const SharedTileset &tileset)
{
    TilesetDocumentPtr tilesetDocument;
    if (auto existingTilesetDocument = findTilesetDocument(tileset))
        tilesetDocument = existingTilesetDocument->sharedFromThis();
    else
        tilesetDocument = TilesetDocumentPtr::create(tileset);

    if (!switchToDocument(tilesetDocument.data()))
        addDocument(tilesetDocument);
}

void DocumentManager::addToTilesetDocument(const SharedTileset &tileset, MapDocument *mapDocument)
{
    if (auto existingTilesetDocument = findTilesetDocument(tileset)) {
        existingTilesetDocument->addMapDocument(mapDocument);
    } else {
        // Create TilesetDocument instance when it doesn't exist yet
        auto tilesetDocument = TilesetDocumentPtr::create(tileset);
        tilesetDocument->addMapDocument(mapDocument);

        mTilesetDocumentsModel->append(tilesetDocument.data());
        emit tilesetDocumentAdded(tilesetDocument.data());
    }
}

void DocumentManager::removeFromTilesetDocument(const SharedTileset &tileset, MapDocument *mapDocument)
{
    auto tilesetDocument = findTilesetDocument(tileset);
    auto tilesetDocumentPtr = tilesetDocument->sharedFromThis();    // keeps alive and may delete

    tilesetDocument->removeMapDocument(mapDocument);

    bool unused = tilesetDocument->mapDocuments().isEmpty();
    bool external = tilesetDocument->tileset()->isExternal();
    int index = findDocument(tilesetDocument);

    // Remove the TilesetDocument when its tileset is no longer reachable
    if (unused && !(index >= 0 && external)) {
        if (index != -1) {
            closeDocumentAt(index);
        } else {
            mTilesetDocumentsModel->remove(tilesetDocument);
            emit tilesetDocumentRemoved(tilesetDocument);
        }
    }
}

MapDocument *DocumentManager::openMapFile(const QString &path)
{
    openFile(path);
    const int i = findDocument(path);
    return i == -1 ? nullptr : qobject_cast<MapDocument*>(mDocuments.at(i).data());
}

TilesetDocument *DocumentManager::openTilesetFile(const QString &path)
{
    openFile(path);
    const int i = findDocument(path);
    return i == -1 ? nullptr : qobject_cast<TilesetDocument*>(mDocuments.at(i).data());
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

    SharedTileset sharedTileset = tileset->sharedPointer();
    QList<Document*> affectedDocuments;

    for (const auto &document : qAsConst(mDocuments)) {
        if (auto mapDocument = qobject_cast<MapDocument*>(document.data())) {
            if (mapDocument->map()->tilesets().contains(sharedTileset))
                affectedDocuments.append(document.data());
        }
    }

    if (TilesetDocument *tilesetDocument = findTilesetDocument(sharedTileset))
        affectedDocuments.append(tilesetDocument);

    if (!affectedDocuments.isEmpty() && askForAdjustment(*tileset)) {
        for (Document *document : qAsConst(affectedDocuments)) {
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

bool DocumentManager::eventFilter(QObject *object, QEvent *event)
{
    if (object == mTabBar && event->type() == QEvent::MouseButtonRelease) {
        // middle-click tab closing
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);

        if (mouseEvent->button() == Qt::MidButton) {
            int index = mTabBar->tabAt(mouseEvent->pos());

            if (index != -1) {
                documentCloseRequested(index);
                return true;
            }
        }
    }

    return false;
}

/**
 * Unsets a flag to stop closeOtherDocuments() and closeDocumentsToRight()
 * when Cancel is pressed
 */
void DocumentManager::abortMultiDocumentClose()
{
    mMultiDocumentClose = false;
}
