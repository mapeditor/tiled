/*
 * rulerwidget.cpp
 * Copyright 2024, Tiled Contributors
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

#include "rulerwidget.h"

#include "mapdocument.h"
#include "maprenderer.h"
#include "mapview.h"
#include "preferences.h"

#include <QPainter>

#include <cmath>

using namespace Tiled;

RulerWidget::RulerWidget(Orientation orientation, MapView *mapView)
    : QWidget(mapView)
    , mOrientation(orientation)
    , mMapView(mapView)
{
    // Rulers sit on top of the viewport; don't let them steal mouse events.
    setAttribute(Qt::WA_TransparentForMouseEvents);
    // Avoid inheriting the view's background.
    setAutoFillBackground(false);
    setAttribute(Qt::WA_NoSystemBackground);
}

void RulerWidget::setMapDocument(MapDocument *mapDocument)
{
    mMapDocument = mapDocument;
    update();
}

void RulerWidget::paintEvent(QPaintEvent *)
{
    if (!mMapDocument)
        return;

    MapRenderer *renderer = mMapDocument->renderer();

    Preferences *prefs = Preferences::instance();

    const QSize gridMajor = prefs->gridMajor();

    // --- Styling: use palette colors so we respect the application theme ---
    const QPalette pal      = palette();
    const QColor bgColor    = pal.color(QPalette::Window);
    const QColor textColor  = pal.color(QPalette::WindowText);
    const QColor tickColor  = pal.color(QPalette::Mid);
    const QColor borderColor= pal.color(QPalette::Dark);
    constexpr int fontSize = 9;

    QPainter painter(this);
    painter.setRenderHint(QPainter::TextAntialiasing);

    painter.fillRect(rect(), bgColor);

    painter.setPen(QPen(borderColor, 1));
    if (mOrientation == Orientation::Horizontal)
        painter.drawLine(rect().bottomLeft(), rect().bottomRight());
    else
        painter.drawLine(rect().topRight(), rect().bottomRight());

    // Map the viewport's visible rect into scene coordinates, then ask the
    // renderer to convert scene → tile coords.
    // mapView->viewport()->rect() is in viewport widget coords.
    // mapToScene converts viewport coords → scene coords.
    const QRect  vpRect    = mMapView->viewport()->rect();
    const QRectF sceneRect = mMapView->mapToScene(vpRect).boundingRect();

    QFont font = painter.font();
    font.setPixelSize(fontSize);
    painter.setFont(font);

    if (mOrientation == Orientation::Horizontal) {
        // Determine the tile X range visible in the viewport
        const int xMin = static_cast<int>(std::floor(renderer->pixelToTileCoords(sceneRect.topLeft()).x()));
        const int xMax = static_cast<int>(std::ceil (renderer->pixelToTileCoords(sceneRect.topRight()).x()));

        for (int x = xMin; x <= xMax; ++x) {
            const qreal sceneX = renderer->tileToPixelCoords(x, 0).x();
            // Convert scene X → viewport X → ruler widget X
            // (ruler widget is positioned at (RULER_SIZE, 0) relative to MapView,
            //  but its x=0 aligns with viewport x=0 because of how we position it)
            const int widgetX = mMapView->mapFromScene(QPointF(sceneX, 0)).x();

            if (widgetX < 0 || widgetX > width())
                continue;

            const bool isMajor = gridMajor.width() != 0 && (x % gridMajor.width()) == 0;

            if (isMajor) {
                painter.setPen(QPen(tickColor, 1));
                painter.drawLine(widgetX, height() - 8, widgetX, height());
                painter.setPen(textColor);
                painter.drawText(QRect(widgetX - 20, 2, 40, height() - 10),
                                 Qt::AlignCenter, QString::number(x));
            } else {
                painter.setPen(QPen(tickColor, 1));
                painter.drawLine(widgetX, height() - 4, widgetX, height());
            }
        }
    } else {
        const int yMin = static_cast<int>(std::floor(renderer->pixelToTileCoords(sceneRect.topLeft()).y()));
        const int yMax = static_cast<int>(std::ceil (renderer->pixelToTileCoords(sceneRect.bottomLeft()).y()));

        for (int y = yMin; y <= yMax; ++y) {
            const qreal sceneY  = renderer->tileToPixelCoords(0, y).y();
            const int   widgetY = mMapView->mapFromScene(QPointF(0, sceneY)).y();

            if (widgetY < 0 || widgetY > height())
                continue;

            const bool isMajor = gridMajor.height() != 0 && (y % gridMajor.height()) == 0;

            if (isMajor) {
                painter.setPen(QPen(tickColor, 1));
                painter.drawLine(width() - 8, widgetY, width(), widgetY);
                painter.setPen(textColor);
                painter.save();
                painter.translate(width() / 2, widgetY);
                painter.rotate(-90);
                painter.drawText(QRect(-20, -fontSize / 2 - 1, 40, fontSize + 2),
                                 Qt::AlignCenter, QString::number(y));
                painter.restore();
            } else {
                painter.setPen(QPen(tickColor, 1));
                painter.drawLine(width() - 4, widgetY, width(), widgetY);
            }
        }
    }
}
