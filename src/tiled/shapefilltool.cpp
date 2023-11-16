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

#include "actionmanager.h"
#include "brushitem.h"
#include "geometry.h"
#include "mapdocument.h"
#include "stampactions.h"

#include <QActionGroup>
#include <QApplication>
#include <QToolBar>
#include <QUndoStack>

#include <memory>

using namespace Tiled;

ShapeFillTool::ShapeFillTool(QObject *parent)
    : AbstractTileFillTool("ShapeFillTool",
                           tr("Shape Fill Tool"),
                           QIcon(QLatin1String(
                                     ":images/22/rectangle-fill.png")),
                           QKeySequence(Qt::Key_P),
                           parent)
    , mToolBehavior(Free)
    , mCurrentShape(Rect)
    , mRectFill(new QAction(this))
    , mCircleFill(new QAction(this))
{
    QIcon rectFillIcon(QLatin1String(":images/22/rectangle-fill.png"));
    QIcon circleFillIcon(QLatin1String(":images/22/ellipse-fill.png"));

    mRectFill->setIcon(rectFillIcon);
    mRectFill->setCheckable(true);
    mRectFill->setChecked(true);

    mCircleFill->setIcon(circleFillIcon);
    mCircleFill->setCheckable(true);

    ActionManager::registerAction(mRectFill, "ShapeFillTool.RectangleFill");
    ActionManager::registerAction(mCircleFill, "ShapeFillTool.CircleFill");

    connect(mRectFill, &QAction::triggered, this,
            [this] { setCurrentShape(Rect); });
    connect(mCircleFill, &QAction::triggered, this,
            [this] { setCurrentShape(Circle); });

    setActionsEnabled(false);

    ShapeFillTool::languageChanged();
}

void ShapeFillTool::activate(MapScene *scene)
{
    AbstractTileFillTool::activate(scene);
    setActionsEnabled(true);
}

void ShapeFillTool::deactivate(MapScene *scene)
{
    setActionsEnabled(false);
    AbstractTileFillTool::deactivate(scene);
}

void ShapeFillTool::mousePressed(QGraphicsSceneMouseEvent *event)
{
    // Right-click cancels drawing a shape
    if (mToolBehavior == MakingShape && event->button() == Qt::RightButton) {
        mToolBehavior = Free;
        clearOverlay();
        updateStatusInfo();
        return;
    }

    AbstractTileFillTool::mousePressed(event);
    if (event->isAccepted())
        return;

    // Left-click starts drawing a shape
    if (mToolBehavior == Free && event->button() == Qt::LeftButton) {
        mStartCorner = tilePosition();
        mToolBehavior = MakingShape;
        updateFillOverlay();
        updateStatusInfo();
    }
}

void ShapeFillTool::mouseReleased(QGraphicsSceneMouseEvent *event)
{
    AbstractTileFillTool::mouseReleased(event);

    if (mToolBehavior != MakingShape)
        return;

    if (event->button() == Qt::LeftButton) {
        mToolBehavior = Free;

        if (!brushItem()->isVisible())
            return;

        auto preview = mPreviewMap;
        if (!preview)
            return;

        mapDocument()->undoStack()->beginMacro(QCoreApplication::translate("Undo Commands", "Shape Fill"));
        mapDocument()->paintTileLayers(*preview, false, &mMissingTilesets);
        mapDocument()->undoStack()->endMacro();

        clearOverlay();
        updateStatusInfo();
    }
}

void ShapeFillTool::modifiersChanged(Qt::KeyboardModifiers modifiers)
{
    mModifiers = modifiers;

    if (mToolBehavior == MakingShape)
        updateFillOverlay();
}

void ShapeFillTool::languageChanged()
{
    setName(tr("Shape Fill Tool"));

    mRectFill->setText(tr("Rectangle Fill"));
    mCircleFill->setText(tr("Circle Fill"));

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

void ShapeFillTool::tilePositionChanged(QPoint tilePos)
{
    if (mToolBehavior == MakingShape)
        updateFillOverlay();
    else
        AbstractTileFillTool::tilePositionChanged(tilePos);
}

void ShapeFillTool::updateStatusInfo()
{
    if (!isBrushVisible() || mToolBehavior != MakingShape) {
        AbstractTileFillTool::updateStatusInfo();
        return;
    }

    const QPoint pos = tilePosition();
    setStatusInfo(tr("%1, %2 - %3: (%4 x %5)")
                  .arg(pos.x()).arg(pos.y())
                  .arg(mCurrentShape == Rect ? tr("Rectangle") : tr("Circle"))
                  .arg(mFillBounds.width()).arg(mFillBounds.height()));
}

void ShapeFillTool::setActionsEnabled(bool enabled)
{
    mRectFill->setEnabled(enabled);
    mCircleFill->setEnabled(enabled);
}

void ShapeFillTool::setCurrentShape(Shape shape)
{
    mCurrentShape = shape;
}

void ShapeFillTool::updateFillOverlay()
{
    int dx = tilePosition().x() - mStartCorner.x();
    int dy = tilePosition().y() - mStartCorner.y();

    if (mModifiers & Qt::ShiftModifier) {
        const int min = std::min(std::abs(dx), std::abs(dy));
        dx = ((dx > 0) - (dx < 0)) * min;
        dy = ((dy > 0) - (dy < 0)) * min;
    }

    const bool alt = mModifiers & Qt::AltModifier;
    const QPoint p1 = alt ? mStartCorner - QPoint(dx, dy)
                          : mStartCorner;
    const QPoint p2 = mStartCorner + QPoint(dx, dy);

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QRect area = QRect(p1, p2).normalized();
    if (area.width() == 0)
        area.adjust(-1, 0, 1, 0);
    if (area.height() == 0)
        area.adjust(0, -1, 0, 1);
#else
    QRect area = QRect::span(p1, p2);
#endif

    switch (mCurrentShape) {
    case Rect:
        updatePreview(area);
        break;
    case Circle:
        updatePreview(ellipseRegion(area));
        break;
    }
}

#include "moc_shapefilltool.cpp"
