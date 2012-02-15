"""
Mappy support for Tiled
2012, <samuli@tuomola.net>
"""
from tiled import *
import os, sys, struct
from os.path import dirname
sys.path.append(dirname(__file__)+'/lib')
from mappy_types import *
from pngc import PNGCanvas

NameFilter("Mappy (*.fmp)")
maps = []

def readchunks(f):
  chunks = {}
  with open(f) as fh:
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

def readtilegfx(img, hd, dat, cmap):
  n = 0
  for i in range(hd.numblockgfx):
    col,row = i%10, i/10
    tx,ty = col*hd.blockwidth, row*hd.blockwidth

    for y in range(hd.blockheight):
      for x in range(hd.blockwidth):
        if hd.blockdepth == 8:
          c = struct.unpack('B', dat[n])[0] & 255
          c *= 3
          r,g,b = struct.unpack('3B', cmap[c:c+3])
        elif hd.blockdepth == 24:
          c = n*3
          r,g,b = struct.unpack('3B', dat[c:c+3])

        #img[tx+x,ty+y] = (r,g,b)
        img.point(tx+x, ty+y, bytearray([r,g,b,0xff]))
        n+=1
  #return [seq[i:i+w] for i in range(0, len(seq), w)]

def readlayer(hd, dat):
  #print len(dat),'vs',hd.mapheight*hd.mapwidth*2
  for i in range(0,hd.mapheight*hd.mapwidth*2,2):
    d = struct.unpack('<H', dat[i:i+2])[0]  #LE?
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

class blk():
  def __init__(self, mod, i):
    self.mod = mod
    self.idx = list(i)
    print self.idx,
    self.idx = [n/mod for n in self.idx]
    print 'after',self.idx

def readmap(f):
  try:
    print 'Loading map at',f
    chunks = readchunks(f)
    hd = chunks['MPHD']
    m = Tiled.Map(Tiled.Map.Orthogonal, hd.mapwidth, hd.mapheight, hd.blockwidth, hd.blockheight)

    if not os.path.exists(f+'.png'):
      print 'Creating',f+'.png'
      #PIL
      #sheetimg = Image.new("RGB", (hd.blockwidth*10, hd.blockheight*hd.numblockgfx/10))
      #readtilegfx(sheetimg.load(), hd, chunks['BGFX'], chunks['CMAP'])
      #sheetimg.save(f+'.gif', 'gif')
      with open(f+'.png', 'wb') as fh:
        #pngpy
        #w = png.Writer(hd.blockwidth*10, hd.blockheight*hd.numblockgfx/10)
        #px = readtilegfx(None, hd, chunks['BGFX'], chunks['CMAP'])
        #img = png.from_array(px, 'RGB')
        #w.write(fh, px)
        #pngc, third time's the charm
        cn = PNGCanvas(hd.blockwidth*10, hd.blockheight*hd.numblockgfx/10)
        readtilegfx(cn, hd, chunks['BGFX'], chunks['CMAP'])
        fh.write(cn.dump())

    t = Tiled.Tileset('Tiles', hd.blockwidth, hd.blockheight, 0, 0)
    loadFromImage(t, f+'.png')
    maps.append(m)  # w/o live ref crashes in mapscene:createLayerItem setVisible

    pos = 0
    blks = []
    mod = hd.blockwidth * hd.blockheight * ((hd.blockdepth+1)/8)

    for i in range(hd.numblockstr):
      b = BLKSTR()
      b.unpack(chunks['BKDT'][pos:pos+hd.blockstrsize])
      if hd.type == 0:
        for i in range(4):
          b.olay[i] /= mod
      #idx = struct.unpack('>4I', chunks['BKDT'][pos:pos+16])
      #b = blk(mod, idx, struct.unpack('>4I', chunks['BKDT'][pos:pos+16]))
      blks.append(b)
      pos += hd.blockstrsize

    for c in ['LYR'+str(i) for i in range(7,0,-1)]+['BODY']:
      if not chunks.has_key(c): continue
      print 'populating',c
      l = Tiled.TileLayer(c,0,0,hd.mapwidth, hd.mapheight)
      lvl = list(readlayer(hd, chunks[c]))
      for o in range(4):
        poplayer(l, o, blks, t, hd, lvl)
      l.setMap(m)  # w/o this crashes in mapscene:createLayerItem dynamic_cast
      m.addLayer(l)
    m.addTileset(t)

    return m

  except Exception as e:
    exc_type, exc_obj, exc_tb = sys.exc_info()
    print 'error @%i:' % exc_tb.tb_lineno, e

def writemap(m, fn):
  return False

def is_supported_file(f):
  return open(f).read(4) == 'FORM'

Supports(is_supported_file)
Read(readmap)
Write(writemap)

