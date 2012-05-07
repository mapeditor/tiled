/*
 * Copyright 2010, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
 *
 * This file is part of libtiled-java.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *    1. Redistributions of source code must retain the above copyright notice,
 *       this list of conditions and the following disclaimer.
 *
 *    2. Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE CONTRIBUTORS ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
 * EVENT SHALL THE CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
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
