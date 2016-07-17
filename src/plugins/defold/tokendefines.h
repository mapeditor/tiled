/*
 * Defold Tiled Plugin
 * Copyright 2016, Nikita Razdobreev <exzo0mex@gmail.com>
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

#ifndef TOKENDEFINES_H
#define TOKENDEFINES_H

const QString cell_t =
  "cell {\n \
    x: {{x}}\n \
    y: {{y}}\n \
    tile: {{tile}}\n \
    h_flip: {{h_flip}}\n \
    v_flip: {{v_flip}}\n \
}\n";

const QString layer_t =
"layers {\n \
  id: \"{{id}}\"\n \
  z: {{z}}\n \
  is_visible: {{is_visible}}\n \
{{cells}}\n \
  }\n";

const QString map_t =
"tile_set: \"{{tile_set}}\"\n \
{{layers}}\n \
material: \"{{material}}\"\n \
blend_mode: {{blend_mode}}\n";

#endif // TOKENDEFINES_H
