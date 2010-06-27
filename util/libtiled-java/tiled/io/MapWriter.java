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

package tiled.io;

import java.io.FileFilter;
import java.io.OutputStream;

import tiled.core.Map;
import tiled.core.TileSet;

/**
 * Used by Tiled to denote a plugin for writing maps. The map file
 * can have any format, as long as the MapWriter implementor accepts
 * instances of {@link tiled.core.Map} and {@link TileSet}.
 *
 * $Id$
 */
public interface MapWriter extends PluggableMapIO, FileFilter
{
    /**
     * Saves a map to a file.
     *
     * @param map
     * @param filename the filename of the map file
     * @throws Exception
     */
    public void writeMap(Map map, String filename) throws Exception;

    /**
     * Saves a tileset to a file.
     *
     * @param set
     * @param filename the filename of the tileset file
     * @throws Exception
     */
    public void writeTileset(TileSet set, String filename) throws Exception;

    /**
     * Writes a map to an already opened stream. Useful
     * for maps which are part of a larger binary dataset
     *
     * @param map the Map to be written
     * @param out
     * @throws Exception
     */
    public void writeMap(Map map, OutputStream out) throws Exception;

    /**
     * Overload this to write a tileset to an open stream.
     *
     * @param set
     * @param out
     * @throws Exception
     */
    public void writeTileset(TileSet set, OutputStream out) throws Exception;
}
