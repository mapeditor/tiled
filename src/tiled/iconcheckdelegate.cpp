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

#include "utils.h"

#include <QMouseEvent>
#include <QKeyEvent>
#include <QPainter>

using namespace Tiled;

IconCheckDelegate::IconCheckDelegate(IconType icon,
                                     bool exclusive,
                                     QObject *parent)
    : QItemDelegate(parent)
    , mExclusive(exclusive)
{
    switch (icon) {
    case LockedIcon:
        mCheckedIcon.addFile(QLatin1String(":/images/14/locked.png"));
        mCheckedIcon.addFile(QLatin1String(":/images/16/locked.png"));
        mCheckedIcon.addFile(QLatin1String(":/images/24/locked.png"));
        mUncheckedIcon.addFile(QLatin1String(":/images/14/unlocked.png"));
        mUncheckedIcon.addFile(QLatin1String(":/images/16/unlocked.png"));
        mUncheckedIcon.addFile(QLatin1String(":/images/24/unlocked.png"));
        break;
    case VisibilityIcon:
        mCheckedIcon.addFile(QLatin1String(":/images/14/visible.png"));
        mCheckedIcon.addFile(QLatin1String(":/images/16/visible.png"));
        mCheckedIcon.addFile(QLatin1String(":/images/24/visible.png"));
        mUncheckedIcon.addFile(QLatin1String(":/images/14/hidden.png"));
        mUncheckedIcon.addFile(QLatin1String(":/images/16/hidden.png"));
        mUncheckedIcon.addFile(QLatin1String(":/images/24/hidden.png"));
        break;
    }

    setClipping(false);
}

int IconCheckDelegate::exclusiveSectionWidth()
{
    return qRound(Utils::dpiScaled(22));
}

/**
 * Override in order to remove the checkbox area condition in case of
 * exclusively displaying a checkbox icon.
 */
bool IconCheckDelegate::editorEvent(QEvent *event,
                                    QAbstractItemModel *model,
                                    const QStyleOptionViewItem &option,
                                    const QModelIndex &index)
{
    if (!mExclusive)
        return QItemDelegate::editorEvent(event, model, option, index);

    // make sure that the item is checkable
    Qt::ItemFlags flags = model->flags(index);
    if (!(flags & Qt::ItemIsUserCheckable) || !(option.state & QStyle::State_Enabled)
            || !(flags & Qt::ItemIsEnabled))
        return false;

    // make sure that we have a check state
    QVariant value = index.data(Qt::CheckStateRole);
    if (!value.isValid())
        return false;

    // make sure that we have the right event type
    if ((event->type() == QEvent::MouseButtonRelease)
            || (event->type() == QEvent::MouseButtonDblClick)
            || (event->type() == QEvent::MouseButtonPress)) {
        QMouseEvent *me = static_cast<QMouseEvent*>(event);
        if (me->button() != Qt::LeftButton)
            return false;

        // eat the double click events inside the check rect
        if ((event->type() == QEvent::MouseButtonPress)
                || (event->type() == QEvent::MouseButtonDblClick))
            return true;

    } else if (event->type() == QEvent::KeyPress) {
        if (static_cast<QKeyEvent*>(event)->key() != Qt::Key_Space
                && static_cast<QKeyEvent*>(event)->key() != Qt::Key_Select)
            return false;
    } else {
        return false;
    }

    Qt::CheckState state = static_cast<Qt::CheckState>(value.toInt());
    if (flags & Qt::ItemIsUserTristate)
        state = static_cast<Qt::CheckState>((state + 1) % 3);
    else
        state = (state == Qt::Checked) ? Qt::Unchecked : Qt::Checked;
    return model->setData(index, state, Qt::CheckStateRole);
}

void IconCheckDelegate::drawCheck(QPainter *painter,
                                  const QStyleOptionViewItem &opt,
                                  const QRect &rect,
                                  Qt::CheckState state) const
{
    const QRect &r = mExclusive ? opt.rect : rect;
    const QIcon &icon = (state == Qt::Checked) ? mCheckedIcon : mUncheckedIcon;
    const QPixmap &pixmap = icon.pixmap(mExclusive ? Utils::smallIconSize() : r.size());

    QSize layoutSize = pixmap.size() / pixmap.devicePixelRatio();
    QRect targetRect(QPoint(0, 0), layoutSize);
    targetRect.moveCenter(r.center());

    painter->drawPixmap(targetRect, pixmap);
}

void IconCheckDelegate::drawDisplay(QPainter *painter,
                                    const QStyleOptionViewItem &option,
                                    const QRect &rect,
                                    const QString &text) const
{
    if (mExclusive) // suppress rendering of selection on top of icon
        return;

    QItemDelegate::drawDisplay(painter, option, rect, text);
}

QSize IconCheckDelegate::sizeHint(const QStyleOptionViewItem &option,
                                  const QModelIndex &index) const
{
    if (mExclusive)
        return Utils::dpiScaled(QSize(22, 20));
    return QItemDelegate::sizeHint(option, index);
}
