/*
 *  Tiled Map Editor, (c) 2004-2008
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  Adam Turk <aturk@biggeruniverse.com>
 *  Bjorn Lindeijer <bjorn@lindeijer.nl>
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
