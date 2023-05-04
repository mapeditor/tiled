/*
 * selectsametiletool.h
 * Copyright 2015, Mamed Ibrahimov <ibramlab@gmail.com>
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


#include "abstracttileselectiontool.h"

#include "tilelayer.h"

namespace Tiled {

class MapDocument;

/**
 * Implements a tool that selects a region with all similar tiles on the layer.
 */
class SelectSameTileTool : public AbstractTileSelectionTool
{
    Q_OBJECT

public:
    SelectSameTileTool(QObject *parent = nullptr);

    void languageChanged() override;

protected:
    void tilePositionChanged(QPoint tilePos) override;
};

} // namespace Tiled
