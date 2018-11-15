/*-
 * #%L
 * This file is part of libtiled-java.
 * %%
 * Copyright (C) 2004 - 2017 Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
 * Copyright (C) 2004 - 2017 Adam Turk <aturk@biggeruniverse.com>
 * Copyright (C) 2016 - 2017 Mike Thomas <mikepthomas@outlook.com>
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

import java.awt.Image;
import java.awt.Rectangle;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;

/**
 * <p>Sprite class.</p>
 *
 * @author Thorbjørn Lindeijer
 * @author Adam Turk
 * @author Mike Thomas
 * @version 1.0.2
 */
public class Sprite {

    private List<KeyFrame> keys;
    private int borderWidth = 0;
    private int fpl = 0;
    private int totalKeys = -1;

    private float currentFrame = 0;
    private Rectangle frameSize;
    private boolean bPlaying = true;

    public class KeyFrame {

        public static final int MASK_ANIMATION = 0x0000000F;

        public static final int KEY_LOOP = 0x01;
        public static final int KEY_STOP = 0x02;
        public static final int KEY_AUTO = 0x04;
        public static final int KEY_REVERSE = 0x08;

        public static final int KEY_NAME_LENGTH_MAX = 32;

        private String name = null;
        private int id = -1;
        private int flags = KEY_LOOP;
        private float frameRate = 1.0f;   //one fps
        private Tile[] frames;

        public KeyFrame() {
            flags = KEY_LOOP;
        }

        public KeyFrame(String name) {
            this();
            this.name = name;
        }

        public KeyFrame(String name, Tile[] tile) {
            this(name);
            frames = tile;
        }

        public void setName(String name) {
            this.name = name;
        }

        public void setFrameRate(float r) {
            frameRate = r;
        }

        public void setId(int id) {
            this.id = id;
        }

        public int getId() {
            return id;
        }

        public int getLastFrame() {
            return frames.length - 1;
        }

        public boolean isFrameLast(int frame) {
            return frames.length - 1 == frame;
        }

        public void setFlags(int f) {
            flags = f;
        }

        public int getFlags() {
            return flags;
        }

        public String getName() {
            return name;
        }

        public Tile getFrame(int f) {
            if (f > 0 && f < frames.length) {
                return frames[f];
            }
            return null;
        }

        public float getFrameRate() {
            return frameRate;
        }

        public int getTotalFrames() {
            return frames.length;
        }

        public boolean equalsIgnoreCase(String n) {
            return name != null && name.equalsIgnoreCase(n);
        }

        @Override
        public String toString() {
            return "(" + name + ")" + id + ": @ " + frameRate;
        }
    }

    private KeyFrame currentKey = null;

    /**
     * <p>Constructor for Sprite.</p>
     */
    public Sprite() {
        frameSize = new Rectangle();
        keys = new ArrayList<>();
    }

    /**
     * <p>Constructor for Sprite.</p>
     *
     * @param frames an array of {@link org.mapeditor.core.Tile} objects.
     */
    public Sprite(Tile[] frames) {
        setFrames(frames);
    }

    /**
     * <p>Constructor for Sprite.</p>
     *
     * @param image a {@link java.awt.Image} object.
     * @param fpl a int.
     * @param border a int.
     * @param totalFrames a int.
     */
    public Sprite(Image image, int fpl, int border, int totalFrames) {
        Tile[] frames = null;
        this.fpl = fpl;
        borderWidth = border;

        //TODO: break up the image into tiles
        //given this information, extrapolate the rest...
        frameSize.width = image.getWidth(null) / (fpl + borderWidth * fpl);
        frameSize.height = (int) (image.getHeight(null) / (Math.ceil(totalFrames / fpl) + Math.ceil(totalFrames / fpl) * borderWidth));
        createKey("", frames, KeyFrame.KEY_LOOP);
    }

    /**
     * <p>setFrames.</p>
     *
     * @param frames an array of {@link org.mapeditor.core.Tile} objects.
     */
    public final void setFrames(Tile[] frames) {
        frameSize = new Rectangle(0, 0, frames[0].getWidth(), frames[0].getHeight());

        createKey("", frames, KeyFrame.KEY_LOOP);
    }

    /**
     * <p>Setter for the field <code>frameSize</code>.</p>
     *
     * @param w a int.
     * @param h a int.
     */
    public void setFrameSize(int w, int h) {
        frameSize.width = w;
        frameSize.height = h;
    }

    /**
     * <p>Setter for the field <code>borderWidth</code>.</p>
     *
     * @param b a int.
     */
    public void setBorderWidth(int b) {
        borderWidth = b;
    }

    /**
     * <p>Setter for the field <code>fpl</code>.</p>
     *
     * @param f a int.
     */
    public void setFpl(int f) {
        fpl = f;
    }

    /**
     * <p>Setter for the field <code>currentFrame</code>.</p>
     *
     * @param c a float.
     */
    public void setCurrentFrame(float c) {
        if (c < 0) {
            switch (currentKey.flags & KeyFrame.MASK_ANIMATION) {
                case KeyFrame.KEY_LOOP:
                    currentFrame = currentKey.getLastFrame();
                    break;
                case KeyFrame.KEY_AUTO:
                    currentKey = getPreviousKey();
                    currentFrame = currentKey.getLastFrame();
                    break;
                case KeyFrame.KEY_REVERSE:
                    currentKey.setFrameRate(-currentKey.getFrameRate());
                    currentFrame = 0;
                    break;
                case KeyFrame.KEY_STOP:
                    bPlaying = false;
                    currentFrame = 0;
                    break;
            }
        } else if (c > currentKey.getLastFrame()) {
            switch (currentKey.flags & KeyFrame.MASK_ANIMATION) {
                case KeyFrame.KEY_LOOP:
                    currentFrame = 0;
                    break;
                case KeyFrame.KEY_AUTO:
                    currentFrame = 0;
                    currentKey = getNextKey();
                    break;
                case KeyFrame.KEY_REVERSE:
                    currentKey.setFrameRate(-currentKey.getFrameRate());
                    currentFrame = currentKey.getLastFrame();
                    break;
                case KeyFrame.KEY_STOP:
                    bPlaying = false;
                    currentFrame = currentKey.getLastFrame();
                    break;
            }
        } else {
            currentFrame = c;
        }
    }

    /**
     * <p>Setter for the field <code>totalKeys</code>.</p>
     *
     * @param t a int.
     */
    public void setTotalKeys(int t) {
        totalKeys = t;
    }

    /**
     * <p>Getter for the field <code>frameSize</code>.</p>
     *
     * @return a {@link java.awt.Rectangle} object.
     */
    public Rectangle getFrameSize() {
        return frameSize;
    }

    /**
     * <p>getTotalFrames.</p>
     *
     * @return a int.
     */
    public int getTotalFrames() {
        int total = 0;
        for (KeyFrame key : keys) {
            total += key.getTotalFrames();
        }

        return total;
    }

    /**
     * <p>Getter for the field <code>borderWidth</code>.</p>
     *
     * @return a int.
     */
    public int getBorderWidth() {
        return borderWidth;
    }

    /**
     * <p>Getter for the field <code>currentFrame</code>.</p>
     *
     * @return a {@link org.mapeditor.core.Tile} object.
     */
    public Tile getCurrentFrame() {
        return currentKey.getFrame((int) currentFrame);
    }

    /**
     * <p>getNextKey.</p>
     *
     * @return a {@link org.mapeditor.core.Sprite.KeyFrame} object.
     */
    public KeyFrame getNextKey() {
        Iterator<KeyFrame> itr = keys.iterator();
        while (itr.hasNext()) {
            KeyFrame k = itr.next();
            if (k == currentKey && itr.hasNext()) {
                return itr.next();
            }
        }

        return keys.get(0);
    }

    /**
     * <p>getPreviousKey.</p>
     *
     * @return a {@link org.mapeditor.core.Sprite.KeyFrame} object.
     */
    public KeyFrame getPreviousKey() {
        //TODO: this
        return null;
    }

    /**
     * <p>Getter for the field <code>currentKey</code>.</p>
     *
     * @return a {@link org.mapeditor.core.Sprite.KeyFrame} object.
     */
    public KeyFrame getCurrentKey() {
        return currentKey;
    }

    /**
     * <p>getFPL.</p>
     *
     * @return a int.
     */
    public int getFPL() {
        return fpl;
    }

    /**
     * <p>Getter for the field <code>totalKeys</code>.</p>
     *
     * @return a int.
     */
    public int getTotalKeys() {
        return keys.size();
    }

    /**
     * <p>setKeyFrameTo.</p>
     *
     * @param name a {@link java.lang.String} object.
     */
    public void setKeyFrameTo(String name) {
        for (KeyFrame k : keys) {
            if (k.equalsIgnoreCase(name)) {
                currentKey = k;
                break;
            }
        }
    }

    /**
     * <p>addKey.</p>
     *
     * @param k a {@link org.mapeditor.core.Sprite.KeyFrame} object.
     */
    public void addKey(KeyFrame k) {
        keys.add(k);
    }

    /**
     * <p>removeKey.</p>
     *
     * @param name a {@link java.lang.String} object.
     */
    public void removeKey(String name) {
        keys.remove(getKey(name));
    }

    /**
     * <p>createKey.</p>
     *
     * @param name a {@link java.lang.String} object.
     * @param frames an array of {@link org.mapeditor.core.Tile} objects.
     * @param flags a int.
     */
    public final void createKey(String name, Tile[] frames, int flags) {
        KeyFrame kf = new KeyFrame(name, frames);
        kf.setName(name);
        kf.setFlags(flags);
        addKey(kf);
    }

    /**
     * <p>iterateFrame.</p>
     */
    public void iterateFrame() {
        if (currentKey != null && bPlaying) {
            setCurrentFrame(currentFrame + currentKey.getFrameRate());
        }
    }

    /**
     * Sets the current frame relative to the starting frame of the current key.
     *
     * @param c a int.
     */
    public void keySetFrame(int c) {
        setCurrentFrame(c);
    }

    /**
     * <p>play.</p>
     */
    public void play() {
        bPlaying = true;
    }

    /**
     * <p>stop.</p>
     */
    public void stop() {
        bPlaying = false;
    }

    /**
     * <p>keyStepBack.</p>
     *
     * @param amt a int.
     */
    public void keyStepBack(int amt) {
        setCurrentFrame(currentFrame - amt);
    }

    /**
     * <p>keyStepForward.</p>
     *
     * @param amt a int.
     */
    public void keyStepForward(int amt) {
        setCurrentFrame(currentFrame + amt);
    }

    /**
     * <p>getKey.</p>
     *
     * @param keyName a {@link java.lang.String} object.
     * @return a {@link org.mapeditor.core.Sprite.KeyFrame} object.
     */
    public KeyFrame getKey(String keyName) {
        for (KeyFrame k : keys) {
            if (k != null && k.equalsIgnoreCase(keyName)) {
                return k;
            }
        }
        return null;
    }

    /**
     * <p>getKey.</p>
     *
     * @param i a int.
     * @return a {@link org.mapeditor.core.Sprite.KeyFrame} object.
     */
    public KeyFrame getKey(int i) {
        return keys.get(i);
    }

    /**
     * <p>Getter for the field <code>keys</code>.</p>
     *
     * @return a {@link java.util.Iterator} object.
     * @throws java.lang.Exception if any.
     */
    public Iterator<KeyFrame> getKeys() throws Exception {
        return keys.iterator();
    }

    /**
     * <p>getCurrentFrameRect.</p>
     *
     * @return a {@link java.awt.Rectangle} object.
     */
    public Rectangle getCurrentFrameRect() {
        int x = 0, y = 0;

        if (frameSize.height > 0 && frameSize.width > 0) {
            y = ((int) currentFrame / fpl) * (frameSize.height + borderWidth);
            x = ((int) currentFrame % fpl) * (frameSize.width + borderWidth);
        }

        return new Rectangle(x, y, frameSize.width, frameSize.height);
    }

    /** {@inheritDoc} */
    @Override
    public String toString() {
        return "Frame: (" + frameSize.width + "x" + frameSize.height + ")\n"
                + "Border: " + borderWidth + "\n"
                + "FPL: " + fpl + "\n"
                + "Total Frames: " + getTotalFrames() + "\n"
                + "Total keys: " + totalKeys;
    }
}
