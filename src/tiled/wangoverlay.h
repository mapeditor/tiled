/*
 * wangoverlay.h
 * Copyright 2020, Thorbjørn Lindeijer <bjorn@lindeijer.nl>
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

#include "wangset.h"

#include <QIcon>

class QPainter;
class QRect;

namespace Tiled {

enum WangOverlayOption {
    WO_TransparentFill  = 0x1,
    WO_Shadow           = 0x2,
    WO_Outline          = 0x4,
};
Q_DECLARE_FLAGS(WangOverlayOptions, WangOverlayOption)
Q_DECLARE_OPERATORS_FOR_FLAGS(WangOverlayOptions);

void paintWangOverlay(QPainter *painter,
                      WangId wangId,
                      const WangSet &wangSet,
                      const QRect &rect,
                      WangOverlayOptions options = WO_TransparentFill | WO_Shadow | WO_Outline);

QIcon wangSetIcon(WangSet::Type type);

} // namespace Tiled
