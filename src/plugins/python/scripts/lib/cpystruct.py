"""
  CreepyStruct - Convenience class for (un)packing structured binaries
  (c)2011, <samuli@tuomola.net>

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
import re, sys, struct

# struct.pack format characters prefixed by :
REFMT = r':[@!<>=]?[0-9xcbBhHiIlLqQfdspP]+'
# possible array definition or value assignment
REARR = r'(?:\[(\w+)\])?(?:\s*=\s*([0-9a-fx]+))?'
# skip C or python style comments to the end of line
RECMT = r'\s*(?:#.*|//.*)?'
# whole line with at least format/struct ref and attribute name
REPCK = r'(%s|[\s\w]+)\s+(\w+)%s%s[,;]?%s' % (REFMT, RECMT, REARR, RECMT)

FORMATS = {
  'c':'char',
  'b':'signed char|SBYTE',
  'B':'unsigned char|uchar|BYTE',
  '?':'_Bool|bool',
  'h':'short|SWORD',
  'H':'unsigned short|ushort|UWORD',
  'i':'int',
  'I':'unsigned int|uint|WORD',
  'l':'long',
  'L':'unsigned long|ulong|DWORD',
  'q':'long long',
  'Q':'unsigned long long',
  'f':'float',
  'd':'double'
}
fdict = {}
for f in FORMATS:
  fdict.update( [(p,f) for p in FORMATS[f].split('|')] )

class CpySkeleton(struct.Struct):
  """ Not to be used directly, use CpyStruct() to build a class """

  def __init__(self, **kws):
    """ Takes keyword arguments to initialize attributes """
    struct.Struct.__init__(self, getattr(self, '__fstr'))
    if kws != None:
      for k in kws: setattr(self, k, kws[k])

  def pack(self):
    ret = ''

    for i,(f,n,a,v) in enumerate(self.formats):
      v = getattr(self, n)
      if issubclass(v.__class__, CpySkeleton):
        ret += v.pack()
        continue

      f = fdict[f]
      if a != '':
        # array (possibly varlength)
        a = int(a) if a.isdigit() else getattr(self, a)
        if f=='c':
          # chararray as string
          f = str(a)+'s'
          a = ''

      if a != '':
        for j in range(a):
          ret += struct.pack(f, v[j])
      else:
        # just an ordinary var
        ret += struct.pack(f, v)

    return ret

  def unpack(self, dat):
    """ buf can be a string, file, mmap or StringIO instance
    atm returns read binary for testing purposes """
    if dat.__class__.__name__ in ('file','mmap', 'StringIO'):
      buf = dat.read(len(self))
    else:
      buf = dat

    unpacked = list(struct.Struct.unpack(self, buf))
    
    for i,(f,n,a,v) in enumerate(self.formats):
      if a != '' and not a.isdigit():
        if i < len(self.formats)-1:
          for b in self.formats[i:]:
            if not b[2].isdigit():
              raise Exception('Varlength arrays only supported as last elements in struct')
        break
      arlen = int(a) if a.isdigit() and fdict[f] != 'c' else 0

      if arlen > 0:
        # set an array for number of elements requested
        v = unpacked[i:i+arlen]
        #del unpacked[i:i+arlen]
      else:
        v = unpacked[i]

      if type(f) == type(struct.Struct):
        if hasattr(f, 'fromval'):
          if arlen > 0:
            # set an array for number of elements requested
            setattr(self, n, [f.fromval(e) for e in v])
          else:
            setattr(self, n, f.fromval(v))
        else:
          raise Exception('please define %s.fromval classmethod' % self)
      else:
        setattr(self, n, v)

    # variable-length members
    for f,n,a,v in self.formats:
      if a != '' and not a.isdigit():
        c = getattr(self, a)
        f = str(c)+'s' if fdict[f]=='c' else str(c)+fdict[f]
        sz = struct.calcsize(f)
        setattr(self, n, struct.unpack(f, dat.read(sz)))
    return buf

  def __len__(self):
    return struct.calcsize(getattr(self, '__fstr'))

  def __str__(self):
    ret = self.__class__.__name__+'['
    for f,n,a,v in self.formats:
      ret += '%s=%s,' % (n,getattr(self, n))
    return ret[:-1]+']'


def peek(s, n):
  p = s.tell()
  r = s.read(n)
  s.seek(p)
  return r

def parseformat(fmt, callscope=None):
  fstr = ''
  for i,(f,n,a,v) in enumerate(fmt):
    if a.isdigit():
      fstr += a
    #elif a != '':
    #  fstr += '{'+a+'}'

    if fdict.has_key(f):
      if a.isdigit():
        fstr += 's' if fdict[f] == 'c' else fdict[f]
      elif a != '' and not a.isdigit():
        # varlength array, read separately
        pass
      else:
        # C type
        fstr += fdict[f]
    elif f[0] == ':':
      # struct format uses colon as prefix for explicitness
      fstr += f[1:]
      fmt[i] = (f[1:],n,a,v)
    elif callscope.f_globals.has_key(f):
      # resolve references to other CpyStructs
      fmt[i] = (callscope.f_globals[f],n,a,v)
      fstr += re.sub('<?','',fmt[i][0].__fstr)
    else:
      raise Exception('Unknown format: '+f)
  return (fmt, fstr)

def CpyStruct(s, bigendian=False):
  """ Call with a string specifying
  C-like struct to get a Struct class """
  # f=format, n=name, a=arraydef, v=default value
  fmt = [(f.strip(),n,a,v) for f,n,a,v in re.findall(REPCK, s)]

  # peek into caller's namespace in case they refer to custom classes
  callscope = sys._getframe(1)
  try:
    (fmt,fstr) = parseformat(fmt, callscope)
  finally:
    del callscope

  #print fmt,fstr
  d = {}
  d['__fstr'] = ('>' if bigendian else '<') + fstr
  d['__slots__'] = [n for f,n,a,v in fmt]
  d['formats'] = fmt

  for f,n,a,v in fmt:
    if v != '': d[n] = int(v,0)

  return type('', (CpySkeleton,), d)

