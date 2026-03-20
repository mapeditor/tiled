#include "mapgriditem.h"
#include "mapgridmaterial.h"

using namespace TiledQuick;

MapGridItem::MapGridItem(QQuickItem *parent)
    : QQuickItem(parent)
{
    setFlag(ItemHasContents);
}

MapGridItem::~MapGridItem() = default;

QSGNode *MapGridItem::updatePaintNode(QSGNode *node, QQuickItem::UpdatePaintNodeData *)
{
    QSGGeometryNode *gridNode = static_cast<QSGGeometryNode *>(node);

    if (!gridNode) {
        gridNode = new QSGGeometryNode;

        auto *geometry = new QSGGeometry(QSGGeometry::defaultAttributes_TexturedPoint2D(), 4);
        geometry->setDrawingMode(QSGGeometry::DrawTriangleStrip);

        gridNode->setGeometry(geometry);
        gridNode->setFlag(QSGNode::OwnsGeometry);

        auto *material = new MapGridMaterial;
        gridNode->setMaterial(material);
        gridNode->setFlag(QSGNode::OwnsMaterial);
    }

    auto *vertices = gridNode->geometry()->vertexDataAsTexturedPoint2D();

    vertices[0].set(0, 0, 0, 0);
    vertices[1].set(width(), 0, 1, 0);
    vertices[2].set(0, height(), 0, 1);
    vertices[3].set(width(), height(), 1, 1);
    gridNode->markDirty(QSGNode::DirtyGeometry);

    auto *material = static_cast<MapGridMaterial *>(gridNode->material());
    material->mColor = mColor;
    material->mScale = mScale;
    material->mPixelWidth = width();
    material->mPixelHeight = height();
    material->mTileWidth = width() / mGridSize.x();
    material->mTileHeight = height() / mGridSize.y();

    gridNode->markDirty(QSGNode::DirtyGeometry);

    return gridNode;
}

void MapGridItem::setGridSize(const QPointF &gridSize)
{
    if (mGridSize != gridSize) {
        mGridSize = gridSize;
        emit gridSizeChanged();
        update();
    }
}

QPointF MapGridItem::gridSize() const
{
    return mGridSize;
}

void MapGridItem::setScale(const qreal &scale)
{
    if (mScale != scale) {
        mScale = scale;
        emit gridSizeChanged();
        update();
    }
}

qreal MapGridItem::scale() const
{
    return mScale;
}

void MapGridItem::setColor(const QColor &color)
{
    if(mColor != color) {
        mColor = color;
        emit colorChanged();
        update();
    }
}

QColor MapGridItem::color() const
{
    return mColor;
}
