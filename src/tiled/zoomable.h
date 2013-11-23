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
#include <QRegExp>
#include <QVector>

class QComboBox;
class QPinchGesture;
class QRegExpValidator;

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
     * Changes the current scale based on the given mouse wheel \a delta.
     *
     * For convenience, the delta is assumed to be in the same units as
     * QWheelEvent::delta, which is the distance that the wheel is rotated,
     * in eighths of a degree.
     */
    void handleWheelDelta(int delta);

    /**
     * Changes the current scale based on the given pinch gesture.
     */
    void handlePinchGesture(QPinchGesture *pinch);

    /**
     * Returns whether images should be smoothly transformed when drawn at the
     * current scale. This is the case when the scale is not 1 and smaller than
     * 2.
     */
    bool smoothTransform() const
    { return mScale != qreal(1) && mScale < qreal(2); }

    void setZoomFactors(const QVector<qreal>& factors);
    void connectToComboBox(QComboBox *comboBox);

public slots:
    void zoomIn();
    void zoomOut();
    void resetZoom();

private slots:
    void comboActivated(int index);
    void comboEdited();

signals:
    void scaleChanged(qreal scale);

private:
    void syncComboBox();

private:
    qreal mScale;
    qreal mGestureStartScale;
    QVector<qreal> mZoomFactors;
    QComboBox *mComboBox;
    QRegExp mComboRegExp;
    QRegExpValidator *mComboValidator;
};

} // namespace Internal
} // namespace Tiled

#endif // ZOOMABLE_H
