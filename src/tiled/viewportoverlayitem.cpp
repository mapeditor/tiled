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
#include "map.h"
#include "preferences.h"

#include <QPen>
#include <QCoreApplication>

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
    
    // Enable hover events for tooltip
    setAcceptHoverEvents(true);
    
    // Only sync if we have a valid map document
    if (mMapDocument)
        syncWithMapDocument();
    else
        setVisible(false);
}

void ViewportOverlayItem::syncWithMapDocument()
{
    // Bail out if no map document
    if (!mMapDocument) {
        setVisible(false);
        return;
    }
    
    // Bail out if map is null
    Map *map = mMapDocument->map();
    if (!map) {
        setVisible(false);
        return;
    }
    
    // Get preferences safely
    Preferences *prefs = Preferences::instance();
    if (!prefs) {
        setVisible(false);
        return;
    }
    
    const bool showViewport = prefs->showViewport();
    const QSize viewportSize = map->viewportSize();
    
    // Validate viewport size (must be positive and reasonable)
    const bool validSize = !viewportSize.isEmpty() && 
                          viewportSize.width() > 0 && 
                          viewportSize.height() > 0 && 
                          viewportSize.width() <= 32768 &&  // Max reasonable size
                          viewportSize.height() <= 32768;
    
    // Only show if preference is enabled and viewport size is valid
    const bool shouldShow = showViewport && validSize;
    
    setVisible(shouldShow);
    
    if (shouldShow) {
        // Center the viewport rectangle at origin (0, 0)
        const qreal halfWidth = viewportSize.width() / 2.0;
        const qreal halfHeight = viewportSize.height() / 2.0;
        setRect(-halfWidth, -halfHeight, viewportSize.width(), viewportSize.height());
        
        // Set tooltip with viewport dimensions
        setToolTip(QCoreApplication::translate("ViewportOverlayItem", "Viewport: %1 × %2 px")
                   .arg(viewportSize.width()).arg(viewportSize.height()));
    } else {
        setToolTip(QString());
    }
}

} // namespace Tiled
