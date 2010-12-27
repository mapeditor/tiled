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

import tiled.core.Map;
import tiled.core.Tile;
import tiled.core.TileLayer;

import java.awt.*;

/**
 * The orthogonal map renderer. This is the most basic map renderer, dealing
 * with maps that use rectangular tiles.
 */
public class OrthogonalRenderer implements MapRenderer
{
    private final Map map;

    public OrthogonalRenderer(Map map) {
        this.map = map;
    }

    public Dimension getMapSize() {
        return new Dimension(
                map.getWidth() * map.getTileWidth(),
                map.getHeight() * map.getTileHeight());
    }

    public void paintTileLayer(Graphics2D g, TileLayer layer) {
        final Rectangle clip = g.getClipBounds();
        final int tileWidth = map.getTileWidth();
        final int tileHeight = map.getTileHeight();
        final Rectangle bounds = layer.getBounds();

        g.translate(bounds.x * tileWidth, bounds.y * tileHeight);
        clip.translate(-bounds.x * tileWidth, -bounds.y * tileHeight);

        clip.height += map.getTileHeightMax();

        final int startX = Math.max(0, clip.x / tileWidth);
        final int startY = Math.max(0, clip.y / tileHeight);
        final int endX = Math.min(layer.getWidth(),
                (int) Math.ceil(clip.getMaxX() / tileWidth));
        final int endY = Math.min(layer.getHeight(),
                (int) Math.ceil(clip.getMaxY() / tileHeight));

        for (int x = startX; x < endX; ++x) {
            for (int y = startY; y < endY; ++y) {
                final Tile tile = layer.getTileAt(x, y);
                if (tile == null)
                    continue;
                final Image image = tile.getImage();
                if (image == null)
                    continue;

                g.drawImage(
                        image,
                        x * tileWidth,
                        (y + 1) * tileHeight - image.getHeight(null),
                        null);
            }
        }



        g.translate(-bounds.x * tileWidth, -bounds.y * tileHeight);
    }
}
