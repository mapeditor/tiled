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
 * Gets notified about changes made to a map. This includes events relating to
 * changes in the list of layers or tilesets used by this map.
 *
 * @version $Id$
 */
public interface MapChangeListener extends EventListener
{
    public void mapChanged(MapChangedEvent e);

    public void tilesetAdded(MapChangedEvent e, TileSet tileset);

    public void tilesetRemoved(MapChangedEvent e, int index);

    public void tilesetsSwapped(MapChangedEvent e, int index0, int index1);
}
