/*
 * propertiesdock.cpp
 * Copyright 2013, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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

#include "propertiesdock.h"

#include "mapdocument.h"
#include "mapobject.h"  // necessary to know MapObject* is an Object*
#include "propertiesview.h"
#include "propertieswidget.h"
#include "tilesetdocument.h"
#include "wangset.h"

#include <QKeyEvent>
#include <QTabBar>
#include <QVBoxLayout>

namespace Tiled {

PropertiesDock::PropertiesDock(QWidget *parent)
    : QDockWidget(parent)
    , mTabBar(new QTabBar(this))
    , mPropertiesWidget(new PropertiesWidget(this))
{
    setObjectName(QLatin1String("propertiesDock"));

    mTabBar->setDocumentMode(true);
    mTabBar->setUsesScrollButtons(true);    // defaults to false on macOS

    mTabBar->insertTab(MapTab, tr("Map"));
    mTabBar->insertTab(LayerTab, tr("Layer"));
    mTabBar->insertTab(MapObjectTab, tr("Object"));
    mTabBar->insertTab(TilesetTab, tr("Tileset"));
    mTabBar->insertTab(TileTab, tr("Tile"));
    mTabBar->insertTab(WangSetTab, tr("Terrain Set"));
    mTabBar->insertTab(WangColorTab, tr("Terrain"));

    auto widget = new QWidget(this);
    auto layout = new QVBoxLayout(widget);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addWidget(mTabBar);
    layout->addWidget(mPropertiesWidget);

    setWidget(widget);

    connect(mTabBar, &QTabBar::currentChanged,
            this, &PropertiesDock::tabChanged);

    connect(mPropertiesWidget, &PropertiesWidget::bringToFront,
            this, &PropertiesDock::bringToFront);

    updateTabs();
    retranslateUi();
}

void PropertiesDock::setDocument(Document *document)
{
    if (mDocument == document)
        return;

    if (mDocument)
        mDocument->disconnect(this);

    if (document) {
        connect(document, &Document::currentObjectChanged,
                this, &PropertiesDock::updateTabs);

        switch (document->type()) {
        case Document::MapDocumentType: {
            auto mapDocument = static_cast<MapDocument *>(document);
            connect(mapDocument, &MapDocument::selectedLayersChanged,
                    this, &PropertiesDock::updateTabs);
            connect(mapDocument, &MapDocument::selectedObjectsChanged,
                    this, &PropertiesDock::updateTabs);
            break;
        }
        case Document::TilesetDocumentType: {
            auto tilesetDocument = static_cast<TilesetDocument *>(document);
            connect(tilesetDocument, &TilesetDocument::selectedTilesChanged,
                    this, &PropertiesDock::updateTabs);
            break;
        }
        case Document::WorldDocumentType:
        case Document::ProjectDocumentType:
            break;
        }
    }

    mDocument = document;
    mPropertiesWidget->setDocument(document);
    updateTabs();
}

void PropertiesDock::selectCustomProperty(const QString &name)
{
    bringToFront();
    mPropertiesWidget->selectCustomProperty(name);
}

bool PropertiesDock::event(QEvent *event)
{
    switch (event->type()) {
    case QEvent::LanguageChange:
        retranslateUi();
        break;
    case QEvent::ShortcutOverride:
        if (static_cast<QKeyEvent *>(event)->key() == Qt::Key_Tab) {
            if (!mPropertiesWidget->propertiesView()->hasFocus()) {
                event->accept();
                return true;
            }
        }
        break;
    default:
        break;
    }

    return QDockWidget::event(event);
}

void PropertiesDock::bringToFront()
{
    show();
    raise();
    mPropertiesWidget->setFocus();
}

void PropertiesDock::retranslateUi()
{
    setWindowTitle(tr("Properties"));

    mTabBar->setTabText(MapTab, tr("Map"));
    mTabBar->setTabText(LayerTab, tr("Layer"));
    mTabBar->setTabText(MapObjectTab, tr("Object"));
    mTabBar->setTabText(TilesetTab, tr("Tileset"));
    mTabBar->setTabText(TileTab, tr("Tile"));
    mTabBar->setTabText(WangSetTab, tr("Terrain Set"));
    mTabBar->setTabText(WangColorTab, tr("Terrain"));
}

void PropertiesDock::updateTabs()
{
    int visibleTabs = 0;
    int currentIndex = -1;

    if (mDocument) {
        // Determine the visible tabs based on selections
        switch (mDocument->type()) {
        case Document::MapDocumentType: {
            const auto mapDocument = static_cast<MapDocument *>(mDocument);
            visibleTabs |= Object::MapType;

            if (!mapDocument->selectedLayers().isEmpty())
                visibleTabs |= Object::LayerType;
            if (!mapDocument->selectedObjects().isEmpty())
                visibleTabs |= Object::MapObjectType;
            break;
        }
        case Document::TilesetDocumentType: {
            const auto tilesetDocument = static_cast<TilesetDocument *>(mDocument);
            visibleTabs |= Object::TilesetType;

            if (!tilesetDocument->selectedTiles().isEmpty())
                visibleTabs |= Object::TileType;
            if (tilesetDocument->selectedWangSet())
                visibleTabs |= Object::WangSetType;
            if (tilesetDocument->selectedWangColor() != 0)
                visibleTabs |= Object::WangColorType;
            break;
        }

        case Document::WorldDocumentType:
        case Document::ProjectDocumentType:
            break;
        }

        // Determine the current tab
        if (const auto currentObject = mDocument->currentObject()) {
            // Current tab might need to be made temporarily visible
            visibleTabs |= currentObject->typeId();

            switch (currentObject->typeId()) {
            case Object::LayerType:     currentIndex = LayerTab;        break;
            case Object::MapObjectType: currentIndex = MapObjectTab;    break;
            case Object::MapType:       currentIndex = MapTab;          break;
            case Object::TilesetType:   currentIndex = TilesetTab;      break;
            case Object::TileType:      currentIndex = TileTab;         break;
            case Object::WangSetType:   currentIndex = WangSetTab;      break;
            case Object::WangColorType: currentIndex = WangColorTab;    break;

            // These types use their own property dialog
            case Object::ProjectType:
            case Object::WorldType:
                currentIndex = -1; break;
                break;
            }
        }

        // If no object is selected, try to select the Map or Tileset tab.
        if (currentIndex == -1) {
            if (visibleTabs & Object::MapType)
                currentIndex = MapTab;
            else if (visibleTabs & Object::TilesetType)
                currentIndex = TilesetTab;
        }
    }

    mTabBar->setTabVisible(MapTab, visibleTabs & Object::MapType);
    mTabBar->setTabVisible(LayerTab, visibleTabs & Object::LayerType);
    mTabBar->setTabVisible(MapObjectTab, visibleTabs & Object::MapObjectType);
    mTabBar->setTabVisible(TilesetTab, visibleTabs & Object::TilesetType);
    mTabBar->setTabVisible(TileTab, visibleTabs & Object::TileType);
    mTabBar->setTabVisible(WangSetTab, visibleTabs & Object::WangSetType);
    mTabBar->setTabVisible(WangColorTab, visibleTabs & Object::WangColorType);

    // Until Qt 6.7, the QTabBar wasn't checking the tab visibility when handling the mouse wheel
    // (fixed in qtbase commit 83e92e25573f98e7530a3dfcaf02910f3932107f)
#if QT_VERSION < QT_VERSION_CHECK(6, 7, 0)
    mTabBar->setTabEnabled(MapTab, visibleTabs & Object::MapType);
    mTabBar->setTabEnabled(LayerTab, visibleTabs & Object::LayerType);
    mTabBar->setTabEnabled(MapObjectTab, visibleTabs & Object::MapObjectType);
    mTabBar->setTabEnabled(TilesetTab, visibleTabs & Object::TilesetType);
    mTabBar->setTabEnabled(TileTab, visibleTabs & Object::TileType);
    mTabBar->setTabEnabled(WangSetTab, visibleTabs & Object::WangSetType);
    mTabBar->setTabEnabled(WangColorTab, visibleTabs & Object::WangColorType);
#endif

    // Force relayouting of tabs, because Qt fails to do so (tested on 5.15 and 6.10)
    mTabBar->setTabButton(0, QTabBar::LeftSide, nullptr);

    mTabBar->setCurrentIndex(currentIndex);
}

void PropertiesDock::tabChanged(int index)
{
    switch (static_cast<TabType>(index)) {
    case MapTab: {
        if (auto mapDocument = qobject_cast<MapDocument *>(mDocument))
            mapDocument->setCurrentObject(mapDocument->map());
        break;
    }
    case LayerTab:
        if (auto mapDocument = qobject_cast<MapDocument *>(mDocument)) {
            if (auto layer = mapDocument->currentLayer())
                mapDocument->setCurrentObject(layer);
        }
        break;
    case MapObjectTab:
        if (auto mapDocument = qobject_cast<MapDocument *>(mDocument)) {
            if (auto last = mapDocument->lastFocusedMapObject())
                mapDocument->setCurrentObject(last);
            else {
                auto selectedObjects = mapDocument->selectedObjects();
                if (!selectedObjects.isEmpty())
                    mapDocument->setCurrentObject(selectedObjects.first());
            }
        }
        break;
    case TilesetTab: {
        if (auto tilesetDocument = qobject_cast<TilesetDocument *>(mDocument))
            tilesetDocument->setCurrentObject(tilesetDocument->tileset().data());
        break;
    }
    case TileTab:
        if (auto tilesetDocument = qobject_cast<TilesetDocument *>(mDocument)) {
            if (auto last = tilesetDocument->lastFocusedTile())
                tilesetDocument->setCurrentObject(last);
            else {
                auto selectedTiles = tilesetDocument->selectedTiles();
                if (!selectedTiles.isEmpty())
                    tilesetDocument->setCurrentObject(selectedTiles.first());
            }
        }
        break;
    case WangSetTab:
        if (auto tilesetDocument = qobject_cast<TilesetDocument *>(mDocument)) {
            if (auto wangSet = tilesetDocument->selectedWangSet())
                tilesetDocument->setCurrentObject(wangSet);
        }
        break;
    case WangColorTab:
        if (auto tilesetDocument = qobject_cast<TilesetDocument *>(mDocument)) {
            qDebug() << "Switching to WangColor tab" << tilesetDocument;
            if (auto wangSet = tilesetDocument->selectedWangSet()) {
                const int color = tilesetDocument->selectedWangColor();
                qDebug() << "  with WangSet" << wangSet->name() << "and color index" << color;
                if (color > 0)
                    tilesetDocument->setCurrentObject(wangSet->colorAt(color).data());
            }
        }
        break;
    }
}

} // namespace Tiled

#include "moc_propertiesdock.cpp"
