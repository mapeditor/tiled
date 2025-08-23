/*
 * BinMap File Format
 * Copyright 2025, bzt <bztsrc@gitlab>
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
 *
 * Simplest format possible. Consist of a 16 bytes long header and then data.
 * See FORMAT.md for details.
 */

extern "C" {

#include <stdint.h>

typedef struct {
    char magic[3];          /* 00 magic "MAP" */
    uint8_t version;        /* 03 format version */
    uint16_t mapWidth;      /* 04 map width */
    uint16_t mapHeight;     /* 06 map height */
    uint16_t numLayers;     /* 08 number of layers */
    uint16_t tileWidth;     /* 0A tile width */
    uint16_t tileHeight;    /* 0C tile height */
    uint16_t orientation;   /* 0E orientation (see O_* enums) */
} __attribute__((packed)) binmap_header_t;

enum { O_ORTHO, O_ISO, O_HEXH, O_HEXV };

/* Header followed by mapWidth * mapHeight * numLayers uint16_t and that's it. */

};
