"""
Pekka Kana 2 map support for Tiled
2012, <samuli@tuomola.net>
"""
import sys, re, string
from tiled import *
from os.path import dirname
from lib import cpystruct
from struct import pack,unpack,Struct

maps = []

class PK2(Plugin):
  @classmethod
  def nameFilter(cls):
    return "Pekka Kana 2 (*.map)"

  @classmethod
  def supportsFile(cls, f):
    return open(f).read(4) == '1.3\0'

  @classmethod
  def read(cls, f):
    lvl = PK2MAP()
    lay1 = 0
    lay2 = 0
    with open(f) as fh:
      lvl.unpack(fh)
      lay1 = PK2MAPLAYER(fh)
      lay2 = PK2MAPLAYER(fh)
      print lvl, lay1, lay2

    m = Tiled.Map(Tiled.Map.Orthogonal, 0,0, 32,32)
    maps.append(m)

    img = QImage()
    img.load(dirname(f)+'/../../gfx/tiles/'+lvl.tileFile,'BMP')
    t = Tiled.Tileset('Tiles', 32,32, 0, 0)
    t.setTransparentColor(QColor(img.color(255)))
    t.loadFromImage(img,'script')
    #loadTilesetFromFile(t, dirname(f)+'/../../gfx/tiles/'+lvl.tileFile)

    maxw, maxh = PK2MAPLAYER.MAXW, PK2MAPLAYER.MAXH

    la1 = Tiled.TileLayer('Back', 0,0, maxw, maxh)
    la1.setMap(m)
    lay1.doTiles(t, la1)

    la2 = Tiled.TileLayer('Front', 0,0, maxw, maxh)
    la2.setMap(m)
    lay2.doTiles(t, la2)

    m.addTileset(t)
    m.addLayer(la1)
    m.addLayer(la2)
    for f in lvl.__slots__:
      val = getattr(lvl, f)
      if type(val) == int or type(val) == str:
        val = re.search('[\w\._]+', str(val)).group(0)
        m.setProperty(f, val)
      else:
        print 'nope',f
    return m

  @classmethod
  def write(cls, m, fn):
    out = PK2MAP()

    for f in m.properties().keys():
      setattr(out, f, m.property(f))

    setattr(out, "sprites", ['pla'])

    with open(fn, 'w') as fh:
      print >>fh, out.pack()

      for i in range(m.layerCount()):
        tiles = []
        l = 0
        if isTileLayerAt(m, i):
          l = tileLayerAt(m, i)
          print l
        elif isObjectGroupAt(m, i):
          #l = objectGroupAt(m, i)
          continue

        for y in range(l.height()):
          for x in range(l.width()):
            if l.cellAt(x, y).tile != None:
              tiles.append( l.cellAt(x, y).tile.id() )
        print >>fh, 0,0, l.width(), l.height()
        print >>fh, bytearray(tiles)

    return True

class asciifile(cpystruct.CpyStruct("char filename[13]")):
  @classmethod
  def fromraw(cls, v):
    #return re.search('[^\w\.]*', v, re.U).group(0)
    return ''.join(re.findall('[\w\.]*', v, re.U))
  def __repr__(self): return self.filename

class asciitxt(cpystruct.CpyStruct("char txt[40]")):
  @classmethod
  def fromraw(cls, v):
    return ''.join(re.findall('[\w\.]', v, re.U))

class asciinum(cpystruct.CpyStruct("char numeric[8]")):
  @classmethod
  def fromraw(cls, v):
    v = re.sub('[^0-9]','',v)
    return 0 if v is '' else int(v)

class PK2MAPLAYER(cpystruct.CpyStruct("asciinum lx, ly, w, h;")):
  MAXW = 256
  MAXH = 224
  MAXSZ = MAXW*MAXH
  def __init__(self, dat):
    # should make cpystruct support this usecase better
    super(self.__class__, self).__init__(dat)

    self.w += 1
    self.h += 1

    #print str(cpystruct.peek(dat, 128))

    self.layer = bytearray(self.MAXSZ)
    for i in range(len(self.layer)): self.layer[i] = 0xff

    for y in range(self.ly, self.ly+self.h):
      for x in range(self.lx, self.lx+self.w):
        self.layer[x+y*self.MAXW] = dat.read(1)

  def findsize(self):
    "find layer size by checking for used tiles"
    width = 0
    height = 0

    for y in range(self.MAXH):
      for x in range(self.MAXW):
        if self.layer[self.lx + x + (self.ly + y) * self.w] != 255:
          if x > width: width = x
          if y > height: height = y

    return width, height

  def doTiles(self, ts, la):
    for y in range(self.h):
      for x in range(self.w):
        tile = self.layer[self.lx + x + (self.ly+y) * self.MAXW]
        if tile != 255:
          if tile == 149: print 'start @',x,y
          if tile == 150: print 'end @',x,y
          ti = ts.tileAt(tile)
          if ti != None:
            # app should check that coords are within layer
            la.setCell(self.lx+x, self.ly+y, Tiled.Cell(ti))

class PK2SPR(cpystruct.CpyStruct("BYTE dat[13];")): pass

class PK2MAP(cpystruct.CpyStruct("""
  char ver[4];
  BYTE nan;
  asciifile tileFile, fieldFile, musicFile;
  asciitxt mapName, author;
  asciinum lvl, air, trig1, trig2, trig3;
  asciinum playTime, v1, background, plrSpr;
  asciinum lvlX, lvlY, icon;
  asciinum spriteCount;
  asciifile sprites[spriteCount];
""")): pass

