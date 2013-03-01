/*
 * tilesetdock.cpp
 * Copyright 2008-2010, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
 * Copyright 2009, Edward Hutchins <eah1@yahoo.com>
 * Copyright 2012, Stefan Beller <stefanbeller@googlemail.com>
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

#include "tilesetdock.h"

#include "addremovemapobject.h"
#include "addremovetileset.h"
#include "documentmanager.h"
#include "editterraindialog.h"
#include "erasetiles.h"
#include "map.h"
#include "mapdocument.h"
#include "mapobject.h"
#include "movetileset.h"
#include "objectgroup.h"
#include "propertiesdialog.h"
#include "terrain.h"
#include "tile.h"
#include "tilelayer.h"
#include "tileset.h"
#include "tilesetmodel.h"
#include "tilesetview.h"
#include "tilesetmanager.h"
#include "tmxmapwriter.h"
#include "utils.h"
#include "zoomable.h"

#include <QMimeData>
#include <QAction>
#include <QDropEvent>
#include <QComboBox>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QMenu>
#include <QMessageBox>
#include <QSignalMapper>
#include <QStackedWidget>
#include <QStylePainter>
#include <QToolBar>
#include <QToolButton>
#include <QUrl>
#include <QVBoxLayout>

using namespace Tiled;
using namespace Tiled::Internal;

namespace {

/**
 * Used for exporting/importing tilesets.
 *
 * @warning Does not work for tilesets that are shared by multiple maps!
 */
class SetTilesetFileName : public QUndoCommand
{
public:
    SetTilesetFileName(MapDocument *mapDocument,
                       Tileset *tileset,
                       const QString &fileName)
        : mMapDocument(mapDocument)
        , mTileset(tileset)
        , mFileName(fileName)
    {
        if (fileName.isEmpty())
            setText(QCoreApplication::translate("Undo Commands",
                                                "Import Tileset"));
        else
            setText(QCoreApplication::translate("Undo Commands",
                                                "Export Tileset"));
    }

    void undo() { swap(); }
    void redo() { swap(); }

private:
    void swap()
    {
        QString previousFileName = mTileset->fileName();
        mMapDocument->setTilesetFileName(mTileset, mFileName);
        mFileName = previousFileName;
    }

    MapDocument *mMapDocument;
    Tileset *mTileset;
    QString mFileName;
};

class RenameTileset : public QUndoCommand
{
public:
    RenameTileset(MapDocument *mapDocument,
                  Tileset *tileset,
                  const QString &newName)
        : QUndoCommand(QCoreApplication::translate("Undo Commands",
                                                   "Change Tileset Name"))
        , mMapDocument(mapDocument)
        , mTileset(tileset)
        , mOldName(tileset->name())
        , mNewName(newName)
    {}

    void undo() { mMapDocument->setTilesetName(mTileset, mOldName); }
    void redo() { mMapDocument->setTilesetName(mTileset, mNewName); }

private:
    MapDocument *mMapDocument;
    Tileset *mTileset;
    QString mOldName;
    QString mNewName;
};


class TilesetMenuButton : public QToolButton
{
public:
    TilesetMenuButton(QWidget *parent = 0)
        : QToolButton(parent)
    {
        setArrowType(Qt::DownArrow);
        setIconSize(QSize(16, 16));
        setPopupMode(QToolButton::InstantPopup);
        setAutoRaise(true);

        setSizePolicy(sizePolicy().horizontalPolicy(),
                      QSizePolicy::Ignored);
    }

protected:
    void paintEvent(QPaintEvent *)
    {
        QStylePainter p(this);
        QStyleOptionToolButton opt;
        initStyleOption(&opt);

        // Disable the menu arrow, since we already got a down arrow icon
        opt.features &= ~QStyleOptionToolButton::HasMenu;

        p.drawComplexControl(QStyle::CC_ToolButton, opt);
    }
};

} // anonymous namespace

TilesetDock::TilesetDock(QWidget *parent):
    QDockWidget(parent),
    mMapDocument(0),
    mTabBar(new QTabBar),
    mViewStack(new QStackedWidget),
    mToolBar(new QToolBar),
    mCurrentTile(0),
    mCurrentTiles(0),
    mImportTileset(new QAction(this)),
    mExportTileset(new QAction(this)),
    mPropertiesTileset(new QAction(this)),
    mDeleteTileset(new QAction(this)),
    mRenameTileset(new QAction(this)),
    mEditTerrain(new QAction(this)),
    mTilesetMenuButton(new TilesetMenuButton(this)),
    mTilesetMenu(new QMenu(this)),
    mTilesetActionGroup(new QActionGroup(this)),
    mTilesetMenuMapper(0)
{
    setObjectName(QLatin1String("TilesetDock"));

    mTabBar->setMovable(true);
    mTabBar->setUsesScrollButtons(true);

    connect(mTabBar, SIGNAL(currentChanged(int)),
            SLOT(updateActions()));
    connect(mTabBar, SIGNAL(tabMoved(int,int)),
            this, SLOT(moveTileset(int,int)));

    QWidget *w = new QWidget(this);

    QHBoxLayout *horizontal = new QHBoxLayout;
    horizontal->setSpacing(0);
    horizontal->addWidget(mTabBar);
    horizontal->addWidget(mTilesetMenuButton);

    QVBoxLayout *vertical = new QVBoxLayout(w);
    vertical->setSpacing(0);
    vertical->setMargin(5);
    vertical->addLayout(horizontal);
    vertical->addWidget(mViewStack);

    horizontal = new QHBoxLayout;
    horizontal->setSpacing(0);
    horizontal->addWidget(mToolBar, 1);
    vertical->addLayout(horizontal);

    mImportTileset->setIcon(QIcon(QLatin1String(":images/16x16/document-import.png")));
    mExportTileset->setIcon(QIcon(QLatin1String(":images/16x16/document-export.png")));
    mPropertiesTileset->setIcon(QIcon(QLatin1String(":images/16x16/document-properties.png")));
    mDeleteTileset->setIcon(QIcon(QLatin1String(":images/16x16/edit-delete.png")));
    mRenameTileset->setIcon(QIcon(QLatin1String(":images/16x16/edit-rename.png")));
    mEditTerrain->setIcon(QIcon(QLatin1String(":images/16x16/terrain.png")));

    Utils::setThemeIcon(mImportTileset, "document-import");
    Utils::setThemeIcon(mExportTileset, "document-export");
    Utils::setThemeIcon(mPropertiesTileset, "document-properties");
    Utils::setThemeIcon(mDeleteTileset, "edit-delete");
    Utils::setThemeIcon(mRenameTileset, "edit-rename");

    connect(mImportTileset, SIGNAL(triggered()),
            SLOT(importTileset()));
    connect(mExportTileset, SIGNAL(triggered()),
            SLOT(exportTileset()));
    connect(mPropertiesTileset, SIGNAL(triggered()),
            SLOT(editTilesetProperties()));
    connect(mDeleteTileset, SIGNAL(triggered()),
            SLOT(removeTileset()));
    connect(mRenameTileset, SIGNAL(triggered()),
            SLOT(renameTileset()));
    connect(mEditTerrain, SIGNAL(triggered()),
            SLOT(editTerrain()));

    mToolBar->setIconSize(QSize(16, 16));
    mToolBar->addAction(mImportTileset);
    mToolBar->addAction(mExportTileset);
    mToolBar->addAction(mPropertiesTileset);
    mToolBar->addAction(mDeleteTileset);
    mToolBar->addAction(mRenameTileset);
    mToolBar->addAction(mEditTerrain);

    mZoomable = new Zoomable(this);
    mZoomable->setZoomFactors(QVector<qreal>() << 0.25 << 0.5 << 0.75 << 1.0 << 1.25 << 1.5 << 1.75 << 2.0 << 4.0);
    mZoomComboBox = new QComboBox;
    mZoomable->connectToComboBox(mZoomComboBox);
    horizontal->addWidget(mZoomComboBox);

    connect(mViewStack, SIGNAL(currentChanged(int)),
            this, SLOT(updateCurrentTiles()));

    connect(TilesetManager::instance(), SIGNAL(tilesetChanged(Tileset*)),
            this, SLOT(tilesetChanged(Tileset*)));

    connect(DocumentManager::instance(), SIGNAL(documentCloseRequested(int)),
            SLOT(documentCloseRequested(int)));

    mTilesetMenuButton->setMenu(mTilesetMenu);
    connect(mTilesetMenu, SIGNAL(aboutToShow()), SLOT(refreshTilesetMenu()));

    setWidget(w);
    retranslateUi();
    setAcceptDrops(true);
    updateActions();
}

TilesetDock::~TilesetDock()
{
    delete mCurrentTiles;
}

void TilesetDock::setMapDocument(MapDocument *mapDocument)
{
    if (mMapDocument == mapDocument)
        return;

    // Hide while we update the tab bar, to avoid repeated layouting
    widget()->hide();

    setCurrentTiles(0);

    if (mMapDocument) {
        // Remember the last visible tileset for this map
        const QString tilesetName = mTabBar->tabText(mTabBar->currentIndex());
        mCurrentTilesets.insert(mMapDocument, tilesetName);
    }

    // Clear previous content
    while (mTabBar->count())
        mTabBar->removeTab(0);
    while (mViewStack->count())
        delete mViewStack->widget(0);

    mTilesets.clear();

    // Clear all connections to the previous document
    if (mMapDocument)
        mMapDocument->disconnect(this);

    mMapDocument = mapDocument;

    if (mMapDocument) {
        mTilesets = mMapDocument->map()->tilesets();

        foreach (Tileset *tileset, mTilesets) {
            TilesetView *view = new TilesetView;
            view->setMapDocument(mMapDocument);
            view->setZoomable(mZoomable);

            mTabBar->addTab(tileset->name());
            mViewStack->addWidget(view);
        }

        connect(mMapDocument, SIGNAL(tilesetAdded(int,Tileset*)),
                SLOT(tilesetAdded(int,Tileset*)));
        connect(mMapDocument, SIGNAL(tilesetRemoved(Tileset*)),
                SLOT(tilesetRemoved(Tileset*)));
        connect(mMapDocument, SIGNAL(tilesetMoved(int,int)),
                SLOT(tilesetMoved(int,int)));
        connect(mMapDocument, SIGNAL(tilesetNameChanged(Tileset*)),
                SLOT(tilesetNameChanged(Tileset*)));
        connect(mMapDocument, SIGNAL(tilesetFileNameChanged(Tileset*)),
                SLOT(updateActions()));

        QString cacheName = mCurrentTilesets.take(mMapDocument);
        for (int i = 0; i < mTabBar->count(); ++i) {
            if (mTabBar->tabText(i) == cacheName) {
                mTabBar->setCurrentIndex(i);
                break;
            }
        }
    }

    updateActions();

    widget()->show();
}

void TilesetDock::changeEvent(QEvent *e)
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

void TilesetDock::dragEnterEvent(QDragEnterEvent *e)
{
    const QList<QUrl> urls = e->mimeData()->urls();
    if (!urls.isEmpty() && !urls.at(0).toLocalFile().isEmpty())
        e->accept();
}

void TilesetDock::dropEvent(QDropEvent *e)
{
    QStringList paths;
    foreach (const QUrl &url, e->mimeData()->urls()) {
        const QString localFile = url.toLocalFile();
        if (!localFile.isEmpty())
            paths.append(localFile);
    }
    if (!paths.isEmpty()) {
        emit tilesetsDropped(paths);
        e->accept();
    }
}

void TilesetDock::updateActions()
{
    bool external = false;
    TilesetView *view = 0;
    const int index = mTabBar->currentIndex();

    if (index > -1) {
        view = tilesetViewAt(index);
        if (view) {
            Tileset *tileset = mTilesets.at(index);

            if (!view->model()) {
                // Lazily set up the model
                view->setModel(new TilesetModel(tileset, view));
                connect(view->selectionModel(),
                        SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
                        SLOT(updateCurrentTiles()));
            }

            mViewStack->setCurrentIndex(index);
            external = tileset->isExternal();
        }
    }

    mRenameTileset->setEnabled(view && !external);
    mImportTileset->setEnabled(view && external);
    mExportTileset->setEnabled(view && !external);
    mPropertiesTileset->setEnabled(view && !external);
    mDeleteTileset->setEnabled(view);
    mEditTerrain->setEnabled(view && !external);
}

void TilesetDock::updateCurrentTiles()
{
    const int viewIndex = mViewStack->currentIndex();
    if (viewIndex == -1)
        return;

    const QItemSelectionModel *s = tilesetViewAt(viewIndex)->selectionModel();
    if (!s)
        return;

    const QModelIndexList indexes = s->selection().indexes();
    if (indexes.isEmpty())
        return;

    const QModelIndex &first = indexes.first();
    int minX = first.column();
    int maxX = first.column();
    int minY = first.row();
    int maxY = first.row();

    foreach (const QModelIndex &index, indexes) {
        if (minX > index.column()) minX = index.column();
        if (maxX < index.column()) maxX = index.column();
        if (minY > index.row()) minY = index.row();
        if (maxY < index.row()) maxY = index.row();
    }

    // Create a tile layer from the current selection
    TileLayer *tileLayer = new TileLayer(QString(), 0, 0,
                                         maxX - minX + 1,
                                         maxY - minY + 1);

    const TilesetModel *model = static_cast<const TilesetModel*>(s->model());
    foreach (const QModelIndex &index, indexes) {
        tileLayer->setCell(index.column() - minX,
                           index.row() - minY,
                           Cell(model->tileAt(index)));
    }

    setCurrentTiles(tileLayer);
    setCurrentTile(model->tileAt(s->currentIndex()));
}

void TilesetDock::tilesetAdded(int index, Tileset *tileset)
{
    TilesetView *view = new TilesetView;
    view->setMapDocument(mMapDocument);
    view->setZoomable(mZoomable);

    mTilesets.insert(index, tileset);
    mTabBar->insertTab(index, tileset->name());
    mViewStack->insertWidget(index, view);

    updateActions();
}

void TilesetDock::tilesetChanged(Tileset *tileset)
{
    // Update the affected tileset model, if it exists
    const int index = mTilesets.indexOf(tileset);
    if (index < 0)
        return;

    if (TilesetModel *model = tilesetViewAt(index)->tilesetModel())
        model->tilesetChanged();
}

void TilesetDock::tilesetRemoved(Tileset *tileset)
{
    // Delete the related tileset view
    const int index = mTilesets.indexOf(tileset);
    Q_ASSERT(index != -1);

    mTilesets.removeAt(index);
    mTabBar->removeTab(index);
    delete tilesetViewAt(index);

    // Make sure we don't reference this tileset anymore
    if (mCurrentTiles) {
        // TODO: Don't clean unnecessarily (but first the concept of
        //       "current brush" would need to be introduced)
        TileLayer *cleaned = static_cast<TileLayer *>(mCurrentTiles->clone());
        cleaned->removeReferencesToTileset(tileset);
        setCurrentTiles(cleaned);
    }
    if (mCurrentTile && mCurrentTile->tileset() == tileset)
        setCurrentTile(0);

    updateActions();
}

void TilesetDock::tilesetMoved(int from, int to)
{
    mTilesets.insert(to, mTilesets.takeAt(from));

    // Move the related tileset views
    QWidget *widget = mViewStack->widget(from);
    mViewStack->removeWidget(widget);
    mViewStack->insertWidget(to, widget);
    mViewStack->setCurrentIndex(mTabBar->currentIndex());

    // Update the titles of the affected tabs
    const int start = qMin(from, to);
    const int end = qMax(from, to);
    for (int i = start; i <= end; ++i) {
        const Tileset *tileset = mTilesets.at(i);
        if (mTabBar->tabText(i) != tileset->name())
            mTabBar->setTabText(i, tileset->name());
    }
}

/**
 * Removes the currently selected tileset.
 */
void TilesetDock::removeTileset()
{
    const int currentIndex = mViewStack->currentIndex();
    if (currentIndex != -1)
        removeTileset(mViewStack->currentIndex());
}

/**
 * Removes the tileset at the given index. Prompting the user when the tileset
 * is in use by the map.
 */
void TilesetDock::removeTileset(int index)
{
    Tileset *tileset = mTilesets.at(index);
    const bool inUse = mMapDocument->map()->isTilesetUsed(tileset);

    // If the tileset is in use, warn the user and confirm removal
    if (inUse) {
        QMessageBox warning(QMessageBox::Warning,
                            tr("Remove Tileset"),
                            tr("The tileset \"%1\" is still in use by the "
                               "map!").arg(tileset->name()),
                            QMessageBox::Yes | QMessageBox::No,
                            this);
        warning.setDefaultButton(QMessageBox::Yes);
        warning.setInformativeText(tr("Remove this tileset and all references "
                                      "to the tiles in this tileset?"));

        if (warning.exec() != QMessageBox::Yes)
            return;
    }

    QUndoCommand *remove = new RemoveTileset(mMapDocument, index, tileset);
    QUndoStack *undoStack = mMapDocument->undoStack();

    if (inUse) {
        // Remove references to tiles in this tileset from the current map
        undoStack->beginMacro(remove->text());
        foreach (Layer *layer, mMapDocument->map()->layers()) {
            if (TileLayer *tileLayer = layer->asTileLayer()) {
                const QRegion refs = tileLayer->tilesetReferences(tileset);
                if (!refs.isEmpty()) {
                    undoStack->push(new EraseTiles(mMapDocument,
                                                   tileLayer, refs));
                }
            } else if (ObjectGroup *objectGroup = layer->asObjectGroup()) {
                foreach (MapObject *object, objectGroup->objects()) {
                    const Tile *tile = object->cell().tile;
                    if (tile && tile->tileset() == tileset) {
                        undoStack->push(new RemoveMapObject(mMapDocument,
                                                            object));
                    }
                }
            }
        }
    }
    undoStack->push(remove);
    if (inUse)
        undoStack->endMacro();
}

void TilesetDock::moveTileset(int from, int to)
{
    QUndoCommand *command = new MoveTileset(mMapDocument, from, to);
    mMapDocument->undoStack()->push(command);
}

void TilesetDock::setCurrentTiles(TileLayer *tiles)
{
    if (mCurrentTiles == tiles)
        return;

    delete mCurrentTiles;
    mCurrentTiles = tiles;

    emit currentTilesChanged(mCurrentTiles);
}

void TilesetDock::setCurrentTile(Tile *tile)
{
    if (mCurrentTile == tile)
        return;

    mCurrentTile = tile;
    emit currentTileChanged(mCurrentTile);
}

void TilesetDock::retranslateUi()
{
    setWindowTitle(tr("Tilesets"));
    mImportTileset->setText(tr("&Import Tileset"));
    mExportTileset->setText(tr("&Export Tileset As..."));
    mPropertiesTileset->setText(tr("Tile&set Properties"));
    mDeleteTileset->setText(tr("&Remove Tileset"));
    mRenameTileset->setText(tr("Rena&me Tileset"));
    mEditTerrain->setText(tr("Edit &Terrain Information"));
}

Tileset *TilesetDock::currentTileset() const
{
    const int index = mTabBar->currentIndex();
    if (index == -1)
        return 0;

    return mTilesets.at(index);
}

TilesetView *TilesetDock::currentTilesetView() const
{
    return static_cast<TilesetView *>(mViewStack->currentWidget());
}

TilesetView *TilesetDock::tilesetViewAt(int index) const
{
    return static_cast<TilesetView *>(mViewStack->widget(index));
}

void TilesetDock::editTilesetProperties()
{
    Tileset *tileset = currentTileset();
    if (!tileset)
        return;

    PropertiesDialog propertiesDialog(tr("Tileset"),
                                      tileset,
                                      mMapDocument->undoStack(),
                                      this);
    propertiesDialog.exec();
}

void TilesetDock::exportTileset()
{
    Tileset *tileset = currentTileset();
    if (!tileset)
        return;

    const QLatin1String extension(".tsx");
    QString suggestedFileName = QFileInfo(mMapDocument->fileName()).path();
    suggestedFileName += QLatin1Char('/');
    suggestedFileName += tileset->name();
    if (!suggestedFileName.endsWith(extension))
        suggestedFileName.append(extension);

    const QString fileName =
            QFileDialog::getSaveFileName(this, tr("Export Tileset"),
                                         suggestedFileName,
                                         tr("Tiled tileset files (*.tsx)"));
    if (fileName.isEmpty())
        return;

    TmxMapWriter writer;

    if (writer.writeTileset(tileset, fileName)) {
        QUndoCommand *command = new SetTilesetFileName(mMapDocument,
                                                       tileset, fileName);
        mMapDocument->undoStack()->push(command);
    }
}

void TilesetDock::importTileset()
{
    Tileset *tileset = currentTileset();
    if (!tileset)
        return;

    QUndoCommand *command = new SetTilesetFileName(mMapDocument,
                                                   tileset, QString());
    mMapDocument->undoStack()->push(command);
}

void TilesetDock::renameTileset()
{
    bool ok;
    const QString oldText = mTabBar->tabText(mTabBar->currentIndex());
    QString newText = QInputDialog::getText(this, tr("Rename Tileset"),
                                         tr("New name:"), QLineEdit::Normal,
                                         oldText, &ok);

    if (!ok || newText == oldText)
        return;

    RenameTileset *name = new RenameTileset(mMapDocument,
                                            currentTileset(),
                                            newText);
    mMapDocument->undoStack()->push(name);
}

void TilesetDock::editTerrain()
{
    Tileset *tileset = currentTileset();
    if (!tileset)
        return;

    EditTerrainDialog editTerrainDialog(mMapDocument, tileset, this);
    editTerrainDialog.exec();
}

void TilesetDock::tilesetNameChanged(Tileset *tileset)
{
    const int index = mTilesets.indexOf(tileset);
    Q_ASSERT(index != -1);

    mTabBar->setTabText(index, tileset->name());
}

void TilesetDock::documentCloseRequested(int index)
{
    DocumentManager *documentManager = DocumentManager::instance();
    mCurrentTilesets.remove(documentManager->documents().at(index));
}

void TilesetDock::refreshTilesetMenu()
{
    mTilesetMenu->clear();

    if (mTilesetMenuMapper) {
        mTabBar->disconnect(mTilesetMenuMapper);
        delete mTilesetMenuMapper;
    }

    mTilesetMenuMapper = new QSignalMapper(this);
    connect(mTilesetMenuMapper, SIGNAL(mapped(int)),
            mTabBar, SLOT(setCurrentIndex(int)));

    const int currentIndex = mTabBar->currentIndex();

    for (int i = 0; i < mTabBar->count(); ++i) {
        QAction *action = new QAction(mTabBar->tabText(i), this);
        action->setCheckable(true);

        mTilesetActionGroup->addAction(action);
        if (i == currentIndex)
            action->setChecked(true);

        mTilesetMenu->addAction(action);
        connect(action, SIGNAL(triggered()), mTilesetMenuMapper, SLOT(map()));
        mTilesetMenuMapper->setMapping(action, i);
    }
}
