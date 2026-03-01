/*
 * viewportoverlayitem.cpp
 * Copyright 2024, Thorbjørn Lindeijer <bjorn@lindeijer.nl>
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

#include "viewportoverlayitem.h"

#include "mapdocument.h"
#include "preferences.h"

#include <QPen>

namespace Tiled {

ViewportOverlayItem::ViewportOverlayItem(MapDocument *mapDocument, QGraphicsItem *parent)
    : QGraphicsRectItem(parent)
    , mMapDocument(mapDocument)
{
    setZValue(10000); // Place above map items
    setPen(QPen(QColor(255, 0, 0, 180), 2.0));
    setBrush(Qt::NoBrush);
    
    // Item should not be selectable or movable
    setFlag(QGraphicsItem::ItemIsSelectable, false);
    setFlag(QGraphicsItem::ItemIsMovable, false);
    
    syncWithMapDocument();
}

void ViewportOverlayItem::syncWithMapDocument()
{
    const bool showViewport = Preferences::instance()->showViewport();
    const QSize viewportSize = mMapDocument->map()->viewportSize();
    
    // Only show if preference is enabled and viewport size is valid
    const bool shouldShow = showViewport && 
                           !viewportSize.isEmpty() && 
                           viewportSize.width() > 0 && 
                           viewportSize.height() > 0;
    
    setVisible(shouldShow);
    
    if (shouldShow) {
        // Center the viewport rectangle at origin (0, 0)
        const qreal halfWidth = viewportSize.width() / 2.0;
        const qreal halfHeight = viewportSize.height() / 2.0;
        setRect(-halfWidth, -halfHeight, viewportSize.width(), viewportSize.height());
    }
}

} // namespace Tiled
