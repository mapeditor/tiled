/*-
 * #%L
 * This file is part of libtiled-java.
 * %%
 * Copyright (C) 2004 - 2016 Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
 * Copyright (C) 2004 - 2016 Adam Turk <aturk@biggeruniverse.com>
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
package tiled.core;

/**
 * Animated tiles take advantage of the Sprite class internally to handle
 * animation using an array of tiles.
 *
 * @see tiled.core.Sprite
 * @author Thorbjørn Lindeijer
 * @author Adam Turk
 * @version 0.17
 */
public class AnimatedTile extends Tile {

    private Sprite sprite;

    /**
     * <p>Constructor for AnimatedTile.</p>
     */
    public AnimatedTile() {
    }

    /**
     * <p>Constructor for AnimatedTile.</p>
     *
     * @param set a {@link tiled.core.TileSet} object.
     */
    public AnimatedTile(TileSet set) {
        super(set);
    }

    /**
     * <p>Constructor for AnimatedTile.</p>
     *
     * @param frames an array of {@link tiled.core.Tile} objects.
     */
    public AnimatedTile(Tile[] frames) {
        this();
        sprite = new Sprite(frames);
    }

    /**
     * <p>Constructor for AnimatedTile.</p>
     *
     * @param s a {@link tiled.core.Sprite} object.
     */
    public AnimatedTile(Sprite s) {
        this();
        setSprite(s);
    }

    /**
     * <p>Setter for the field <code>sprite</code>.</p>
     *
     * @param s a {@link tiled.core.Sprite} object.
     */
    public final void setSprite(Sprite s) {
        sprite = s;
    }

    /**
     * <p>countAnimationFrames.</p>
     *
     * @return a int.
     */
    public int countAnimationFrames() {
        return sprite.getTotalFrames();
    }

    /**
     * <p>countKeys.</p>
     *
     * @return a int.
     */
    public int countKeys() {
        return sprite.getTotalKeys();
    }

    /**
     * <p>Getter for the field <code>sprite</code>.</p>
     *
     * @return a {@link tiled.core.Sprite} object.
     */
    public Sprite getSprite() {
        return sprite;
    }
}
