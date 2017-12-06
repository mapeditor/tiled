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

#include "wangcolormodel.h"

#include <QStyledItemDelegate>
#include <QContextMenuEvent>
#include <QColorDialog>
#include <QMenu>
#include <QPainter>
#include <QLineEdit>
#include <QSortFilterProxyModel>

using namespace Tiled;
using namespace Internal;

namespace {

class WangColorDelegate : public QStyledItemDelegate
{
public:
    WangColorDelegate(WangColorView *wangColorView, QObject *parent = nullptr)
        : QStyledItemDelegate(parent)
        , mWangColorView(wangColorView)
    {}

    QSize sizeHint(const QStyleOptionViewItem &option,
                  const QModelIndex &index) const override;

    void paint(QPainter *painter,
               const QStyleOptionViewItem &option,
               const QModelIndex &index) const override;

    void setEditorData(QWidget *editor, const QModelIndex &index) const override;

private:
    WangColorView *mWangColorView;
};

QSize WangColorDelegate::sizeHint(const QStyleOptionViewItem &, const QModelIndex &) const
{
    //TODO allow resizing.
    return QSize(1, 40);
}

void WangColorDelegate::paint(QPainter *painter,
                              const QStyleOptionViewItem &option,
                              const QModelIndex &index) const
{
    painter->save();

    const QRect &rect = option.rect;
    const QBrush &brush = index.data(Qt::BackgroundRole).value<QBrush>();
    QFont font = index.data(Qt::FontRole).value<QFont>();
    const QPixmap &image = index.data(Qt::DecorationRole).value<QPixmap>();
    const QString &text = index.data().toString();

    QFontMetrics fontMetrics(font);

    painter->setClipRect(rect);
    painter->setPen(Qt::NoPen);

    painter->setBrush(brush);
    painter->drawRect(rect);

    QRect imageRect = QRect(rect.topLeft(),
                            QSize(rect.height(),
                                  rect.height())).adjusted(2, 2, -2, -2);

    if (!image.isNull())
        painter->drawPixmap(imageRect, image);

    if (index.parent().isValid()) {
        painter->save();

        QColor darkerColor = brush.color().darker(150);
        painter->setPen(QPen(darkerColor));
        if (mWangColorView->selectionModel()->currentIndex() == index) {
            painter->setBrush(QBrush(darkerColor));
            painter->setOpacity(0.5);
        } else {
            painter->setBrush(Qt::NoBrush);
        }

        painter->drawRect(rect.adjusted(0, 0, -1, -1));

        painter->restore();
    }

    if (!text.isEmpty()) {
        QPoint textPos;
        if (index.parent().isValid() && !image.isNull())
            textPos = QPoint(imageRect.right() + 6,
                             rect.center().y() + (fontMetrics.height() / 2) + 2);
        else
            textPos = QPoint(rect.left() + 6,
                             rect.center().y() + (fontMetrics.height() / 2) + 2);

        if (index.parent().isValid()) {
            painter->setBrush(QBrush(brush.color().lighter()));
            painter->drawRect(QRect(textPos + QPoint(-2, -fontMetrics.height() - 2),
                                    QPoint(rect.right(), textPos.y() + 2)));
        }

        painter->setPen(QPen(Qt::black));
        painter->setFont(font);
        painter->drawText(textPos, index.data().toString());
    }

    painter->restore();
}

void WangColorDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    QLineEdit *lineEdit = qobject_cast<QLineEdit *>(editor);
    Q_ASSERT(lineEdit);

    int top = editor->y();
    int height = editor->geometry().height();
    int left;
    if (index.data(Qt::DecorationRole).value<QPixmap>().isNull())
        left = 2;
    else
        left = height;
    int width = mWangColorView->width() - left;

    lineEdit->setGeometry(left, top, width, height);

    lineEdit->setText(index.data().toString());
    lineEdit->selectAll();
}

}

WangColorView::WangColorView(QWidget *parent)
    : QTreeView(parent)
{
    setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    setHeaderHidden(true);
    setItemsExpandable(false);
    setIndentation(0);
    setUniformRowHeights(true);
    setSelectionMode(QAbstractItemView::NoSelection);
    setItemDelegate(new WangColorDelegate(this, this));
}

WangColorView::~WangColorView()
{
}

void WangColorView::contextMenuEvent(QContextMenuEvent *event)
{
    const QSortFilterProxyModel *filterModel = static_cast<QSortFilterProxyModel *>(model());
    const WangColorModel *wangColorModel = static_cast<WangColorModel *>(filterModel->sourceModel());
    const QModelIndex filterModelIndex = indexAt(event->pos());

    if (!wangColorModel || !wangColorModel->hasTilesetDocument() || !filterModelIndex.isValid())
        return;

    const QModelIndex index = filterModel->mapToSource(filterModelIndex);
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

    if (mClickedWangColor->color() != color) {
        emit wangColorColorPicked(color,
                                  mClickedWangColor->isEdge(),
                                  mClickedWangColor->colorIndex());
    }

    mClickedWangColor.clear();
}
