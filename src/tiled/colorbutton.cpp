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

#include "utils.h"

#include <QColorDialog>
#include <QEvent>
#include <QStyle>

using namespace Tiled;

ColorButton::ColorButton(QWidget *parent)
    : QToolButton(parent)
{
    const int defaultIconSize = style()->pixelMetric(QStyle::PM_ButtonIconSize);
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
        const int defaultIconSize = style()->pixelMetric(QStyle::PM_ButtonIconSize);
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
    const QColor newColor = QColorDialog::getColor(mColor, this);
    if (newColor.isValid())
        setColor(newColor);
}

void ColorButton::updateIcon()
{
    setIcon(Utils::colorIcon(mColor, iconSize()));
}

#include "moc_colorbutton.cpp"
