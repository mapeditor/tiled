#include "mapgriditem.h"

#include <QQuickWindow>
#include <QSGFlatColorMaterial>

using namespace TiledQuick;

MapGridItem::MapGridItem(QQuickItem *parent)
    : QQuickItem(parent)
{
    setFlag(ItemHasContents);
}

MapGridItem::~MapGridItem() = default;

QSGNode *MapGridItem::updatePaintNode(QSGNode *node, QQuickItem::UpdatePaintNodeData *)
{
    int gridWidth = mGridSize.x();
    int gridHeight = mGridSize.y();
    int tileWidth = width() / gridWidth;
    int tileHeight = height() / gridHeight;
    float segmentDrawLength = static_cast<float>(mSegmentLength / mScale);
    qDebug() << segmentDrawLength;

    // Currently only top line
    int wVertexCountPerTile = 2 * ceil(0.5 * tileWidth * mScale / mSegmentLength);
    int hVertexCountPerTile = 2 * ceil(0.5 * tileHeight * mScale / mSegmentLength);

    int newVertexCount = (gridWidth * gridHeight) *
                         (wVertexCountPerTile + hVertexCountPerTile);

    qDebug() << gridWidth << "x" << gridHeight << ":" << tileWidth << "x" << tileHeight << mScale << "v:" << wVertexCountPerTile << "x" << hVertexCountPerTile << "x" << gridWidth << "x" << gridHeight;

    QSGGeometryNode *gridNode = static_cast<QSGGeometryNode *>(node);

    if (!gridNode) {
        gridNode = new QSGGeometryNode;

        auto *geometry = new QSGGeometry(QSGGeometry::defaultAttributes_Point2D(), newVertexCount);
        geometry->setDrawingMode(QSGGeometry::DrawLines);

        gridNode->setGeometry(geometry);
        gridNode->setFlag(QSGNode::OwnsGeometry);

        auto *material = new QSGFlatColorMaterial;
        gridNode->setMaterial(material);
        gridNode->setFlag(QSGNode::OwnsMaterial);
    }

    auto *geometry = gridNode->geometry();

    if (geometry->vertexCount() != newVertexCount) {
        geometry->allocate(newVertexCount);
    }

    auto *vertex = geometry->vertexDataAsPoint2D();
    int currentVertex = 0;

    for (int gridX = 0; gridX < gridWidth; gridX++) {
        for (int gridY = 0; gridY < gridHeight; gridY++) {
            for (int i = 0; i < wVertexCountPerTile/2; i++) {
                float startX = i * segmentDrawLength * 2 + gridX * tileWidth;
                float endX = startX + segmentDrawLength;

                vertex[currentVertex++].set(startX, gridY * tileHeight);
                vertex[currentVertex++].set(endX, gridY * tileHeight);
            }

            for (int i = 0; i < hVertexCountPerTile/2; i++) {
                float startY = i * segmentDrawLength * 2 + gridY * tileHeight;
                float endY = startY + segmentDrawLength;

                vertex[currentVertex++].set(gridX * tileWidth, startY);
                vertex[currentVertex++].set(gridX * tileWidth, endY);
            }
        }
    }


    qDebug() << currentVertex;


    gridNode->markDirty(QSGNode::DirtyGeometry);

    static_cast<QSGFlatColorMaterial *>(gridNode->material())->setColor(mColor);

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
