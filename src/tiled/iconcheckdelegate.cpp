/*
 * iconcheckdelegate.cpp
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

#include "iconcheckdelegate.h"

#include <QPainter>

using namespace Tiled;
using namespace Tiled::Internal;

IconCheckDelegate::IconCheckDelegate(QObject *parent, bool lock):
    QItemDelegate(parent)
{
    if (lock) {
        mCheckedIcon.addFile(QLatin1String(":/images/14x14/locked.png"));
        mCheckedIcon.addFile(QLatin1String(":/images/16x16/locked.png"));
        mCheckedIcon.addFile(QLatin1String(":/images/24x24/locked.png"));
        mUncheckedIcon.addFile(QLatin1String(":/images/14x14/unlocked.png"));
        mUncheckedIcon.addFile(QLatin1String(":/images/16x16/unlocked.png"));
        mUncheckedIcon.addFile(QLatin1String(":/images/24x24/unlocked.png"));
    } else {
        mCheckedIcon.addFile(QLatin1String(":/images/14x14/visible.png"));
        mCheckedIcon.addFile(QLatin1String(":/images/16x16/visible.png"));
        mCheckedIcon.addFile(QLatin1String(":/images/24x24/visible.png"));
        mUncheckedIcon.addFile(QLatin1String(":/images/14x14/hidden.png"));
        mUncheckedIcon.addFile(QLatin1String(":/images/16x16/hidden.png"));
        mUncheckedIcon.addFile(QLatin1String(":/images/24x24/hidden.png"));
    }
}

void IconCheckDelegate::drawCheck(QPainter *painter, const QStyleOptionViewItem &,
                                      const QRect &rect, Qt::CheckState state) const
{
    const QIcon &icon = (state == Qt::Checked) ? mCheckedIcon : mUncheckedIcon;
    const QPixmap &pixmap = icon.pixmap(rect.size());

    QSize layoutSize = pixmap.size() / pixmap.devicePixelRatio();
    QRect targetRect(QPoint(0, 0), layoutSize);
    targetRect.moveCenter(rect.center());

    painter->drawPixmap(targetRect, pixmap);
}
