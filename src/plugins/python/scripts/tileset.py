"""
Trivial example of a Tileset export plugin. Place it under ~/.tiled.
2024, <pablo.duboue@gmail.com>
"""

from tiled import *

class TExample(TilesetPlugin):
    @classmethod
    def nameFilter(cls):
        return "TExample files (*.texample)"

    @classmethod
    def shortName(cls):
        return "texample"

    @classmethod
    def write(cls, tileset, fileName):
        with open(fileName, 'w') as f:
            f.write("{}\n".format(tileset.tileCount()))
            for idx in range(tileset.tileCount()):
                tile = tileset.tileAt(idx)
                f.write("\t{}. {}: {} {}x{}\n".format(idx, tile.id(), tile.type(), tile.width(), tile.height()))
        return True
