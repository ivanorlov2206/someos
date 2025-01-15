#include "framebuffer.h"
#include "paging.h"
#include "string.h"

static const uint8_t letters[256][8] = {
{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},  // 0000 (uni0000.dup1)
    {0x7e, 0x81, 0xa5, 0x81, 0xbd, 0x99, 0x81, 0x7e},  // 0001 (uni0001)
    {0x7e, 0xff, 0xdb, 0xff, 0xc3, 0xe7, 0xff, 0x7e},  // 0002 (uni0002)
    {0x6c, 0xfe, 0xfe, 0xfe, 0x7c, 0x38, 0x10, 0x00},  // 0003 (uni0003)
    {0x10, 0x38, 0x7c, 0xfe, 0x7c, 0x38, 0x10, 0x00},  // 0004 (uni0004)
    {0x38, 0x7c, 0x38, 0xfe, 0xfe, 0x7c, 0x38, 0x7c},  // 0005 (uni0005)
    {0x10, 0x10, 0x38, 0x7c, 0xfe, 0x7c, 0x38, 0x7c},  // 0006 (uni0006)
    {0x00, 0x00, 0x18, 0x3c, 0x3c, 0x18, 0x00, 0x00},  // 0007 (uni0007)
    {0xff, 0xff, 0xe7, 0xc3, 0xc3, 0xe7, 0xff, 0xff},  // 0008 (uni0008)
    {0x00, 0x3c, 0x66, 0x42, 0x42, 0x66, 0x3c, 0x00},  // 0009 (uni0009)
    {0xff, 0xc3, 0x99, 0xbd, 0xbd, 0x99, 0xc3, 0xff},  // 000a (uni000A)
    {0x0f, 0x07, 0x0f, 0x7d, 0xcc, 0xcc, 0xcc, 0x78},  // 000b (uni000B)
    {0x3c, 0x66, 0x66, 0x66, 0x3c, 0x18, 0x7e, 0x18},  // 000c (uni000C)
    {0x3f, 0x33, 0x3f, 0x30, 0x30, 0x70, 0xf0, 0xe0},  // 000d (uni000D)
    {0x7f, 0x63, 0x7f, 0x63, 0x63, 0x67, 0xe6, 0xc0},  // 000e (uni000E)
    {0x99, 0x5a, 0x3c, 0xe7, 0xe7, 0x3c, 0x5a, 0x99},  // 000f (uni000F)
    {0x80, 0xe0, 0xf8, 0xfe, 0xf8, 0xe0, 0x80, 0x00},  // 0010 (uni0010)
    {0x02, 0x0e, 0x3e, 0xfe, 0x3e, 0x0e, 0x02, 0x00},  // 0011 (uni0011)
    {0x18, 0x3c, 0x7e, 0x18, 0x18, 0x7e, 0x3c, 0x18},  // 0012 (uni0012)
    {0x66, 0x66, 0x66, 0x66, 0x66, 0x00, 0x66, 0x00},  // 0013 (uni0013)
    {0x7f, 0xdb, 0xdb, 0x7b, 0x1b, 0x1b, 0x1b, 0x00},  // 0014 (uni0014)
    {0x3e, 0x63, 0x38, 0x6c, 0x6c, 0x38, 0xcc, 0x78},  // 0015 (uni0015)
    {0x00, 0x00, 0x00, 0x00, 0x7e, 0x7e, 0x7e, 0x00},  // 0016 (uni0016)
    {0x18, 0x3c, 0x7e, 0x18, 0x7e, 0x3c, 0x18, 0xff},  // 0017 (uni0017)
    {0x18, 0x3c, 0x7e, 0x18, 0x18, 0x18, 0x18, 0x00},  // 0018 (uni0018)
    {0x18, 0x18, 0x18, 0x18, 0x7e, 0x3c, 0x18, 0x00},  // 0019 (uni0019)
    {0x00, 0x18, 0x0c, 0xfe, 0x0c, 0x18, 0x00, 0x00},  // 001a (uni001A)
    {0x00, 0x30, 0x60, 0xfe, 0x60, 0x30, 0x00, 0x00},  // 001b (uni001B)
    {0x00, 0x00, 0xc0, 0xc0, 0xc0, 0xfe, 0x00, 0x00},  // 001c (uni001C)
    {0x00, 0x24, 0x66, 0xff, 0x66, 0x24, 0x00, 0x00},  // 001d (uni001D)
    {0x00, 0x18, 0x3c, 0x7e, 0xff, 0xff, 0x00, 0x00},  // 001e (uni001E)
    {0x00, 0xff, 0xff, 0x7e, 0x3c, 0x18, 0x00, 0x00},  // 001f (uni001F)
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},  // 0020 (space)
    {0x30, 0x78, 0x78, 0x30, 0x30, 0x00, 0x30, 0x00},  // 0021 (exclam)
    {0x6c, 0x6c, 0x6c, 0x00, 0x00, 0x00, 0x00, 0x00},  // 0022 (quotedbl)
    {0x6c, 0x6c, 0xfe, 0x6c, 0xfe, 0x6c, 0x6c, 0x00},  // 0023 (numbersign)
    {0x30, 0x7c, 0xc0, 0x78, 0x0c, 0xf8, 0x30, 0x00},  // 0024 (dollar)
    {0x00, 0xc6, 0xcc, 0x18, 0x30, 0x66, 0xc6, 0x00},  // 0025 (percent)
    {0x38, 0x6c, 0x38, 0x76, 0xdc, 0xcc, 0x76, 0x00},  // 0026 (ampersand)
    {0x60, 0x60, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00},  // 0027 (quotesingle)
    {0x18, 0x30, 0x60, 0x60, 0x60, 0x30, 0x18, 0x00},  // 0028 (parenleft)
    {0x60, 0x30, 0x18, 0x18, 0x18, 0x30, 0x60, 0x00},  // 0029 (parenright)
    {0x00, 0x66, 0x3c, 0xff, 0x3c, 0x66, 0x00, 0x00},  // 002a (asterisk)
    {0x00, 0x30, 0x30, 0xfc, 0x30, 0x30, 0x00, 0x00},  // 002b (plus)
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 0x30, 0x60},  // 002c (comma)
    {0x00, 0x00, 0x00, 0xfc, 0x00, 0x00, 0x00, 0x00},  // 002d (hyphen)
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 0x30, 0x00},  // 002e (period)
    {0x06, 0x0c, 0x18, 0x30, 0x60, 0xc0, 0x80, 0x00},  // 002f (slash)
    {0x7c, 0xc6, 0xce, 0xde, 0xf6, 0xe6, 0x7c, 0x00},  // 0030 (zero)
    {0x30, 0x70, 0x30, 0x30, 0x30, 0x30, 0xfc, 0x00},  // 0031 (one)
    {0x78, 0xcc, 0x0c, 0x38, 0x60, 0xc4, 0xfc, 0x00},  // 0032 (two)
    {0x78, 0xcc, 0x0c, 0x38, 0x0c, 0xcc, 0x78, 0x00},  // 0033 (three)
    {0x1c, 0x3c, 0x6c, 0xcc, 0xfe, 0x0c, 0x1e, 0x00},  // 0034 (four)
    {0xfc, 0xc0, 0xf8, 0x0c, 0x0c, 0xcc, 0x78, 0x00},  // 0035 (five)
    {0x38, 0x60, 0xc0, 0xf8, 0xcc, 0xcc, 0x78, 0x00},  // 0036 (six)
    {0xfc, 0xcc, 0x0c, 0x18, 0x30, 0x30, 0x30, 0x00},  // 0037 (seven)
    {0x78, 0xcc, 0xcc, 0x78, 0xcc, 0xcc, 0x78, 0x00},  // 0038 (eight)
    {0x78, 0xcc, 0xcc, 0x7c, 0x0c, 0x18, 0x70, 0x00},  // 0039 (nine)
    {0x00, 0x30, 0x30, 0x00, 0x00, 0x30, 0x30, 0x00},  // 003a (colon)
    {0x00, 0x30, 0x30, 0x00, 0x30, 0x30, 0x60, 0x00},  // 003b (semicolon)
    {0x18, 0x30, 0x60, 0xc0, 0x60, 0x30, 0x18, 0x00},  // 003c (less)
    {0x00, 0x00, 0xfc, 0x00, 0x00, 0xfc, 0x00, 0x00},  // 003d (equal)
    {0x60, 0x30, 0x18, 0x0c, 0x18, 0x30, 0x60, 0x00},  // 003e (greater)
    {0x78, 0xcc, 0x0c, 0x18, 0x30, 0x00, 0x30, 0x00},  // 003f (question)
    {0x7c, 0xc6, 0xde, 0xde, 0xde, 0xc0, 0x78, 0x00},  // 0040 (at)
    {0x30, 0x78, 0xcc, 0xcc, 0xfc, 0xcc, 0xcc, 0x00},  // 0041 (A)
    {0xfc, 0x66, 0x66, 0x7c, 0x66, 0x66, 0xfc, 0x00},  // 0042 (B)
    {0x3c, 0x66, 0xc0, 0xc0, 0xc0, 0x66, 0x3c, 0x00},  // 0043 (C)
    {0xf8, 0x6c, 0x66, 0x66, 0x66, 0x6c, 0xf8, 0x00},  // 0044 (D)
    {0xfe, 0x62, 0x68, 0x78, 0x68, 0x62, 0xfe, 0x00},  // 0045 (E)
    {0xfe, 0x62, 0x68, 0x78, 0x68, 0x60, 0xf0, 0x00},  // 0046 (F)
    {0x3c, 0x66, 0xc0, 0xc0, 0xce, 0x66, 0x3e, 0x00},  // 0047 (G)
    {0xcc, 0xcc, 0xcc, 0xfc, 0xcc, 0xcc, 0xcc, 0x00},  // 0048 (H)
    {0x78, 0x30, 0x30, 0x30, 0x30, 0x30, 0x78, 0x00},  // 0049 (I)
    {0x1e, 0x0c, 0x0c, 0x0c, 0xcc, 0xcc, 0x78, 0x00},  // 004a (J)
    {0xe6, 0x66, 0x6c, 0x78, 0x6c, 0x66, 0xe6, 0x00},  // 004b (K)
    {0xf0, 0x60, 0x60, 0x60, 0x62, 0x66, 0xfe, 0x00},  // 004c (L)
    {0xc6, 0xee, 0xfe, 0xfe, 0xd6, 0xc6, 0xc6, 0x00},  // 004d (M)
    {0xc6, 0xe6, 0xf6, 0xde, 0xce, 0xc6, 0xc6, 0x00},  // 004e (N)
    {0x38, 0x6c, 0xc6, 0xc6, 0xc6, 0x6c, 0x38, 0x00},  // 004f (O)
    {0xfc, 0x66, 0x66, 0x7c, 0x60, 0x60, 0xf0, 0x00},  // 0050 (P)
    {0x78, 0xcc, 0xcc, 0xcc, 0xdc, 0x78, 0x1c, 0x00},  // 0051 (Q)
    {0xfc, 0x66, 0x66, 0x7c, 0x6c, 0x66, 0xe6, 0x00},  // 0052 (R)
    {0x78, 0xcc, 0xe0, 0x70, 0x1c, 0xcc, 0x78, 0x00},  // 0053 (S)
    {0xfc, 0xb4, 0x30, 0x30, 0x30, 0x30, 0x78, 0x00},  // 0054 (T)
    {0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xfc, 0x00},  // 0055 (U)
    {0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0x78, 0x30, 0x00},  // 0056 (V)
    {0xc6, 0xc6, 0xc6, 0xd6, 0xfe, 0xee, 0xc6, 0x00},  // 0057 (W)
    {0xc6, 0xc6, 0x6c, 0x38, 0x38, 0x6c, 0xc6, 0x00},  // 0058 (X)
    {0xcc, 0xcc, 0xcc, 0x78, 0x30, 0x30, 0x78, 0x00},  // 0059 (Y)
    {0xfe, 0xc6, 0x8c, 0x18, 0x32, 0x66, 0xfe, 0x00},  // 005a (Z)
    {0x78, 0x60, 0x60, 0x60, 0x60, 0x60, 0x78, 0x00},  // 005b (bracketleft)
    {0xc0, 0x60, 0x30, 0x18, 0x0c, 0x06, 0x02, 0x00},  // 005c (backslash)
    {0x78, 0x18, 0x18, 0x18, 0x18, 0x18, 0x78, 0x00},  // 005d (bracketright)
    {0x10, 0x38, 0x6c, 0xc6, 0x00, 0x00, 0x00, 0x00},  // 005e (asciicircum)
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff},  // 005f (underscore)
    {0x30, 0x30, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00},  // 0060 (grave)
    {0x00, 0x00, 0x78, 0x0c, 0x7c, 0xcc, 0x76, 0x00},  // 0061 (a)
    {0xe0, 0x60, 0x60, 0x7c, 0x66, 0x66, 0xdc, 0x00},  // 0062 (b)
    {0x00, 0x00, 0x78, 0xcc, 0xc0, 0xcc, 0x78, 0x00},  // 0063 (c)
    {0x1c, 0x0c, 0x0c, 0x7c, 0xcc, 0xcc, 0x76, 0x00},  // 0064 (d)
    {0x00, 0x00, 0x78, 0xcc, 0xfc, 0xc0, 0x78, 0x00},  // 0065 (e)
    {0x38, 0x6c, 0x60, 0xf0, 0x60, 0x60, 0xf0, 0x00},  // 0066 (f)
    {0x00, 0x00, 0x76, 0xcc, 0xcc, 0x7c, 0x0c, 0xf8},  // 0067 (g)
    {0xe0, 0x60, 0x6c, 0x76, 0x66, 0x66, 0xe6, 0x00},  // 0068 (h)
    {0x30, 0x00, 0x70, 0x30, 0x30, 0x30, 0x78, 0x00},  // 0069 (i)
    {0x0c, 0x00, 0x0c, 0x0c, 0x0c, 0xcc, 0xcc, 0x78},  // 006a (j)
    {0xe0, 0x60, 0x66, 0x6c, 0x78, 0x6c, 0xe6, 0x00},  // 006b (k)
    {0x70, 0x30, 0x30, 0x30, 0x30, 0x30, 0x78, 0x00},  // 006c (l)
    {0x00, 0x00, 0xcc, 0xfe, 0xfe, 0xd6, 0xc6, 0x00},  // 006d (m)
    {0x00, 0x00, 0xf8, 0xcc, 0xcc, 0xcc, 0xcc, 0x00},  // 006e (n)
    {0x00, 0x00, 0x78, 0xcc, 0xcc, 0xcc, 0x78, 0x00},  // 006f (o)
    {0x00, 0x00, 0xdc, 0x66, 0x66, 0x7c, 0x60, 0xf0},  // 0070 (p)
    {0x00, 0x00, 0x76, 0xcc, 0xcc, 0x7c, 0x0c, 0x1e},  // 0071 (q)
    {0x00, 0x00, 0xdc, 0x76, 0x66, 0x60, 0xf0, 0x00},  // 0072 (r)
    {0x00, 0x00, 0x7c, 0xc0, 0x78, 0x0c, 0xf8, 0x00},  // 0073 (s)
    {0x10, 0x30, 0x7c, 0x30, 0x30, 0x34, 0x18, 0x00},  // 0074 (t)
    {0x00, 0x00, 0xcc, 0xcc, 0xcc, 0xcc, 0x76, 0x00},  // 0075 (u)
    {0x00, 0x00, 0xcc, 0xcc, 0xcc, 0x78, 0x30, 0x00},  // 0076 (v)
    {0x00, 0x00, 0xc6, 0xd6, 0xfe, 0xfe, 0x6c, 0x00},  // 0077 (w)
    {0x00, 0x00, 0xc6, 0x6c, 0x38, 0x6c, 0xc6, 0x00},  // 0078 (x)
    {0x00, 0x00, 0xcc, 0xcc, 0xcc, 0x7c, 0x0c, 0xf8},  // 0079 (y)
    {0x00, 0x00, 0xfc, 0x98, 0x30, 0x64, 0xfc, 0x00},  // 007a (z)
    {0x1c, 0x30, 0x30, 0xe0, 0x30, 0x30, 0x1c, 0x00},  // 007b (braceleft)
    {0x18, 0x18, 0x18, 0x00, 0x18, 0x18, 0x18, 0x00},  // 007c (bar)
    {0xe0, 0x30, 0x30, 0x1c, 0x30, 0x30, 0xe0, 0x00},  // 007d (braceright)
    {0x76, 0xdc, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},  // 007e (asciitilde)
    {0x00, 0x10, 0x38, 0x6c, 0xc6, 0xc6, 0xfe, 0x00},  // 007f (uni007F)
    // ---------------------------  160-255 ---------------------------
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},  // 00a0 (uni00A0)
    {0x18, 0x18, 0x00, 0x18, 0x18, 0x18, 0x18, 0x00},  // 00a1 (exclamdown)
    {0x18, 0x18, 0x7e, 0xc0, 0xc0, 0x7e, 0x18, 0x18},  // 00a2 (cent)
    {0x38, 0x6c, 0x64, 0xf0, 0x60, 0xe6, 0xfc, 0x00},  // 00a3 (sterling)
    {0x00, 0xc6, 0x7c, 0xc6, 0xc6, 0x7c, 0xc6, 0x00},  // 00a4 (currency)
    {0xcc, 0xcc, 0x78, 0xfc, 0x30, 0xfc, 0x30, 0x30},  // 00a5 (yen)
    {0x18, 0x18, 0x18, 0x00, 0x18, 0x18, 0x18, 0x00},  // 00a6 (brokenbar)
    {0x3e, 0x63, 0x38, 0x6c, 0x6c, 0x38, 0xcc, 0x78},  // 00a7 (section)
    {0xc6, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},  // 00a8 (dieresis)
    {0x7e, 0x81, 0x9d, 0xa1, 0xa1, 0x9d, 0x81, 0x7e},  // 00a9 (copyright)
    {0x3c, 0x6c, 0x6c, 0x3e, 0x00, 0x7e, 0x00, 0x00},  // 00aa (ordfeminine)
    {0x00, 0x33, 0x66, 0xcc, 0x66, 0x33, 0x00, 0x00},  // 00ab (guillemotleft)
    {0x00, 0x00, 0x00, 0xfc, 0x0c, 0x0c, 0x00, 0x00},  // 00ac (logicalnot)
    {0x00, 0x00, 0x00, 0x7e, 0x00, 0x00, 0x00, 0x00},  // 00ad (uni00AD)
    {0x7e, 0x81, 0xb9, 0xa5, 0xb9, 0xa5, 0x81, 0x7e},  // 00ae (registered)
    {0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},  // 00af (macron)
    {0x38, 0x6c, 0x6c, 0x38, 0x00, 0x00, 0x00, 0x00},  // 00b0 (degree)
    {0x30, 0x30, 0xfc, 0x30, 0x30, 0x00, 0xfc, 0x00},  // 00b1 (plusminus)
    {0x70, 0x18, 0x30, 0x60, 0x78, 0x00, 0x00, 0x00},  // 00b2 (uni00B2)
    {0x70, 0x18, 0x30, 0x18, 0x70, 0x00, 0x00, 0x00},  // 00b3 (uni00B3)
    {0x0c, 0x18, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00},  // 00b4 (acute)
    {0x00, 0x66, 0x66, 0x66, 0x66, 0x7c, 0x60, 0xc0},  // 00b5 (mu)
    {0x7f, 0xdb, 0xdb, 0x7b, 0x1b, 0x1b, 0x1b, 0x00},  // 00b6 (paragraph)
    {0x00, 0x00, 0x00, 0x00, 0x18, 0x00, 0x00, 0x00},  // 00b7 (periodcentered)
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x0c, 0x38},  // 00b8 (cedilla)
    {0x30, 0x70, 0x30, 0x30, 0x78, 0x00, 0x00, 0x00},  // 00b9 (uni00B9)
    {0x38, 0x6c, 0x6c, 0x38, 0x00, 0x7c, 0x00, 0x00},  // 00ba (ordmasculine)
    {0x00, 0xcc, 0x66, 0x33, 0x66, 0xcc, 0x00, 0x00},  // 00bb (guillemotright)
    {0xc3, 0xc6, 0xcc, 0xdb, 0x37, 0x6f, 0xcf, 0x03},  // 00bc (onequarter)
    {0xc3, 0xc6, 0xcc, 0xde, 0x33, 0x66, 0xcc, 0x0f},  // 00bd (onehalf)
    {0xe0, 0x33, 0x66, 0x3c, 0xfb, 0x37, 0x6f, 0xc3},  // 00be (threequarters)
    {0x30, 0x00, 0x30, 0x60, 0xc0, 0xcc, 0x78, 0x00},  // 00bf (questiondown)
    {0xc0, 0x38, 0x6c, 0xc6, 0xfe, 0xc6, 0xc6, 0x00},  // 00c0 (Agrave)
    {0x06, 0x38, 0x6c, 0xc6, 0xfe, 0xc6, 0xc6, 0x00},  // 00c1 (Aacute)
    {0x7c, 0xc6, 0x38, 0x6c, 0xc6, 0xfe, 0xc6, 0x00},  // 00c2 (Acircumflex)
    {0x76, 0xdc, 0x38, 0x6c, 0xc6, 0xfe, 0xc6, 0x00},  // 00c3 (Atilde)
    {0xc6, 0x38, 0x6c, 0xc6, 0xfe, 0xc6, 0xc6, 0x00},  // 00c4 (Adieresis)
    {0x30, 0x30, 0x00, 0x78, 0xcc, 0xfc, 0xcc, 0x00},  // 00c5 (Aring)
    {0x3e, 0x6c, 0xcc, 0xfe, 0xcc, 0xcc, 0xce, 0x00},  // 00c6 (AE)
    {0x78, 0xcc, 0xc0, 0xcc, 0x78, 0x18, 0x0c, 0x78},  // 00c7 (Ccedilla)
    {0xe0, 0x00, 0xfc, 0x60, 0x78, 0x60, 0xfc, 0x00},  // 00c8 (Egrave)
    {0x1c, 0x00, 0xfc, 0x60, 0x78, 0x60, 0xfc, 0x00},  // 00c9 (Eacute)
    {0x78, 0xcc, 0xfc, 0x60, 0x78, 0x60, 0xfc, 0x00},  // 00ca (Ecircumflex)
    {0xcc, 0x00, 0xfc, 0x60, 0x78, 0x60, 0xfc, 0x00},  // 00cb (Edieresis)
    {0xe0, 0x00, 0x78, 0x30, 0x30, 0x30, 0x78, 0x00},  // 00cc (Igrave)
    {0x1c, 0x00, 0x78, 0x30, 0x30, 0x30, 0x78, 0x00},  // 00cd (Iacute)
    {0x7e, 0xc3, 0x3c, 0x18, 0x18, 0x18, 0x3c, 0x00},  // 00ce (Icircumflex)
    {0xcc, 0x00, 0x78, 0x30, 0x30, 0x30, 0x78, 0x00},  // 00cf (Idieresis)
    {0xf8, 0x6c, 0x66, 0xf6, 0x66, 0x6c, 0xf8, 0x00},  // 00d0 (Eth)
    {0xfc, 0x00, 0xcc, 0xec, 0xfc, 0xdc, 0xcc, 0x00},  // 00d1 (Ntilde)
    {0xc0, 0x38, 0x6c, 0xc6, 0xc6, 0x6c, 0x38, 0x00},  // 00d2 (Ograve)
    {0x06, 0x38, 0x6c, 0xc6, 0xc6, 0x6c, 0x38, 0x00},  // 00d3 (Oacute)
    {0x7c, 0xc6, 0x38, 0x6c, 0xc6, 0x6c, 0x38, 0x00},  // 00d4 (Ocircumflex)
    {0x76, 0xdc, 0x38, 0x6c, 0xc6, 0x6c, 0x38, 0x00},  // 00d5 (Otilde)
    {0xc3, 0x18, 0x3c, 0x66, 0x66, 0x3c, 0x18, 0x00},  // 00d6 (Odieresis)
    {0x00, 0xc6, 0x6c, 0x38, 0x6c, 0xc6, 0x00, 0x00},  // 00d7 (multiply)
    {0x3a, 0x6c, 0xce, 0xd6, 0xe6, 0x6c, 0xb8, 0x00},  // 00d8 (Oslash)
    {0xe0, 0x00, 0xcc, 0xcc, 0xcc, 0xcc, 0x78, 0x00},  // 00d9 (Ugrave)
    {0x1c, 0x00, 0xcc, 0xcc, 0xcc, 0xcc, 0x78, 0x00},  // 00da (Uacute)
    {0x7c, 0xc6, 0x00, 0xc6, 0xc6, 0xc6, 0x7c, 0x00},  // 00db (Ucircumflex)
    {0xcc, 0x00, 0xcc, 0xcc, 0xcc, 0xcc, 0x78, 0x00},  // 00dc (Udieresis)
    {0x1c, 0x00, 0xcc, 0xcc, 0x78, 0x30, 0x78, 0x00},  // 00dd (Yacute)
    {0xf0, 0x60, 0x7c, 0x66, 0x7c, 0x60, 0xf0, 0x00},  // 00de (Thorn)
    {0x78, 0xcc, 0xcc, 0xd8, 0xcc, 0xc6, 0xcc, 0x00},  // 00df (germandbls)
    {0xe0, 0x00, 0x78, 0x0c, 0x7c, 0xcc, 0x7e, 0x00},  // 00e0 (agrave)
    {0x1c, 0x00, 0x78, 0x0c, 0x7c, 0xcc, 0x7e, 0x00},  // 00e1 (aacute)
    {0x7e, 0xc3, 0x3c, 0x06, 0x3e, 0x66, 0x3f, 0x00},  // 00e2 (acircumflex)
    {0x76, 0xdc, 0x78, 0x0c, 0x7c, 0xcc, 0x7e, 0x00},  // 00e3 (atilde)
    {0xcc, 0x00, 0x78, 0x0c, 0x7c, 0xcc, 0x7e, 0x00},  // 00e4 (adieresis)
    {0x30, 0x30, 0x78, 0x0c, 0x7c, 0xcc, 0x7e, 0x00},  // 00e5 (aring)
    {0x00, 0x00, 0x7f, 0x0c, 0x7f, 0xcc, 0x7f, 0x00},  // 00e6 (ae)
    {0x00, 0x00, 0x78, 0xc0, 0xc0, 0x78, 0x0c, 0x38},  // 00e7 (ccedilla)
    {0xe0, 0x00, 0x78, 0xcc, 0xfc, 0xc0, 0x78, 0x00},  // 00e8 (egrave)
    {0x1c, 0x00, 0x78, 0xcc, 0xfc, 0xc0, 0x78, 0x00},  // 00e9 (eacute)
    {0x7e, 0xc3, 0x3c, 0x66, 0x7e, 0x60, 0x3c, 0x00},  // 00ea (ecircumflex)
    {0xcc, 0x00, 0x78, 0xcc, 0xfc, 0xc0, 0x78, 0x00},  // 00eb (edieresis)
    {0xe0, 0x00, 0x70, 0x30, 0x30, 0x30, 0x78, 0x00},  // 00ec (igrave)
    {0x38, 0x00, 0x70, 0x30, 0x30, 0x30, 0x78, 0x00},  // 00ed (iacute)
    {0x7c, 0xc6, 0x38, 0x18, 0x18, 0x18, 0x3c, 0x00},  // 00ee (icircumflex)
    {0xcc, 0x00, 0x70, 0x30, 0x30, 0x30, 0x78, 0x00},  // 00ef (idieresis)
    {0x30, 0x7e, 0x0c, 0x7c, 0xcc, 0xcc, 0x78, 0x00},  // 00f0 (eth)
    {0x00, 0xf8, 0x00, 0xf8, 0xcc, 0xcc, 0xcc, 0x00},  // 00f1 (ntilde)
    {0x00, 0xe0, 0x00, 0x78, 0xcc, 0xcc, 0x78, 0x00},  // 00f2 (ograve)
    {0x00, 0x1c, 0x00, 0x78, 0xcc, 0xcc, 0x78, 0x00},  // 00f3 (oacute)
    {0x78, 0xcc, 0x00, 0x78, 0xcc, 0xcc, 0x78, 0x00},  // 00f4 (ocircumflex)
    {0x76, 0xdc, 0x00, 0x78, 0xcc, 0xcc, 0x78, 0x00},  // 00f5 (otilde)
    {0x00, 0xcc, 0x00, 0x78, 0xcc, 0xcc, 0x78, 0x00},  // 00f6 (odieresis)
    {0x30, 0x30, 0x00, 0xfc, 0x00, 0x30, 0x30, 0x00},  // 00f7 (divide)
    {0x00, 0x02, 0x7c, 0xce, 0xd6, 0xe6, 0x7c, 0x80},  // 00f8 (oslash)
    {0x00, 0xe0, 0x00, 0xcc, 0xcc, 0xcc, 0x7e, 0x00},  // 00f9 (ugrave)
    {0x00, 0x1c, 0x00, 0xcc, 0xcc, 0xcc, 0x7e, 0x00},  // 00fa (uacute)
    {0x78, 0xcc, 0x00, 0xcc, 0xcc, 0xcc, 0x7e, 0x00},  // 00fb (ucircumflex)
    {0x00, 0xcc, 0x00, 0xcc, 0xcc, 0xcc, 0x7e, 0x00},  // 00fc (udieresis)
    {0x00, 0x1c, 0x00, 0xcc, 0xcc, 0x7c, 0x0c, 0x78},  // 00fd (yacute)
    {0xe0, 0x60, 0x7c, 0x66, 0x66, 0x7c, 0x60, 0xf0},  // 00fe (thorn)
    {0x00, 0xcc, 0x00, 0xcc, 0xcc, 0x7c, 0x0c, 0xf8},  // 00ff (ydieresis) 
};

struct fb_console {
	uint64_t cols;
	uint64_t rows;

	uint64_t curr_r;
	uint64_t curr_c;

	uint32_t color;

	struct multiboot_tag_framebuffer *fb;
};

static struct fb_console fb_c = {
	.cols = 80,
	.rows = 40,
	.color = 0xFFFFFF,
};

static void put_pixel(struct multiboot_tag_framebuffer *fb, uint64_t x, uint64_t y, uint32_t color)
{
	uint64_t where = y * fb->common.framebuffer_pitch + x * fb->common.framebuffer_bpp / 8;

	uint8_t *screen = (uint8_t *)fb->common.framebuffer_addr;
	screen[where] = color & 0xFF;
	screen[where + 1] = (color >> 8) & 0xFF;
	screen[where + 2] = (color >> 16) & 0xFF;
}

static void draw_letter(struct multiboot_tag_framebuffer *fb, const uint8_t *letter, uint64_t x, uint64_t y, uint32_t color)
{
	int i, j;
	for (i = 0; i < 8; i++) {
		for (j = 0; j < 8; j++) {
			put_pixel(fb, x + (7 - j), y + i, letter[i] & (1 << j) ? color : 0);
		}
	}
}

static void fb_console_init(struct console *c)
{
	c->opaque = &fb_c;
}

void init_framebuffer(struct multiboot_tag_framebuffer *fb)
{
	fb_c.fb = fb;

	uint64_t bytes_to_map = fb->common.framebuffer_width * fb->common.framebuffer_height * fb->common.framebuffer_bpp + PAGE_SIZE - 1;
	map_range(fb->common.framebuffer_addr, fb->common.framebuffer_addr + bytes_to_map, fb->common.framebuffer_addr);
}

#define CHAR_W 8
#define CHAR_H 8
#define H_SPACE 4

static void fb_move_up(struct fb_console *f, int n)
{
	uint64_t bytes_per_row = (f->fb->common.framebuffer_width * f->fb->common.framebuffer_bpp * (CHAR_H + H_SPACE)) >> 3;
	uint64_t addr = f->fb->common.framebuffer_addr;

	memcpy((uint8_t *)addr, (uint8_t *)(addr + bytes_per_row * n), bytes_per_row * (f->rows - n));
	memset((uint8_t *)(addr + bytes_per_row * (f->rows - n)), 0, bytes_per_row * n);
}

static void fb_console_putc(struct console *c, char ch)
{
	struct fb_console *f = (struct fb_console *)c->opaque;
	if (!f->fb)
		return;

	if (ch == '\n') {
		f->curr_r++;
		f->curr_c = 0;
	} else {
		draw_letter(f->fb, letters[ch], f->curr_c * CHAR_W, f->curr_r * CHAR_H + H_SPACE * f->curr_r, f->color);
		f->curr_c++;

		if (f->curr_c >= f->cols) {
			f->curr_c = 0;
			f->curr_r++;
		}
	}

	if (f->curr_r >= f->rows) {
		fb_move_up(f, 1);
		f->curr_r--;
		f->curr_c = 0;
	}
}

static void fb_console_clear(struct console *c)
{
	struct fb_console *f = (struct fb_console *)c->opaque;
	struct multiboot_tag_framebuffer *fb = f->fb;

	if (!fb)
		return;

	uint64_t bytes_to_clean = fb->common.framebuffer_width * fb->common.framebuffer_height * fb->common.framebuffer_bpp + PAGE_SIZE - 1;

	memset((uint8_t *)fb->common.framebuffer_addr, 0, bytes_to_clean);
}

static void fb_console_set_color(struct console *c, enum console_color color)
{
	struct fb_console *f = (struct fb_console *)c->opaque;

	switch (color) {
	case CONSOLE_COLOR_SUCCESS:
		f->color = 0xFF00;
		break;
	case CONSOLE_COLOR_WARN:
		f->color = 0xFFFF00;
		break;
	case CONSOLE_COLOR_FAIL:
		f->color = 0xFF0000;
		break;
	default:
		f->color = 0xFFFFFF;
		break;
	}
}

struct console framebuffer_console = {
	.init = fb_console_init,
	.putc = fb_console_putc,
	.clear = fb_console_clear,
	.set_color = fb_console_set_color,
};
