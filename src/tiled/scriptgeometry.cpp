/*
 * scriptgeometry.h
 * Copyright 2023, Thorbjørn Lindeijer <bjorn@lindeijer.nl>
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

#include "scriptgeometry.h"

#include "geometry.h"
#include "regionvaluetype.h"

#include <QJSEngine>

namespace Tiled {

/**
 * This class makes select geometry functions available to the scripting API.
 */
class ScriptGeometry : public QObject
{
    Q_OBJECT

public:
    explicit ScriptGeometry(QObject *parent = nullptr)
        : QObject(parent)
    {}

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    Q_INVOKABLE QJSValue pointsOnLine(QPoint a, QPoint b, bool manhattan = false);
    Q_INVOKABLE QJSValue pointsOnEllipse(QPoint center, int radiusX, int radiusY);
#else
    Q_INVOKABLE QVector<QPoint> pointsOnLine(QPoint a, QPoint b, bool manhattan = false)
    { return Tiled::pointsOnLine(a, b, manhattan); }

    Q_INVOKABLE QVector<QPoint> pointsOnEllipse(QPoint center, int radiusX, int radiusY)
    { return Tiled::pointsOnEllipse(center, radiusX, radiusY); }
#endif

    Q_INVOKABLE Tiled::RegionValueType ellipseRegion(QRect rect)
    { return RegionValueType { Tiled::ellipseRegion(rect) }; }

    Q_INVOKABLE Tiled::RegionValueType ellipseRegion(int x0, int y0, int x1, int y1)
    { return RegionValueType { Tiled::ellipseRegion(x0, y0, x1, y1) }; }
};

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
static QJSValue toJSValue(const QVector<QPoint> &points, QJSEngine *engine)
{
    if (!engine)
        return QJSValue();

    QJSValue array = engine->newArray(points.size());

    for (int i = 0; i < points.size(); ++i) {
        QJSValue pointObject = engine->newObject();
        pointObject.setProperty(QStringLiteral("x"), points.at(i).x());
        pointObject.setProperty(QStringLiteral("y"), points.at(i).y());
        array.setProperty(i, pointObject);
    }

    return array;
}

QJSValue ScriptGeometry::pointsOnLine(QPoint a, QPoint b, bool manhattan)
{
    return toJSValue(Tiled::pointsOnLine(a, b, manhattan),
                     qjsEngine(this));
}

QJSValue ScriptGeometry::pointsOnEllipse(QPoint center, int radiusX, int radiusY)
{
    return toJSValue(Tiled::pointsOnEllipse(center, radiusX, radiusY),
                     qjsEngine(this));
}
#endif

void registerGeometry(QJSEngine *jsEngine)
{
    jsEngine->globalObject().setProperty(QStringLiteral("Geometry"),
                                         jsEngine->newQObject(new ScriptGeometry));

}

} // namespace Tiled

#include "scriptgeometry.moc"
