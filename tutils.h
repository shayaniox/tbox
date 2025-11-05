#ifndef _TUTILS_H
#define _TUTILS_H

#define up(N) printf("\033[%dA", N)
#define down(N) printf("\033[%dB", N)
#define left(N) printf("\033[%dD", N)
#define right(N) printf("\033[%dC", N)

#define clrline() printf("\033[2K")
#define clrscreen() printf("\033[2J")

#define srtaltbuf() printf("\033[?1049h")
#define qtaltbuf() printf("\033[?1049l")

#define pos0() printf("\033[J\033[H")

#define savestate() printf("\0337")
#define restorestate() printf("\0338")

#endif
