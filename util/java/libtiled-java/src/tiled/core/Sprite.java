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

import java.awt.Image;
import java.awt.Rectangle;
import java.util.Iterator;
import java.util.Vector;

public class Sprite
{
    private Vector<KeyFrame> keys;
    private int borderWidth = 0;
    private int fpl = 0;
    private int totalKeys = -1;

    private float currentFrame = 0;
    private Rectangle frameSize;
    private boolean bPlaying = true;

    public class KeyFrame
    {
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

        public String toString() {
            return "(" + name + ")" + id + ": @ " + frameRate;
        }
    }

    private KeyFrame currentKey = null;

    public Sprite() {
        frameSize = new Rectangle();
        keys = new Vector<KeyFrame>();
    }

    public Sprite(Tile[] frames) {
        setFrames(frames);
    }

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

    public void setFrames(Tile[] frames) {
        frameSize = new Rectangle(0, 0, frames[0].getWidth(), frames[0].getHeight());

        createKey("", frames, KeyFrame.KEY_LOOP);
    }

    public void setFrameSize(int w, int h) {
        frameSize.width = w;
        frameSize.height = h;
    }

    public void setBorderWidth(int b) {
        borderWidth = b;
    }

    public void setFpl(int f) {
        fpl = f;
    }

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

    public void setTotalKeys(int t) {
        totalKeys = t;
    }

    public Rectangle getFrameSize() {
        return frameSize;
    }

    public int getTotalFrames() {
        int total = 0;
        for (KeyFrame key : keys) {
            total += key.getTotalFrames();
        }

        return total;
    }

    public int getBorderWidth() {
        return borderWidth;
    }

    public Tile getCurrentFrame() {
        return currentKey.getFrame((int) currentFrame);
    }

    public KeyFrame getNextKey() {
        Iterator<KeyFrame> itr = keys.iterator();
        while (itr.hasNext()) {
            KeyFrame k = itr.next();
            if (k == currentKey) {
                if (itr.hasNext())
                    return itr.next();
            }
        }

        return keys.get(0);
    }

    public KeyFrame getPreviousKey() {
        //TODO: this
        return null;
    }

    public KeyFrame getCurrentKey() {
        return currentKey;
    }

    public int getFPL() {
        return fpl;
    }

    public int getTotalKeys() {
        return keys.size();
    }

    public void setKeyFrameTo(String name) {
        for (KeyFrame k : keys) {
            if (k.equalsIgnoreCase(name)) {
                currentKey = k;
                break;
            }
        }
    }


    public void addKey(KeyFrame k) {
        keys.add(k);
    }

    public void removeKey(String name) {
        keys.remove(getKey(name));
    }

    public void createKey(String name, Tile[] frames, int flags) {
        KeyFrame kf = new KeyFrame(name, frames);
        kf.setName(name);
        kf.setFlags(flags);
        addKey(kf);
    }

    public void iterateFrame() {

        if (currentKey != null) {
            if (bPlaying) {
                setCurrentFrame(currentFrame + currentKey.getFrameRate());
            }
        }
    }

    /**
     * Sets the current frame relative to the starting frame of the
     * current key.
     *
     * @param c
     */
    public void keySetFrame(int c) {
        setCurrentFrame(c);
    }

    public void play() {
        bPlaying = true;
    }

    public void stop() {
        bPlaying = false;
    }

    public void keyStepBack(int amt) {
        setCurrentFrame(currentFrame - amt);
    }

    public void keyStepForward(int amt) {
        setCurrentFrame(currentFrame + amt);
    }

    public KeyFrame getKey(String keyName) {
        for (KeyFrame k : keys) {
            if (k != null && k.equalsIgnoreCase(keyName)) {
                return k;
            }
        }
        return null;
    }

    public KeyFrame getKey(int i) {
        return keys.get(i);
    }

    public Iterator<KeyFrame> getKeys() throws Exception {
        return keys.iterator();
    }

    public Rectangle getCurrentFrameRect() {
        int x = 0, y = 0;

        if (frameSize.height > 0 && frameSize.width > 0) {
            y = ((int) currentFrame / fpl) * (frameSize.height + borderWidth);
            x = ((int) currentFrame % fpl) * (frameSize.width + borderWidth);
        }

        return new Rectangle(x, y, frameSize.width, frameSize.height);
    }

    /**
     * @see Object#toString()
     */
    public String toString() {
        return "Frame: (" + frameSize.width + "x" + frameSize.height + ")\n" +
                "Border: " + borderWidth + "\n" +
                "FPL: " + fpl + "\n" +
                "Total Frames: " + getTotalFrames() + "\n" +
                "Total keys: " + totalKeys;
    }
}

