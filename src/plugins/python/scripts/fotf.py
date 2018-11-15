"""
Fury of the Furries level loader for Tiled
2012, <samuli@tuomola.net>
"""

import sys, re
import tiled as T
from os.path import dirname
from lib import cpystruct, lbm, utils
from struct import pack,unpack
from collections import namedtuple

maps = {}
tilesets = {}

MetaData = namedtuple(
  'MetaData', 'startX, startY, lType,'+
  'fin1X, fin1Y, u1, fin2X, fin2Y, lvl'
  )

class Fury(T.Plugin):
  @classmethod
  def nameFilter(cls):
    return "Fury of the Furries (*.bin)"

  @classmethod
  def supportsFile(cls, f):
    return open(f,'rb').read(4) == b'byt4'

  @classmethod
  def read(cls, f):
    print('Loading map at',f)
    fr = Fury(f)

    m = T.Tiled.Map(T.Tiled.Map.Orthogonal, fr.w, fr.h, 16, 16)
    maps[f] = m

    # probably defined explicitly somewhere in the data
    decs = [1,3,4,2,8,6,7,9,10,5]
    decnum = (int(re.findall('[0-9]+', f).pop())-1)/10
    if decnum >= len(decs): decnum %= len(decs)
    gfxf = 'DEC/DECOR%02i.LBM' % decs[int(decnum)]
    gfxf = utils.find_sensitive_path(dirname(f)+'/../', gfxf)
    t = T.Tiled.Tileset.create('DECOR', 16,16, 0, 0)
    t.data().loadFromImage(fr.readtilegfx(gfxf), '')
    l = T.Tiled.TileLayer('Tiles',0,0, fr.w, fr.h)
    fr.populatetiles(l, t.data())
    # have to pass ownership so can't add tileset before populating layer
    m.addTileset(t)
    m.addLayer(l)

    #if not f.endswith('DATA19.BIN'):
    #  del fr.lvl[:21*2] # skip some zeros, todo: more deterministic
    print( len(fr.lvl), unpack('<40h', fr.lvl[:80]) )
    print( MetaData(*unpack('<9h', fr.lvl[:18])) )

    return m

  @classmethod
  def write(cls, m, fn):
    print("-- script doesn't support writing yet")
    fn += '.testing'
    print(".. map(%i,%i) to" % (m.width(),m.height()), fn)
    lvl = []

    with open(fn, 'w') as fh:
      for i in range(m.layerCount()):
        tiles = []
        l = 0
        if isTileLayerAt(m, i):
          l = tileLayerAt(m, i)
          print(l)
        elif isObjectGroupAt(m, i):
          #l = objectGroupAt(m, i)
          continue

        for x in range(l.width()):
          for y in range(l.height()):
            tiles.append( l.cellAt(x, y).tile.id() )

        print(tiles, file=fh)

    return False

  def __init__(self, f):
    dat = bytearray()
    with open(f, 'rb') as fh:
      fh.read(4)  #skip sig
      ldata = LevelData()
      rdata = RleData()
      while ldata.unpack(fh) and ldata.len != 0 :
        if len(ldata.d)==1 and ldata.d[0] == 0: break
        rdata.unpack(fh)
        dat.extend(ldata.d)
        dat.extend([rdata.val for i in range(rdata.rep)])

    print('le',len(dat))
    self.w, self.h = unpack("<2H", dat[:4])
    del dat[:4]
    self.lvl = dat

  def readtilegfx(self, fn):
    lc = dict(lbm.parselbm(fn))
    #print(lc['BMHD'])
    bd = list(lbm.readbody(lc['BODY'], lc['BMHD']))
    img = T.qt.QImage(lc['BMHD'].sz.w, lc['BMHD'].sz.h, T.qt.QImage.Format_Indexed8)
    img.setColorTable(lc['CMAP'])

    # there should be a faster alternative
    for y in range(img.height()):
      for x in range(img.width()):
        img.setPixel(x, y, bd[y*img.width()+x])

    return img

  def populatetiles(self, l, t):
    i = 0
    for y in range(self.h):
      for x in range(self.w):
        tpos = self.lvl[i+1]*t.columnCount() + self.lvl[i]
        if tpos < t.tileCount():
          ti = t.tileAt(tpos)
          if ti != None:
            l.setCell(x, y, T.Tiled.Cell(ti))
        i += 2
      if self.w < 78: # padded to width of 78 tiles
        i += (78-self.w)*2

    del self.lvl[:self.w*self.h*2]

class LevelData(cpystruct.CpyStruct('ushort len; BYTE d[len];')): pass

class RleData(cpystruct.CpyStruct('BYTE rep, sig, val')): pass

