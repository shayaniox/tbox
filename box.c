#include "box.h"
#include "colors.h"
#include "estring.h"
#include "log.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <unistd.h>

#define pchr(X) printf("%s", X)

void move_cursor(int row, int column)
{
    printf("\033[%d;%dH", row, column);
    fflush(stdout);
}

struct box *box_init(int row, int column, string_t text)
{
    struct box *b = malloc(sizeof(struct box));
    if (b == NULL)
        return NULL;

    b->row = row <= 0 ? 1 : row;
    b->column = column <= 0 ? 1 : column;
    b->text = text;

    return b;
}

void box_draw(struct box *b)
{
    move_cursor(b->row, b->column);

    green();
    pchr(UL);
    for (size_t i = 0; i < b->text->len + 1; i++)
        pchr(H);
    pchr(UR);

    move_cursor(b->row + 1, b->column);

    pchr(V);
    printf("%.*s ", (int)(b->text->len), b->text->data);
    pchr(V);

    move_cursor(b->row + 2, b->column);

    pchr(LL);
    for (size_t i = 0; i < b->text->len + 1; i++)
        pchr(H);
    pchr(LR);
    noc();

    move_cursor(b->row + 1, b->text->len + 2);
}

void box_addcolumn(struct box *b, char ch)
{
    str_append(b->text, &ch, 1);

    green();

    // Add the character
    move_cursor(b->row, b->column + b->text->len);
    pchr(H);
    move_cursor(b->row + 1, b->column + b->text->len);
    putchar(ch);
    move_cursor(b->row + 2, b->column + b->text->len);
    pchr(H);

    // Add space character
    move_cursor(b->row, b->column + b->text->len + 1);
    pchr(H);
    move_cursor(b->row + 1, b->column + b->text->len + 1);
    putchar(' ');
    move_cursor(b->row + 2, b->column + b->text->len + 1);
    pchr(H);

    // Right border of box
    move_cursor(b->row, b->column + b->text->len + 2);
    pchr(UR);
    move_cursor(b->row + 1, b->column + b->text->len + 2);
    pchr(V);
    move_cursor(b->row + 2, b->column + b->text->len + 2);
    pchr(LR);

    noc();
    move_cursor(b->row + 1, b->column + b->text->len + 1);
}

void box_done(struct box *b)
{
    green();

    move_cursor(b->row, b->column + b->text->len + 1);
    pchr(UR);
    move_cursor(b->row + 1, b->column + b->text->len + 1);
    pchr(V);
    move_cursor(b->row + 2, b->column + b->text->len + 1);
    pchr(LR);

    move_cursor(b->row, b->column + b->text->len + 2);
    putchar(' ');
    move_cursor(b->row + 1, b->column + b->text->len + 2);
    putchar(' ');
    move_cursor(b->row + 2, b->column + b->text->len + 2);
    putchar(' ');

    noc();
}

void box_new(struct box *b)
{
    struct winsize w;
    int rows;

    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == 0) {
        rows = w.ws_row;
    }
    else {
        rows = -1;
    }
    b->row += 3;
    if (rows - b->row < 2) {
        move_cursor(rows, 0);
        for (int i = 0; i < 2 - (rows - b->row); i++) {
            putchar('\n');
        }
        b->row = rows - 2;
    }
    log_file("new row: %d\n", b->row);
    str_clear(b->text);
    move_cursor(b->row, b->column);
}
