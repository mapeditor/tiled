/*
 * abstracttileselectiontool.h
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


#include "abstracttiletool.h"

class QAction;
class QActionGroup;

namespace Tiled {

class MapDocument;

class AbstractTileSelectionTool : public AbstractTileTool
{
    Q_OBJECT

public:
    AbstractTileSelectionTool(Id id,
                              const QString &name,
                              const QIcon &icon,
                              const QKeySequence &shortcut,
                              QObject *parent = nullptr);

    void mousePressed(QGraphicsSceneMouseEvent *event) override;
    void mouseReleased(QGraphicsSceneMouseEvent *event) override;

    void modifiersChanged(Qt::KeyboardModifiers modifiers) override;

    void languageChanged() override;

    void populateToolBar(QToolBar *toolBar) override;

protected:
    enum SelectionMode {
        Replace,
        Add,
        Subtract,
        Intersect
    };

    SelectionMode selectionMode() const { return mSelectionMode; }

    QRegion selectedRegion() const { return mSelectedRegion; }
    void setSelectedRegion(QRegion region) { mSelectedRegion = region; }

    void updateBrushVisibility() override;

private:
    SelectionMode mSelectionMode;
    SelectionMode mDefaultMode;

    QRegion mSelectedRegion;

    QAction *mReplace;
    QAction *mAdd;
    QAction *mSubtract;
    QAction *mIntersect;
    QActionGroup *mActionGroup;
};

} // namespace Tiled
