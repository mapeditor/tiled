#include "mapborderitem.h"

#include <QQuickWindow>
#include <QSGFlatColorMaterial>

using namespace TiledQuick;

MapBorderItem::MapBorderItem(QQuickItem *parent)
    : QQuickItem(parent)
{
    setFlag(ItemHasContents);
}

MapBorderItem::~MapBorderItem() = default;

QSGNode *MapBorderItem::updatePaintNode(QSGNode *node, QQuickItem::UpdatePaintNodeData *)
{
    QSGGeometryNode *borderNode = static_cast<QSGGeometryNode *>(node);

    if (!borderNode) {
        borderNode = new QSGGeometryNode;

        auto *geometry = new QSGGeometry(QSGGeometry::defaultAttributes_Point2D(), 5);
        geometry->setDrawingMode(QSGGeometry::DrawLineStrip);

        borderNode->setGeometry(geometry);
        borderNode->setFlag(QSGNode::OwnsGeometry);

        auto *material = new QSGFlatColorMaterial;
        borderNode->setMaterial(material);
        borderNode->setFlag(QSGNode::OwnsMaterial);
    }

    auto *geometry = borderNode->geometry();
    auto *vertex = geometry->vertexDataAsPoint2D();
    vertex[0].set(0, 0);
    vertex[1].set(width(), 0);
    vertex[2].set(width(), height());
    vertex[3].set(0, height());
    vertex[4].set(0, 0);
    borderNode->markDirty(QSGNode::DirtyGeometry);

    static_cast<QSGFlatColorMaterial *>(borderNode->material())->setColor(mColor);

    return borderNode;
}

void MapBorderItem::setColor(const QColor &c)
{
    if (mColor != c) {
        mColor = c;
        emit colorChanged();
        update();
    }
}

QColor MapBorderItem::color() const
{
    return mColor;
}
