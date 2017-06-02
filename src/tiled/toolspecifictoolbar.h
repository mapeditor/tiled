/*
 * toolspecifictoolbar.h
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

#pragma once

#include <QIcon>
#include <QToolBar>

class QToolButton;

namespace Tiled {
namespace Internal {

class AbstractTool;
class MapEditor;

class ToolSpecificToolBar : public QToolBar
{
public:
    ToolSpecificToolBar(QWidget *parent = nullptr, MapEditor *mapEditor = nullptr);

protected:
    void changeEvent(QEvent *event) override;

public slots:
	void setSelectedTool(AbstractTool *tool);

private slots:
    void onOrientationChanged(Qt::Orientation orientation);

private:
	void addRandomTool(bool checked = false);

	void addFlippingTools();

	void addRotatingTools();

    void retranslateUi();

    MapEditor *mMapEditor;

    QIcon mDiceIcon;
    QIcon mFlipHorizontalIcon;
	QIcon mFlipVerticalIcon;
	QIcon mRotateLeftIcon;
	QIcon mRotateRightIcon;

	QList<QToolButton*> mButtons;
};

} // namespace Internal
} // namespace Tiled
