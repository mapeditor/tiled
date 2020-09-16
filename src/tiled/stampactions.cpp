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

#include "actionmanager.h"

#include <QAction>
#include <QToolBar>

using namespace Tiled;

StampActions::StampActions(QObject *parent) : QObject(parent)
{
    QIcon diceIcon(QLatin1String(":images/24/dice.png"));
    QIcon wangIcon(QLatin1String(":images/24/wangtile.png"));
    QIcon flipHorizontalIcon(QLatin1String(":images/24/flip-horizontal.png"));
    QIcon flipVerticalIcon(QLatin1String(":images/24/flip-vertical.png"));
    QIcon rotateLeftIcon(QLatin1String(":images/24/rotate-left.png"));
    QIcon rotateRightIcon(QLatin1String(":images/24/rotate-right.png"));

    diceIcon.addFile(QLatin1String(":images/32/dice.png"));
    wangIcon.addFile(QLatin1String(":images/32/wangtile.png"));
    flipHorizontalIcon.addFile(QLatin1String(":images/32/flip-horizontal.png"));
    flipVerticalIcon.addFile(QLatin1String(":images/32/flip-vertical.png"));
    rotateLeftIcon.addFile(QLatin1String(":images/32/rotate-left.png"));
    rotateRightIcon.addFile(QLatin1String(":images/32/rotate-right.png"));

    mRandom = new QAction(this);
    mRandom->setIcon(diceIcon);
    mRandom->setCheckable(true);
    mRandom->setShortcut(Qt::Key_D);

    mWangFill = new QAction(this);
    mWangFill->setIcon(wangIcon);
    mWangFill->setCheckable(true);

    mFlipHorizontal = new QAction(this);
    mFlipHorizontal->setIcon(flipHorizontalIcon);
    mFlipHorizontal->setShortcut(Qt::Key_X);

    mFlipVertical = new QAction(this);
    mFlipVertical->setIcon(flipVerticalIcon);
    mFlipVertical->setShortcut(Qt::Key_Y);

    mRotateLeft = new QAction(this);
    mRotateLeft->setIcon(rotateLeftIcon);
    mRotateLeft->setShortcut(Qt::SHIFT + Qt::Key_Z);

    mRotateRight = new QAction(this);
    mRotateRight->setIcon(rotateRightIcon);
    mRotateRight->setShortcut(Qt::Key_Z);

    ActionManager::registerAction(mRandom, "RandomMode");
    ActionManager::registerAction(mWangFill, "WangFillMode");
    ActionManager::registerAction(mFlipHorizontal, "FlipHorizontal");
    ActionManager::registerAction(mFlipVertical, "FlipVertical");
    ActionManager::registerAction(mRotateLeft, "RotateLeft");
    ActionManager::registerAction(mRotateRight, "RotateRight");

    languageChanged();
}

void StampActions::languageChanged()
{
    mRandom->setText(tr("Random Mode"));
    mWangFill->setText(tr("Wang Fill Mode"));
    mFlipHorizontal->setText(tr("Flip Horizontally"));
    mFlipVertical->setText(tr("Flip Vertically"));
    mRotateLeft->setText(tr("Rotate Left"));
    mRotateRight->setText(tr("Rotate Right"));
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
