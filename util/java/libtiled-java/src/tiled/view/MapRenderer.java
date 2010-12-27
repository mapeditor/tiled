/*
 * Copyright 2010, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
 *
 * This file is part of libtiled-java.
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1 of the License, or (at
 * your option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public
 * License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library;  If not, see <http://www.gnu.org/licenses/>.
 */

package tiled.view;

import tiled.core.TileLayer;

import java.awt.*;

/**
 * An interface defining methods to render a map.
 */
public interface MapRenderer
{
    /**
     * Calculates the dimensions of the map this renderer applies to.
     *
     * @return the dimensions of the given map in pixels
     */
    public Dimension getMapSize();

    /**
     * Paints the given tile layer, taking into account the clip rect of the
     * given graphics context.
     *
     * @param g     the graphics context to paint to
     * @param layer the layer to paint
     */
    public void paintTileLayer(Graphics2D g, TileLayer layer);
}
