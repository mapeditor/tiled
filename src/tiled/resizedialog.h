/*
 * resizedialog.h
 * Copyright 2008-2009, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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

#include <QDialog>

#include <functional>

namespace Ui {
class ResizeDialog;
}

namespace Tiled {

class ResizeDialog : public QDialog
{
    Q_OBJECT

public:
    ResizeDialog(QWidget *parent = nullptr);

    ~ResizeDialog();

    void setOldSize(QSize size);

    QSize newSize() const;
    QPoint offset() const;

    bool removeObjects() const;

    void setMiniMapRenderer(std::function<QImage (QSize)> renderer);

private:
    void removeObjectsToggled(bool removeObjects);
    void updateOffsetBounds(const QRect &bounds);

    Ui::ResizeDialog *mUi;
};

} // namespace Tiled
