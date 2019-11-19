/*
 * abstracttileselectiontool.cpp
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

#include "abstracttileselectiontool.h"

#include "brushitem.h"
#include "changeselectedarea.h"
#include "mapdocument.h"

#include <QAction>
#include <QActionGroup>
#include <QApplication>
#include <QToolBar>

using namespace Tiled;

AbstractTileSelectionTool::AbstractTileSelectionTool(Id id,
                                                     const QString &name,
                                                     const QIcon &icon,
                                                     const QKeySequence &shortcut,
                                                     QObject *parent)
    : AbstractTileTool(id, name, icon, shortcut, nullptr, parent)
    , mSelectionMode(Replace)
    , mDefaultMode(Replace)
{
    QIcon replaceIcon(QLatin1String(":images/16/selection-replace.png"));
    QIcon addIcon(QLatin1String(":images/16/selection-add.png"));
    QIcon subtractIcon(QLatin1String(":images/16/selection-subtract.png"));
    QIcon intersectIcon(QLatin1String(":images/16/selection-intersect.png"));

    mReplace = new QAction(this);
    mReplace->setIcon(replaceIcon);
    mReplace->setCheckable(true);
    mReplace->setChecked(true);

    mAdd = new QAction(this);
    mAdd->setIcon(addIcon);
    mAdd->setCheckable(true);

    mSubtract = new QAction(this);
    mSubtract->setIcon(subtractIcon);
    mSubtract->setCheckable(true);

    mIntersect = new QAction(this);
    mIntersect->setIcon(intersectIcon);
    mIntersect->setCheckable(true);

    mActionGroup = new QActionGroup(this);
    mActionGroup->addAction(mReplace);
    mActionGroup->addAction(mAdd);
    mActionGroup->addAction(mSubtract);
    mActionGroup->addAction(mIntersect);

    connect(mReplace, &QAction::triggered,
            [this] { mSelectionMode = mDefaultMode = Replace; });
    connect(mAdd, &QAction::triggered,
            [this] { mSelectionMode = mDefaultMode = Add; });
    connect(mSubtract, &QAction::triggered,
            [this] { mSelectionMode = mDefaultMode = Subtract; });
    connect(mIntersect, &QAction::triggered,
            [this] { mSelectionMode = mDefaultMode = Intersect; });

    AbstractTileSelectionTool::languageChanged();
}

void AbstractTileSelectionTool::mousePressed(QGraphicsSceneMouseEvent *event)
{
    const Qt::MouseButton button = event->button();

    if (button == Qt::LeftButton || (button == Qt::RightButton && event->modifiers() == Qt::NoModifier)) {
        MapDocument *document = mapDocument();
        QRegion selection;

        // Left button modifies selection, right button clears selection
        if (button == Qt::LeftButton) {
            selection = document->selectedArea();

            switch (mSelectionMode) {
            case Replace:   selection = mSelectedRegion; break;
            case Add:       selection += mSelectedRegion; break;
            case Subtract:  selection -= mSelectedRegion; break;
            case Intersect: selection &= mSelectedRegion; break;
            }
        }

        if (selection != document->selectedArea()) {
            QUndoCommand *cmd = new ChangeSelectedArea(document, selection);
            document->undoStack()->push(cmd);
        }

        return;
    }

    AbstractTileTool::mousePressed(event);
}

void AbstractTileSelectionTool::mouseReleased(QGraphicsSceneMouseEvent *)
{
}

void AbstractTileSelectionTool::modifiersChanged(Qt::KeyboardModifiers modifiers)
{
    if (modifiers == Qt::ControlModifier)
        mSelectionMode = Subtract;
    else if (modifiers == Qt::ShiftModifier)
        mSelectionMode = Add;
    else if (modifiers == (Qt::ControlModifier | Qt::ShiftModifier))
        mSelectionMode = Intersect;
    else
        mSelectionMode = mDefaultMode;

    switch (mSelectionMode) {
    case Replace:   mReplace->setChecked(true); break;
    case Add:       mAdd->setChecked(true); break;
    case Subtract:  mSubtract->setChecked(true); break;
    case Intersect: mIntersect->setChecked(true); break;
    }
}

void AbstractTileSelectionTool::languageChanged()
{
    mReplace->setToolTip(tr("Replace Selection"));
    mAdd->setToolTip(tr("Add Selection"));
    mSubtract->setToolTip(tr("Subtract Selection"));
    mIntersect->setToolTip(tr("Intersect Selection"));
}

void AbstractTileSelectionTool::populateToolBar(QToolBar *toolBar)
{
    toolBar->addAction(mReplace);
    toolBar->addAction(mAdd);
    toolBar->addAction(mSubtract);
    toolBar->addAction(mIntersect);
}

// Override to ignore whether the current layer is a visible tile layer
void AbstractTileSelectionTool::updateBrushVisibility()
{
    brushItem()->setVisible(isBrushVisible());
}
