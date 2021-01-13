/*
 * wangtemplateview.cpp
 * Copyright 2017, Benjamin Trotter <bdtrotte@ucsc.edu>
 * Copyright 2020, Thorbjørn Lindeijer <bjorn@lindeijer.nl>
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

#include "wangtemplateview.h"

#include "utils.h"
#include "wangoverlay.h"
#include "wangset.h"
#include "wangtemplatemodel.h"
#include "zoomable.h"

#include <QAbstractItemDelegate>
#include <QApplication>
#include <QCoreApplication>
#include <QGesture>
#include <QGestureEvent>
#include <QPainter>
#include <QPinchGesture>
#include <QWheelEvent>

using namespace Tiled;

namespace {

/**
 * The delegate for drawing the Wang tile templates.
 */
class WangTemplateDelegate : public QAbstractItemDelegate
{
public:
    WangTemplateDelegate(WangTemplateView *wangtemplateView, QObject *parent = nullptr)
        : QAbstractItemDelegate(parent)
        , mWangTemplateView(wangtemplateView)
    {}

    void paint(QPainter *painter,
               const QStyleOptionViewItem &option,
               const QModelIndex &index) const override;

    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;

private:
    WangTemplateView *mWangTemplateView;
};

void WangTemplateDelegate::paint(QPainter *painter,
                                 const QStyleOptionViewItem &option,
                                 const QModelIndex &index) const
{
    const WangTemplateModel *model = static_cast<const WangTemplateModel*>(index.model());
    const WangId wangId = model->wangIdAt(index);
    if (!wangId)
        return;

    painter->setClipRect(option.rect);

    if (WangSet *wangSet = mWangTemplateView->wangSet())
        paintWangOverlay(painter, wangId, *wangSet, option.rect, WO_Outline);

    // Highlight currently selected tile.
    if (mWangTemplateView->currentIndex() == index) {
        QColor high = option.palette.highlight().color();
        painter->setBrush(Qt::NoBrush);
        painter->setPen(QPen(high, 4));
        painter->drawRect(option.rect);
    }

    // Shade tile if used already
    if (mWangTemplateView->wangIdIsUsed(wangId)) {
        painter->setBrush(QColor(0,0,0,100));
        painter->setPen(Qt::NoPen);
        painter->drawRect(option.rect.adjusted(2,2,-2,-2));
    }
}

QSize WangTemplateDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(option)
    Q_UNUSED(index)

    return QSize(32 * mWangTemplateView->scale(),
                 32 * mWangTemplateView->scale());
}

} // anonymous namespace

WangTemplateView::WangTemplateView(QWidget *parent)
    : QListView(parent)
    , mZoomable(new Zoomable(this))
{
    setWrapping(true);
    setFlow(QListView::LeftToRight);
    setResizeMode(QListView::Adjust);
    setSelectionMode(QAbstractItemView::SingleSelection);
    setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    setItemDelegate(new WangTemplateDelegate(this, this));
    setUniformItemSizes(true);

    connect(mZoomable, &Zoomable::scaleChanged,
            this, &WangTemplateView::adjustScale);
}

qreal WangTemplateView::scale() const
{
    return mZoomable->scale();
}

WangSet *WangTemplateView::wangSet() const
{
    const WangTemplateModel *model = wangTemplateModel();
    return model ? model->wangSet() : nullptr;
}

bool WangTemplateView::wangIdIsUsed(WangId wangId) const
{
    if (WangSet *set = wangSet())
        return set->wangIdIsUsed(wangId);

    return false;
}

bool WangTemplateView::event(QEvent *event)
{
    if (event->type() == QEvent::Gesture) {
        QGestureEvent *gestureEvent = static_cast<QGestureEvent *>(event);
        if (QGesture *gesture = gestureEvent->gesture(Qt::PinchGesture))
            mZoomable->handlePinchGesture(static_cast<QPinchGesture *>(gesture));
    } else if (event->type() == QEvent::ShortcutOverride) {
        auto keyEvent = static_cast<QKeyEvent*>(event);
        if (Utils::isZoomInShortcut(keyEvent) ||
                Utils::isZoomOutShortcut(keyEvent) ||
                Utils::isResetZoomShortcut(keyEvent)) {
            event->accept();
            return true;
        }
    }

    return QListView::event(event);
}

void WangTemplateView::keyPressEvent(QKeyEvent *event)
{
    if (Utils::isZoomInShortcut(event)) {
        mZoomable->zoomIn();
        return;
    }
    if (Utils::isZoomOutShortcut(event)) {
        mZoomable->zoomOut();
        return;
    }
    if (Utils::isResetZoomShortcut(event)) {
        mZoomable->resetZoom();
        return;
    }
    return QListView::keyPressEvent(event);
}

void WangTemplateView::wheelEvent(QWheelEvent *event)
{
    if (event->modifiers() & Qt::ControlModifier &&
            event->angleDelta().y())
    {
        mZoomable->handleWheelDelta(event->angleDelta().y());
        return;
    }

    QListView::wheelEvent(event);
}

void WangTemplateView::adjustScale()
{
    scheduleDelayedItemsLayout();
}

#include "moc_wangtemplateview.cpp"
