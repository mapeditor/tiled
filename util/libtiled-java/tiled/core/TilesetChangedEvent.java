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

import java.util.EventObject;

/**
 * An event indicating that a certain tileset changed.
 *
 * @version $Id$
 */
public class TilesetChangedEvent extends EventObject
{
    public TilesetChangedEvent(TileSet set) {
        super(set);
    }

    public TileSet getTileset() {
        return (TileSet) getSource();
    }
}
