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

namespace Tiled {
namespace Internal {

class TiledProxyStyle : public QProxyStyle
{
    Q_OBJECT

public:
    TiledProxyStyle(const QPalette &palette, QStyle *style = nullptr);

    void setPalette(const QPalette &palette);

    void drawPrimitive(PrimitiveElement element,
                       const QStyleOption *option,
                       QPainter *painter,
                       const QWidget *widget) const override;

    void drawControl(ControlElement element,
                     const QStyleOption *option,
                     QPainter *painter,
                     const QWidget *widget) const override;

    void drawComplexControl(ComplexControl control,
                            const QStyleOptionComplex *option,
                            QPainter *painter,
                            const QWidget *widget = nullptr) const override;

    int pixelMetric(PixelMetric metric,
                    const QStyleOption *option,
                    const QWidget *widget) const override;

    QSize sizeFromContents(ContentsType type,
                           const QStyleOption *option,
                           const QSize &contentsSize,
                           const QWidget *widget) const override;

    QRect subElementRect(SubElement subElement,
                         const QStyleOption *option,
                         const QWidget *widget) const override;

    int styleHint(StyleHint styleHint,
                  const QStyleOption *option,
                  const QWidget *widget,
                  QStyleHintReturn *returnData) const override;

private:
    QPalette mPalette;
    bool mIsDark;
};

} // namespace Internal
} // namespace Tiled

#endif // TILEDPROXYSTYLE_H
