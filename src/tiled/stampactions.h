/*
 * stampactions.h
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

#include <QObject>

class QAction;
class QToolBar;

namespace Tiled {
namespace Internal {

class StampActions : public QObject
{
    Q_OBJECT

public:
    StampActions(QObject *parent = nullptr);
    ~StampActions();

    void languageChanged();

    void populateToolBar(QToolBar *toolBar, bool isRandom);

    QAction *random() { return mRandom; }
    QAction *flipHorizontal() { return mFlipHorizontal; }
    QAction *flipVertical() { return mFlipVertical; }
    QAction *rotateLeft() { return mRotateLeft; }
    QAction *rotateRight() { return mRotateRight; }

private:
    QAction *mRandom;
    QAction *mFlipHorizontal;
    QAction *mFlipVertical;
    QAction *mRotateLeft;
    QAction *mRotateRight;
};

} // namespace Internal
} // namespace Tiled
