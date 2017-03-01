/*
 * eyevisibilitydelegate.h
 * Copyright 2008-2013, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
 * Copyright 2010, Andrew G. Crowell <overkill9999@gmail.com>
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

#pragma once

#include <QItemDelegate>
#include <QPixmap>
#include <QTreeView>

namespace Tiled {
namespace Internal {

/**
 * Delegate for drawing an eye icon in LayerView when the layer is visible.
 */
class EyeVisibilityDelegate: public QItemDelegate
{
public:
    explicit EyeVisibilityDelegate(QObject *parent = nullptr);

protected:
    void drawCheck(QPainter *painter, const QStyleOptionViewItem &option,
        const QRect &rect, Qt::CheckState state) const override;

private:
    QPixmap mVisiblePixmap;
    QPixmap mInvisiblePixmap;
};

} // namespace Internal
} // namespace Tiled
