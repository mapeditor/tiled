/*
 * Replica Island Tiled Plugin
 * Copyright 2011, Eric Kidd <eric@kiddsoftware.com>
 * Copyright 2011, seeseekey <seeseekey@googlemail.com>
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

#ifndef REPLICAISLAND_GLOBAL_H
#define REPLICAISLAND_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(REPLICAISLAND_LIBRARY)
#  define REPLICAISLANDSHARED_EXPORT Q_DECL_EXPORT
#  define REPLICAISLANDSHARED_IMPORT Q_DECL_EXPORT
#else
#  define REPLICAISLANDSHARED_EXPORT Q_DECL_IMPORT
#  define REPLICAISLANDSHARED_IMPORT Q_DECL_IMPORT
#endif

#endif // REPLICAISLAND_GLOBAL_H
