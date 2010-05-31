/*
 * colorbutton.h
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

#ifndef COLORBUTTON_H
#define COLORBUTTON_H

#include <QColor>
#include <QToolButton>

namespace Tiled {
namespace Internal {

/**
 * A tool button for letting the user pick a color. When clicked it shows a
 * color dialog and it has an icon to represent the currently chosen color.
 */
class ColorButton : public QToolButton
{
    Q_OBJECT

public:
    ColorButton(QWidget *parent = 0);

    QColor color() const { return mColor; }
    void setColor(const QColor &color);

signals:
    void colorChanged(const QColor &color);

private slots:
    void pickColor();

private:
    QColor mColor;
};

} // namespace Internal
} // namespace Tiled

#endif // COLORBUTTON_H
