#define _DEFAULT_SOURCE
#include "box.h"
#include "errfunc.h"
#include "estring.h"
#include "log.h"
#include "modes.h"
#include "tutils.h"
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

struct termios usertp;

static void handler(int sig)
{
    (void)sig;

    qtaltbuf();

    if (log_file("%s", "from SIGTSTP") == -1) {
        perror("log_file");
        exit(EXIT_FAILURE);
    }

    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &usertp) == -1)
        err_exit("tcsetattr");

    _exit(EXIT_SUCCESS);
}

// NOTE:
// 1. keep current errno
// 2. save current terminal attributes
// 3. fire SIGTSTP and set the default handler for this signal
// 4. unblock SIGTSTP by `sigaddset`
static void tstp_handler(int sig)
{
    (void)(sig);

    int saved_errno = errno;

    struct termios tp;
    if (tcgetattr(STDIN_FILENO, &tp) == -1)
        err_exit("tcgetattr");

    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &usertp) == -1)
        err_exit("tcsetattr");

    // Set SIGTSTP to Kernel default handler
    if (signal(SIGTSTP, SIG_DFL) == SIG_ERR)
        err_exit("signal");

    // Send the SIGTSTP to the current process →
    raise(SIGTSTP);

    sigset_t tstp_mask, prev_mask;
    sigemptyset(&tstp_mask);
    sigaddset(&tstp_mask, SIGTSTP);
    if (sigprocmask(SIG_UNBLOCK, &tstp_mask, &prev_mask) == -1)
        err_exit("sigprocmask unblock");

    // The following code runs after recevining the signal SIGCONT
    if (sigprocmask(SIG_SETMASK, &prev_mask, NULL) == -1)
        err_exit("sigprocmask setmask");

    struct sigaction sa;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    sa.sa_handler = tstp_handler;
    if (sigaction(SIGTSTP, &sa, NULL) == -1)
        err_exit("sigaction");

    if (tcgetattr(STDIN_FILENO, &usertp) == -1)
        err_exit("tcgetattr");

    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &tp) == -1)
        err_exit("tcsetattr");

    errno = saved_errno;
}

int main(int argc, char *argv[])
{
    (void)(argv);

    struct termios tp;
    if (tcgetattr(STDIN_FILENO, &tp) == -1) {
        perror("tcgetattr");
        exit(EXIT_FAILURE);
    }

    usertp = tp;

    struct sigaction sa, prev;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    sa.sa_handler = handler;

    if (argc > 1) {
        if (ttySetCbreak(STDIN_FILENO, &tp) == -1)
            err_exit("ttySetCbreak");

        // Handle following signals:
        // 1. SIGINT
        // 2. SIGQUIT
        // 3. SIGTSTP
        if (sigaction(SIGQUIT, NULL, &prev) == -1)
            err_exit("sigaction SIGQUIT");
        if (prev.sa_handler != SIG_IGN)
            if (sigaction(SIGQUIT, &sa, NULL) == -1)
                err_exit("sigaction SIGQUIT");

        if (sigaction(SIGINT, NULL, &prev) == -1)
            err_exit("sigaction SIGINT");
        if (prev.sa_handler != SIG_IGN)
            if (sigaction(SIGINT, &sa, NULL) == -1)
                err_exit("sigaction SIGINT");

        sa.sa_handler = tstp_handler;
        if (sigaction(SIGTSTP, NULL, &prev) == -1)
            err_exit("sigaction SIGTSTP");
        if (prev.sa_handler != SIG_IGN)
            if (sigaction(SIGTSTP, &sa, NULL) == -1)
                err_exit("sigaction SIGTSTP");
    }
    else {
        if (ttySetRaw(STDIN_FILENO, &usertp) == -1)
            err_exit("ttySetRaw");
    }

    sa.sa_handler = handler;
    setbuf(stdout, NULL);

    savestate();
    srtaltbuf();
    clrscreen();
    pos0();

    /* for (int i = 0; i < 40; i++) { */
    /*     printf("i: %d\r\n", i); */
    /*     usleep(50000); */
    /* } */
    /* qtaltbuf(); */
    /* restorestate(); */
    /* if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &usertp) == -1) */
    /*     err_exit("tcsetattr"); */
    /* return 0; */

    int n;
    char ch;

    n = read(STDIN_FILENO, &ch, 1);
    if (n == 0)
        return EXIT_SUCCESS;

    string_t str = str_init();
    str_append(str, &ch, 1);
    struct box *b = box_init(0, 0, str);

    box_draw(b);

    for (;;) {
        n = read(STDIN_FILENO, &ch, 1);
        if (n == 0)
            break;

        if (ch == 27) { // esc character
            char *msg = "esc character received\n";
            log_file(msg, strlen(msg));
            continue;
        }
        if (ch == 13) { // newline -> carriage return
            if (b->text->len == 0) {
                putchar('\n');
            }
            else {
                box_done(b);
                box_new(b);
            }
            continue;
        }
        if (ch == 'q')
            break;

        if (b->text->len == 0) {
            str_append(b->text, &ch, 1);
            box_draw(b);
            continue;
        }
        box_addcolumn(b, ch);
    }

    qtaltbuf();
    restorestate();
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &usertp) == -1)
        err_exit("tcsetattr");

    return EXIT_SUCCESS;
}
