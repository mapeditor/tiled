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

#include "utils.h"

#include <QAbstractScrollArea>
#include <QApplication>
#include <QMainWindow>
#include <QPainter>
#include <QPixmapCache>
#include <QScrollBar>
#include <QStringBuilder>
#include <QStyleOptionComplex>
#include <QtMath>

using namespace Tiled;

/*
 * Below there are a lot of helper functions which are copied from various
 * private Qt parts that are used by the Fusion style, on which the Tiled style
 * is based.
 *
 * These parts are Copyright (C) 2015 The Qt Company Ltd.
 * Used under the terms of the GNU Lesser General Public License version 2.1
 */

// internal helper. Converts an integer value to an unique string token
template <typename T>
        struct HexString
{
    inline explicit HexString(const T t)
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

static QString uniqueName(const QString &key, const QStyleOption *option, QSize size)
{
    const QStyleOptionComplex *complexOption = qstyleoption_cast<const QStyleOptionComplex *>(option);
    QString tmp = key % HexString<uint>(option->state)
                      % HexString<uint>(option->direction)
                      % HexString<uint>(complexOption ? uint(complexOption->activeSubControls) : 0u)
                      % HexString<quint64>(option->palette.cacheKey())
                      % HexString<uint>(size.width())
                      % HexString<uint>(size.height());

    if (const QStyleOptionSpinBox *spinBox = qstyleoption_cast<const QStyleOptionSpinBox *>(option)) {
        tmp = tmp % HexString<uint>(spinBox->buttonSymbols)
                  % HexString<uint>(spinBox->stepEnabled)
                  % QLatin1Char(spinBox->frame ? '1' : '0'); ;
    }

    return tmp;
}

static QColor mergedColors(const QColor &colorA, const QColor &colorB, int factor = 50)
{
    const int maxFactor = 100;
    QColor tmp = colorA;
    tmp.setRed((tmp.red() * factor) / maxFactor + (colorB.red() * (maxFactor - factor)) / maxFactor);
    tmp.setGreen((tmp.green() * factor) / maxFactor + (colorB.green() * (maxFactor - factor)) / maxFactor);
    tmp.setBlue((tmp.blue() * factor) / maxFactor + (colorB.blue() * (maxFactor - factor)) / maxFactor);
    return tmp;
}

static inline QPixmap styleCachePixmap(const QSize &size)
{
    const qreal pixelRatio = qApp->devicePixelRatio();
    QPixmap cachePixmap = QPixmap(size * pixelRatio);
    cachePixmap.setDevicePixelRatio(pixelRatio);
    return cachePixmap;
}

static void qt_fusion_draw_arrow(Qt::ArrowType type, QPainter *painter, const QStyleOption *option, const QRect &rect, const QColor &color)
{
    const int arrowWidth = Utils::dpiScaled(14);
    const int arrowHeight = Utils::dpiScaled(8);

    const int arrowMax = qMin(arrowHeight, arrowWidth);
    const int rectMax = qMin(rect.height(), rect.width());
    const int size = qMin(arrowMax, rectMax);

    QPixmap cachePixmap;
    QString cacheKey = uniqueName(QLatin1String("fusion-arrow"), option, rect.size())
            % HexString<uint>(type)
            % HexString<uint>(color.rgba());
    if (!QPixmapCache::find(cacheKey, &cachePixmap)) {
        cachePixmap = styleCachePixmap(rect.size());
        cachePixmap.fill(Qt::transparent);
        QPainter cachePainter(&cachePixmap);

        QRectF arrowRect;
        arrowRect.setWidth(size);
        arrowRect.setHeight(arrowHeight * size / arrowWidth);
        if (type == Qt::LeftArrow || type == Qt::RightArrow)
            arrowRect = QRectF(arrowRect.topLeft(), arrowRect.size().transposed());
        arrowRect.moveTo((rect.width() - arrowRect.width()) / 2.0,
                         (rect.height() - arrowRect.height()) / 2.0);

        QPolygonF triangle;
        triangle.reserve(3);
        switch (type) {
        case Qt::DownArrow:
            triangle << arrowRect.topLeft() << arrowRect.topRight() << QPointF(arrowRect.center().x(), arrowRect.bottom());
            break;
        case Qt::RightArrow:
            triangle << arrowRect.topLeft() << arrowRect.bottomLeft() << QPointF(arrowRect.right(), arrowRect.center().y());
            break;
        case Qt::LeftArrow:
            triangle << arrowRect.topRight() << arrowRect.bottomRight() << QPointF(arrowRect.left(), arrowRect.center().y());
            break;
        default:
            triangle << arrowRect.bottomLeft() << arrowRect.bottomRight() << QPointF(arrowRect.center().x(), arrowRect.top());
            break;
        }

        cachePainter.setPen(Qt::NoPen);
        cachePainter.setBrush(color);
        cachePainter.setRenderHint(QPainter::Antialiasing);
        cachePainter.drawPolygon(triangle);

        QPixmapCache::insert(cacheKey, cachePixmap);
    }

    painter->drawPixmap(rect, cachePixmap);
}

enum Direction {
    TopDown,
    FromLeft,
    BottomUp,
    FromRight
};

// The default button and handle gradient
static QLinearGradient qt_fusion_gradient(const QRect &rect, const QBrush &baseColor, Direction direction = TopDown)
{
    int x = rect.center().x();
    int y = rect.center().y();
    QLinearGradient gradient;
    switch (direction) {
    case FromLeft:
        gradient = QLinearGradient(rect.left(), y, rect.right(), y);
        break;
    case FromRight:
        gradient = QLinearGradient(rect.right(), y, rect.left(), y);
        break;
    case BottomUp:
        gradient = QLinearGradient(x, rect.bottom(), x, rect.top());
        break;
    case TopDown:
    default:
        gradient = QLinearGradient(x, rect.top(), x, rect.bottom());
        break;
    }
    if (baseColor.gradient())
        gradient.setStops(baseColor.gradient()->stops());
    else {
        QColor gradientStartColor = baseColor.color().lighter(124);
        QColor gradientStopColor = baseColor.color().lighter(102);
        gradient.setColorAt(0, gradientStartColor);
        gradient.setColorAt(1, gradientStopColor);
    }
    return gradient;
}

static QColor getOutlineColor(const QPalette &pal)
{
    return pal.window().color().darker(140);
}

static QColor getLightOutlineColor(const QPalette &pal)
{
    return pal.window().color().lighter(140);
}

static QColor getHighlightedOutline(const QPalette &pal)
{
    QColor highlightedOutline = pal.highlight().color().darker(125);
    if (highlightedOutline.value() > 160)
        highlightedOutline.setHsl(highlightedOutline.hue(), highlightedOutline.saturation(), 160);
    return highlightedOutline;
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

static QColor lightShade()
{
    return QColor(255, 255, 255, 90);
}

static QColor darkShade()
{
    return QColor(0, 0, 0, 60);
}

static QColor getTabFrameColor(const QPalette &pal)
{
    return getButtonColor(pal).lighter(104);
}

static qreal roundedRectRadius()
{
    static qreal radius = Utils::dpiScaled(2.0);
    return radius;
}


TiledProxyStyle::TiledProxyStyle(const QPalette &palette, QStyle *style)
    : QProxyStyle(style)
    , mPalette(palette)
    , mIsDark(palette.window().color().value() <= 128)
    , mDockClose(QLatin1String("://images/dock-close.png"))
    , mDockRestore(QLatin1String("://images/dock-restore.png"))
{
    setObjectName(QLatin1String("tiled"));
}

void TiledProxyStyle::setPalette(const QPalette &palette)
{
    mPalette = palette;
    mIsDark = palette.window().color().value() <= 128;
}

void TiledProxyStyle::drawPrimitive(PrimitiveElement element,
                                    const QStyleOption *option,
                                    QPainter *painter,
                                    const QWidget *widget) const
{
    switch (element) {
    case PE_FrameGroupBox:
    {
        int topMargin = qMax(pixelMetric(PM_ExclusiveIndicatorHeight), option->fontMetrics.height()) + 3;
        QRect frame = option->rect.adjusted(0, topMargin, -1, -1);
        QColor tabFrameColor = getTabFrameColor(option->palette);

        painter->save();
        painter->setRenderHint(QPainter::Antialiasing);
        painter->translate(0.5, 0.5);
        painter->setPen(mergedColors(getOutlineColor(option->palette), tabFrameColor));
        painter->setBrush(mergedColors(option->palette.window().color(), tabFrameColor));
        painter->drawRoundedRect(frame, roundedRectRadius(), roundedRectRadius());
        painter->restore();
        break;
    }
    case PE_FrameTabBarBase:
        if (const QStyleOptionTabBarBase *tbb
                = qstyleoption_cast<const QStyleOptionTabBarBase *>(option)) {
            painter->save();
            painter->setPen(QPen(getOutlineColor(option->palette)));

            QStyleOptionTab tabOverlap;
            tabOverlap.shape = tbb->shape;
            int overlap = proxy()->pixelMetric(QStyle::PM_TabBarBaseOverlap, &tabOverlap, widget);
            QColor tabFrameColor = option->palette.button().color().darker(mIsDark ? 128 : 116);
            QLinearGradient fillGradient;
            fillGradient.setColorAt(0, tabFrameColor.darker(108));
            fillGradient.setColorAt(0.2, tabFrameColor);
            fillGradient.setColorAt(1, tabFrameColor);

            switch (tbb->shape) {
            case QTabBar::RoundedNorth: {
                QRect backgroundRect(tbb->rect.left(), tbb->tabBarRect.top(),
                                     tbb->rect.width(), tbb->tabBarRect.height() - overlap);

                fillGradient.setStart(backgroundRect.topLeft());
                fillGradient.setFinalStop(backgroundRect.bottomLeft() + QPoint(0, overlap));

                painter->fillRect(backgroundRect, fillGradient);
                painter->drawLine(tbb->rect.topLeft(), tbb->rect.topRight());
            }
                break;
            case QTabBar::RoundedWest:
                painter->drawLine(tbb->rect.left(), tbb->rect.top(),
                                  tbb->rect.left(), tbb->rect.bottom());
                break;
            case QTabBar::RoundedSouth: {
                QRect backgroundRect(tbb->rect.left(), tbb->tabBarRect.top() + overlap,
                                     tbb->rect.width(), tbb->tabBarRect.height() - overlap - 1);

                fillGradient.setStart(backgroundRect.topLeft());
                fillGradient.setFinalStop(backgroundRect.bottomLeft() + QPoint(0, overlap));

                painter->fillRect(backgroundRect, fillGradient);
                painter->drawLine(tbb->rect.left(), tbb->rect.bottom(),
                                  tbb->rect.right(), tbb->rect.bottom());
                break;
            }
            case QTabBar::RoundedEast:
                painter->drawLine(tbb->rect.topRight(), tbb->rect.bottomRight());
                break;
            case QTabBar::TriangularNorth:
            case QTabBar::TriangularEast:
            case QTabBar::TriangularWest:
            case QTabBar::TriangularSouth:
                painter->restore();
                QCommonStyle::drawPrimitive(element, option, painter, widget);
                return;
            }
            painter->restore();
        }
        return;
    case PE_IndicatorCheckBox:
        painter->save();
        if (const QStyleOptionButton *checkbox = qstyleoption_cast<const QStyleOptionButton*>(option)) {
            painter->setRenderHint(QPainter::Antialiasing, true);
            painter->translate(0.5, 0.5);
            QRect rect = option->rect.adjusted(0, 0, -1, -1);
            int state = option->state;

            QColor pressedColor = mergedColors(option->palette.base().color(),
                                               option->palette.windowText().color(), 85);
            painter->setBrush(Qt::NoBrush);

            // Gradient fill
            QLinearGradient gradient(rect.topLeft(), rect.bottomLeft());
            gradient.setColorAt(0, (state & State_Sunken) ? pressedColor : option->palette.base().color().darker(115));
            gradient.setColorAt(0.15, (state & State_Sunken) ? pressedColor : option->palette.base().color());
            gradient.setColorAt(1, (state & State_Sunken) ? pressedColor : option->palette.base().color());

            painter->setBrush((state & State_Sunken) ? QBrush(pressedColor) : gradient);

            QColor boxOutline(mIsDark ? getLightOutlineColor(option->palette) :
                                        getOutlineColor(option->palette));
            boxOutline.setAlpha(200);
            painter->setPen(boxOutline);

            if (option->state & State_HasFocus && option->state & State_KeyboardFocusChange)
                painter->setPen(QPen(getHighlightedOutline(option->palette)));
            painter->drawRect(rect);

            QColor checkMarkColor = option->palette.text().color().darker(120);
            const int checkMarkPadding = Utils::dpiScaled(3);

            if (checkbox->state & State_NoChange) {
                gradient = QLinearGradient(rect.topLeft(), rect.bottomLeft());
                checkMarkColor.setAlpha(80);
                gradient.setColorAt(0, checkMarkColor);
                checkMarkColor.setAlpha(140);
                gradient.setColorAt(1, checkMarkColor);
                checkMarkColor.setAlpha(180);
                painter->setPen(QPen(checkMarkColor, 1));
                painter->setBrush(gradient);
                painter->drawRect(rect.adjusted(checkMarkPadding, checkMarkPadding, -checkMarkPadding, -checkMarkPadding));

            } else if (checkbox->state & (State_On)) {
                QPen checkPen = QPen(checkMarkColor, Utils::dpiScaled(1.8));
                checkMarkColor.setAlpha(210);
                painter->translate(-1, 0.5);
                painter->setPen(checkPen);
                painter->setBrush(Qt::NoBrush);
                painter->translate(0.2, 0.0);

                // Draw checkmark
                QPainterPath path;
                path.moveTo(2 + checkMarkPadding, rect.height() / 2.0);
                path.lineTo(rect.width() / 2.0, rect.height() - checkMarkPadding);
                path.lineTo(rect.width() - checkMarkPadding, checkMarkPadding);
                painter->drawPath(path.translated(rect.topLeft()));
            }
        }
        painter->restore();
        break;
    case PE_IndicatorTabClose:
    {
        bool hovered = (option->state & State_Enabled) && (option->state & State_MouseOver);
        if (hovered)
            proxy()->drawPrimitive(PE_PanelButtonCommand, option, painter, widget);

        QColor textColor = option->palette.text().color();
        qreal penWidth = Utils::dpiScaled(1.25);
        QPen foregroundPen(textColor, penWidth, Qt::SolidLine, Qt::RoundCap);
        QPen shadowPen(QColor(0, 0, 0, 200), penWidth, Qt::SolidLine, Qt::RoundCap);

        if (!mIsDark) {
            if (!hovered && !(option->state & State_Selected))
                shadowPen.setColor(QColor(255, 255, 255, 200));
            else
                shadowPen.setColor(QColor(255, 255, 255, 255));
        } else {
            if (!hovered && !(option->state & State_Selected)) {
                textColor.setAlpha(192);
                foregroundPen.setColor(textColor);
            }
        }

        QRect iconRect = Utils::dpiScaled(QRectF(0, 0, 8, 8)).toRect();
        iconRect.moveCenter(option->rect.center());

        const QPoint lines[] = {
            iconRect.topLeft(), iconRect.bottomRight(),
            iconRect.topRight(), iconRect.bottomLeft(),
        };

        painter->save();

        painter->setRenderHint(QPainter::Antialiasing, true);
        painter->translate(0.5, 1.5);
        painter->setPen(shadowPen);
        painter->drawLines(lines, 2);
        painter->translate(0.0, -1.0);
        painter->setPen(foregroundPen);
        painter->drawLines(lines, 2);

        painter->restore();
        break;
    }
    case PE_PanelButtonCommand:
    {
        painter->save();

        bool isDefault = false;
        bool isFlat = false;
        bool isDown = (option->state & State_Sunken) || (option->state & State_On);
        QRect r;

        if (const QStyleOptionButton *button = qstyleoption_cast<const QStyleOptionButton*>(option)) {
            isDefault = (button->features & QStyleOptionButton::DefaultButton) && (button->state & State_Enabled);
            isFlat = (button->features & QStyleOptionButton::Flat);
        }

        if (isFlat && !isDown) {
            if (isDefault) {
                r = option->rect.adjusted(0, 1, 0, -1);
                painter->setPen(QPen(Qt::black));
                const QLine lines[4] = {
                    QLine(QPoint(r.left() + 2, r.top()),
                    QPoint(r.right() - 2, r.top())),
                    QLine(QPoint(r.left(), r.top() + 2),
                    QPoint(r.left(), r.bottom() - 2)),
                    QLine(QPoint(r.right(), r.top() + 2),
                    QPoint(r.right(), r.bottom() - 2)),
                    QLine(QPoint(r.left() + 2, r.bottom()),
                    QPoint(r.right() - 2, r.bottom()))
                };
                painter->drawLines(lines, 4);
                const QPoint points[4] = {
                    QPoint(r.right() - 1, r.bottom() - 1),
                    QPoint(r.right() - 1, r.top() + 1),
                    QPoint(r.left() + 1, r.bottom() - 1),
                    QPoint(r.left() + 1, r.top() + 1)
                };
                painter->drawPoints(points, 4);
            }

            painter->restore();
            return;
        }

        r = option->rect.adjusted(0, 1, -1, 0);

        bool isEnabled = option->state & State_Enabled;
        bool hasFocus = (option->state & State_HasFocus && option->state & State_KeyboardFocusChange);
        QColor buttonColor = getButtonColor(option->palette);

        QColor darkOutline = getOutlineColor(option->palette);
        if (hasFocus | isDefault) {
            darkOutline = getHighlightedOutline(option->palette);
        }

        if (isDefault)
            buttonColor = mergedColors(buttonColor, getHighlightedOutline(option->palette).lighter(130), 90);

        painter->setRenderHint(QPainter::Antialiasing, true);
        painter->translate(0.5, -0.5);

        QColor downColor = buttonColor.darker(110);

        if (option->state & State_On)
            downColor = option->palette.button().color().darker(mIsDark ? 128 : 116);

        QLinearGradient gradient = qt_fusion_gradient(option->rect, (isEnabled && option->state & State_MouseOver ) ? buttonColor : buttonColor.darker(104));
        painter->setPen(Qt::transparent);
        painter->setBrush(isDown ? QBrush(downColor) : gradient);
        painter->drawRoundedRect(r, roundedRectRadius(), roundedRectRadius());
        painter->setBrush(Qt::NoBrush);

        // Outline
        painter->setPen(!isEnabled ? QPen(darkOutline.lighter(115)) : QPen(darkOutline));
        painter->drawRoundedRect(r, roundedRectRadius(), roundedRectRadius());

        painter->setPen(innerContrastLine());
        painter->drawRoundedRect(r.adjusted(1, 1, -1, -1), roundedRectRadius(), roundedRectRadius());

        painter->restore();
        break;
    }
    default:
        QProxyStyle::drawPrimitive(element, option, painter, widget);
        break;
    }
}

void TiledProxyStyle::drawControl(ControlElement element,
                                  const QStyleOption *option,
                                  QPainter *painter,
                                  const QWidget *widget) const
{
    QRect rect = option->rect;
    QColor outline = getOutlineColor(option->palette);
    QColor shadow = darkShade();

    switch (element) {
    case CE_Splitter:               // Copied to adjust to DPI
    {
        // Don't draw handle for single pixel splitters
        if (option->rect.width() > 1 && option->rect.height() > 1) {
            //draw grips
            int size = qRound(Utils::dpiScaled(3));
            int offset = -size / 2 + 1;

            if (option->state & State_Horizontal) {
                for (int j = -size * 2 ; j < size * 4; j += size) {
                    painter->fillRect(rect.center().x() + offset, rect.center().y() + offset + j, size - 1, size - 1, lightShade());
                    painter->fillRect(rect.center().x() + offset, rect.center().y() + offset + j, size - 2, size - 2, shadow);
                }
            } else {
                for (int i = -size * 2; i < size * 4; i += size) {
                    painter->fillRect(rect.center().x() + offset + i, rect.center().y() + offset, size - 1, size - 1, lightShade());
                    painter->fillRect(rect.center().x() + offset + i, rect.center().y() + offset, size - 2, size - 2, shadow);
                }
            }
        }
        break;
    }
    case CE_MenuBarEmptyArea:       // Copied to change bottom line color
        painter->save();
    {
        painter->fillRect(rect, option->palette.window());
        painter->setPen(option->palette.mid().color());
        painter->drawLine(option->rect.bottomLeft(), option->rect.bottomRight());
    }
        painter->restore();
        break;

    case CE_ToolBar:                // A lot of code copied from Fusion style, just to tweak the colors
        if (const QStyleOptionToolBar *toolBar = qstyleoption_cast<const QStyleOptionToolBar *>(option)) {
            // Reserve the beveled appearance only for mainwindow toolbars
            if (widget && !(qobject_cast<const QMainWindow*> (widget->parentWidget())))
                break;

            // Draws the light line above and the dark line below menu bars and
            // tool bars.
            QLinearGradient gradient(option->rect.topLeft(), option->rect.bottomLeft());
            if (!(option->state & State_Horizontal))
                gradient = QLinearGradient(rect.left(), rect.center().y(),
                                           rect.right(), rect.center().y());
            gradient.setColorAt(0, option->palette.window().color());
            gradient.setColorAt(1, option->palette.window().color().darker(104));
            painter->fillRect(option->rect, gradient);

            QColor light = option->palette.midlight().color();
            QColor shadow = option->palette.mid().color();

            QPen oldPen = painter->pen();
            if (toolBar->toolBarArea == Qt::TopToolBarArea) {
                if (toolBar->positionOfLine == QStyleOptionToolBar::End
                        || toolBar->positionOfLine == QStyleOptionToolBar::OnlyOne) {
                    // The end and onlyone top toolbar lines draw a double
                    // line at the bottom to blend with the central
                    // widget.
                    painter->setPen(light);
                    painter->drawLine(option->rect.bottomLeft(), option->rect.bottomRight());
                    painter->setPen(shadow);
                    painter->drawLine(option->rect.left(), option->rect.bottom() - 1,
                                      option->rect.right(), option->rect.bottom() - 1);
                } else {
                    // All others draw a single dark line at the bottom.
                    painter->setPen(shadow);
                    painter->drawLine(option->rect.bottomLeft(), option->rect.bottomRight());
                }
            } else if (toolBar->toolBarArea == Qt::BottomToolBarArea) {
                if (toolBar->positionOfLine == QStyleOptionToolBar::End
                        || toolBar->positionOfLine == QStyleOptionToolBar::Middle) {
                    // The end and middle bottom tool bar lines draw a dark
                    // line at the bottom.
                    painter->setPen(shadow);
                    painter->drawLine(option->rect.bottomLeft(), option->rect.bottomRight());
                }
                if (toolBar->positionOfLine == QStyleOptionToolBar::Beginning
                        || toolBar->positionOfLine == QStyleOptionToolBar::OnlyOne) {
                    // The beginning and only one tool bar lines draw a
                    // double line at the bottom to blend with the
                    // status bar.
                    // ### The styleoption could contain whether the
                    // main window has a menu bar and a status bar, and
                    // possibly dock widgets.
                    painter->setPen(shadow);
                    painter->drawLine(option->rect.left(), option->rect.bottom() - 1,
                                      option->rect.right(), option->rect.bottom() - 1);
                    painter->setPen(light);
                    painter->drawLine(option->rect.bottomLeft(), option->rect.bottomRight());
                }
                if (toolBar->positionOfLine == QStyleOptionToolBar::End) {
                    painter->setPen(shadow);
                    painter->drawLine(option->rect.topLeft(), option->rect.topRight());
                    painter->setPen(light);
                    painter->drawLine(option->rect.left(), option->rect.top() + 1,
                                      option->rect.right(), option->rect.top() + 1);

                } else {
                    // All other bottom toolbars draw a light line at the top.
                    painter->setPen(light);
                    painter->drawLine(option->rect.topLeft(), option->rect.topRight());
                }
            }
            if (toolBar->toolBarArea == Qt::LeftToolBarArea) {
                if (toolBar->positionOfLine == QStyleOptionToolBar::Middle
                        || toolBar->positionOfLine == QStyleOptionToolBar::End) {
                    // The middle and left end toolbar lines draw a light
                    // line to the left.
                    painter->setPen(light);
                    painter->drawLine(option->rect.topLeft(), option->rect.bottomLeft());
                }
                if (toolBar->positionOfLine == QStyleOptionToolBar::End) {
                    // All other left toolbar lines draw a dark line to the right
                    painter->setPen(shadow);
                    painter->drawLine(option->rect.right() - 1, option->rect.top(),
                                      option->rect.right() - 1, option->rect.bottom());
                    painter->setPen(light);
                    painter->drawLine(option->rect.topRight(), option->rect.bottomRight());
                } else {
                    // All other left toolbar lines draw a dark line to the right
                    painter->setPen(shadow);
                    painter->drawLine(option->rect.topRight(), option->rect.bottomRight());
                }
            } else if (toolBar->toolBarArea == Qt::RightToolBarArea) {
                if (toolBar->positionOfLine == QStyleOptionToolBar::Middle
                        || toolBar->positionOfLine == QStyleOptionToolBar::End) {
                    // Right middle and end toolbar lines draw the dark right line
                    painter->setPen(shadow);
                    painter->drawLine(option->rect.topRight(), option->rect.bottomRight());
                }
                if (toolBar->positionOfLine == QStyleOptionToolBar::End
                        || toolBar->positionOfLine == QStyleOptionToolBar::OnlyOne) {
                    // The right end and single toolbar draws the dark
                    // line on its left edge
                    painter->setPen(shadow);
                    painter->drawLine(option->rect.topLeft(), option->rect.bottomLeft());
                    // And a light line next to it
                    painter->setPen(light);
                    painter->drawLine(option->rect.left() + 1, option->rect.top(),
                                      option->rect.left() + 1, option->rect.bottom());
                } else {
                    // Other right toolbars draw a light line on its left edge
                    painter->setPen(light);
                    painter->drawLine(option->rect.topLeft(), option->rect.bottomLeft());
                }
            }
            painter->setPen(oldPen);
        }
        break;

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

            QColor tabFrameColor = (tab->features & QStyleOptionTab::HasFrame) ?
                        getTabFrameColor(option->palette) :
                        option->palette.window().color();

            if (!selected) {
                int f = mIsDark ? 128 : 116;
                tabFrameColor = option->palette.button().color().darker(f);
            }

            QLinearGradient fillGradient(rect.topLeft(), rect.bottomLeft());
            QPen outlinePen = outline;
            if (selected) {
                fillGradient.setColorAt(0, tabFrameColor.lighter(104));

                // colorful selected tab
//                QLinearGradient outlineGradient(rect.topLeft(), rect.bottomLeft());
//                QColor highlight = option->palette.highlight().color();
//                fillGradient.setColorAt(0, highlight.lighter(130));
//                outlineGradient.setColorAt(0, highlight.darker(130));
//                fillGradient.setColorAt(0.10, highlight);
//                outlineGradient.setColorAt(0.10, highlight.darker(130));
//                fillGradient.setColorAt(0.1001, tabFrameColor);
//                outlineGradient.setColorAt(0.1001, highlight.darker(130));
//                outlineGradient.setColorAt(1, outline);
//                outlinePen = QPen(outlineGradient, 1);

                fillGradient.setColorAt(1, tabFrameColor);
            } else {
                fillGradient.setColorAt(0, tabFrameColor);
                fillGradient.setColorAt(0.85, tabFrameColor);
                fillGradient.setColorAt(1, tabFrameColor.darker(116));
            }

            QRect drawRect = rect.adjusted(0, 0, 0, 3);
            painter->setPen(outlinePen);
            painter->save();
            painter->setClipRect(rect.adjusted(-1, -1, 1, selected ? -2 : -3));
            painter->setBrush(fillGradient);
            painter->drawRect(drawRect.adjusted(0, 0, -1, -1));
            painter->setBrush(Qt::NoBrush);
            painter->setPen(innerContrastLine());
            painter->drawRect(drawRect.adjusted(1, 1, -2, -1));
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
    case CE_TabBarTabLabel:
        if (const QStyleOptionTab *tab = qstyleoption_cast<const QStyleOptionTab *>(option)) {
            // A small hack to change the color for non-selected tabs when using a dark theme.
            // This is done in order to reduce the contrast of the text on the darker tabs.
            if (mIsDark && !(tab->state & State_Selected)) {
                QStyleOptionTab unselectedTab{*tab};
                QColor textColor = unselectedTab.palette.color(QPalette::WindowText);
                textColor.setAlpha(192);
                unselectedTab.palette.setColor(QPalette::WindowText, textColor);
                QProxyStyle::drawControl(element, &unselectedTab, painter, widget);
            } else {
                QProxyStyle::drawControl(element, option, painter, widget);
            }
        }
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
    case CC_ScrollBar:              // replaced for higher contrast and thinner slider
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

            QColor arrowColor = option->palette.windowText().color();
            arrowColor.setAlpha(220);

            const QColor bgColor = mPalette.color(QPalette::Base);
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

                int margin = qRound(Utils::dpiScaled(2));

                QRect sliderRect = scrollBarSlider.adjusted(margin + 1, margin, -margin - 1, -margin - 1);
                if (horizontal)
                    sliderRect = scrollBarSlider.adjusted(margin, margin + 1, -margin - 1, -margin - 1);
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
                painter->drawRoundedRect(sliderRect, roundedRectRadius(), roundedRectRadius());
                painter->setPen(innerContrastLine());
                painter->drawRoundedRect(sliderRect.adjusted(1, 1, -1, -1), roundedRectRadius(), roundedRectRadius());
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

                QRect upRect = scrollBarSubLine.adjusted(horizontal ? 0 : 1, horizontal ? 1 : 0, horizontal ? -2 : -1, horizontal ? -1 : -2);
                painter->setBrush(Qt::NoBrush);
                painter->setPen(innerContrastLine());
                painter->drawRect(upRect);

                // Arrows
                Qt::ArrowType arrowType = Qt::UpArrow;
                if (horizontal)
                    arrowType = option->direction == Qt::LeftToRight ? Qt::LeftArrow : Qt::RightArrow;
                qt_fusion_draw_arrow(arrowType, painter, option, upRect, arrowColor);
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

                QRect downRect = scrollBarAddLine.adjusted(1, 1, -1, -1);
                painter->setPen(innerContrastLine());
                painter->setBrush(Qt::NoBrush);
                painter->drawRect(downRect);

                Qt::ArrowType arrowType = Qt::DownArrow;
                if (horizontal)
                    arrowType = option->direction == Qt::LeftToRight ? Qt::RightArrow : Qt::LeftArrow;
                qt_fusion_draw_arrow(arrowType, painter, option, downRect, arrowColor);
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
    case PM_TabBarTabShiftHorizontal:
    case PM_TabBarTabShiftVertical:
        return 0;                   // no shifting of tabs
    case PM_TabBarTabOverlap:
        return 1;                   // should not get DPI scaled
    case PM_TabBarBaseOverlap:
        return 2;                   // should not get DPI scaled
    default:
        return QProxyStyle::pixelMetric(metric, option, widget);
    }
}

inline static bool verticalTabs(QTabBar::Shape shape)
{
    return shape == QTabBar::RoundedWest
           || shape == QTabBar::RoundedEast
           || shape == QTabBar::TriangularWest
           || shape == QTabBar::TriangularEast;
}

QSize TiledProxyStyle::sizeFromContents(ContentsType type,
                                        const QStyleOption *option,
                                        const QSize &contentsSize,
                                        const QWidget *widget) const
{
    QSize size(contentsSize);

    switch (type) {
    case CT_MenuBarItem:            // make the menu bar item itself wider
        if (!size.isEmpty())
            size += QSize(Utils::dpiScaled(16.),
                          Utils::dpiScaled(5.));
        break;
    case CT_ItemViewItem:           // give item view items a little more space
        size = QCommonStyle::sizeFromContents(type, option, contentsSize, widget);
        size += QSize(0, Utils::dpiScaled(2.));
        break;
    case CT_TabBarTab:
        if (const QStyleOptionTab *tab = qstyleoption_cast<const QStyleOptionTab *>(option)) {
            // This code is mostly taken from QTabBar::sizeHint, adjusted only
            // to not add the PM_TabBarTabVSpace to the height of the buttons
            // when calculating the tab height.
            QSize iconSize = tab->icon.isNull() ? QSize(0, 0) : tab->iconSize;
            int hframe = proxy()->pixelMetric(PM_TabBarTabHSpace, tab, widget);
            int vframe = proxy()->pixelMetric(PM_TabBarTabVSpace, tab, widget);
            const QFontMetrics &fm = option->fontMetrics;
            int maxWidgetHeight = qMax(tab->leftButtonSize.height(), tab->rightButtonSize.height());
            int maxWidgetWidth = qMax(tab->leftButtonSize.width(), tab->rightButtonSize.width());
            int widgetWidth = 0;
            int widgetHeight = 0;
            int padding = 0;
            if (!tab->leftButtonSize.isEmpty()) {
                padding += Utils::dpiScaled(4);
                widgetWidth += tab->leftButtonSize.width();
                widgetHeight += tab->leftButtonSize.height();
            }
            if (!tab->rightButtonSize.isEmpty()) {
                padding += Utils::dpiScaled(4);
                widgetWidth += tab->rightButtonSize.width();
                widgetHeight += tab->rightButtonSize.height();
            }
            if (!tab->icon.isNull())
                padding += Utils::dpiScaled(4);
            if (verticalTabs(tab->shape)) {
                size = QSize(qMax(maxWidgetWidth, qMax(fm.height(), iconSize.height()) + vframe),
                        fm.size(Qt::TextShowMnemonic, tab->text).width() + iconSize.width() + hframe + widgetHeight + padding);
            } else {
                size = QSize(fm.size(Qt::TextShowMnemonic, tab->text).width() + iconSize.width() + hframe
                      + widgetWidth + padding,
                      qMax(maxWidgetHeight, qMax(fm.height(), iconSize.height()) + vframe));
            }
        }
        break;
    default:
        size = QProxyStyle::sizeFromContents(type, option, contentsSize, widget);
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
            int hpadding = Utils::dpiScaled(4.);       // normally half the PM_TabBarTabHSpace

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

int TiledProxyStyle::styleHint(StyleHint styleHint,
                               const QStyleOption *option,
                               const QWidget *widget,
                               QStyleHintReturn *returnData) const
{
    switch (styleHint) {
    case SH_EtchDisabledText:
        return !mIsDark;    // etch only when bright
    default:
        return QProxyStyle::styleHint(styleHint, option, widget, returnData);
    }
}

QIcon TiledProxyStyle::standardIcon(QStyle::StandardPixmap standardIcon,
                                    const QStyleOption *option,
                                    const QWidget *widget) const
{
    switch (standardIcon) {
    // TODO: Look into overriding drawComplexControl(QStyle::CC_ToolButton) and
    // checking for "qt_dockwidget_floatbutton" and "qt_dockwidget_closebutton"
    // object names to draw better integrated buttons.
    case SP_DockWidgetCloseButton:
    case SP_TitleBarCloseButton:
        return mDockClose;
    case SP_TitleBarNormalButton:
        return mDockRestore;
    default:
        return QProxyStyle::standardIcon(standardIcon, option, widget);
    }
}
