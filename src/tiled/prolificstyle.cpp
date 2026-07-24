/*
 * prolificstyle.cpp
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

#include "prolificstyle.h"

#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
#include "utils.h"
#endif

#include <QAbstractButton>
#include <QAbstractItemView>
#include <QAbstractSlider>
#include <QAbstractSpinBox>
#include <QComboBox>
#include <QPainter>
#include <QProgressBar>
#include <QScrollBar>
#include <QSplitterHandle>
#include <QStyleOption>

namespace Tiled {

// todo: de-duplicate between tiledproxystyle.cpp

#ifdef Q_OS_DARWIN
static const qreal baseDpi = 72;
#else
static const qreal baseDpi = 96;
#endif

static qreal dpi(const QStyleOption *option)
{
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
    // Expect that QStyleOption::QFontMetrics::QFont has the correct DPI set
    if (option)
        return option->fontMetrics.fontDpi();
    return baseDpi;
#else
    Q_UNUSED(option)
    return Utils::defaultDpi();
#endif
}

static qreal dpiScaled(qreal value, qreal dpi)
{
    return value * dpi / baseDpi;
}

static int dpiScaled(int value, const QStyleOption *option)
{
    return qRound(dpiScaled(qreal(value), dpi(option)));
}


ProlificStyle::ProlificStyle()
{
    setObjectName(QStringLiteral("prolific"));
}

void ProlificStyle::drawPrimitive(PrimitiveElement pe, const QStyleOption *opt, QPainter *p, const QWidget *w) const
{
    switch (pe) {
    case PE_IndicatorButtonDropDown:
    case PE_PanelButtonBevel:
    case PE_PanelButtonCommand:
    case PE_PanelButtonTool:
        p->save();
        p->setRenderHint(QPainter::Antialiasing);
        p->setPen(QPen(opt->palette.light().color(), 1.0));
        p->translate(0.5, 0.5);
        p->drawRoundedRect(opt->rect.adjusted(1, 1, -1, -1), 5.0, 5.0);
        p->restore();
        return;
    default:
        QCommonStyle::drawPrimitive(pe, opt, p, w);
    }
}

void ProlificStyle::drawControl(ControlElement element, const QStyleOption *opt, QPainter *p, const QWidget *w) const
{
    switch (element) {
    case CE_ScrollBarAddLine:
    case CE_ScrollBarAddPage:
    case CE_ScrollBarSubLine:
    case CE_ScrollBarSubPage:
        return; // this style does not support these control elements
    case CE_ScrollBarSlider:
        if (const QStyleOptionSlider *scrollBar = qstyleoption_cast<const QStyleOptionSlider *>(opt))
            scrollBarDrawSlider(scrollBar, p);
        return;
    default:
        QCommonStyle::drawControl(element, opt, p, w);
    }
}

QRect ProlificStyle::subElementRect(SubElement r, const QStyleOption *opt, const QWidget *widget) const
{
    switch (r) {
    default:
        return QCommonStyle::subElementRect(r, opt, widget);
    }
}

void ProlificStyle::drawComplexControl(ComplexControl cc, const QStyleOptionComplex *opt, QPainter *p, const QWidget *w) const
{
    switch (cc) {
    case CC_ScrollBar:
        if (const QStyleOptionSlider *scrollBar = qstyleoption_cast<const QStyleOptionSlider *>(opt)) {
            scrollBarDrawGroove(scrollBar, p);

            QStyleOptionSlider newScrollBar = *scrollBar;
            newScrollBar.subControls &= SC_ScrollBarSlider; // we only need the slider to be rendered
            QCommonStyle::drawComplexControl(cc, &newScrollBar, p, w);
        }
        return;
    default:
        QCommonStyle::drawComplexControl(cc, opt, p, w);
    }
}

QRect ProlificStyle::subControlRect(ComplexControl cc, const QStyleOptionComplex *opt, SubControl sc, const QWidget *w) const
{
    switch (cc) {
    case CC_ScrollBar:
        if (const QStyleOptionSlider *scrollBar = qstyleoption_cast<const QStyleOptionSlider *>(opt))
            return scrollBarSubControlRect(scrollBar, sc, w);
        break;
    default:
        return QCommonStyle::subControlRect(cc, opt, sc, w);
    }

    return QRect();
}

int ProlificStyle::pixelMetric(PixelMetric m, const QStyleOption *opt, const QWidget *widget) const
{
    switch (m) {
    case PM_ScrollBarExtent:
        return scrollBarWidth(opt);
    case PM_ScrollView_ScrollBarOverlap:
        return mScrollBarOverlaps ? scrollBarWidth(opt) : 0;
    case PM_ScrollBarSliderMin:
        return scrollBarSliderMin(opt);
    default:
        return QCommonStyle::pixelMetric(m, opt, widget);
    }
}

int ProlificStyle::styleHint(StyleHint sh, const QStyleOption *opt, const QWidget *w, QStyleHintReturn *shret) const
{
    switch (sh) {
    case SH_ScrollBar_MiddleClickAbsolutePosition:
    case SH_Slider_StopMouseOverSlider:
        return true;
    case SH_ItemView_ScrollMode:
        return QAbstractItemView::ScrollPerPixel;
    default:
        return QCommonStyle::styleHint(sh, opt, w, shret);
    }
}

static bool doesHoverOrNonOpaquePaint(QWidget *w)
{
    return (qobject_cast<QAbstractButton*>(w) ||
            qobject_cast<QComboBox *>(w) ||
            qobject_cast<QProgressBar *>(w) ||
            qobject_cast<QScrollBar *>(w) ||
            qobject_cast<QSplitterHandle *>(w) ||
            qobject_cast<QAbstractSlider *>(w) ||
            qobject_cast<QAbstractSpinBox *>(w) ||
            w->inherits("QDockSeparator") ||
            w->inherits("QDockWidgetSeparator"));
}

void ProlificStyle::polish(QWidget *w)
{
    QCommonStyle::polish(w);

    if (doesHoverOrNonOpaquePaint(w)) {
        w->setAttribute(Qt::WA_OpaquePaintEvent, false);
        w->setAttribute(Qt::WA_Hover, true);
    }
}

void ProlificStyle::unpolish(QWidget *w)
{
    if (doesHoverOrNonOpaquePaint(w)) {
        w->setAttribute(Qt::WA_OpaquePaintEvent, true);
        w->setAttribute(Qt::WA_Hover, false);
    }

    QCommonStyle::unpolish(w);
}

QRect ProlificStyle::scrollBarSubControlRect(const QStyleOptionSlider *opt,
                                             SubControl sc,
                                             const QWidget *w) const
{
    switch (sc) {
    default:
    case SC_ScrollBarAddLine:
    case SC_ScrollBarSubLine:
    case SC_ScrollBarFirst:
    case SC_ScrollBarLast:
        // These sub-controls are not supported by this style
        return QRect();

    case SC_ScrollBarGroove:
        // The groove covers the entire widget
        return opt->rect;

    case SC_ScrollBarSlider:
    case SC_ScrollBarAddPage:
    case SC_ScrollBarSubPage:
        break;
    }

    const bool isHorizontal = opt->orientation == Qt::Horizontal;
    const QSize scrollBarSize = isHorizontal ? opt->rect.size().transposed()
                                             : opt->rect.size();

    const int maxlen = scrollBarSize.height();
    int sliderlen = maxlen;

    // calculate slider length
    if (opt->maximum != opt->minimum) {
        uint range = opt->maximum - opt->minimum;
        sliderlen = (qint64(opt->pageStep) * maxlen) / (range + opt->pageStep);

        int slidermin = proxy()->pixelMetric(PM_ScrollBarSliderMin, opt, w);
        if (sliderlen < slidermin || range > INT_MAX / 2)
            sliderlen = slidermin;
        if (sliderlen > maxlen)
            sliderlen = maxlen;
    }

    const int sliderstart = sliderPositionFromValue(opt->minimum,
                                                    opt->maximum,
                                                    opt->sliderPosition,
                                                    maxlen - sliderlen,
                                                    opt->upsideDown);

    QRect ret;

    switch (sc) {
    case SC_ScrollBarSlider:
        ret.setRect(0, sliderstart, scrollBarSize.width(), sliderlen);
        break;
    case SC_ScrollBarAddPage:
        ret.setRect(0, sliderstart + sliderlen,
                    scrollBarSize.width(), maxlen - sliderstart - sliderlen);
        break;
    case SC_ScrollBarSubPage:
        ret.setRect(0, 0, scrollBarSize.width(), sliderstart);
        break;
    default:
        break;
    }

    if (isHorizontal)
        ret.setRect(ret.y(), ret.x(), ret.height(), ret.width());

    return visualRect(opt->direction, opt->rect, ret);
}

void ProlificStyle::scrollBarDrawGroove(const QStyleOptionSlider *opt, QPainter *p) const
{
    // Draw the background only on mouse hover or while the scroll bar is used
    if (mScrollBarOverlaps && !opt->activeSubControls)
        return;

    const bool isHorizontal = opt->orientation == Qt::Horizontal;
    QRect rect = opt->rect;
    const int margin = (isHorizontal ? rect.height()
                                     : rect.width()) / 8;
    rect.adjust(margin, margin, -margin, -margin);
    const qreal radius = std::min(rect.width(), rect.height()) * 0.5;

    QColor color = opt->palette.dark().color();
    if (mScrollBarOverlaps)
        color.setAlpha(128);

    p->save();

    p->setPen(Qt::NoPen);
    p->setBrush(color);
    p->setRenderHint(QPainter::Antialiasing);

    p->drawRoundedRect(rect, radius, radius);

    p->restore();
}

void ProlificStyle::scrollBarDrawSlider(const QStyleOptionSlider *opt, QPainter *p) const
{
    const bool isHorizontal = opt->orientation == Qt::Horizontal;
    QRect rect = opt->rect;
    if (mScrollBarOverlaps && !opt->activeSubControls) {
        if (isHorizontal)
            rect.adjust(0, rect.height() / 2, 0, 0);
        else
            rect.adjust(rect.width() / 2, 0, 0, 0);
    }

    const int margin = (isHorizontal ? rect.height()
                                     : rect.width()) / 4;
    rect.adjust(margin, margin, -margin, -margin);
    const qreal radius = std::min(rect.width(), rect.height()) * 0.5;

    // Use text color for slider, since it contrasts well with the base color
    const bool isPressed = opt->state & State_Sunken;
    QColor foreground = isPressed ? opt->palette.highlight().color()
                                  : opt->palette.text().color();
    if (!isPressed && (mScrollBarOverlaps || !opt->activeSubControls))
        foreground.setAlpha(192);

    p->save();

    p->setPen(Qt::NoPen);
    p->setBrush(foreground);
    p->setRenderHint(QPainter::Antialiasing);

    p->drawRoundedRect(rect, radius, radius);

    p->restore();
}

int ProlificStyle::scrollBarWidth(const QStyleOption *opt) const
{
    return dpiScaled(8, opt);
}

int ProlificStyle::scrollBarSliderMin(const QStyleOption *opt) const
{
    return dpiScaled(12, opt);
}

} // namespace Tiled
