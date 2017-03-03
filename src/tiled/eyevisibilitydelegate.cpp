/*
 * eyevisibilitydelegate.cpp
 * Copyright 2017, Ketan Gupta <ketan19972010@gmail.com>
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

#include "eyevisibilitydelegate.h"

#include <QPainter>

using namespace Tiled;
using namespace Tiled::Internal;

EyeVisibilityDelegate::EyeVisibilityDelegate(QObject *parent):
    QItemDelegate(parent),
    mVisiblePixmap(QLatin1String(":/images/14x14/layer-visible.png")),
    mInvisiblePixmap(QLatin1String(":/images/14x14/layer-invisible.png"))
{
}

void EyeVisibilityDelegate::drawCheck(QPainter *painter, const QStyleOptionViewItem &option,
    const QRect &rect, Qt::CheckState state) const
{
    Q_UNUSED(option)

    if (state == Qt::Checked)
        painter->drawPixmap(rect, mVisiblePixmap);
    else if (state == Qt::Unchecked)
        painter->drawPixmap(rect, mInvisiblePixmap);

}
