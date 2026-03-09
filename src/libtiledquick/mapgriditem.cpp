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
    return node;
}
