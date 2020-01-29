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

/**
 * Delegate for drawing an icon in LayerView and ObjectsView for displaying
 * visibility and locked state.
 */
class IconCheckDelegate : public QItemDelegate
{
public:
    enum IconType {
        VisibilityIcon,
        LockedIcon
    };

    explicit IconCheckDelegate(IconType icon,
                               bool exclusive,
                               QObject *parent = nullptr);

    static int exclusiveSectionWidth();

    bool editorEvent(QEvent *event,
                     QAbstractItemModel *model,
                     const QStyleOptionViewItem &option,
                     const QModelIndex &index) override;

protected:
    void drawCheck(QPainter *painter,
                   const QStyleOptionViewItem &option,
                   const QRect &rect,
                   Qt::CheckState state) const override;

    void drawDisplay(QPainter *painter,
                     const QStyleOptionViewItem &option,
                     const QRect &rect,
                     const QString &text) const override;

    QSize sizeHint(const QStyleOptionViewItem &option,
                   const QModelIndex &index) const override;


private:
    QIcon mCheckedIcon;
    QIcon mUncheckedIcon;
    bool mExclusive;
};

} // namespace Tiled
