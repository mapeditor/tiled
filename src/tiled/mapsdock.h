/*
 * mapsdock.h
 * Copyright 2012, Tim Baker <treectrl@hotmail.com>
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

#include <QDockWidget>

class QLineEdit;

namespace Tiled {

class MapsView;

class MapsDock : public QDockWidget
{
    Q_OBJECT

public:
    MapsDock(QWidget *parent = nullptr);

protected:
    void changeEvent(QEvent *e) override;

private:
    void browse();
    void editedMapsDirectory();
    void onMapsDirectoryChanged();

    void retranslateUi();

    QLineEdit *mDirectoryEdit;
    MapsView *mMapsView;
};

} // namespace Tiled
