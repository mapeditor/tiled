/*
 * eyevisibilitydelegate.cpp
 * Copyright 2008-2017, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
 * Copyright 2010, Andrew G. Crowell <overkill9999@gmail.com>
 * Copyright 2010, Jeff Bland <jksb@member.fsf.org>
 * Copyright 2011, Stefan Beller <stefanbeller@googlemail.com>
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

#include <QApplication>
#include <QPainter>

using namespace Tiled;
using namespace Tiled::Internal;

EyeVisibilityDelegate::EyeVisibilityDelegate(QObject *parent):
    QItemDelegate(parent),
    mVisiblePixmap(QLatin1String(":/images/16x16/layer-visible.png")),
    mInvisiblePixmap(QLatin1String(":/images/16x16/layer-invisible.png"))
{
}

void EyeVisibilityDelegate::drawCheck(QPainter *painter, const QStyleOptionViewItem &option,
    const QRect &rect, Qt::CheckState state) const
{
    Q_UNUSED(option)

    if (state == Qt::Checked)
        painter->drawPixmap(rect, mVisiblePixmap, QRect(0, 0, rect.width(), rect.height()));
    else if (state == Qt::Unchecked)
        painter->drawPixmap(rect, mInvisiblePixmap, QRect(0, 0, rect.width(), rect.height()));

}
