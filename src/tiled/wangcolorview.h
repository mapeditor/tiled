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

#pragma once

#include <QTreeView>

namespace Tiled {

class WangColor;

namespace Internal {

class WangColorView : public QTreeView
{
    Q_OBJECT

public:
    WangColorView(QWidget *parent);
    ~WangColorView() override;

protected:
    void contextMenuEvent(QContextMenuEvent *event) override;

signals:
    void wangColorColorPicked(const QColor &color, bool isEdge, int index);

private slots:
    void pickColor();
    void colorPicked(const QColor &color);

private:
    QSharedPointer<WangColor> mClickedWangColor;
};

} // namespace Internal
} // namespace Tiled
