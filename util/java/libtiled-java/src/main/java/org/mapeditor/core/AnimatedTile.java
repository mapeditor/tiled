/*-
 * #%L
 * This file is part of libtiled-java.
 * %%
 * Copyright (C) 2004 - 2020 Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
 * Copyright (C) 2004 - 2020 Adam Turk <aturk@biggeruniverse.com>
 * Copyright (C) 2016 - 2020 Mike Thomas <mikepthomas@outlook.com>
 * %%
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDERS OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 * #L%
 */
package org.mapeditor.core;

import java.awt.image.BufferedImage;
import java.util.List;

/**
 * Animated tiles take advantage of the Sprite class internally to handle
 * animation using an array of tiles.
 *
 * @see org.mapeditor.core.Sprite
 * @version 1.4.2
 */
public class AnimatedTile extends Tile {

    private static final int DEFAULT_FRAME_DURATION_MS = 100;

    private Sprite sprite;
    private final long animationStartTimeMs = System.currentTimeMillis();

    /**
     * Constructor for AnimatedTile.
     */
    public AnimatedTile() {
    }

    /**
     * Constructor for AnimatedTile.
     *
     * @param set a {@link org.mapeditor.core.TileSet} object.
     */
    public AnimatedTile(TileSet set) {
        super(set);
    }

    /**
     * Constructor for AnimatedTile.
     *
     * @param frames an array of {@link org.mapeditor.core.Tile} objects.
     */
    public AnimatedTile(Tile[] frames) {
        this();
        sprite = new Sprite(frames);
    }

    /**
     * Constructor for AnimatedTile.
     *
     * @param s a {@link org.mapeditor.core.Sprite} object.
     */
    public AnimatedTile(Sprite s) {
        this();
        setSprite(s);
    }

    /**
     * Setter for the field <code>sprite</code>.
     *
     * @param s a {@link org.mapeditor.core.Sprite} object.
     */
    public final void setSprite(Sprite s) {
        sprite = s;
    }

    /**
     * countAnimationFrames.
     *
     * @return a int.
     */
    public int countAnimationFrames() {
        return sprite.getTotalFrames();
    }

    /**
     * countKeys.
     *
     * @return a int.
     */
    public int countKeys() {
        return sprite.getTotalKeys();
    }

    /**
     * Getter for the field <code>sprite</code>.
     *
     * @return a {@link org.mapeditor.core.Sprite} object.
     */
    public Sprite getSprite() {
        return sprite;
    }

    /** {@inheritDoc} */
    @Override
    public BufferedImage getImage() {
        final Animation animation = getAnimation();
        if (animation != null && animation.getFrame() != null && !animation.getFrame().isEmpty()) {
            final TileSet tileSet = getTileSet();
            if (tileSet == null) {
                return super.getImage();
            }

            final List<Frame> frames = animation.getFrame();
            int totalDuration = 0;
            for (Frame frame : frames) {
                int duration = frame.getDuration() != null ? frame.getDuration() : DEFAULT_FRAME_DURATION_MS;
                if (duration > 0) {
                    totalDuration += duration;
                }
            }
            if (totalDuration <= 0) {
                return super.getImage();
            }

            final long elapsed = (System.currentTimeMillis() - animationStartTimeMs) % totalDuration;
            long time = 0;
            for (Frame frame : frames) {
                int duration = frame.getDuration() != null ? frame.getDuration() : DEFAULT_FRAME_DURATION_MS;
                if (duration <= 0) {
                    duration = DEFAULT_FRAME_DURATION_MS;
                }
                time += duration;
                if (elapsed < time) {
                    Tile frameTile = tileSet.getTile(frame.getTileid());
                    if (frameTile != null && frameTile != this) {
                        return frameTile.getImage();
                    }
                    break;
                }
            }
        }

        if (sprite != null && sprite.getCurrentKey() != null) {
            return sprite.getCurrentFrame().getImage();
        }
        return super.getImage();
    }
}
