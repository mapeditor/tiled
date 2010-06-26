/*
 * zoomable.h
 * Copyright 2009-2010, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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

#ifndef ZOOMABLE_H
#define ZOOMABLE_H

#include <QObject>

namespace Tiled {
namespace Internal {

/**
 * This class represents something zoomable. Is has a zoom factor and methods
 * to change this factor in several ways.
 *
 * A class that wants to be zoomable would aggregate this class, and connect
 * to the scaleChanged() signal in order to adapt to the new zoom factor.
 */
class Zoomable : public QObject
{
    Q_OBJECT

public:
    Zoomable(QObject *parent = 0);

    void setScale(qreal scale);
    qreal scale() const { return mScale; }

    bool canZoomIn() const;
    bool canZoomOut() const;

    /**
     * Returns whether images should be smoothly transformed when drawn at the
     * current scale. This is the case when the scale is not a whole number.
     */
    bool smoothTransform() const
    { return mScale != (int) mScale; }

public slots:
    void zoomIn();
    void zoomOut();
    void resetZoom();

signals:
    void scaleChanged(qreal scale);

private:
    qreal mScale;
};

} // namespace Internal
} // namespace Tiled

#endif // ZOOMABLE_H
