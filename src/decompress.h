/*
 * Tiled Map Editor (Qt port)
 * Copyright 2008 Tiled (Qt port) developers (see AUTHORS file)
 *
 * This file is part of Tiled (Qt port).
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
 * this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 * Place, Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef DECOMPRESS_H
#define DECOMPRESS_H

class QByteArray;

namespace Tiled {
namespace Internal {

/**
 * Decompresses either zlib or gzip compressed memory. Returns a null
 * QByteArray if decompressing failed.
 *
 * Needed because qUncompress does not support gzip compressed data. Also,
 * this method does not need the expected size to be prepended to the data,
 * but it can be passed as optional parameter.
 *
 * @param data         the compressed data
 * @param expectedSize the expected size of the uncompressed data in bytes
 * @return the uncompressed data, or a null QByteArray if decompressing failed
 */
QByteArray decompress(const QByteArray &data, int expectedSize = 1024);

} // namespace Internal
} // namespace Tiled

#endif // DECOMPRESS_H
