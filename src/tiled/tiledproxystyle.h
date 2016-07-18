/*
 * tiledproxystyle.h
 * Copyright 2016, Thorbjørn Lindeijer <bjorn@lindeijer.nl>
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

#ifndef TILEDPROXYSTYLE_H
#define TILEDPROXYSTYLE_H

#include <QProxyStyle>

class TiledProxyStyle : public QProxyStyle
{
public:
    TiledProxyStyle(QStyle *style = nullptr);

    void drawComplexControl(ComplexControl control,
                            const QStyleOptionComplex *option,
                            QPainter *painter,
                            const QWidget *widget = nullptr) const override;

    int pixelMetric(PixelMetric metric,
                    const QStyleOption *option,
                    const QWidget *widget) const override;

    QSize sizeFromContents(ContentsType type,
                           const QStyleOption *opt,
                           const QSize &contentsSize,
                           const QWidget *w) const override;
};

#endif // TILEDPROXYSTYLE_H
