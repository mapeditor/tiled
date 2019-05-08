/*
 * tmxviewer.cpp
 * Copyright 2010, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
 *
 * This file is part of the TMX Viewer example.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *    1. Redistributions of source code must retain the above copyright notice,
 *       this list of conditions and the following disclaimer.
 *
 *    2. Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE CONTRIBUTORS ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
 * EVENT SHALL THE CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "tmxviewer.h"

#include "hexagonalrenderer.h"
#include "isometricrenderer.h"
#include "map.h"
#include "mapobject.h"
#include "mapreader.h"
#include "objectgroup.h"
#include "orthogonalrenderer.h"
#include "staggeredrenderer.h"
#include "tilelayer.h"
#include "tileset.h"

#include <QCoreApplication>
#include <QDebug>
#include <QGraphicsItem>
#include <QGraphicsScene>
#include <QStyleOptionGraphicsItem>

using namespace Tiled;

/**
 * Item that represents a map object.
 */
class MapObjectItem : public QGraphicsItem
{
public:
    MapObjectItem(MapObject *mapObject, MapRenderer *renderer,
                  QGraphicsItem *parent = nullptr)
        : QGraphicsItem(parent)
        , mMapObject(mapObject)
        , mRenderer(renderer)
    {
        const QPointF &position = mapObject->position();
        const QPointF pixelPos = renderer->pixelToScreenCoords(position);

        QRectF boundingRect = renderer->boundingRect(mapObject);
        boundingRect.translate(-pixelPos);
        mBoundingRect = boundingRect;

        setPos(pixelPos);
        setRotation(mapObject->rotation());
    }

    QRectF boundingRect() const override
    {
        return mBoundingRect;
    }

    void paint(QPainter *p, const QStyleOptionGraphicsItem *, QWidget *) override
    {
        const QColor &color = mMapObject->objectGroup()->color();
        p->translate(-pos());
        mRenderer->drawMapObject(p, mMapObject,
                                 color.isValid() ? color : Qt::darkGray);
    }

private:
    MapObject *mMapObject;
    MapRenderer *mRenderer;
    QRectF mBoundingRect;
};

/**
 * Item that represents a tile layer.
 */
class TileLayerItem : public QGraphicsItem
{
public:
    TileLayerItem(TileLayer *tileLayer, MapRenderer *renderer,
                  QGraphicsItem *parent = nullptr)
        : QGraphicsItem(parent)
        , mTileLayer(tileLayer)
        , mRenderer(renderer)
    {
        setFlag(QGraphicsItem::ItemUsesExtendedStyleOption);
        setPos(mTileLayer->offset());
    }

    QRectF boundingRect() const override
    {
        return mRenderer->boundingRect(mTileLayer->bounds());
    }

    void paint(QPainter *p, const QStyleOptionGraphicsItem *option, QWidget *) override
    {
        mRenderer->drawTileLayer(p, mTileLayer, option->rect);
    }

private:
    TileLayer *mTileLayer;
    MapRenderer *mRenderer;
};

/**
 * Item that represents an object group.
 */
class ObjectGroupItem : public QGraphicsItem
{
public:
    ObjectGroupItem(ObjectGroup *objectGroup, MapRenderer *renderer,
                    QGraphicsItem *parent = nullptr)
        : QGraphicsItem(parent)
    {
        setFlag(QGraphicsItem::ItemHasNoContents);
        setPos(objectGroup->offset());

        const ObjectGroup::DrawOrder drawOrder = objectGroup->drawOrder();

        // Create a child item for each object
        for (MapObject *object : objectGroup->objects()) {
            MapObjectItem *item = new MapObjectItem(object, renderer, this);
            if (drawOrder == ObjectGroup::TopDownOrder)
                item->setZValue(item->y());
        }
    }

    QRectF boundingRect() const override { return QRectF(); }
    void paint(QPainter *, const QStyleOptionGraphicsItem *, QWidget *) override {}
};

/**
 * Item that represents a map.
 */
class MapItem : public QGraphicsItem
{
public:
    MapItem(Map *map, MapRenderer *renderer, QGraphicsItem *parent = nullptr)
        : QGraphicsItem(parent)
    {
        setFlag(QGraphicsItem::ItemHasNoContents);

        // Create a child item for each layer
        for (Layer *layer : map->layers()) {
            if (TileLayer *tileLayer = layer->asTileLayer()) {
                new TileLayerItem(tileLayer, renderer, this);
            } else if (ObjectGroup *objectGroup = layer->asObjectGroup()) {
                new ObjectGroupItem(objectGroup, renderer, this);
            }
        }
    }

    QRectF boundingRect() const override { return QRectF(); }
    void paint(QPainter *, const QStyleOptionGraphicsItem *, QWidget *) override {}
};


TmxViewer::TmxViewer(QWidget *parent) :
    QGraphicsView(parent),
    mScene(new QGraphicsScene(this))
{
    setWindowTitle(tr("TMX Viewer"));

    setScene(mScene);
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    setDragMode(QGraphicsView::ScrollHandDrag);
    setOptimizationFlags(QGraphicsView::DontAdjustForAntialiasing
                         | QGraphicsView::DontSavePainterState);
    setBackgroundBrush(Qt::black);
    setFrameStyle(QFrame::NoFrame);

    viewport()->setAttribute(Qt::WA_StaticContents);
}

TmxViewer::~TmxViewer() = default;

bool TmxViewer::viewMap(const QString &fileName)
{
    mScene->clear();
    centerOn(0, 0);

    mRenderer.reset();

    MapReader reader;
    mMap = reader.readMap(fileName);
    if (!mMap) {
        qWarning().noquote() << "Error:" << reader.errorString();
        return false;
    }

    switch (mMap->orientation()) {
    case Map::Isometric:
        mRenderer = std::make_unique<IsometricRenderer>(mMap.get());
        break;
    case Map::Staggered:
        mRenderer = std::make_unique<StaggeredRenderer>(mMap.get());
        break;
    case Map::Hexagonal:
        mRenderer = std::make_unique<HexagonalRenderer>(mMap.get());
        break;
    case Map::Orthogonal:
    default:
        mRenderer = std::make_unique<OrthogonalRenderer>(mMap.get());
        break;
    }

    mScene->addItem(new MapItem(mMap.get(), mRenderer.get()));

    return true;
}
