"""
Pekka Kana 2 map support for Tiled
2012, <samuli@tuomola.net>

Notes:
- should make PK2 classes mixins with Tiled ones
"""
import os, sys, re, string
from tiled import *
from tiled.qt import *
from lib import cpystruct
from os.path import dirname, exists
from struct import pack,unpack,Struct
from base64 import b64encode, b64decode

maps = []

class PK2(Plugin):
  @classmethod
  def nameFilter(cls):
    return "Pekka Kana 2 (*.map)"

  @classmethod
  def shortName(cls):
    return "pk2"

  @classmethod
  def supportsFile(cls, f):
    return open(f, 'rb').read(4) == '1.3\0'

  @classmethod
  def read(cls, f):
    lvl = PK2MAP()
    with open(f, 'rb') as fh:
      lvl.unpack(fh)
      # spriteCount is +1
      fh.seek(-len(lvl.spriteFiles[0]), 1)
      lay1 = PK2MAPLAYER(fh)
      lay2 = PK2MAPLAYER(fh)
      lay3 = PK2MAPLAYER(fh)
      print lvl

    # -- tileset
    img = QImage()
    imgfile = dirname(f)+'/../../gfx/tiles/'+str(lvl.tileFile)
    img.load(imgfile, 'BMP')
    t = Tiled.Tileset.create('Tiles', 32,32, 0, 0)
    t.data().setTransparentColor(QColor(img.color(255)))
    t.data().loadFromImage(img, imgfile)

    # find common bounding box for the layers
    bb = ['','',10,10]
    for l in [lay1,lay2,lay3]:
      print l
      bb[0] = min([bb[0], l.lx.num])
      bb[1] = min([bb[1], l.ly.num])
      bb[2] = max([bb[2], l.width()])
      bb[3] = max([bb[3], l.height()])

    print 'bounds', bb

    m = Tiled.Map(Tiled.Map.Orthogonal, bb[2], bb[3], 32,32)
    maps.append(m)

    # -- background image
    lai = Tiled.ImageLayer('Scenery', bb[2], bb[3])
    img = QImage()
    imgfile = dirname(f)+'/../../gfx/scenery/'+str(lvl.fieldFile)
    img.load(imgfile, 'BMP')
    lai.loadFromImage(img, imgfile)

    # -- layers
    la1 = Tiled.TileLayer('Back', 0,0, bb[2], bb[3])
    lay1.doTiles(t, la1, bb)

    la2 = Tiled.TileLayer('Front', 0,0, bb[2], bb[3])
    lay2.doTiles(t, la2, bb)

    sprdir = dirname(f)+'/../../sprites/'
    lay3.sprites = [0]
    lay3.spriteGfx = {}

    for s in lvl.spriteFiles:
      # spriteCount is +1
      if not exists(sprdir+str(s)): break

      spr = PK2SPR(sprdir+str(s), m)

      if not lay3.spriteGfx.has_key(str(spr.kuvatiedosto)):
        sprfile = find_case_insensitive_filename(sprdir, str(spr.kuvatiedosto))
        img = QImage()
        img.load(sprdir+sprfile, 'BMP')
        print 'loading', sprdir+sprfile
        sprts = Tiled.Tileset.create(sprfile, 32,32, 0, 0)
        sprts.data().setTransparentColor(QColor(img.color(255)))
        sprts.data().loadFromImage(img, sprdir+sprfile)
        lay3.spriteGfx[str(spr.kuvatiedosto)] = sprts

      #sprgfx[(str(spr.kuvatiedosto]
      lay3.sprites.append(spr)

      #print spr

    la3 = Tiled.ObjectGroup('Sprites', bb[2], bb[3])
    lay3.doSprites(la3, bb)
    m.addLayer(lai)
    m.addTileset(t)
    for sprts in lay3.spriteGfx.values():
      m.addTileset(sprts)
    m.addLayer(la1)
    m.addLayer(la2)
    m.addLayer(la3)

    for f in lvl.__slots__:
      val = repr(getattr(lvl, f))
      m.setProperty(f, b64encode(val))

    return m

  @classmethod
  def write(cls, m, fn):
    out = PK2MAP()

    for f in m.properties().keys():
      if not f.startswith('__'):
        setattr(out, f, b64decode(m.property(f)))

    #setattr(out, "sprites", ['pla'])

    with open(fn, 'wb') as fh:
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

def find_case_insensitive_filename(path, fn):
  for f in os.listdir(path):
    if f.lower() == fn.lower():
      return f

class asciilongfile(cpystruct.CpyStruct("char filename[100]")):
  @classmethod
  def fromraw(cls, v):
    return re.search('[\w\.]*', v, re.U).group(0)
  def __repr__(self):
    return str(self)
  def __str__(self):
    return self.filename

class asciifile(cpystruct.CpyStruct("char filename[13]")):
  @classmethod
  def fromraw(cls, v):
    return re.search('[\w\.]*', v, re.U).group(0)
  def __repr__(self):
    return str(self)
  def __str__(self):
    return self.filename

class asciitxt(cpystruct.CpyStruct("char txt[40]")):
  @classmethod
  def fromraw(cls, v):
    return re.search('[\w\. ]', v, re.U).group(0)

class asciinum(cpystruct.CpyStruct("char num[8]")):
  @classmethod
  def fromraw(cls, v):
    #v = ''.join(re.findall('[0-9]', v))
    v = re.sub('[^0-9]','',v)
    return 0 if not v.strip().isdigit() else int(v)

class PK2MAPLAYER(cpystruct.CpyStruct("asciinum lx, ly, w, h;")):
  MAXW = 256
  MAXH = 224
  MAXSZ = MAXW*MAXH

  def width(self):
    return self.w.num + 1
  def height(self):
    return self.h.num + 1

  def __init__(self, dat):
    # should make cpystruct support this usecase better
    super(self.__class__, self).__init__(dat)

    #print str(cpystruct.peek(dat, 128))

    self.layer = bytearray(self.MAXSZ)
    for i in range(len(self.layer)): self.layer[i] = 0xff

    for y in range(self.ly.num, self.ly.num+self.height()):
      for x in range(self.lx.num, self.lx.num+self.width()):
        self.layer[x+y*self.MAXW] = dat.read(1)

  def findBounds(self):
    "find bounding box for coords that have tiles"
    mx,my,mw,mh = None,None,10,10

    for y in range(self.ly, self.ly+self.height()):
      for x in range(self.lx, self.lx+self.width()):
        if self.layer[x + y * self.MAXW] != 255:
          if not my: my = y
          if not mx or x < mx: mx = x
          if x > mw: mw = x
          if y > mh: mh = y

    if not mx: mx = 0
    if not my: my = 0
    return mx, my, mw, mh

  def doSprites(self, la, bb):
    for y in range(self.height()):
      for x in range(self.width()):
        sprite = self.layer[self.lx.num + x + (self.ly.num + y) * self.MAXW]
        if sprite != 255:
          #if sprite > len(self.sprites):
          #  print 'invalid spr',sprite
          #  continue
          rx = self.lx.num + x - bb[0]
          ry = self.ly.num + y - bb[1] + 1
          spr = self.sprites[sprite]
          obj = Tiled.MapObject(str(spr.kuvatiedosto), '', QPointF(rx, ry), QSizeF(1, 1)) #spr.width, spr.height))
          # 0 should point to the actual sprite but how?
          obj.setCell(Tiled.Cell(self.spriteGfx[str(spr.kuvatiedosto)].data().tileAt(0)))
          la.addObject(obj)

  def doTiles(self, ts, la, bb):
    for y in range(self.height()):
      for x in range(self.width()):
        tile = self.layer[self.lx.num + x + (self.ly.num + y) * self.MAXW]
        if tile != 255:
          rx = self.lx.num + x - bb[0]
          ry = self.ly.num + y - bb[1]
          if tile == 149: print 'start @',rx,ry
          if tile == 150: print 'end @',rx,ry
          ti = ts.data().tileAt(tile)
          if ti != None and rx < bb[2] and ry < bb[3]:
            # app should check that coords are within layer
            #print rx,ry,self.ly,y
            la.setCell(rx, ry, Tiled.Cell(ti))
          else:
            print 'invalid',rx,ry

class PK2SPR_ANIM(cpystruct.CpyStruct("""
  uchar   seq[10];
  uchar   frames;
  bool    loop;
""")):
  def __repr__(self):
    return str(self)

class PK2SPR(cpystruct.CpyStruct("""
asciinum  tyyppi;
asciilongfile  kuvatiedosto;
asciilongfile  aanitiedostot[7];
int     aanet[7];
uchar   frameja;
PK2SPR_ANIM animaatiot[20];
uchar   animaatioita;
uchar   frame_rate;
int     kuva_x;
int     kuva_y;
int     kuva_frame_leveys;
int     kuva_frame_korkeus;
int     kuva_frame_vali;
char    nimi[30];
int     width, height;
double  paino;
bool    vihollinen;
int     energia;
int     vahinko;
uchar   vahinko_tyyppi;
uchar   suojaus;
int     pisteet;
int     AI[10];
uchar   max_hyppy;
double  max_nopeus;
int     latausaika;
uchar   vari;
bool    este;
int     tuhoutuminen;
bool    avain;
bool    tarisee;
uchar   bonusten_lkm;
int     hyokkays1_aika;
int     hyokkays2_aika;
int     pallarx_kerroin;
char    muutos_sprite[100];
char    bonus_sprite[100];
char    ammus1_sprite[100];
char    ammus2_sprite[100];

bool    tiletarkistus;
DWORD   aani_frq;
bool    random_frq;

bool    este_ylos;
bool    este_alas;
bool    este_oikealle;
bool    este_vasemmalle;

uchar   lapinakyvyys;
bool    hehkuu;
int     tulitauko;
bool    liitokyky;
bool    boss;
bool    bonus_aina;
bool    osaa_uida;
""")):
  def __init__(self, f, m):
    with open(f, 'rb') as fh:
      super(self.__class__, self).__init__(fh)

class PK2MAP(cpystruct.CpyStruct("""
  char ver[4];
  BYTE nan;
  asciifile tileFile, fieldFile, musicFile;
  asciitxt mapName, author;
  asciinum lvl, air, trig1, trig2, trig3;
  asciinum playTime, v1, background, plrSpr;
  asciinum lvlX, lvlY, icon;
  asciinum spriteCount;
  asciifile spriteFiles[spriteCount];
""")): pass

