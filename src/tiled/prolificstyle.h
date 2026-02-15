/*
 * prolificstyle.h
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

#include <QCommonStyle>

#pragma once

class QStyleOptionButton;
class QStyleOptionSlider;

namespace Tiled {

class ProlificStyle : public QCommonStyle
{
    Q_OBJECT

public:
    ProlificStyle();

    void drawPrimitive(PrimitiveElement pe, const QStyleOption *opt, QPainter *p, const QWidget *w = nullptr) const override;
    void drawControl(ControlElement element, const QStyleOption *opt, QPainter *p, const QWidget *w = nullptr) const override;
    QRect subElementRect(SubElement r, const QStyleOption *opt, const QWidget *widget = nullptr) const override;
    void drawComplexControl(ComplexControl cc, const QStyleOptionComplex *opt, QPainter *p,
                            const QWidget *w = nullptr) const override;
//    SubControl hitTestComplexControl(ComplexControl cc, const QStyleOptionComplex *opt,
//                                     const QPoint &pt, const QWidget *w = nullptr) const override;
    QRect subControlRect(ComplexControl cc, const QStyleOptionComplex *opt, SubControl sc,
                         const QWidget *w = nullptr) const override;
//    QSize sizeFromContents(ContentsType ct, const QStyleOption *opt,
//                           const QSize &contentsSize, const QWidget *widget = nullptr) const override;

    int pixelMetric(PixelMetric m, const QStyleOption *opt = nullptr, const QWidget *widget = nullptr) const override;

    int styleHint(StyleHint sh, const QStyleOption *opt = nullptr, const QWidget *w = nullptr,
                  QStyleHintReturn *shret = nullptr) const override;

//    QIcon standardIcon(StandardPixmap standardIcon, const QStyleOption *opt = nullptr,
//                       const QWidget *widget = nullptr) const override;
//    QPixmap standardPixmap(StandardPixmap sp, const QStyleOption *opt = nullptr,
//                           const QWidget *widget = nullptr) const override;

//    QPixmap generatedIconPixmap(QIcon::Mode iconMode, const QPixmap &pixmap,
//                                const QStyleOption *opt) const override;
//    int layoutSpacing(QSizePolicy::ControlType control1, QSizePolicy::ControlType control2,
//                      Qt::Orientation orientation, const QStyleOption *option = nullptr,
//                      const QWidget *widget = nullptr) const override;

//    void polish(QPalette &) override;
//    void polish(QApplication *app) override;
    void polish(QWidget *widget) override;
    void unpolish(QWidget *widget) override;
//    void unpolish(QApplication *application) override;

private:
    QRect scrollBarSubControlRect(const QStyleOptionSlider *opt, SubControl sc, const QWidget *w) const;
    void scrollBarDrawGroove(const QStyleOptionSlider *opt, QPainter *p) const;
    void scrollBarDrawSlider(const QStyleOptionSlider *opt, QPainter *p) const;
    int scrollBarWidth(const QStyleOption *opt) const;
    int scrollBarSliderMin(const QStyleOption *opt) const;

    bool mScrollBarOverlaps = true;
};

} // namespace Tiled
