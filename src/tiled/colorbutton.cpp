/*
 * colorbutton.cpp
 * Copyright 2009, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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

#include "colorbutton.h"

#include <QColorDialog>
#include <QEvent>
#include <QPainter>
#include <QStyle>

using namespace Tiled;

ColorButton::ColorButton(QWidget *parent)
    : QToolButton(parent)
{
    int defaultIconSize = style()->pixelMetric(QStyle::PM_ButtonIconSize);
    setIconSize(QSize(defaultIconSize * 2, defaultIconSize));
    setColor(Qt::white);

    connect(this, &QToolButton::clicked, this, &ColorButton::pickColor);
}

void ColorButton::setColor(const QColor &color)
{
    if (mColor == color || !color.isValid())
        return;

    mColor = color;

    updateIcon();

    emit colorChanged(color);
}

void ColorButton::changeEvent(QEvent *e)
{
    QToolButton::changeEvent(e);

    switch (e->type()) {
    case QEvent::StyleChange: {
        int defaultIconSize = style()->pixelMetric(QStyle::PM_ButtonIconSize);
        setIconSize(QSize(defaultIconSize * 2, defaultIconSize));
        updateIcon();
        break;
    }
    default:
        break;
    }
}

void ColorButton::pickColor()
{
    QColor newColor = QColorDialog::getColor(mColor, this);
    if (newColor.isValid())
        setColor(newColor);
}

void ColorButton::updateIcon()
{
    QSize size(iconSize());
    size.rwidth() -= 2;
    size.rheight() -= 2;

    QPixmap pixmap(size);
    pixmap.fill(mColor);

    QPainter painter(&pixmap);
    QColor border(Qt::black);
    border.setAlpha(128);
    painter.setPen(border);
    painter.drawRect(0, 0, pixmap.width() - 1, pixmap.height() - 1);

    setIcon(QIcon(pixmap));
}
