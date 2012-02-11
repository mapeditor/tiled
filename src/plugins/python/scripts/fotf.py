"""
Fury of the Furries level loader for Tiled
2012, <samuli@tuomola.net>

Because tiled doesn't support ILBM at the moment you can convert
tileset images by installing imagemagic, netpmb and running following
in the game directory on GNU/bash (windows users: good luck with PSP):
for f in DEC/*LBM; do convert $f DEC/$(basename $f).gif; done
"""
from tiled import *
import sys
from os.path import dirname
sys.path.append(dirname(__file__)+'/lib')
from cpystruct import *
from struct import pack,unpack

NameFilter("Fury of the Furries (*.bin)")
maps = []

class TileData(CpyStruct('ushort len, BYTE d[len]')): pass
class RleData(CpyStruct('BYTE rep, BYTE sig, BYTE val')): pass

def unpacklvl(f):
  lvl = []

  with open(f) as fh:
    fh.read(4)  #skip sig
    tdata = TileData(len=-1)
    rdata = RleData()
    while tdata.len != 0:
      tdata.unpack(fh)
      if tdata.len == 0: break
      rdata.unpack(fh)
      #if len(lvl) == 0: print tdata,rdata
      lvl += tdata.d
      lvl += [rdata.val for i in range(rdata.rep)]
  return lvl
 
def readmap(f):
  try:
    print 'Loading map at',f
    lvl = unpacklvl(f)
    # sometimes the two ints are split so can't unpack before
    # tiledata, and have to repack in case they're >255 
    w,h = unpack("<2H",pack("<4b",*lvl[:4]))
    del lvl[:4]
    #print len(lvl),lvl[:32]

    m = Tiled.Map(Tiled.Map.Orthogonal, w,h, 16,16)
    maps.append(m)
    t = Tiled.Tileset('DECOR', 16,16, 0, 0)
    loadFromImage(t, dirname(f)+'/../DEC/DECOR01.LBM.gif')
    l = Tiled.TileLayer('Tiles',0,0,w,h)

    l.setMap(m)
    for i in range(0, w*h*2, 2):
      y = i/2/w
      x = i/2-(w*y)
      ti = t.tileAt(lvl[i+1]*20 + lvl[i])
      l.setCell(x, y, Tiled.Cell(ti))
    #print t.fileName(),l.isEmpty(),l.referencesTileset(t)
    m.addTileset(t)
    m.addLayer(l)
    return m

  except Exception as e:
    exc_type, exc_obj, exc_tb = sys.exc_info()
    print 'error @%i:' % exc_tb.tb_lineno, e

def writemap(m, fn):
  try:
    print "Writing map(%i,%i) to" % (m.width(),m.height()), fn
    l = m.layerAt(0)
    lvl = []
    """have to do something for layerAt to give the correct type 
    for x in range(l.width()):
      for y in range(l.height()):
#      lvl += l.cellAt(x, y)
        print l.cellAt(x, y).tile.id()
    print lvl
    """
    return True
  except Exception as e:
    exc_type, exc_obj, exc_tb = sys.exc_info()
    print 'error @%i:' % exc_tb.tb_lineno, e

def is_supported_file(f):
	return open(f).read(4) == 'byt4'


Supports(is_supported_file)
Read(readmap)
Write(writemap)

