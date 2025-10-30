#include "box.h"
#include "colors.h"
#include <stddef.h>
#include <stdio.h>

#define pchr(X) printf("%s", X)

int draw(const char *msg, size_t l)
{
    green();
    pchr(UL);
    for (size_t i = 0; i < l; i++)
        pchr(H);
    pchr(UR);

    putchar('\n'); // Next line

    pchr(V);
    printf("%.*s", (int)(l), msg);
    pchr(V);

    putchar('\n'); // Next line

    pchr(LL);
    for (size_t i = 0; i < l; i++)
        pchr(H);
    pchr(LR);

    noc();
    putchar('\n');

    return 0;
}
