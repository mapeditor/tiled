"""
Mappy support for Tiled
2012-2013, <samuli@tuomola.net>
"""
from tiled import *
from tiled.qt import *
import os, sys, struct
from lib.mappy_types import BLKSTR, MPHD, fmpchunk
import pickle
from StringIO import StringIO
from collections import OrderedDict
from base64 import b64encode, b64decode

class FMPPicklerMixin:

  @classmethod
  def unpackchunks(cls, f):

    chunks = OrderedDict()
    with open(f, 'rb') as fh:
      frm = fmpchunk()
      frm.unpack(fh)
      frm.data = fh.read(4)
      filelen = frm.len - 16
      chunks[frm.id] = frm
      print frm

      while fh.tell() < filelen:
        fc = fmpchunk()
        if not fc.unpack(fh): break
        print fh.tell(), fc
        fc.data = fh.read(fc.len)
        chunks[fc.id] = fc

    return chunks

  @classmethod
  def packchunks(cls, fn, chunks):

    with open(fn, 'wb') as fh:
      for k,v in chunks.items():
        print k, len(v) + v.len # chunk header + data
        fh.write(v.pack())
        fh.write(v.data)

  @classmethod
  def picklechunks(cls, chunks):
    src = StringIO()
    pi = pickle.Pickler(src, 2)
    pi.dump(chunks)
    print "packlen", src.len
    return b64encode(src.getvalue())

  @classmethod
  def unpicklechunks(cls, data):
    src = StringIO(b64decode(data))
    print "unpacklen",src.len
    return pickle.Unpickler(src).load()


class Mappy(Plugin, FMPPicklerMixin):

  @classmethod
  def nameFilter(cls):
    return "Mappy (*.fmp)"

  @classmethod
  def shortName(cls):
    return "mappy"

  @classmethod
  def supportsFile(cls, f):
    return open(f).read(4) == 'FORM'

  @classmethod
  def read(cls, f):

    print 'Loading map at',f
    chunks = cls.unpackchunks(f)
    hd = MPHD()
    # perhaps cpystruct should only read as many bytes as it can handle?
    hd.unpack(chunks['MPHD'].data[:len(hd)])

    m = Tiled.Map(Tiled.Map.Orthogonal, hd.mapwidth, hd.mapheight,
                    hd.blockwidth, hd.blockheight)
    if hd.type == 2:
      print 'Isometric maps not supported at the moment'
      return m

    m.setProperty('chunks', cls.picklechunks(chunks))

    tset = Tiled.Tileset.create('Tiles', hd.blockwidth, hd.blockheight, 0, 0)
    cmap = list(FMPColormap.unpack(chunks['CMAP'].data))
    tset.data().loadFromImage(FMPTileGfx.unpack(hd, chunks['BGFX'].data, cmap), "")

    blks = FMPBlocks(chunks['BKDT'].data, hd).blocks

    for c in ['LYR'+str(i) for i in range(7,0,-1)]+['BODY']:
      if not chunks.has_key(c): continue
      print 'populating',c
      lay = Tiled.TileLayer(c,0,0,hd.mapwidth, hd.mapheight)
      lvl = list(FMPLayer.unpack(hd, chunks[c].data))
      FMPLayer.populate(lay, blks, tset.data(), hd, lvl)
      m.addLayer(lay)

    m.addTileset(tset)

    return m


  @classmethod
  def write(cls, m, fn):

    if m.orientation() != Tiled.Map.Orthogonal:
      print 'Isometric maps not supported at the moment'
      return False
    if not m.property('chunks'):
      raise Exception("Export depends on unparsed binary blobs from original "
                      +"fmp to be stored in the map property 'chunks'")

    print 'Writing map at',fn

    chunks = cls.unpicklechunks(m.property('chunks'))
    hd = MPHD()
    hd.unpack(chunks['MPHD'].data[:len(hd)])
    blks = FMPBlocks(chunks['BKDT'].data, hd).blocks

    for i in range(m.layerCount()):

      if not isTileLayerAt(m, i): continue
      l = tileLayerAt(m, i)

      chunks[l.name()].data = FMPLayer.pack(hd, blks, l, i)

    cls.packchunks(fn, chunks)

    return True


class FMPBlocks:

  @classmethod
  def unpack(cls, chunk, hd):
    mod = hd.blockwidth * hd.blockheight * ((hd.blockdepth+1)/8)
    for pos in range(0, hd.numblockstr*hd.blockstrsize, hd.blockstrsize):
      b = BLKSTR()
      b.unpack(chunk[pos:pos+hd.blockstrsize])
      if hd.type == 0:
        for i in range(4):
          b.olay[i] /= mod
      yield b

  def __init__(self, chunk, hd):
    self.blocks = list(FMPBlocks.unpack(chunk, hd))


class FMPTileGfx:

  @classmethod
  def unpack(cls, hd, dat, cmap):
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
          c = struct.unpack('B', dat[n])[0]
          if hd.blockdepth==8:
            img.setPixel(tx+x, ty+y, c)
          else:
            img.setPixel(tx+x, ty+y, cmap[c])
          n+=1

    return img


class FMPLayer:

  @classmethod
  def unpack(cls, hd, dat):
    for i in range(0,hd.mapheight*hd.mapwidth*2,2):
      d = struct.unpack('<H', dat[i:i+2])[0]
      #d<0 is anim?
      if hd.type == 0: d /= hd.blockstrsize

      yield d

  @classmethod
  def pack(cls, hd, blocks, layer, laynum):
    dat = StringIO()
    print hd
    for y in range(layer.height()):
      for x in range(layer.width()):
        cell = layer.cellAt(x, y)
        tid = 0 if cell.isEmpty() else cell.tile.id()
        n = blocks[tid].olay[laynum]
        if hd.type == 0:
          n *= hd.blockstrsize
        dat.write( struct.pack('<H', n) )

    return dat.getvalue()

  @classmethod
  def populate(cls, layer, blocks, tileset, hd, ldata):

    for fg in range(4):
      i = 0

      for y in range(hd.mapheight):
        for x in range(hd.mapwidth):
          v = ldata[i]

          if v >= len(blocks):
            pass#print 'unknown block at',i
          else:
            n = blocks[ldata[i]].olay[fg]
            if n != 0:
              ti = tileset.tileAt(n)
              if ti is None:
                print 'invalid tile',n,'at',x,y
              else:
                layer.setCell(x, y, Tiled.Cell(ti))
          i += 1


class FMPColormap:

  @classmethod
  def unpack(self, cmap):
    """cmap -- rgb bytearray"""
    print 'got',len(cmap)
    for i in range(0,len(cmap),3):
      r,g,b = struct.unpack('3B', cmap[i:i+3])
      yield QColor(r,g,b).rgb()

  @classmethod
  def pack(self, cmap):
    """cmap -- QImage.colorTable"""
    yield fmpchunk(id='CMAP', len=len(list(cmap))).pack()
    for c in cmap:
      yield struct.pack('3B', (c.qRed,c.qGreen,c.qBlue))

