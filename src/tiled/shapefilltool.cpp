/*
 * shapefilltool.cpp
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

#include "shapefilltool.h"

#include "addremovetileset.h"
#include "brushitem.h"
#include "geometry.h"
#include "mapdocument.h"
#include "painttilelayer.h"
#include "stampactions.h"

#include <QApplication>
#include <QActionGroup>
#include <QToolBar>

using namespace Tiled;
using namespace Internal;

ShapeFillTool::ShapeFillTool(QObject *parent)
    : AbstractTileFillTool(tr("Shape Fill Tool"),
                           QIcon(QLatin1String(
                                     ":images/22x22/rectangle-fill.png")),
                           QKeySequence(tr("P")),
                           nullptr,
                           parent)
    , mToolBehavior(Free)
    , mCurrentShape(Rect)
    , mRectFill(new QAction(this))
    , mCircleFill(new QAction(this))
{
    QIcon rectFillIcon(QLatin1String(":images/22x22/rectangle-fill.png"));
    QIcon circleFillIcon(QLatin1String(":images/22x22/ellipse-fill.png"));

    mRectFill->setIcon(rectFillIcon);
    mRectFill->setCheckable(true);
    mRectFill->setChecked(true);

    mCircleFill->setIcon(circleFillIcon);
    mCircleFill->setCheckable(true);

    connect(mRectFill, &QAction::triggered,
            [this] { setCurrentShape(Rect); });
    connect(mCircleFill, &QAction::triggered,
            [this] { setCurrentShape(Circle); });

    languageChanged();
}

void ShapeFillTool::mousePressed(QGraphicsSceneMouseEvent *event)
{
    AbstractTileFillTool::mousePressed(event);
    if (event->isAccepted())
        return;

    if (mToolBehavior != Free)
        return;

    if (event->button() == Qt::LeftButton) {
        QPoint pos = tilePosition();
        TileLayer *tileLayer = currentTileLayer();
        if (!tileLayer)
            return;

        mStartCorner = pos;
        mToolBehavior = MakingShape;
    }
}

void ShapeFillTool::mouseReleased(QGraphicsSceneMouseEvent *event)
{
    AbstractTileFillTool::mouseReleased(event);

    if (mToolBehavior != MakingShape)
        return;

    if (event->button() == Qt::LeftButton) {
        mToolBehavior = Free;

        TileLayer *tileLayer = currentTileLayer();
        if (!tileLayer)
            return;

        if (!brushItem()->isVisible() || !tileLayer->isUnlocked())
            return;

        const TileLayer *preview = mFillOverlay.data();
        if (!preview)
            return;

        PaintTileLayer *paint = new PaintTileLayer(mapDocument(),
                                                   tileLayer,
                                                   preview->x(),
                                                   preview->y(),
                                                   preview);

        paint->setText(QCoreApplication::translate("Undo Commands", "Shape Fill"));

        if (!mMissingTilesets.isEmpty()) {
            for (const SharedTileset &tileset : mMissingTilesets) {
                if (!mapDocument()->map()->tilesets().contains(tileset))
                    new AddTileset(mapDocument(), tileset, paint);
            }

            mMissingTilesets.clear();
        }

        QRegion fillRegion(mFillRegion);
        mapDocument()->undoStack()->push(paint);
        emit mapDocument()->regionEdited(fillRegion, currentTileLayer());

        mFillRegion = QRegion();
        mFillOverlay.clear();
        brushItem()->clear();
    }
}

void ShapeFillTool::modifiersChanged(Qt::KeyboardModifiers)
{
    if (mToolBehavior == MakingShape)
        updateFillOverlay();
}

void ShapeFillTool::languageChanged()
{
    setName(tr("Shape Fill Tool"));
    setShortcut(QKeySequence(tr("P")));

    mRectFill->setToolTip(tr("Rectangle Fill"));
    mCircleFill->setToolTip(tr("Circle Fill"));

    mStampActions->languageChanged();
}

void ShapeFillTool::populateToolBar(QToolBar *toolBar)
{
    AbstractTileFillTool::populateToolBar(toolBar);

    QActionGroup *actionGroup = new QActionGroup(toolBar);
    actionGroup->addAction(mRectFill);
    actionGroup->addAction(mCircleFill);

    toolBar->addSeparator();
    toolBar->addActions(actionGroup->actions());
}

void ShapeFillTool::tilePositionChanged(const QPoint &tilePos)
{
    if (mToolBehavior == MakingShape)
        updateFillOverlay();
    else
        AbstractTileFillTool::tilePositionChanged(tilePos);
}

void ShapeFillTool::setCurrentShape(Shape shape)
{
    mCurrentShape = shape;
    return;
}

void ShapeFillTool::updateFillOverlay()
{
    TileLayer *tileLayer = currentTileLayer();
    if (!tileLayer)
        return;

    int width = tilePosition().x() - mStartCorner.x();
    int height = tilePosition().y() - mStartCorner.y();

    if (QApplication::keyboardModifiers() & Qt::ShiftModifier) {
        int min = std::min(abs(width), abs(height));
        width = ((width > 0) - (width < 0))*min;
        height = ((height > 0) - (height < 0))*min;
    }

    int left = std::min(mStartCorner.x(), mStartCorner.x() + width);
    int top = std::min(mStartCorner.y(), mStartCorner.y() + height);

    QRect boundingRect(left, top, abs(width), abs(height));

    switch (mCurrentShape) {
    case Rect:
        mFillRegion = boundingRect;
        break;
    case Circle:
        mFillRegion = ellipseRegion(mStartCorner.x(),
                                    mStartCorner.y(),
                                    mStartCorner.x() + width,
                                    mStartCorner.y() + height);
        break;
    }

    const QRect fillBound = mFillRegion.boundingRect();
    mFillOverlay = SharedTileLayer::create(QString(),
                                           fillBound.x(),
                                           fillBound.y(),
                                           fillBound.width(),
                                           fillBound.height());

    switch (mFillMethod) {
    case TileFill:
        fillWithStamp(*mFillOverlay,
                      mStamp,
                      mFillRegion.translated(-mFillOverlay->position()));
        break;
    case RandomFill:
        randomFill(*mFillOverlay, mFillRegion);
        break;
    case WangFill:
        wangFill(*mFillOverlay, *tileLayer, mFillRegion);
        break;
    }

    brushItem()->setTileLayer(mFillOverlay);
}
