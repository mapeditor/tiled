"""
  ILBM pure python decoder
  2012, <samuli@tuomola.net>

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
"""
import tiled as T
from .cpystruct import *
#from PyQt4.Qt import QColor  # to test outside tiled
import struct

class IFFhead(CpyStruct(":4s id; :I len; :4s type;", True)): pass

class IFFchunk(CpyStruct(":4s id; :I len;", True)):
  @classmethod
  def parsefile(cls, f):
    """
    Reads whole file consisting of IFF chunks
    Returns tuple generator of chunk name and body
    """
    with open(f, 'rb') as fh:
      # header
      ih = IFFhead(fh)
      yield ih.type, ih
      # chunks
      ic = IFFchunk()
      filelen = ih.len - len(ih)
      while fh.tell() < filelen and ic.unpack(fh) != None:
        yield ic.id, fh.read(ic.len)

class BMHDsize(CpyStruct("short w, h", True)):
  @classmethod
  def fromraw(cls, v):
    return struct.unpack(getattr(cls,'__fstr'), v)

class BMHD(CpyStruct("""
  BMHDsize sz; short x, y;
  BYTE planes, mask; short comp, trans;
  BYTE xaspect, yaspect; short pgw, pgh;
""")): pass

class CRNG(CpyStruct("short pad, cycrate, cycle, lowhireg;", True)): pass

class CMAP(CpyStruct("BYTE color[3];", True)):
  @classmethod
  def parse(cls, dat):
    c = CMAP()
    sz = struct.calcsize(getattr(cls, '__fstr'))
    for i in range(int(len(dat)/sz)):
      c.unpack(dat[i*sz:i*sz+sz])
      yield T.qt.QColor(c.color[0],c.color[1],c.color[2]).rgb()

def uncomp(dat):
  i = 0
  ret = bytearray()
  while i < len(dat):
    v = struct.unpack('b', dat[i:i+1])[0]
    i += 1
    if v >= 0:
      for n in range(v+1):
        ret += dat[i:i+1]
        i += 1
    elif v != -128:
      for n in range(v-1,0,1):
        ret += dat[i:i+1]
      i += 1
  return ret

def readbody(bd, ch):
  bj = [0,int((ch.sz.w+15) / 16) * 2]
  for p in range(2,ch.planes):
    bj.append(bj[1] * p)

  sj = bj[1] * ch.planes
  for sl in range(0, sj*ch.sz.h, sj):
    for x in range(ch.sz.w):
      b = x & 7
      bm = 128 >> b
      ib = sl + (x >> 3)
      px = (bd[ib] & bm)
      # only 4 planes tested
      for p in range(1,ch.planes,1):
        px |= (bd[ib+bj[p]] & bm) << p
      yield px >> (7 - b)

def parselbm(f):
  for id,dat in list(IFFchunk.parsefile(f)):
    if id == 'BMHD':
      ch = BMHD(dat)
      yield id, ch
    elif id == 'CMAP':
      yield id, list(CMAP.parse(dat))
    elif id == 'CRNG':
      yield id, CRNG(dat)
    elif id == 'CAMG':
      raise Exception('HAM/HALFBRITE not supported at the moment')
    elif id == 'BODY':
      if ch.comp == 1:
        bd = uncomp(dat)
      else:
        bd = dat
      yield id, bd

if __name__ == "__main__":
  import sys
  print( list(parselbm(sys.argv[1])) )

