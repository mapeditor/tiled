"""
Initial support for SNES emulator save states in Tiled
2012, <samuli@tuomola.net>

Note:
- assumes 8x8px tiles in 64x32 map of 4bpp gfx of max 7 palgroups
- only supports bg1 of bgmode 1
http://snesemu.black-ship.net/misc/techdocs/snesdoc.html#GraphicsFormat
"""
import sys, re, string
from tiled import *
from tiled.qt import *
from os.path import dirname
from struct import pack,unpack,Struct
from collections import namedtuple

class ZST(Plugin):
  @classmethod
  def nameFilter(cls):
    return "zSNES Save State (*.zs?)"

  @classmethod
  def shortName(cls):
    return "zst"

  @classmethod
  def supportsFile(cls, f):
    return open(f).read(26) == 'ZSNES Save State File V0.6'

  @classmethod
  def read(cls, f):
    m = Tiled.Map(Tiled.Map.Orthogonal, 64,32, 8,8)

    # bg1-4 layer bpp counts
    bgmodes = ( (2, 4, 4, 8, 8, 4, 4, 0),
        (2, 4, 4, 4, 2, 2, 0, 0),
        (2, 2, 0, 0, 0, 0, 0, 0),
        (2, 0, 0, 0, 0, 0, 0, 0))

    img = QImage(32*8, 32*8, QImage.Format_Indexed8)
    #img.fill(255)
    cmap = []
    with open(f, 'rb') as fh:
      #fh.seek(0x66)
      #bgmode = unpack('B', fh.read(1))[0]
      #if bgmode != 7: raise Exception('only mode7 supported atm: '+str(bgmode))

      # supporting new uncompressed save state formats should be just
      # a matter of adding correct addresses for these
      cgrambase = 0x618
      tilemapbase = 0x20C13 # vram:0
      tilebase = 0x2000 # vram offset

      fh.seek(cgrambase)
      cmap = list(parseColors(fh.read(0x200)))
      #img.setColorTable(cmap)

      colors = 16
      tsets = []
      for pal in range(7):
        img.setColorTable(cmap[pal*colors:pal*colors+colors])
        fh.seek(tilemapbase+tilebase)
        readTileset(fh, img)

        tsets.append(Tiled.Tileset.create('Pal%i'%pal, 8,8, 0, 0))
        tsets[pal].data().setTransparentColor(QColor(img.color(0)))
        tsets[pal].data().loadFromImage(img, 'script')

      la = Tiled.TileLayer('Back', 0,0, 64,32)
      fh.seek(tilemapbase)

      for y in range(la.height()):
        for x in range(32):
          t = parseTile(unpack('H', fh.read(2))[0])
          try:
            tile = tsets[t.pal].data().tileAt(t.idx)
            pix = tile.image()
            tile.setImage(pix)
            """ overriding tile gfx could be an alternative to palgroup layers
            img = pix.toImage()
            img.setColorTable(cmap[t.pal*16:t.pal*16+16])
            pix.convertFromImage(img)
            """
            la.setCell(x, y, Tiled.Cell(tile))
          except:
            print 'out of range %i,%i: %i' % (x,y,t.idx)

      if la.width() > 32:
        for y in range(la.height()):
          for x in range(32, la.width()):
            t = parseTile(unpack('H', fh.read(2))[0])
            try:
              tile = tsets[t.pal].data().tileAt(t.idx)
              la.setCell(x, y, Tiled.Cell(tile))
            except:
              print 'out of range %i,%i: %i' % (x,y,t.idx)

      for pal in range(7):
        m.addTileset(tsets[pal])
      m.addLayer(la)
    return m

def readTileset(dat, img, tvert=32, thoriz=32):
  tw, th = 8, 8
  # iterate tiles
  for i in range(tvert*thoriz):
    tx,ty = i%tvert*tw, i/thoriz*th
    l1 = unpack('>16B', dat.read(16))
    l2 = unpack('>16B', dat.read(16))
    colordat = deplane4bpp([l1,l2])
    # iterate lines in a tile
    for l in range(th):
      # ..pixels in a line
      for p in range(tw):
        c = colordat[l][p]
        img.setPixel(tx+p, ty+l, c)

def deplane2bpp(src, dst):
  "takes 16 bytes, simplify"
  for i in range(8):
    b0 = src[i*2]
    b1 = src[i*2+1]
    if i <= len(dst): dst.append(bytearray(8))
    dst[i][0] += (b0 >> 7) | b1 >> 6 & 2
    dst[i][1] += (b0 >> 6 & 1) | b1 >> 5 & 2
    dst[i][2] += (b0 >> 5 & 1) | b1 >> 4 & 2
    dst[i][3] += (b0 >> 4 & 1) | b1 >> 3 & 2
    dst[i][4] += (b0 >> 3 & 1) | b1 >> 2 & 2
    dst[i][5] += (b0 >> 2 & 1) | b1 >> 1 & 2
    dst[i][6] += (b0 >> 1 & 1) | b1 & 2
    dst[i][7] += (b0 & 1) | b1 & 1 << 1
  return dst

#deplane2bpp( bytearray('3C3C4266 99 66 99 66  99 66 99 3C  42 00 3C 3C'.replace(' ','').decode('hex')) )

def deplane4bpp(src):
  "decode planar 4bpp line of 8px into packed color indexes"
  dst = deplane2bpp(src[1], list())
  for i in range(len(dst)):
    for j in range(len(dst[i])):
      dst[i][j] <<= 2
  return deplane2bpp(src[0], dst)

def parseColors(cgram):
  for i in range(0, len(cgram), 2):
    col = unpack('H', cgram[i:i+2])[0]
    r = (col << 3) & 0xf8
    g = ((col >> 5) << 3) & 0xf8
    b = ((col >> 10) << 3) & 0xf8
    #print (r,g,b),
    yield QColor(r,g,b).rgb()

def parseTile(t):
  ret = namedtuple('Tile', 'idx pal prio flipx flipy')

  return ret(idx = t & 1023, pal = t >> 10 & 7, prio = t >> 13,
      flipx = t >> 14 != 0, flipy = t >> 15 != 0)

