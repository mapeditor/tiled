/*
 * tiledproxystyle.cpp
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

#include "tiledproxystyle.h"

#include <QAbstractScrollArea>
#include <QPainter>
#include <QPixmapCache>
#include <QScrollBar>
#include <QStringBuilder>
#include <QStyleOptionComplex>

/*
 * Below there are a lot of helper functions which are copied from various
 * private Qt parts that are used by the Fusion style, on which the Tiled style
 * is based.
 *
 * These parts are Copyright (C) 2015 The Qt Company Ltd.
 * Used under the terms of the GNU Lesser General Public License version 2.1
 */

static Q_DECL_CONSTEXPR inline int qt_div_255(int x) { return (x + (x>>8) + 0x80) >> 8; }

// internal helper. Converts an integer value to an unique string token
template <typename T>
        struct HexString
{
    inline HexString(const T t)
        : val(t)
    {}
    inline void write(QChar *&dest) const
    {
        const ushort hexChars[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f' };
        const char *c = reinterpret_cast<const char *>(&val);
        for (uint i = 0; i < sizeof(T); ++i) {
            *dest++ = hexChars[*c & 0xf];
            *dest++ = hexChars[(*c & 0xf0) >> 4];
            ++c;
        }
    }
    const T val;
};
// specialization to enable fast concatenating of our string tokens to a string
template <typename T>
        struct QConcatenable<HexString<T> >
{
    typedef HexString<T> type;
    enum { ExactSize = true };
    static int size(const HexString<T> &) { return sizeof(T) * 2; }
    static inline void appendTo(const HexString<T> &str, QChar *&out) { str.write(out); }
    typedef QString ConvertTo;
};

static QPixmap colorizedImage(const QString &fileName, const QColor &color, int rotation = 0)
{
    QString pixmapName = QLatin1String("$qt_ia-") % fileName % HexString<uint>(color.rgba()) % QString::number(rotation);
    QPixmap pixmap;
    if (!QPixmapCache::find(pixmapName, pixmap)) {
        QImage image(fileName);
        if (image.format() != QImage::Format_ARGB32_Premultiplied)
            image = image.convertToFormat( QImage::Format_ARGB32_Premultiplied);
        int width = image.width();
        int height = image.height();
        int source = color.rgba();
        unsigned char sourceRed = qRed(source);
        unsigned char sourceGreen = qGreen(source);
        unsigned char sourceBlue = qBlue(source);
        for (int y = 0; y < height; ++y)
        {
            QRgb *data = (QRgb*) image.scanLine(y);
            for (int x = 0 ; x < width ; x++) {
                QRgb col = data[x];
                unsigned int colorDiff = (qBlue(col) - qRed(col));
                unsigned char gray = qGreen(col);
                unsigned char red = gray + qt_div_255(sourceRed * colorDiff);
                unsigned char green = gray + qt_div_255(sourceGreen * colorDiff);
                unsigned char blue = gray + qt_div_255(sourceBlue * colorDiff);
                unsigned char alpha = qt_div_255(qAlpha(col) * qAlpha(source));
                data[x] = qRgba(std::min(alpha, red),
                                std::min(alpha, green),
                                std::min(alpha, blue),
                                alpha);
            }
        }
        if (rotation != 0) {
            QTransform transform;
            transform.translate(-image.width()/2, -image.height()/2);
            transform.rotate(rotation);
            transform.translate(image.width()/2, image.height()/2);
            image = image.transformed(transform);
        }
        pixmap = QPixmap::fromImage(image);
        QPixmapCache::insert(pixmapName, pixmap);
    }
    return pixmap;
}

namespace QStyleHelper {

static QColor backgroundColor(const QPalette &pal, const QWidget* widget)
{
    if (qobject_cast<const QScrollBar *>(widget) && widget->parent() &&
            qobject_cast<const QAbstractScrollArea *>(widget->parent()->parent()))
        return widget->parentWidget()->parentWidget()->palette().color(QPalette::Base);
    return pal.color(QPalette::Base);
}

} // namespace QStyleHelper

static QColor getOutlineColor(const QPalette &pal)
{
    return pal.window().color().darker(140);
}

static QColor getSliderColor(const QPalette &pal, bool isDarkBg)
{
    return isDarkBg ? pal.button().color() : pal.window().color().darker(170);
}

static QColor getSliderOutline(const QPalette &pal, bool isDarkBg)
{
    return isDarkBg ? getOutlineColor(pal) : getSliderColor(pal, isDarkBg);
}

static QColor getButtonColor(const QPalette &pal)
{
    QColor buttonColor = pal.button().color();
    int val = qGray(buttonColor.rgb());
    buttonColor = buttonColor.lighter(100 + qMax(1, (180 - val)/6));
    buttonColor.setHsv(buttonColor.hue(), buttonColor.saturation() * 0.75, buttonColor.value());
    return buttonColor;
}

static QColor innerContrastLine()
{
    return QColor(255, 255, 255, 30);
}

TiledProxyStyle::TiledProxyStyle(QStyle *style)
    : QProxyStyle(style)
{
    setObjectName(QLatin1String("tiled"));
}

void TiledProxyStyle::drawComplexControl(ComplexControl control,
                                    const QStyleOptionComplex *option,
                                    QPainter *painter,
                                    const QWidget *widget) const
{
    QColor buttonColor = getButtonColor(option->palette);
    QColor gradientStartColor = buttonColor.lighter(118);
    QColor gradientStopColor = buttonColor;
    QColor outline = getOutlineColor(option->palette);

    switch (control) {
    case CC_ScrollBar:
        painter->save();
        if (const QStyleOptionSlider *scrollBar = qstyleoption_cast<const QStyleOptionSlider *>(option)) {
            bool horizontal = scrollBar->orientation == Qt::Horizontal;
            bool sunken = scrollBar->state & State_Sunken;

            QRect scrollBarSubLine = proxy()->subControlRect(control, scrollBar, SC_ScrollBarSubLine, widget);
            QRect scrollBarAddLine = proxy()->subControlRect(control, scrollBar, SC_ScrollBarAddLine, widget);
            QRect scrollBarSlider = proxy()->subControlRect(control, scrollBar, SC_ScrollBarSlider, widget);

            QRect rect = option->rect;
            QColor alphaOutline = outline;
            alphaOutline.setAlpha(180);

            QColor arrowColor = option->palette.foreground().color();
            arrowColor.setAlpha(220);

            const QColor bgColor = QStyleHelper::backgroundColor(option->palette, widget);
            const bool isDarkBg = bgColor.red() < 128 && bgColor.green() < 128 && bgColor.blue() < 128;

            // Paint groove
            if (scrollBar->subControls & SC_ScrollBarGroove) {
                QLinearGradient gradient(rect.center().x(), rect.top(),
                                         rect.center().x(), rect.bottom());
                if (!horizontal)
                    gradient = QLinearGradient(rect.left(), rect.center().y(),
                                               rect.right(), rect.center().y());

                gradient.setColorAt(0, bgColor.darker(150));
                gradient.setColorAt(0.5, bgColor.darker(120));
                gradient.setColorAt(1, bgColor.darker(110));

                painter->fillRect(rect, gradient);
                painter->setPen(outline);
                if (horizontal)
                    painter->drawLine(rect.topLeft(), rect.topRight());
                else
                    painter->drawLine(rect.topLeft(), rect.bottomLeft());
            }

            QRect pixmapRect = scrollBarSlider;
            QLinearGradient gradient(pixmapRect.center().x(), pixmapRect.top(),
                                     pixmapRect.center().x(), pixmapRect.bottom());
            if (!horizontal)
                gradient = QLinearGradient(pixmapRect.left(), pixmapRect.center().y(),
                                           pixmapRect.right(), pixmapRect.center().y());

            gradient.setColorAt(0, buttonColor.lighter(108));
            gradient.setColorAt(1, buttonColor);

            QLinearGradient highlightedGradient = gradient;
            highlightedGradient.setColorAt(0, gradientStartColor.darker(102));
            highlightedGradient.setColorAt(1, gradientStopColor.lighter(102));

            // Paint slider
            if (scrollBar->subControls & SC_ScrollBarSlider) {
                QColor sliderColor = getSliderColor(option->palette, isDarkBg);

                QRect sliderRect = scrollBarSlider.adjusted(3, 2, -3, -3);
                if (horizontal)
                    sliderRect = scrollBarSlider.adjusted(2, 3, -3, -3);
                painter->setPen(QPen(getSliderOutline(option->palette, isDarkBg)));
                if (sunken && scrollBar->activeSubControls & SC_ScrollBarSlider) {
                    QLinearGradient sunkenGradient = gradient;
                    sunkenGradient.setColorAt(0, sliderColor.lighter(130));
                    sunkenGradient.setColorAt(1, sliderColor.lighter(105));
                    painter->setBrush(sunkenGradient);
                } else if (option->state & State_MouseOver && scrollBar->activeSubControls & SC_ScrollBarSlider) {
                    QLinearGradient highlightedGradient = gradient;
                    highlightedGradient.setColorAt(0, sliderColor.lighter(135));
                    highlightedGradient.setColorAt(1, sliderColor.lighter(110));
                    painter->setBrush(highlightedGradient);
                } else {
                    QLinearGradient sliderGradient = gradient;
                    sliderGradient.setColorAt(0, sliderColor.lighter(120));
                    sliderGradient.setColorAt(1, sliderColor);
                    painter->setBrush(sliderGradient);
                }
                painter->save();
                painter->setRenderHint(QPainter::Antialiasing, true);
                painter->translate(0.5, 0.5);
                painter->drawRoundedRect(sliderRect, 2, 2);
                painter->setPen(innerContrastLine());
                painter->drawRoundedRect(sliderRect.adjusted(1, 1, -1, -1), 2, 2);
                painter->restore();
            }

            // The SubLine (up/left) buttons
            if (scrollBar->subControls & SC_ScrollBarSubLine) {
                if ((scrollBar->activeSubControls & SC_ScrollBarSubLine) && sunken)
                    painter->setBrush(gradientStopColor);
                else if ((scrollBar->activeSubControls & SC_ScrollBarSubLine))
                    painter->setBrush(highlightedGradient);
                else
                    painter->setBrush(Qt::NoBrush);

                painter->setPen(Qt::NoPen);
                painter->drawRect(scrollBarSubLine.adjusted(horizontal ? 0 : 1, horizontal ? 1 : 0, 0, 0));
                painter->setPen(QPen(alphaOutline));
                if (horizontal) {
                    if (option->direction == Qt::RightToLeft) {
                        pixmapRect.setLeft(scrollBarSubLine.left());
                        painter->drawLine(pixmapRect.topLeft(), pixmapRect.bottomLeft());
                    } else {
                        pixmapRect.setRight(scrollBarSubLine.right());
                        painter->drawLine(pixmapRect.topRight(), pixmapRect.bottomRight());
                    }
                } else {
                    pixmapRect.setBottom(scrollBarSubLine.bottom());
                    painter->drawLine(pixmapRect.bottomLeft(), pixmapRect.bottomRight());
                }

                painter->setBrush(Qt::NoBrush);
                painter->setPen(innerContrastLine());
                painter->drawRect(scrollBarSubLine.adjusted(horizontal ? 0 : 1, horizontal ? 1 : 0 ,  horizontal ? -2 : -1, horizontal ? -1 : -2));

                // Arrows
                int rotation = 0;
                if (horizontal)
                    rotation = option->direction == Qt::LeftToRight ? -90 : 90;
                QRect upRect = scrollBarSubLine.translated(horizontal ? -2 : -1, 0);
                QPixmap arrowPixmap = colorizedImage(QLatin1String(":/qt-project.org/styles/commonstyle/images/fusion_arrow.png"), arrowColor, rotation);
                painter->drawPixmap(QRectF(upRect.center().x() - arrowPixmap.width() / 4.0  + 2.0,
                                          upRect.center().y() - arrowPixmap.height() / 4.0 + 1.0,
                                          arrowPixmap.width() / 2.0, arrowPixmap.height() / 2.0),
                                          arrowPixmap, QRectF(QPoint(0.0, 0.0), arrowPixmap.size()));
            }

            // The AddLine (down/right) button
            if (scrollBar->subControls & SC_ScrollBarAddLine) {
                if ((scrollBar->activeSubControls & SC_ScrollBarAddLine) && sunken)
                    painter->setBrush(gradientStopColor);
                else if ((scrollBar->activeSubControls & SC_ScrollBarAddLine))
                    painter->setBrush(highlightedGradient);
                else
                    painter->setBrush(Qt::NoBrush);

                painter->setPen(Qt::NoPen);
                painter->drawRect(scrollBarAddLine.adjusted(horizontal ? 0 : 1, horizontal ? 1 : 0, 0, 0));
                painter->setPen(QPen(alphaOutline, 1));
                if (horizontal) {
                    if (option->direction == Qt::LeftToRight) {
                        pixmapRect.setLeft(scrollBarAddLine.left());
                        painter->drawLine(pixmapRect.topLeft(), pixmapRect.bottomLeft());
                    } else {
                        pixmapRect.setRight(scrollBarAddLine.right());
                        painter->drawLine(pixmapRect.topRight(), pixmapRect.bottomRight());
                    }
                } else {
                    pixmapRect.setTop(scrollBarAddLine.top());
                    painter->drawLine(pixmapRect.topLeft(), pixmapRect.topRight());
                }

                painter->setPen(innerContrastLine());
                painter->setBrush(Qt::NoBrush);
                painter->drawRect(scrollBarAddLine.adjusted(1, 1, -1, -1));

                int rotation = 180;
                if (horizontal)
                    rotation = option->direction == Qt::LeftToRight ? 90 : -90;
                QRect downRect = scrollBarAddLine.translated(-1, 1);
                QPixmap arrowPixmap = colorizedImage(QLatin1String(":/qt-project.org/styles/commonstyle/images/fusion_arrow.png"), arrowColor, rotation);
                painter->drawPixmap(QRectF(downRect.center().x() - arrowPixmap.width() / 4.0 + 2.0,
                                           downRect.center().y() - arrowPixmap.height() / 4.0,
                                           arrowPixmap.width() / 2.0, arrowPixmap.height() / 2.0),
                                           arrowPixmap, QRectF(QPoint(0.0, 0.0), arrowPixmap.size()));
            }
        }
        painter->restore();
        break;
    default:
        QProxyStyle::drawComplexControl(control, option, painter, widget);
        break;
    }
}

int TiledProxyStyle::pixelMetric(QStyle::PixelMetric metric,
                                 const QStyleOption *option,
                                 const QWidget *widget) const
{
    switch (metric) {
    case PM_MenuBarItemSpacing:
        return 0;                   // no space between menu bar items
    default:
        return QProxyStyle::pixelMetric(metric, option, widget);
    }
}

QSize TiledProxyStyle::sizeFromContents(ContentsType type,
                                        const QStyleOption *opt,
                                        const QSize &contentsSize,
                                        const QWidget *w) const
{
    QSize size(contentsSize);

    switch (type) {
    case CT_MenuBarItem:
        if (!size.isEmpty())
            size += QSize(16, 5);   // make the menu bar items itself wider
        break;
    case CT_ItemViewItem:           // give item view items a little more space
        size = QCommonStyle::sizeFromContents(type, opt, contentsSize, w);
        size += QSize(0, QStyleHelper::dpiScaled(2.));
        break;
    default:
        size = QProxyStyle::sizeFromContents(type, opt, contentsSize, w);
        break;
    }

    return size;
}
