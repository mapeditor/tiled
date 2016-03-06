/*
 * maintoolbar.h
 * Copyright 2016, Thorbjørn Lindeijer <bjorn@lindijer.nl>
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

#ifndef TILED_INTERNAL_MAINTOOLBAR_H
#define TILED_INTERNAL_MAINTOOLBAR_H

#include <QToolBar>

namespace Tiled {
namespace Internal {

class MainToolBar : public QToolBar
{
public:
    MainToolBar(QWidget *parent = nullptr);

protected:
    void changeEvent(QEvent *event) override;

private slots:
    void onOrientationChanged(Qt::Orientation orientation);

private:
    void retranslateUi();

    QAction *mOpenAction;
    QAction *mSaveAction;
    QAction *mUndoAction;
    QAction *mRedoAction;
};

} // namespace Internal
} // namespace Tiled

#endif // TILED_INTERNAL_MAINTOOLBAR_H
