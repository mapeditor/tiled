/*
 * Tiled Map Editor (Qt)
 * Copyright 2008 Tiled (Qt) developers (see AUTHORS file)
 *
 * This file is part of Tiled (Qt).
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
 * this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 * Place, Suite 330, Boston, MA 02111-1307, USA.
 */

#include "mapview.h"

using namespace Tiled::Internal;

MapView::MapView(QWidget *parent):
    QGraphicsView(parent),
    mScale(1)
{
    setTransformationAnchor(QGraphicsView::AnchorViewCenter);

    QWidget *v = viewport();

    /* Since Qt 4.5, setting this attribute yields significant repaint
     * reduction when the view is being resized. */
    v->setAttribute(Qt::WA_StaticContents);

    /* Since Qt 4.6, mouse tracking is disabled when no graphics item uses
     * hover events. We need to set it since our scene wants the events. */
    v->setMouseTracking(true);
}

void MapView::setScale(qreal scale)
{
    if (scale == mScale)
        return;

    mScale = scale;
    setTransform(QTransform::fromScale(mScale, mScale));
    setRenderHint(QPainter::SmoothPixmapTransform, mScale < qreal(1));
    emit scaleChanged(mScale);
}

static const int zoomFactorCount = 10;
static const qreal zoomFactors[zoomFactorCount] = {
    0.0625,
    0.125,
    0.25,
    0.5,
    0.75,
    1.0,
    1.5,
    2.0,
    3.0,
    4.0
};

void MapView::zoomIn()
{
    for (int i = 0; i < zoomFactorCount; ++i) {
        if (zoomFactors[i] > mScale) {
            setScale(zoomFactors[i]);
            break;
        }
    }
}

void MapView::zoomOut()
{
    for (int i = zoomFactorCount - 1; i >= 0; --i) {
        if (zoomFactors[i] < mScale) {
            setScale(zoomFactors[i]);
            break;
        }
    }
}

void MapView::resetZoom()
{
    setScale(1);
}
