/*
 *  Tiled Map Editor, (c) 2004-2006
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
 * This exception is thrown when an attempt is made to perform a modification
 * on a locked layer.
 *
 * @version $Id$
 */
public class LayerLockedException extends Throwable {
    public LayerLockedException(String s) {
        super(s);
    }
}
