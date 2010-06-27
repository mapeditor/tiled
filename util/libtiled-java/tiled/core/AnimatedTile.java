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

import java.awt.Graphics;
import java.awt.Image;
import java.util.Iterator;

import tiled.core.Sprite.KeyFrame;

/**
 * Animated tiles take advantage of the Sprite class internally to handle
 * animation using an array of tiles.
 *
 * @see tiled.core.Sprite
 */
public class AnimatedTile extends Tile {

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

    /**
     * In an AnimatedTile we must take the extra step of zooming all frames of
     * the animation. This function can be somewhat slower than calling
     * getScaledImage() on a Tile, but it depends on several factors.
     *
     * @see tiled.core.Tile#getScaledImage(double)
     */
    public Image getScaledImage(double zoom) {
        try {
            Iterator<KeyFrame> itr = sprite.getKeys();

            while (itr.hasNext()) {
                KeyFrame key = itr.next();
                for (int i = 0;i < key.getTotalFrames(); i++) {
                    key.getFrame(i).getScaledImage(zoom);
                }
            }
        } catch (Exception e) {}
        return sprite.getCurrentFrame().getScaledImage(zoom);
    }

    /**
     * Handles drawing the correct frame, and iterating by the
     * frame rate
     *
     * @see tiled.core.Tile#draw(Graphics, int, int, double)
     */
    public void draw(Graphics g, int x, int y, double zoom) {
        sprite.getCurrentFrame().draw(g, x, y, zoom);
        sprite.iterateFrame();
    }
}
