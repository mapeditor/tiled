"""
  CreepyStruct - Convenience class for (un)packing structured binaries
  (c)2011-2012, <samuli@tuomola.net>

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
REPCK = r'(%s|[\s\w]+|,)\s+(\w+)%s%s[;]?%s' % (REFMT, RECMT, REARR, RECMT)

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

  def __init__(self, dat=None, **kws):
    """ Takes keyword arguments to initialize attributes """
    struct.Struct.__init__(self, getattr(self, '__fstr'))

    self.validate()

    if len(kws) > 0:
      for k in kws:
        setattr(self, k, kws[k])

    if dat != None:
      self.unpack(dat)

  def validate(self):
    "check that the extending classes specify valid struct"

    for i,(f,n,a,v) in enumerate(self.formats):
      if a != '' and not a.isdigit() and i < len(self.formats):
        for b in self.formats[i:]:
          if a=='' or b[2].isdigit():
            raise Exception('Varlength arrays only '+
              'supported at end of struct: %s[%s]' % (n,a))

  def pack(self):
    "convert member values to binary"
    ret = ''

    for i,(f,n,a,v) in enumerate(self.formats):
      v = getattr(self, n)
      if issubclass(v.__class__, CpySkeleton):
        ret += v.pack()
        continue

      if type(f) != type(struct.Struct) and f in fdict:
        f = fdict[f]

      if a != '':
        # array (possibly varlength)
        a = int(a) if a.isdigit() else getattr(self, a)
        if f=='c':
          # chararray as string
          f = str(a)+'s'
          a = 0

      if type(f) == type(struct.Struct):
        fstr = getattr(f, '__fstr')
        # temp hack because of qmap<str,str> limitations in tiled map props
        sz = struct.calcsize(fstr)
        m = re.match('<[0-9]*[sc]',fstr)
        if a != '' and a > 0 and (m is None or m.group(0) != fstr):
          for j in range(len(v)): #a):
            v2 = f(v[j].ljust(sz,'\0'))
            ret += v2.pack()
        elif type(v) == list:
          print('pack list') # todo
        else:
          #v = f(v.ljust(sz,'\0'))
          ret += struct.pack(getattr(f, '__fstr'), v)
      else:
        f = getattr(self, '__endianflag') + f
        if type(v) is list:
          for x in v:
            ret += struct.pack(f, x)
        else:
          # just an ordinary var
          ret += struct.pack(f, v)

    return ret

  def unpack(self, dat):
    """
    Takes a string, file, mmap or StringIO instance
    """
    rawpos = 0  # position in binary for custom types
    pos = 0  # in case substruct handles multiple values

    # readable, e.g. file, mmap, StringIO, BufferedReader
    if hasattr(dat,'read') and callable(dat.read):
      # this doesn't cover varlen members, for that dat is read directly
      buf = dat.read(len(self))
    else:
      buf = dat

    unpacked = None

    if hasattr(self, 'fromraw'):
      sz = struct.calcsize(getattr(self, '__fstr'))
      unpacked = self.fromraw(buf[:sz])

    if unpacked is None:
      unpacked = list(struct.Struct.unpack(self, buf))

    if hasattr(self, 'fromval'):
      unpacked = self.fromval(unpacked)

    if not hasattr(unpacked, '__iter__'):
      unpacked = [unpacked]

    #if len(self.formats) != len(unpacked):
    #  print unpacked
    #  raise LookupError('Unmatched number of unpacked variables: %i != %i'
    #    % (len(self.formats),len(unpacked)))

    for i,(f,n,a,v) in enumerate(self.formats):
      if a != '' and not a.isdigit(): break

      arlen = 0
      if a.isdigit() and (type(f) is type or fdict[f] != 'c'):
        arlen = int(a)

      v = unpacked[pos]

      if type(f) == type(struct.Struct):
        sz = struct.calcsize(getattr(f, '__fstr'))
        if arlen > 0:
          arr = []
          for i in range(arlen):
            arr.append( f(buf[rawpos:rawpos+sz]) )
            rawpos += sz
            pos += len(f.formats)
          setattr(self, n, arr)
        else:
          setattr(self, n, f(buf[rawpos:rawpos+sz]))
          rawpos += sz
          pos += len(f.formats)

      else:
        if arlen > 0:
          setattr(self, n, unpacked[pos:pos+arlen])
          pos += arlen
        else:
          #print(f,n,a,v)
          if f.endswith('s'): v = v.decode()  # strings for py3
          setattr(self, n, v)
          pos += 1

        rawpos += getattr(self, "__fsz")[i]

    # variable-length members
    for f,n,a,v in self.formats:
      if a != '' and not a.isdigit():
        c = getattr(self, a)

        # one level of custom varlen types should be enough for everyone
        if isinstance(c, struct.Struct):
          # would it be better to delegate this to the custom class?
          c = getattr(c, c.__slots__[0])

        #dat.seek(rawpos)
        if type(f) == type(struct.Struct):
          # custom types
          sz = struct.calcsize(getattr(f, '__fstr'))
          arr = []
          for i in range(c):
            arr.append( f(dat.read(sz)) )
            rawpos += sz
          setattr(self, n, arr)
        else:
          # primitives
          f = str(c)+'s' if fdict[f]=='c' else str(c)+fdict[f]
          sz = struct.calcsize(f)
          val = struct.unpack(f, dat.read(sz))
          #if len(val) == 1: val = val[0]
          rawpos += sz
          setattr(self, n, val)

    return buf

  def __len__(self):
    return struct.calcsize(getattr(self, '__fstr'))

  def __str__(self):
    ret = self.__class__.__name__+'['
    for f,n,a,v in self.formats:
      ret += '%s=%s,' % (n,getattr(self, n, ''))
    return ret[:-1]+']'


def peek(s, n):
  p = s.tell()
  r = s.read(n)
  s.seek(p)
  return r

def parseformat(fmt, callscope=None):
  fstr = ''
  sz = []
  for i,(f,n,a,v) in enumerate(fmt):
    if f == ',' and i > 0:
      fmt[i] = (fmt[i-1][0],n,a,v)
      f = fmt[i-1][0]
    elif f == ',':
      raise Exception('Unexpected comma at '+str(fmt[i]))

    fs = ''
    if f in fdict:
      if a.isdigit():
        fs = 's' if fdict[f] == 'c' else fdict[f]
      elif a != '' and not a.isdigit():
        # varlength array, read separately
        pass
      else:
        # C type
        fs = fdict[f]
    elif type(f) is type(CpySkeleton):
      # might have an alignment issue when mixing endians..
      fs = re.sub('[<>]?','',f.__fstr)
    elif f in callscope.f_globals:
      # resolve references to other CpyStructs
      fmt[i] = (callscope.f_globals[f],n,a,v)
      fs = re.sub('[<>]?','',fmt[i][0].__fstr)
    elif f[0] == ':':
      # struct format uses colon as prefix for explicitness
      fs = f[1:]
      fmt[i] = (f[1:],n,a,v)
    else:
      raise Exception('Unknown format at '+str(fmt[i]))

    if a.isdigit():
      # in case it's e.g. custom type of a string
      if fs[0].isdigit():
        fs *= int(a)
      else:
        fs = a + fs
    #elif a != '':
    #  fstr += '{'+a+'}'

    sz.append(struct.calcsize(fs))
    fstr += fs

  return (fmt, fstr, sz)

def CpyStruct(s, endianflg='<'):
  """ Call with a string specifying
  C-like struct to get a Struct class """
  # f=format, n=name, a=arraydef, v=default value
  fmt = [(f.strip(),n,a,v) for f,n,a,v in re.findall(REPCK, s)]
  # peek into caller's namespace in case they refer to custom classes
  callscope = sys._getframe(1)
  try:
    fmt, fstr, fsz = parseformat(fmt, callscope)
  finally:
    del callscope

  # for backwards compatibility
  if endianflg == True:
    endianflg = '>'
  elif endianflg == False:
    endianflg = '<'

  d = {}
  d['__endianflag'] = endianflg
  d['__fstr'] = endianflg + fstr
  d['__fsz'] = fsz
  d['__slots__'] = [n for f,n,a,v in fmt]
  d['formats'] = fmt

  for f,n,a,v in fmt:
    if v != '': d[n] = int(v,0)

  return type('', (CpySkeleton,), d)

