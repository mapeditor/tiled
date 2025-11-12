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
#include "mapobject.h"
#include "propertiesview.h"
#include "propertieswidget.h"
#include "tilesetdocument.h"

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
    const bool isMap = mDocument && mDocument->type() == Document::MapDocumentType;
    const bool isTileset = mDocument && mDocument->type() == Document::TilesetDocumentType;
    const auto currentObject = mDocument ? mDocument->currentObject() : nullptr;

    bool layersSelected = false;
    bool objectsSelected = false;
    bool tilesSelected = false;
    bool wangSetSelected = false;
    bool wangColorSelected = false;

    if (isMap) {
        const auto mapDocument = static_cast<MapDocument *>(mDocument);
        layersSelected = !mapDocument->selectedLayers().isEmpty();
        objectsSelected = !mapDocument->selectedObjects().isEmpty();
    }

    if (isTileset) {
        const auto tilesetDocument = static_cast<TilesetDocument *>(mDocument);
        tilesSelected = !tilesetDocument->selectedTiles().isEmpty();

        // todo: keep track of selected wang set and color in TilesetDocument
        wangSetSelected = currentObject && currentObject->typeId() == Object::WangSetType;
        wangColorSelected = currentObject && currentObject->typeId() == Object::WangColorType;
    }

    mTabBar->setTabVisible(MapTab, isMap);
    mTabBar->setTabVisible(LayerTab, layersSelected);
    mTabBar->setTabVisible(MapObjectTab, objectsSelected);
    mTabBar->setTabVisible(TilesetTab, isTileset);
    mTabBar->setTabVisible(TileTab, tilesSelected);
    mTabBar->setTabVisible(WangSetTab, wangSetSelected);
    mTabBar->setTabVisible(WangColorTab, wangColorSelected || wangSetSelected);
    mTabBar->setTabButton(0, QTabBar::LeftSide, nullptr);   // Force recalculation of tab sizes

    int currentIndex = -1;
    if (currentObject) {
        switch (currentObject->typeId()) {
        case Object::LayerType:     currentIndex = LayerTab; break;
        case Object::MapObjectType: currentIndex = MapObjectTab; break;
        case Object::MapType:       currentIndex = MapTab; break;
        case Object::TilesetType:   currentIndex = TilesetTab; break;
        case Object::TileType:      currentIndex = TileTab; break;
        case Object::WangSetType:   currentIndex = WangSetTab; break;
        case Object::WangColorType: currentIndex = WangColorTab; break;

        case Object::ProjectType:
        case Object::WorldType:
            currentIndex = -1; break;
            break;
        }
    }
    if (currentIndex == -1) {
        if (isMap)
            currentIndex = MapTab;
        else if (isTileset)
            currentIndex = TilesetTab;
    }
    if (currentIndex != -1 && !mTabBar->isTabVisible(currentIndex))
        currentIndex = -1;
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
        // todo: keep track of last focused object
        if (auto mapDocument = qobject_cast<MapDocument *>(mDocument)) {
            auto selectedObjects = mapDocument->selectedObjects();
            if (!selectedObjects.isEmpty())
                mapDocument->setCurrentObject(selectedObjects.first());
        }
        break;
    case TilesetTab: {
        if (auto tilesetDocument = qobject_cast<TilesetDocument *>(mDocument))
            tilesetDocument->setCurrentObject(tilesetDocument->tileset().data());
        break;
    }
    case TileTab:
        // todo: keep track of last focused tile
        if (auto tilesetDocument = qobject_cast<TilesetDocument *>(mDocument)) {
            auto selectedTiles = tilesetDocument->selectedTiles();
            if (!selectedTiles.isEmpty())
                tilesetDocument->setCurrentObject(selectedTiles.first());
        }
    case WangSetTab:
    case WangColorTab:
        // todo: keep track of selected wang set and color
        break;
    }
}

} // namespace Tiled

#include "moc_propertiesdock.cpp"
