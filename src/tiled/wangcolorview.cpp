/*
 * wangcolorview.h
 * Copyright 2017, Benjamin Trotter <bdtrotte@ucsc.edu>
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

#include "wangcolorview.h"

#include "utils.h"
#include "wangcolormodel.h"

#include <QAbstractProxyModel>
#include <QColorDialog>
#include <QContextMenuEvent>
#include <QLineEdit>
#include <QMenu>
#include <QPainter>
#include <QStyledItemDelegate>

using namespace Tiled;

namespace {

class WangColorDelegate : public QStyledItemDelegate
{
public:
    WangColorDelegate(QObject *parent = nullptr)
        : QStyledItemDelegate(parent)
    {}

protected:
    void initStyleOption(QStyleOptionViewItem *option, const QModelIndex &index) const override;
};

void WangColorDelegate::initStyleOption(QStyleOptionViewItem *option, const QModelIndex &index) const
{
    QSize decorationSize = option->decorationSize;
    QPixmap tileImage = index.data(Qt::DecorationRole).value<QPixmap>();

    QPixmap pixmap(decorationSize);
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);

    // Draw the tile image in the background
    const QSizeF size = tileImage.size();
    if (!size.isEmpty()) {
        const qreal scaleX = decorationSize.width() / size.width();
        const qreal scaleY = decorationSize.height() / size.height();
        const qreal scale = std::max(scaleX, scaleY);
        const QSizeF targetSize = size * scale;

        painter.setRenderHint(QPainter::SmoothPixmapTransform, scale < 1.0);
        painter.drawPixmap(QRectF(decorationSize.width() - targetSize.width(),
                                  decorationSize.height() - targetSize.height(),
                                  targetSize.width(),
                                  targetSize.height()),
                           tileImage, tileImage.rect());
    }

    // Draw the Wang color on top
    const QColor wangColor = index.data(WangColorModel::ColorRole).value<QColor>();
    const QPointF topRight = QPointF(pixmap.width() * 0.75, 0);
    const QPointF bottomLeft = QPointF(0, pixmap.height() * 0.75);
    painter.setBrush(wangColor);
    painter.setPen(Qt::NoPen);
    painter.drawPolygon(QVector<QPointF> { QPointF(), topRight, bottomLeft });
    QColor border(Qt::black);
    border.setAlpha(128);
    painter.setPen(QPen(border, 2.0));
    painter.drawLine(topRight, bottomLeft);

    QStyledItemDelegate::initStyleOption(option, index);

    // Reset the icon
    option->features |= QStyleOptionViewItem::HasDecoration;
    option->decorationSize = decorationSize;
    option->icon = pixmap;
}

} // anonymous namespace

WangColorView::WangColorView(QWidget *parent)
    : QTreeView(parent)
{
    setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    setHeaderHidden(true);
    setItemsExpandable(false);
    setIndentation(0);
    setUniformRowHeights(true);
    setItemDelegate(new WangColorDelegate(this));
}

WangColorView::~WangColorView()
{
}

void WangColorView::setTileSize(QSize size)
{
    static const int minSize = 16;
    static const int maxSize = Utils::dpiScaled(32);
    setIconSize(QSize(qBound(minSize, size.width(), maxSize),
                      qBound(minSize, size.height(), maxSize)));
}

void WangColorView::setReadOnly(bool readOnly)
{
    static const auto defaultTriggers = editTriggers();
    mReadOnly = readOnly;
    setEditTriggers(readOnly ? NoEditTriggers : defaultTriggers);
}

void WangColorView::contextMenuEvent(QContextMenuEvent *event)
{
    if (mReadOnly)
        return;

    const QAbstractProxyModel *proxyModel = static_cast<QAbstractProxyModel *>(model());
    const WangColorModel *wangColorModel = static_cast<WangColorModel *>(proxyModel->sourceModel());
    const QModelIndex filterModelIndex = indexAt(event->pos());

    if (!wangColorModel || !filterModelIndex.isValid())
        return;

    const QModelIndex index = proxyModel->mapToSource(filterModelIndex);
    mClickedWangColor = wangColorModel->wangColorAt(index);

    QMenu menu;

    QAction *pickColor = menu.addAction(tr("Pick Custom Color"));
    connect(pickColor, &QAction::triggered,
            this, &WangColorView::pickColor);

    menu.exec(event->globalPos());
}

void WangColorView::pickColor()
{
    QColorDialog *colorPicker = new QColorDialog(this);
    colorPicker->setAttribute(Qt::WA_DeleteOnClose);
    colorPicker->setCurrentColor(mClickedWangColor->color());
    connect(colorPicker, &QColorDialog::colorSelected,
            this, &WangColorView::colorPicked);

    colorPicker->open();
}

void WangColorView::colorPicked(const QColor &color)
{
    if (!mClickedWangColor)
        return;

    if (mClickedWangColor->color() != color)
        emit wangColorColorPicked(mClickedWangColor.data(), color);

    mClickedWangColor.clear();
}

#include "moc_wangcolorview.cpp"
