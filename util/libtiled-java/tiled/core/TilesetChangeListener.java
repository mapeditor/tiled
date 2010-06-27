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

import java.util.EventListener;

/**
 * A tileset listener can get notified about changes to a tileset through
 * receiving {@link TilesetChangedEvent}s.
 *
 * @version $Id$
 */
public interface TilesetChangeListener extends EventListener
{
    void tilesetChanged(TilesetChangedEvent event);

    void nameChanged(TilesetChangedEvent event, String oldName, String newName);

    void sourceChanged(TilesetChangedEvent event, String oldSource, String newSource);
}
