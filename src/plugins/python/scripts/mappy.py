"""
Mappy support for Tiled
2012, <samuli@tuomola.net>
"""
from tiled import *
from tiled.qt import *
import os, sys, struct
from lib.mappy_types import BLKSTR, MPHD, fmpchunk

class Mappy(Plugin):
  @classmethod
  def nameFilter(cls):
    return "Mappy (*.fmp)"

  @classmethod
  def supportsFile(cls, f):
    return open(f).read(4) == 'FORM'

  @classmethod
  def read(cls, f):
    print 'Loading map at',f
    chunks = readchunks(f)
    hd = chunks['MPHD']
    m = Tiled.Map(Tiled.Map.Orthogonal, hd.mapwidth, hd.mapheight, hd.blockwidth, hd.blockheight)
    if hd.type == 2:
      print 'Isometric maps not supported at the moment'
      return m

    tset = Tiled.Tileset('Tiles', hd.blockwidth, hd.blockheight, 0, 0)
    cmap = list(readcmap(chunks['CMAP']))
    tset.loadFromImage(readtilegfx(hd, chunks['BGFX'], cmap), "")

    blks = list(readblockdata(chunks['BKDT'], hd))

    for c in ['LYR'+str(i) for i in range(7,0,-1)]+['BODY']:
      if not chunks.has_key(c): continue
      print 'populating',c
      lay = Tiled.TileLayer(c,0,0,hd.mapwidth, hd.mapheight)
      lvl = list(readlayer(hd, chunks[c]))
      for o in range(4):
        poplayer(lay, o, blks, tset, hd, lvl)
      lay.setMap(m)  # w/o this crashes in mapscene:createLayerItem dynamic_cast
      m.addLayer(lay)
    m.addTileset(tset)

    return m

  @classmethod
  def write(cls, m, fn):
    return False


def readchunks(f):
  chunks = {}
  with open(f, 'rb') as fh:
    fc = fmpchunk()
    fc.unpack(fh)
    filelen = fc.len - 16
    print fc, fh.read(4)
    while fh.tell() < filelen and fc.unpack(fh) != None:
      print fh.tell(), fc
      if fc.id == 'MPHD':
        mp = MPHD()
        mp.unpack(fh)
        realsz = struct.calcsize(mp.__fstr)
        #if fc.len > realsz:
        #  raise Exception('Unread data: '+str(fc.len-realsz))
        fh.read(fc.len-realsz)
        print mp
        chunks[fc.id] = mp
      else:
        chunks[fc.id] = fh.read(fc.len)
  return chunks


def readtilegfx(hd, dat, cmap):
  n = 0
  w,h = hd.blockwidth*10, hd.blockheight*hd.numblockgfx/10+hd.blockheight
  fmt = QImage.Format_Indexed8 if hd.blockdepth==8 else QImage.Format_ARGB32
  img = QImage(w, h, fmt)
  img.setColorTable(cmap)

  for i in range(hd.numblockgfx):
    col,row = i%10, i/10
    tx,ty = col*hd.blockwidth, row*hd.blockheight

    for y in range(hd.blockheight):
      for x in range(hd.blockwidth):
        if hd.blockdepth==8:
          c = struct.unpack('B', dat[n])[0]
          img.setPixel(tx+x, ty+y, c)
        else:
          img.setPixel(tx+x, ty+y, cmap[c])
        n+=1

  return img


def readlayer(hd, dat):
  #print len(dat),'vs',hd.mapheight*hd.mapwidth*2
  for i in range(0,hd.mapheight*hd.mapwidth*2,2):
    d = struct.unpack('<H', dat[i:i+2])[0]
    #d<0 is anim?
    if hd.type == 0: d /= hd.blockstrsize
    yield d

def poplayer(layer, fg, blocks, tileset, hd, l):
  i = 0
  for y in range(hd.mapheight):
    for x in range(hd.mapwidth):
      v = l[i]
      if v >= len(blocks):
        pass#print 'unknown block at',i
      else:
        n = blocks[l[i]].olay[fg]
        if n != 0:
          ti = tileset.tileAt(n)
          if ti is None:
            print 'invalid tile',n,'at',x,y
          else:
            layer.setCell(x, y, Tiled.Cell(ti))
      i += 1

def readblockdata(chunk, hd):
  mod = hd.blockwidth * hd.blockheight * ((hd.blockdepth+1)/8)
  for pos in range(0, hd.numblockstr*hd.blockstrsize, hd.blockstrsize):
    b = BLKSTR()
    b.unpack(chunk[pos:pos+hd.blockstrsize])
    if hd.type == 0:
      for i in range(4):
        b.olay[i] /= mod
    yield b

def readcmap(cmap):
  for i in range(0,len(cmap),3):
    r,g,b = struct.unpack('3B', cmap[i:i+3])
    yield QColor(r,g,b).rgb()

