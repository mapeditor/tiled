
from .cpystruct import *

"""
#define AN_END -1     /* Animation types, AN_END = end of anims */
#define AN_NONE 0     /* No anim defined */
#define AN_LOOPF 1    /* Loops from start to end, then jumps to start etc */
#define AN_LOOPR 2    /* As above, but from end to start */
#define AN_ONCE 3     /* Only plays once */
#define AN_ONCEH 4    /* Only plays once, but holds end frame */
#define AN_PPFF 5     /* Ping Pong start-end-start-end-start etc */
#define AN_PPRR 6     /* Ping Pong end-start-end-start-end etc */
#define AN_PPRF 7     /* Used internally by playback */
#define AN_PPFR 8     /* Used internally by playback */
#define AN_ONCES 9    /* Used internally by playback */
"""

class BLKSTR(CpyStruct("""
int olay[4];
unsigned int user1;
unsigned int user2; // user long data */
unsigned short user3;
unsigned short user4;  // user short data */
unsigned char user5;
unsigned char user6;
unsigned char user7;  // user byte data */
unsigned char tl; // bits for collision detection */
""")): pass
"""for newer fmp version?
unsigned char tr;
unsigned char bl;
unsigned char br;
unsigned char trigger;  // bit to trigger an event */
unsigned char unused1;
unsigned char unused2;
unsigned char unused3;
"""

class ANISTR(CpyStruct("""
signed char antype; // Type of anim, AN_? */
signed char andelay;  // Frames to go before next frame */
signed char ancount;  // Counter, decs each frame, till 0, then resets to andelay */
signed char anuser; // User info */
long ancuroff;  // Points to current offset in list */
long anstartoff;  // Points to start of blkstr offsets list, AFTER ref. blkstr offset */
long anendoff;
""")): pass

class OBJSTR(CpyStruct("""
int xpos; int ypos; // pixel position in map to handle */
int gfxid; int tileid;
int gxoff; int gyoff; // offset into graphic */
int gwidth; int gheight;
int ghandlexoff; int ghandleyoff; // handle pos, from gxoff, gyoff */
int show; // display mode */
int user1; int user2; int user3; int user4; int user5; int user6; int user7;
int flags;
""")): pass

class GENHEAD(CpyStruct("""
char id1; char id2; char id3; char id4;  // 4 byte header id. */
long headsize;    // size of header chunk. */
""")): pass

#// char M,P,H,D;  4 byte chunk identification. */
#// long int mphdsize; size of map header. */
class MPHD(CpyStruct("""
BYTE mapverhigh;  // map version number to left of . (ie X.0). */
BYTE mapverlow;   // map version number to right of . (ie 0.X). */
BYTE lsb;   // if 1, data stored LSB first, otherwise MSB first. */
BYTE type;  // 0=old, 1=new, 2=iso
short mapwidth; // width in blocks. */
short mapheight;  // height in blocks. */
short reserved1;
short reserved2;
short blockwidth; // width of a block (tile) in pixels. */
short blockheight;  // height of a block (tile) in pixels. */
short blockdepth; // depth of a block (tile) in planes (ie. 256 colours is 8) */
short blockstrsize; // size of a block data structure */
short numblockstr;  // Number of block structures in BKDT */
short numblockgfx;  // Number of 'blocks' in graphics (BODY) */
BYTE skip;
SBYTE trans8bit;
short transhi;
""")): pass

#// char E,D,H,D;  4 byte chunk identification. */
#// long int edhdsize; size of editor header. */
class EDHD(CpyStruct("""
short  xmapoffset; // editor offset, in blocks, from left. */
short  ymapoffset; // editor offset, in blocks, from right. */
long fgcolour;  // fg colour for text, buttons etc. */
long bgcolour;  // bg colour for text, buttons etc. */
short swidth;   // width of current screen res */
short sheight;  // height of current screen res */
short strtstr;  // first structure in view */
short strtblk;  // first block graphic in view */
short curstr;   // current block structure */
short curanim;  // current anim structure */
short animspd;  // gap in frames between anims */
short span;   // control panel height */
short numbrushes; // number of brushes to follow. */
""")): pass

class MAPFILL(CpyStruct("short leftedge, width, yoff;")): pass

class fmpchunk(CpyStruct(":4s id; :I len;",True)): pass

class BODY(CpyStruct(":H dat[len]", True)): pass

