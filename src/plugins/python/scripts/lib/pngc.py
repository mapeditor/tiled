#!/usr/bin/env python
"""
Based on: Simple PNG Canvas for Python by Rui Carmo (http://the.taoofmac.com)
"""
import os, sys, zlib, struct

signature = struct.pack("8B", 137, 80, 78, 71, 13, 10, 26, 10)

# alpha blends two colors, using the alpha given by c2
def blend(c1, c2):
    return [c1[i]*(0xFF-c2[3]) + c2[i]*c2[3] >> 8 for i in range(3)]

class PNGCanvas:
  def __init__(self, width, height, bgcolor=bytearray([0xff,0xff,0xff,0xff]),color=bytearray([0,0,0,0xff])):
    self.width = width
    self.height = height
    self.color = color #rgba
    self.bgcolor = bgcolor
    self.canvas = bytearray(self.bgcolor * 4 * width * height)

  def _offset(self, x, y):
    return y * self.width * 4 + x * 4

  def point(self,x,y,color=None):
    if x<0 or y<0 or x>self.width-1 or y>self.height-1: return
    if color == None:
        color = self.color
    o = self._offset(x,y)
    self.canvas[o:o+3] = blend(self.canvas[o:o+3],bytearray(color))

  def dump(self):
    scanlines = bytearray()
    for y in range(self.height):
      scanlines.append('\0') # filter type 0 (None)
      #print y * self.width * 4, (y+1) * self.width * 4
      #print self.canvas[y * self.width * 4:(y+1) * self.width * 4]
      scanlines.extend(self.canvas[(y * self.width * 4):((y+1) * self.width * 4)])
    # image represented as RGBA tuples, no interlacing
    return signature + \
      self.pack_chunk('IHDR', struct.pack("!2I5B",self.width,self.height,8,6,0,0,0)) + \
      self.pack_chunk('IDAT', zlib.compress(str(scanlines),9)) + \
      self.pack_chunk('IEND', '')

  def pack_chunk(self,tag,data):
    to_check = tag + data
    return struct.pack("!I",len(data)) + to_check + struct.pack("!I", zlib.crc32(to_check) & 0xFFFFFFFF)

  def load(self,f):
    assert f.read(8) == signature
    for tag, data in self.chunks(f):
      if tag == "IHDR":
        ( width,
          height,
          bitdepth,
          colortype,
          compression, filter, interlace ) = struct.unpack("!2I5B",data)
        self.width = width
        self.height = height
        self.canvas = bytearray(self.bgcolor * 4 * width * height)
        if (bitdepth,colortype,compression, filter, interlace) != (8,6,0,0,0):
          raise TypeError('Unsupported PNG format')
      # we ignore tRNS for the moment
      elif tag == 'IDAT':
        raw_data = zlib.decompress(data)
        rows = []
        i = 0
        for y in range(height):
          filtertype = ord(raw_data[i])
          i = i + 1
          cur = [ord(x) for x in raw_data[i:i+width*4]]
          if y == 0:
            rgba = self.defilter(cur,None,filtertype,4)
          else:
            rgba = self.defilter(cur,prev,filtertype,4)
          prev = cur
          i = i + width * 4
          row = []
          j = 0
          for x in range(width):
            self.point(x,y,rgba[j:j+4])
            j = j + 4

  def defilter(self,cur,prev,filtertype,bpp=3):
    if filtertype == 0: # No filter
      return cur
    elif filtertype == 1: # Sub
      xp = 0
      for xc in range(bpp,len(cur)):
        cur[xc] = (cur[xc] + cur[xp]) % 256
        xp = xp + 1
    elif filtertype == 2: # Up
      for xc in range(len(cur)):
        cur[xc] = (cur[xc] + prev[xc]) % 256
    elif filtertype == 3: # Average
      xp = 0
      for xc in range(len(cur)):
        cur[xc] = (cur[xc] + (cur[xp] + prev[xc])/2) % 256
        xp = xp + 1
    elif filtertype == 4: # Paeth
      xp = 0
      for i in range(bpp):
        cur[i] = (cur[i] + prev[i]) % 256
      for xc in range(bpp,len(cur)):
        a = cur[xp]
        b = prev[xc]
        c = prev[xp]
        p = a + b - c
        pa = abs(p - a)
        pb = abs(p - b)
        pc = abs(p - c)
        if pa <= pb and pa <= pc:
          value = a
        elif pb <= pc:
          value = b
        else:
          value = c
        cur[xc] = (cur[xc] + value) % 256
        xp = xp + 1
    else:
      raise TypeError('Unrecognized scanline filter type')
    return cur

  def chunks(self,f):
    while 1:
      try:
        length = struct.unpack("!I",f.read(4))[0]
        tag = f.read(4)
        data = f.read(length)
        crc = struct.unpack("!i",f.read(4))[0]
      except:
        return
      if zlib.crc32(tag + data) != crc:
        raise IOError
      yield [tag,data]

