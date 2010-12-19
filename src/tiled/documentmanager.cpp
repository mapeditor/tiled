/*
 * documentmanager.cpp
 * Copyright 2010, Stefan Beller <stefanbeller@googlemail.com>
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

#include "toolmanager.h"
#include "abstracttool.h"

#include <QTabWidget>
#include <QFileInfo>

using namespace Tiled;
using namespace Tiled::Internal;

DocumentManager::DocumentManager(QObject *parent)
    : QObject(parent)
    , mTabWidget(new QTabWidget)
    , mActiveTool(0)
    , mIndex(0)
    , mUntitledFileName(tr("untitled.tmx"))
{
    mTabWidget->setDocumentMode(true);

    // since we need at least one view and mapscene, add an empty document
    emptyView = false;
    addDocument(0);
    // set emptyView after adding the empty view!
    emptyView = true;

    connect(mTabWidget, SIGNAL(currentChanged(int)),
            SLOT(currentIndexChanged(int)));
    connect(mTabWidget, SIGNAL(tabCloseRequested(int)),
            SIGNAL(documentCloseRequested(int)));

    ToolManager *toolManager = ToolManager::instance();
    setSelectedTool(toolManager->selectedTool());
    connect(toolManager, SIGNAL(selectedToolChanged(AbstractTool*)),
            SLOT(setSelectedTool(AbstractTool*)));
}

DocumentManager::~DocumentManager()
{
    // assume that closeMapDocuments was called
    // so there is no opened MapDocument
    Q_ASSERT(emptyView);
    Q_ASSERT(mMaps.size() == 1);
    // the MapScene should be deleted
    delete mMaps.at(0).first;
    // TODO: correct this behavior:
    // the map view will get deleted by ui!
    // so do not delete mMaps.at(0).second;

    delete mTabWidget;
}

QWidget *DocumentManager::widget() const
{
    return mTabWidget;
}

MapDocument *DocumentManager::currentDocument() const
{
    return mMaps.at(mIndex).first->mapDocument();
}

MapView *DocumentManager::currentMapView() const
{
    return mMaps.at(mIndex).second;
}

MapScene *DocumentManager::currentMapScene() const
{
    return mMaps.at(mIndex).first;
}

int DocumentManager::documentCount() const
{
    return emptyView ? 0 : mMaps.size();
}

void DocumentManager::switchToDocument(int index)
{
    mTabWidget->setCurrentIndex(index);
}

void DocumentManager::addDocument(MapDocument *mapDocument)
{
    mMaps.append(QPair<MapScene*, MapView*>(new MapScene(this), new MapView()));

    if (mapDocument) {
        if (QFileInfo(mapDocument->fileName()).exists())
            mTabWidget->addTab(mMaps.at(mMaps.size() - 1).second,
                            QFileInfo(mapDocument->fileName()).fileName());
        else
            mTabWidget->addTab(mMaps.at(mMaps.size() - 1).second,
                               mUntitledFileName);
    } else {
        mTabWidget->addTab(mMaps.at(mMaps.size() - 1).second, tr(""));
    }

    // remove the tab with empty view, which is displayed when nothing is loaded
    if (emptyView == true) {
        mTabWidget->removeTab(0);
        delete mMaps.at(0).first;
        delete mMaps.at(0).second;
        mMaps.removeAt(0);
        emptyView = false;
    }
    int index = mMaps.size() - 1;
    mMaps.at(index).first->setMapDocument(mapDocument);

    mMaps.at(index).second->setScene(mMaps.at(index).first);

    QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    sizePolicy.setHorizontalStretch(1);
    sizePolicy.setVerticalStretch(1);
    sizePolicy.setHeightForWidth(
            mMaps.at(index).second->sizePolicy().hasHeightForWidth());
    mMaps.at(index).second->setSizePolicy(sizePolicy);

    mMaps.at(index).second->setAlignment(Qt::AlignCenter);

    if (mMaps.size() > 1)
        mTabWidget->setTabsClosable(true);

    currentIndexChanged(index);
    mTabWidget->setCurrentIndex(index);
}

void DocumentManager::closeCurrentDocument()
{
    // there must be always a view and mapscene, so add an empty,
    // if there was removed the last useful view.
    if (mMaps.size() == 1) {
        addDocument(0);
        // set emptyView *after* adding the empty View!
        emptyView = true;
        // addMapDocument changed mIndex to the just added document
        // correct mIndex to the proper value
        mIndex = 0;
    }
    // make a copy of mIndex, since that is modified in currentIndexChanged()
    int index = mIndex;
    MapScene *mapScene = currentMapScene();
    MapView *mapView = currentMapView();

    // let the QTabWidget find the best next tab
    // so remove the current tab and take currentIndex() as next tab
    mTabWidget->removeTab(index);
    int newindex = mTabWidget->currentIndex();

    if (newindex >= index)
        // +1 needs to be added, because in mMaps still holds one element more
        // the 'index' element was removed from the tabs but not from mMaps
        // this needs to be done in this order, so currentIndexChanged can
        // deactivate the tool at the map to be closed.
        currentIndexChanged(newindex + 1);
    else
        // both indices are equal, since there is no additional element within
        currentIndexChanged(newindex);

    mMaps.removeAt(index);

    // mIndex needs to be corrected, if we have deleted an element before the
    // current element
    if (newindex >= index)
        mIndex--;

    delete mapScene->mapDocument();
    delete mapScene;
    delete mapView;

    if (mMaps.size() < 2)
        mTabWidget->setTabsClosable(false);

    documentsFileNameChanged();
}

void DocumentManager::closeAllDocuments()
{
    while (!emptyView)
        closeCurrentDocument();
}

QList<MapDocument*> DocumentManager::documents() const
{
    QList<MapDocument*> ret;

    if (emptyView)
        return ret;

    for (int i = 0; i < mMaps.size(); i++) {
        // assert it is no null pointer
        Q_ASSERT(mMaps.at(i).first->mapDocument());
        ret.append(mMaps.at(i).first->mapDocument());
    }

    return ret;
}

void DocumentManager::currentIndexChanged(int index)
{
    MapScene *mapScene = currentMapScene();
    mapScene->disableSelectedTool();

    mIndex = index;

    emit currentDocumentChanged();

    mapScene = currentMapScene();
    mapScene->setSelectedTool(mActiveTool);
    mapScene->enableSelectedTool();
}

void DocumentManager::setSelectedTool(AbstractTool *tool)
{
    if (mActiveTool)
        currentMapScene()->disableSelectedTool();

    currentMapScene()->setSelectedTool(tool);

    if (tool) {
        mActiveTool = tool;
        currentMapScene()->enableSelectedTool();
    }
}

void DocumentManager::documentsFileNameChanged()
{
    for (int i = 0; i < mMaps.size(); i++) {
        MapDocument *md = mMaps.at(i).first->mapDocument();
        if (md) {
            if (QFileInfo(md->fileName()).exists()) {
                QString fName = QFileInfo(md->fileName()).fileName();
                if (fName == mUntitledFileName)
                    mTabWidget->setTabText(i, md->fileName());
                else
                    mTabWidget->setTabText(i, QFileInfo(md->fileName()).fileName());
            } else {
                mTabWidget->setTabText(i, mUntitledFileName);
            }
        } else {
            mTabWidget->setTabText(i, tr(""));
        }
    }
}
