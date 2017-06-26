/*
 * iconcheckdelegate.h
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

#pragma once

#include <QItemDelegate>
#include <QIcon>

namespace Tiled {
namespace Internal {

/**
 * Delegate for drawing an eye icon in LayerView when the layer is visible.
 */
class IconCheckDelegate: public QItemDelegate
{
public:
    explicit IconCheckDelegate(QObject *parent = nullptr, bool lock = false);

protected:
    void drawCheck(QPainter *painter, const QStyleOptionViewItem &option,
                   const QRect &rect, Qt::CheckState state) const override;

private:
    QIcon mCheckedIcon;
    QIcon mUncheckedIcon;
};

} // namespace Internal
} // namespace Tiled
