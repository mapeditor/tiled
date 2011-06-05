/*
 * Lua Tiled Plugin
 * Copyright 2011, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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

#ifndef LUA_GLOBAL_H
#define LUA_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(LUA_LIBRARY)
#  define LUASHARED_EXPORT Q_DECL_EXPORT
#else
#  define LUASHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // LUA_GLOBAL_H
