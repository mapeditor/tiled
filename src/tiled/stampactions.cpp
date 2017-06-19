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
    QIcon mDiceIcon(QLatin1String(":images/24x24/dice.png"));
    QIcon mFlipHorizontalIcon(QLatin1String(":images/24x24/flip-horizontal.png"));
    QIcon mFlipVerticalIcon(QLatin1String(":images/24x24/flip-vertical.png"));
    QIcon mRotateLeftIcon(QLatin1String(":images/24x24/rotate-left.png"));
    QIcon mRotateRightIcon(QLatin1String(":images/24x24/rotate-right.png"));

    mDiceIcon.addFile(QLatin1String(":images/32x32/dice.png"));
    mFlipHorizontalIcon.addFile(QLatin1String(":images/32x32/flip-horizontal.png"));
    mFlipVerticalIcon.addFile(QLatin1String(":images/32x32/flip-vertical.png"));
    mRotateLeftIcon.addFile(QLatin1String(":images/32x32/rotate-left.png"));
    mRotateRightIcon.addFile(QLatin1String(":images/32x32/rotate-right.png"));

    mRandom = new QAction(this);
    mRandom->setIcon(mDiceIcon);
    mRandom->setCheckable(true);
    mRandom->setToolTip(tr("Random Mode"));
    mRandom->setShortcut(QKeySequence(tr("D")));

    mFlipHorizontal = new QAction(this);
    mFlipHorizontal->setIcon(mFlipHorizontalIcon);
    mFlipHorizontal->setToolTip(tr("Flip Horizontally"));
    mFlipHorizontal->setShortcut(QKeySequence(tr("X")));

    mFlipVertical = new QAction(this);
    mFlipVertical->setIcon(mFlipVerticalIcon);
    mFlipHorizontal->setToolTip(tr("Flip Vertically"));
    mFlipVertical->setShortcut(QKeySequence(tr("Y")));

    mRotateLeft = new QAction(this);
    mRotateLeft->setIcon(mRotateLeftIcon);
    mRotateLeft->setToolTip(tr("Rotate Left"));
    mRotateLeft->setShortcut(QKeySequence(tr("Shift+Z")));

    mRotateRight = new QAction(this);
    mRotateRight->setIcon(mRotateRightIcon);
    mRotateLeft->setToolTip(tr("Rotate Right"));
    mRotateRight->setShortcut(QKeySequence(tr("Z")));
}

StampActions::~StampActions()
{
}

void StampActions::languageChanged()
{
    mRandom->setToolTip(tr("Random Mode"));
    mFlipHorizontal->setToolTip(tr("Flip Horizontally"));
    mFlipHorizontal->setToolTip(tr("Flip Vertically"));
    mRotateLeft->setToolTip(tr("Rotate Left"));
    mRotateLeft->setToolTip(tr("Rotate Right"));

    mRandom->setShortcut(QKeySequence(tr("D")));
    mFlipHorizontal->setShortcut(QKeySequence(tr("X")));
    mFlipVertical->setShortcut(QKeySequence(tr("Y")));
    mRotateLeft->setShortcut(QKeySequence(tr("Shift+Z")));
    mRotateRight->setShortcut(QKeySequence(tr("Z")));
}

void StampActions::populateToolBar(QToolBar *toolBar, bool isRandom)
{
    mRandom->setChecked(isRandom);
    toolBar->addAction(mRandom);
    toolBar->addAction(mFlipHorizontal);
    toolBar->addAction(mFlipVertical);
    toolBar->addAction(mRotateLeft);
    toolBar->addAction(mRotateRight);
}
