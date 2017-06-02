/*
 * toolspecifictoolbar.cpp
 * Copyright 2016, Ketan Gupta <ketan19972010@gmail.com>
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

#include "toolspecifictoolbar.h"

#include "abstracttool.h"
#include "bucketfilltool.h"
#include "mapeditor.h"
#include "stampbrush.h"

#include <QEvent>
#include <QToolButton>

namespace Tiled {
namespace Internal {

ToolSpecificToolBar::ToolSpecificToolBar(QWidget *parent, MapEditor *mapEditor)
    : QToolBar(parent)
    , mMapEditor(mapEditor)
    , mDiceIcon(QLatin1String(":images/24x24/dice.png"))
    , mFlipHorizontalIcon(QLatin1String(":images/24x24/flip-horizontal.png"))
    , mFlipVerticalIcon(QLatin1String(":images/24x24/flip-vertical.png"))
    , mRotateLeftIcon(QLatin1String(":images/24x24/rotate-left.png"))
    , mRotateRightIcon(QLatin1String(":images/24x24/rotate-right.png"))
{
    setObjectName(QLatin1String("ToolSpecificToolBar"));
    setWindowTitle(tr("Tool Specific Tool Bar"));
    setToolButtonStyle(Qt::ToolButtonFollowStyle);

    mDiceIcon.addFile(QLatin1String(":images/32x32/dice.png"));
	mFlipHorizontalIcon.addFile(QLatin1String(":images/32x32/flip-horizontal.png"));
	mFlipVerticalIcon.addFile(QLatin1String(":images/32x32/flip-vertical.png"));
	mRotateLeftIcon.addFile(QLatin1String(":images/32x32/rotate-left.png"));
	mRotateRightIcon.addFile(QLatin1String(":images/32x32/rotate-right.png"));

    retranslateUi();
}

void ToolSpecificToolBar::changeEvent(QEvent *event)
{
    QToolBar::changeEvent(event);
    switch (event->type()) {
    case QEvent::LanguageChange:
        retranslateUi();
        break;
    default:
        break;
    }
}

void ToolSpecificToolBar::setSelectedTool(AbstractTool *tool)
{
	qDeleteAll(mButtons);
	mButtons.clear();

	clear();

	if (!tool)
		return;

	StampBrush *stampBrush = qobject_cast<StampBrush*>(tool);
	if (stampBrush) {
		addRandomTool(stampBrush->random());
		addFlippingTools();
		addRotatingTools();
	}

	BucketFillTool *bucketFillTool = qobject_cast<BucketFillTool*>(tool);
	if (bucketFillTool) {
		addRandomTool(bucketFillTool->random());
		addFlippingTools();
		addRotatingTools();
	}
}

void ToolSpecificToolBar::onOrientationChanged(Qt::Orientation orientation)
{
    setToolButtonStyle(orientation == Qt::Horizontal ? Qt::ToolButtonFollowStyle :
                                                       Qt::ToolButtonIconOnly);
}

void ToolSpecificToolBar::addRandomTool(bool checked)
{
	QToolButton *mRandomButton = new QToolButton(this);
    mRandomButton->setIcon(mDiceIcon);
    mRandomButton->setCheckable(true);
    mRandomButton->setToolTip(tr("Random Mode"));
    mRandomButton->setShortcut(QKeySequence(tr("D")));
    mRandomButton->setChecked(checked);

    addWidget(mRandomButton);
    mButtons.append(mRandomButton);

    connect(mRandomButton, &QToolButton::toggled, mMapEditor, &MapEditor::random);
}

void ToolSpecificToolBar::addFlippingTools()
{
	QToolButton *mFlipHorizontalButton = new QToolButton(this);
    mFlipHorizontalButton->setIcon(mFlipHorizontalIcon);
    mFlipHorizontalButton->setToolTip(tr("Flip Horizontally"));
    mFlipHorizontalButton->setShortcut(QKeySequence(tr("X")));

    QToolButton *mFlipVerticalButton = new QToolButton(this);
    mFlipVerticalButton->setIcon(mFlipVerticalIcon);
    mFlipHorizontalButton->setToolTip(tr("Flip Vertically"));
    mFlipVerticalButton->setShortcut(QKeySequence(tr("Y")));

	addWidget(mFlipHorizontalButton);
    addWidget(mFlipVerticalButton);
    mButtons.append(mFlipHorizontalButton);
    mButtons.append(mFlipVerticalButton);

	connect(mFlipHorizontalButton, &QToolButton::clicked, mMapEditor, &MapEditor::flipHorizontally);
    connect(mFlipVerticalButton, &QToolButton::clicked, mMapEditor, &MapEditor::flipVertically);
}

void ToolSpecificToolBar::addRotatingTools()
{
	QToolButton *mRotateLeft = new QToolButton(this);
    mRotateLeft->setIcon(mRotateLeftIcon);
    mRotateLeft->setToolTip(tr("Rotate Left"));
    mRotateLeft->setShortcut(QKeySequence(tr("Shift+Z")));

    QToolButton *mRotateRight = new QToolButton(this);
    mRotateRight->setIcon(mRotateRightIcon);
    mRotateLeft->setToolTip(tr("Rotate Right"));
    mRotateRight->setShortcut(QKeySequence(tr("Z")));

    addWidget(mRotateLeft);
    addWidget(mRotateRight);
    mButtons.append(mRotateLeft);
    mButtons.append(mRotateRight);

    connect(mRotateLeft, &QToolButton::clicked, mMapEditor, &MapEditor::rotateRight);
    connect(mRotateRight, &QToolButton::clicked, mMapEditor, &MapEditor::rotateLeft);
}

void ToolSpecificToolBar::retranslateUi()
{
    // TODO
}

} // namespace Internal
} // namespace Tiled
