/*
 * popupwidget.cpp
 * Copyright 2021, Thorbjørn Lindeijer <bjorn@lindeijer.nl>
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

#include "popupwidget.h"

#include "stylehelper.h"

#include <QApplication>
#include <QGraphicsDropShadowEffect>

namespace Tiled {

static QColor mergedColors(const QColor &colorA, const QColor &colorB, int factor = 50)
{
    constexpr int maxFactor = 100;
    QColor tmp = colorA;
    tmp.setRed((tmp.red() * factor) / maxFactor + (colorB.red() * (maxFactor - factor)) / maxFactor);
    tmp.setGreen((tmp.green() * factor) / maxFactor + (colorB.green() * (maxFactor - factor)) / maxFactor);
    tmp.setBlue((tmp.blue() * factor) / maxFactor + (colorB.blue() * (maxFactor - factor)) / maxFactor);
    return tmp;
}

PopupWidget::PopupWidget(QWidget *parent)
    : QFrame(parent)
{
    setAttribute(Qt::WA_DeleteOnClose);
    setFrameStyle(QFrame::StyledPanel | QFrame::Plain);
    setAutoFillBackground(true);

    updateBackgroundColor();
    connect(StyleHelper::instance(), &StyleHelper::styleApplied,
            this, &PopupWidget::updateBackgroundColor);

    auto dropShadow = new QGraphicsDropShadowEffect;
    dropShadow->setBlurRadius(10.0);
    dropShadow->setOffset(2.5);
    dropShadow->setColor(QColor(0, 0, 0, 64));
    setGraphicsEffect(dropShadow);
}

void PopupWidget::setTint(const QColor &tint)
{
    if (mTint == tint)
        return;

    mTint = tint;
    updateBackgroundColor();
}

void PopupWidget::updateBackgroundColor()
{
    QPalette pal = QApplication::palette();
    auto tint = mTint.isValid() ? mTint : pal.highlight().color();
    pal.setColor(QPalette::Window, mergedColors(pal.window().color(), tint, 75));
    pal.setColor(QPalette::Link, pal.link().color());
    pal.setColor(QPalette::LinkVisited, pal.linkVisited().color());
    setPalette(pal);
}

} // namespace Tiled
