/*
 * tileseteditor.cpp
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

#include "tileseteditor.h"

#include "tileanimationeditor.h"
#include "tilecollisioneditor.h"
#include "propertiesdock.h"
#include "tilesetdocument.h"
#include "tilesetmodel.h"
#include "tilesetview.h"

#include <QMainWindow>
#include <QStackedWidget>

namespace Tiled {
namespace Internal {

TilesetEditor::TilesetEditor(QObject *parent)
    : Editor(parent)
    , mMainWindow(new QMainWindow)
    , mWidgetStack(new QStackedWidget(mMainWindow))
    , mPropertiesDock(new PropertiesDock(mMainWindow))
    , mTileAnimationEditor(new TileAnimationEditor(mMainWindow))
    , mTileCollisionEditor(new TileCollisionEditor(mMainWindow))
    , mCurrentTilesetDocument(nullptr)
{
    mMainWindow->setCentralWidget(mWidgetStack);
    mMainWindow->addDockWidget(Qt::LeftDockWidgetArea, mPropertiesDock);
}

void TilesetEditor::addDocument(Document *document)
{
    TilesetDocument *tilesetDocument = qobject_cast<TilesetDocument*>(document);
    Q_ASSERT(tilesetDocument);

    TilesetView *view = new TilesetView(mWidgetStack);

    Tileset *tileset = tilesetDocument->tileset().data();
    view->setModel(new TilesetModel(tileset, view));

//    QItemSelectionModel *s = view->selectionModel();
//    connect(s, &QItemSelectionModel::selectionChanged,
//            this, &TilesetDock::selectionChanged);
//    connect(s, &QItemSelectionModel::currentChanged,
//            this, &TilesetDock::currentChanged);
//    connect(view, &TilesetView::pressed,
//            this, &TilesetDock::indexPressed);

    // todo: Connect the TilesetView tile selection to these editors
    // maybe only connect to the currently visible view
    //    connect(mTilesetDock, &TilesetDock::currentTileChanged,
    //            mTileAnimationEditor, &TileAnimationEditor::setTile);
    //    connect(mTilesetDock, &TilesetDock::currentTileChanged,
    //            mTileCollisionEditor, &TileCollisionEditor::setTile);

    mWidgetStack->addWidget(view);
    mViewForTileset.insert(tilesetDocument, view);
}

void TilesetEditor::removeDocument(Document *document)
{
    TilesetDocument *tilesetDocument = qobject_cast<TilesetDocument*>(document);
    Q_ASSERT(tilesetDocument);
    Q_ASSERT(mViewForTileset.contains(tilesetDocument));

    TilesetView *view = mViewForTileset.take(tilesetDocument);
    // remove first, to keep it valid while the current widget changes
    mWidgetStack->removeWidget(view);
    delete view;
}

void TilesetEditor::setCurrentDocument(Document *document)
{
    TilesetDocument *tilesetDocument = qobject_cast<TilesetDocument*>(document);
    Q_ASSERT(tilesetDocument || !document);

    if (document) {
        Q_ASSERT(mViewForTileset.contains(tilesetDocument));
        mWidgetStack->setCurrentWidget(mViewForTileset.value(tilesetDocument));
    }

    mPropertiesDock->setDocument(document);
    mTileAnimationEditor->setTilesetDocument(tilesetDocument);
    mTileCollisionEditor->setTilesetDocument(tilesetDocument);

    mCurrentTilesetDocument = tilesetDocument;
}

Document *TilesetEditor::currentDocument() const
{
    return mCurrentTilesetDocument;
}

QWidget *TilesetEditor::editorWidget() const
{
    return mMainWindow;
}

} // namespace Internal
} // namespace Tiled
