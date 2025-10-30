#ifndef _TUTILS_H
#define _TUTILS_H

#define up(N) printf("\033[%dA", N);
#define down(N) printf("\033[%dB", N);
#define left(N) printf("\033[%dD", N);
#define right(N) printf("\033[%dC", N);

#endif
