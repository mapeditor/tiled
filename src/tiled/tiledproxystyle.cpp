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
#include <QtMath>

Q_GUI_EXPORT int qt_defaultDpiX();

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

static qreal dpiScaled(qreal value)
{
#ifdef Q_OS_MAC
    // On mac the DPI is always 72 so we should not scale it
    return value;
#else
    static const qreal scale = qreal(qt_defaultDpiX()) / 96.0;
    return value * scale;
#endif
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

static QColor darkShade()
{
    return QColor(0, 0, 0, 60);
}

static QColor getTabFrameColor(const QPalette &pal)
{
    return getButtonColor(pal).lighter(104);
}


TiledProxyStyle::TiledProxyStyle(QStyle *style)
    : QProxyStyle(style)
{
    setObjectName(QLatin1String("tiled"));
}

void TiledProxyStyle::drawControl(QStyle::ControlElement element,
                                  const QStyleOption *option,
                                  QPainter *painter,
                                  const QWidget *widget) const
{
    QRect rect = option->rect;
    QColor outline = getOutlineColor(option->palette);
    QColor shadow = darkShade();

    switch (element) {
    case CE_TabBarTabShape:
        painter->save();
        if (const QStyleOptionTab *tab = qstyleoption_cast<const QStyleOptionTab *>(option)) {

            bool rtlHorTabs = (tab->direction == Qt::RightToLeft
                               && (tab->shape == QTabBar::RoundedNorth
                                   || tab->shape == QTabBar::RoundedSouth));
            bool selected = tab->state & State_Selected;
            bool lastTab = ((!rtlHorTabs && tab->position == QStyleOptionTab::End)
                            || (rtlHorTabs
                                && tab->position == QStyleOptionTab::Beginning));
            bool onlyOne = tab->position == QStyleOptionTab::OnlyOneTab;
            int tabOverlap = pixelMetric(PM_TabBarTabOverlap, option, widget);
            rect = option->rect.adjusted(0, 0, (onlyOne || lastTab) ? 0 : tabOverlap, 0);

            QRect r2(rect);
            int x1 = r2.left();
            int x2 = r2.right();
            int y1 = r2.top();
            int y2 = r2.bottom();

            painter->setPen(innerContrastLine());

            QTransform rotMatrix;
            bool flip = false;
            painter->setPen(shadow);

            switch (tab->shape) {
            case QTabBar::RoundedNorth:
                break;
            case QTabBar::RoundedSouth:
                rotMatrix.rotate(180);
                rotMatrix.translate(0, -rect.height() + 1);
                rotMatrix.scale(-1, 1);
                painter->setTransform(rotMatrix, true);
                break;
            case QTabBar::RoundedWest:
                rotMatrix.rotate(180 + 90);
                rotMatrix.scale(-1, 1);
                flip = true;
                painter->setTransform(rotMatrix, true);
                break;
            case QTabBar::RoundedEast:
                rotMatrix.rotate(90);
                rotMatrix.translate(0, - rect.width() + 1);
                flip = true;
                painter->setTransform(rotMatrix, true);
                break;
            default:
                painter->restore();
                QCommonStyle::drawControl(element, tab, painter, widget);
                return;
            }

            if (flip) {
                QRect tmp = rect;
                rect = QRect(tmp.y(), tmp.x(), tmp.height(), tmp.width());
                int temp = x1;
                x1 = y1;
                y1 = temp;
                temp = x2;
                x2 = y2;
                y2 = temp;
            }

            painter->setRenderHint(QPainter::Antialiasing, true);
            painter->translate(0.5, 0.5);

            QColor tabFrameColor = tab->features & QStyleOptionTab::HasFrame ?
                        getTabFrameColor(option->palette) :
                        option->palette.window().color();

            QLinearGradient fillGradient(rect.topLeft(), rect.bottomLeft());
            QLinearGradient outlineGradient(rect.topLeft(), rect.bottomLeft());
            QPen outlinePen = outline.lighter(110);
            if (selected) {
                fillGradient.setColorAt(0, tabFrameColor.lighter(104));
                //                QColor highlight = option->palette.highlight().color();
                //                if (option->state & State_HasFocus && option->state & State_KeyboardFocusChange) {
                //                    fillGradient.setColorAt(0, highlight.lighter(130));
                //                    outlineGradient.setColorAt(0, highlight.darker(130));
                //                    fillGradient.setColorAt(0.14, highlight);
                //                    outlineGradient.setColorAt(0.14, highlight.darker(130));
                //                    fillGradient.setColorAt(0.1401, tabFrameColor);
                //                    outlineGradient.setColorAt(0.1401, highlight.darker(130));
                //                }
                fillGradient.setColorAt(1, tabFrameColor);
                outlineGradient.setColorAt(1, outline);
                outlinePen = QPen(outlineGradient, 1);
            } else {
                fillGradient.setColorAt(0, tabFrameColor.darker(108));
                fillGradient.setColorAt(0.85, tabFrameColor.darker(108));
                fillGradient.setColorAt(1, tabFrameColor.darker(116));
            }

            QRect drawRect = rect.adjusted(0, selected ? 0 : 2, 0, 3);
            painter->setPen(outlinePen);
            painter->save();
            painter->setClipRect(rect.adjusted(-1, -1, 1, selected ? -2 : -3));
            painter->setBrush(fillGradient);
            painter->drawRoundedRect(drawRect.adjusted(0, 0, -1, -1), 2.0, 2.0);
            painter->setBrush(Qt::NoBrush);
            painter->setPen(innerContrastLine());
            painter->drawRoundedRect(drawRect.adjusted(1, 1, -2, -1), 2.0, 2.0);
            painter->restore();

            if (selected) {
                painter->fillRect(rect.left() + 1, rect.bottom() - 1, rect.width() - 2, rect.bottom() - 1, tabFrameColor);
                painter->fillRect(QRect(rect.bottomRight() + QPoint(-2, -1), QSize(1, 1)), innerContrastLine());
                painter->fillRect(QRect(rect.bottomLeft() + QPoint(0, -1), QSize(1, 1)), innerContrastLine());
                painter->fillRect(QRect(rect.bottomRight() + QPoint(-1, -1), QSize(1, 1)), innerContrastLine());
            }
        }
        painter->restore();
        break;
    default:
        QProxyStyle::drawControl(element, option, painter, widget);
        break;
    }
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

QRect TiledProxyStyle::subElementRect(QStyle::SubElement subElement, const QStyleOption *option, const QWidget *widget) const
{
    QRect r;

    switch (subElement) {
    case SE_TabBarTabLeftButton:    // moving the tab buttons closer to the corners
    case SE_TabBarTabRightButton:
        if (const QStyleOptionTab *tab = qstyleoption_cast<const QStyleOptionTab *>(option)) {
            bool selected = tab->state & State_Selected;
            int verticalShift = proxy()->pixelMetric(QStyle::PM_TabBarTabShiftVertical, tab, widget);
            int horizontalShift = proxy()->pixelMetric(QStyle::PM_TabBarTabShiftHorizontal, tab, widget);
            int hpadding = proxy()->pixelMetric(QStyle::PM_TabBarTabHSpace, option, widget) / 2;
            hpadding = qMax(hpadding, 4); //workaround KStyle returning 0 because they workaround an old bug in Qt

            bool verticalTabs = tab->shape == QTabBar::RoundedEast
                    || tab->shape == QTabBar::RoundedWest
                    || tab->shape == QTabBar::TriangularEast
                    || tab->shape == QTabBar::TriangularWest;
            QRect tr = tab->rect;
            if (tab->shape == QTabBar::RoundedSouth || tab->shape == QTabBar::TriangularSouth)
                verticalShift = -verticalShift;
            if (verticalTabs) {
                qSwap(horizontalShift, verticalShift);
                horizontalShift *= -1;
                verticalShift *= -1;
            }
            if (tab->shape == QTabBar::RoundedWest || tab->shape == QTabBar::TriangularWest)
                horizontalShift = -horizontalShift;
            tr.adjust(0, 0, horizontalShift, verticalShift);
            if (selected)
            {
                tr.setBottom(tr.bottom() - verticalShift);
                tr.setRight(tr.right() - horizontalShift);
            }
            QSize size = (subElement == SE_TabBarTabLeftButton) ? tab->leftButtonSize : tab->rightButtonSize;
            int w = size.width();
            int h = size.height();
            int midHeight = static_cast<int>(qCeil(float(tr.height() - h) / 2));
            int midWidth = ((tr.width() - w) / 2);
            bool atTheTop = true;
            switch (tab->shape) {
            case QTabBar::RoundedWest:
            case QTabBar::TriangularWest:
                atTheTop = (subElement == SE_TabBarTabLeftButton);
                break;
            case QTabBar::RoundedEast:
            case QTabBar::TriangularEast:
                atTheTop = (subElement == SE_TabBarTabRightButton);
                break;
            default:
                if (subElement == SE_TabBarTabLeftButton)
                    r = QRect(tab->rect.x() + hpadding, midHeight, w, h);
                else
                    r = QRect(tab->rect.right() - w - hpadding, midHeight, w, h);
                r = visualRect(tab->direction, tab->rect, r);
            }
            if (verticalTabs) {
                if (atTheTop)
                    r = QRect(midWidth, tr.y() + tab->rect.height() - hpadding - h, w, h);
                else
                    r = QRect(midWidth, tr.y() + hpadding, w, h);
            }
        }
        break;

    default:
        r = QProxyStyle::subElementRect(subElement, option, widget);
        break;
    }

    return r;
}
