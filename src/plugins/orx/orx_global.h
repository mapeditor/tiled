/*
 * Orx Engine Tiled Plugin
 * Copyright 2017, Denis Brachet aka Ainvar <thegwydd@gmail.com>
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

#ifndef ORX_GLOBAL_H
#define ORX_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(ORX_LIBRARY)
#  define ORX_SHARED_EXPORT Q_DECL_EXPORT
#else
#  define ORX_SHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // ORX_GLOBAL_H
