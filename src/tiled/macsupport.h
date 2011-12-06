/*
 * macsupport.h
 * Copyright 2011, Vsevolod Klementjev <klemix@inbox.lv>
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

#ifndef MACSUPPORT_H
#define MACSUPPORT_H

#include "mainwindow.h"

class MacSupport
{
public:
    /**
     * Returns whether the current system is Lion.
     */
    static bool isLion();

    /**
     * Adds fullscreen button to window for Lion.
     */
    static void addFullscreen(Tiled::Internal::MainWindow *window);
};

#endif // MACSUPPORT_H
