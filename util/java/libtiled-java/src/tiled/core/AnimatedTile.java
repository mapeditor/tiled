/*
 * Copyright 2004-2006, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
 * Copyright 2004-2006, Adam Turk <aturk@biggeruniverse.com>
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
