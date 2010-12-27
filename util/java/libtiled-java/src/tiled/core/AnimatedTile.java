/*
 * Copyright 2004-2006, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
 * Copyright 2004-2006, Adam Turk <aturk@biggeruniverse.com>
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

package tiled.core;

/**
 * Animated tiles take advantage of the Sprite class internally to handle
 * animation using an array of tiles.
 *
 * @see tiled.core.Sprite
 */
public class AnimatedTile extends Tile
{
    private Sprite sprite;

    public AnimatedTile() {
    }

    public AnimatedTile(TileSet set) {
        super(set);
    }

    public AnimatedTile(Tile[] frames) {
        this();
        sprite = new Sprite(frames);
    }

    public AnimatedTile(Sprite s) {
        this();
        setSprite(s);
    }

    public void setSprite(Sprite s) {
        sprite = s;
    }

    public int countAnimationFrames() {
        return sprite.getTotalFrames();
    }

    public int countKeys() {
        return sprite.getTotalKeys();
    }

    public Sprite getSprite() {
        return sprite;
    }
}
