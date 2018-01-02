/*
 * stampactions.cpp
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

#include "stampactions.h"

#include <QAction>
#include <QToolBar>

using namespace Tiled;
using namespace Tiled::Internal;

StampActions::StampActions(QObject *parent) : QObject(parent)
{
    QIcon diceIcon(QLatin1String(":images/24x24/dice.png"));
    QIcon wangIcon(QLatin1String(":images/24x24/wangtile.png"));
    QIcon flipHorizontalIcon(QLatin1String(":images/24x24/flip-horizontal.png"));
    QIcon flipVerticalIcon(QLatin1String(":images/24x24/flip-vertical.png"));
    QIcon rotateLeftIcon(QLatin1String(":images/24x24/rotate-left.png"));
    QIcon rotateRightIcon(QLatin1String(":images/24x24/rotate-right.png"));

    diceIcon.addFile(QLatin1String(":images/32x32/dice.png"));
    wangIcon.addFile(QLatin1String(":images/32x32/wangtile.png"));
    flipHorizontalIcon.addFile(QLatin1String(":images/32x32/flip-horizontal.png"));
    flipVerticalIcon.addFile(QLatin1String(":images/32x32/flip-vertical.png"));
    rotateLeftIcon.addFile(QLatin1String(":images/32x32/rotate-left.png"));
    rotateRightIcon.addFile(QLatin1String(":images/32x32/rotate-right.png"));

    mRandom = new QAction(this);
    mRandom->setIcon(diceIcon);
    mRandom->setCheckable(true);
    mRandom->setToolTip(tr("Random Mode"));
    mRandom->setShortcut(QKeySequence(tr("D")));

    mWangFill = new QAction(this);
    mWangFill->setIcon(wangIcon);
    mWangFill->setCheckable(true);
    mWangFill->setToolTip(tr("Wang Fill Mode"));

    mFlipHorizontal = new QAction(this);
    mFlipHorizontal->setIcon(flipHorizontalIcon);

    mFlipVertical = new QAction(this);
    mFlipVertical->setIcon(flipVerticalIcon);

    mRotateLeft = new QAction(this);
    mRotateLeft->setIcon(rotateLeftIcon);

    mRotateRight = new QAction(this);
    mRotateRight->setIcon(rotateRightIcon);

    languageChanged();
}

StampActions::~StampActions()
{
}

void StampActions::languageChanged()
{
    mRandom->setToolTip(tr("Random Mode"));
    mWangFill->setToolTip(tr("Wang Fill Mode"));
    mFlipHorizontal->setToolTip(tr("Flip Horizontally"));
    mFlipVertical->setToolTip(tr("Flip Vertically"));
    mRotateLeft->setToolTip(tr("Rotate Left"));
    mRotateRight->setToolTip(tr("Rotate Right"));

    mRandom->setShortcut(QKeySequence(tr("D")));
    mFlipHorizontal->setShortcut(QKeySequence(tr("X")));
    mFlipVertical->setShortcut(QKeySequence(tr("Y")));
    mRotateLeft->setShortcut(QKeySequence(tr("Shift+Z")));
    mRotateRight->setShortcut(QKeySequence(tr("Z")));
}

void StampActions::populateToolBar(QToolBar *toolBar, bool isRandom, bool isWangFill)
{
    mRandom->setChecked(isRandom);
    mWangFill->setChecked(isWangFill);
    toolBar->addAction(mRandom);
    toolBar->addAction(mWangFill);
    toolBar->addSeparator();
    toolBar->addAction(mFlipHorizontal);
    toolBar->addAction(mFlipVertical);
    toolBar->addAction(mRotateLeft);
    toolBar->addAction(mRotateRight);
}
