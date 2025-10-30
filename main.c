#include "box.h"
#define _DEFAULT_SOURCE
#include "error_functions.h"
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>

struct termios usertp;

void pb(int n)
{
    short int found = 0;
    printf("0b");
    for (int i = (sizeof(int) * 8) - 1; i >= 0; i--) {
        if (n & (1 << i)) {
            found = 1;
            putchar('1');
        }
        else if (found) {
            putchar('0');
        }
    }
    if (!found) putchar('0');
    putchar('\n');
}

void animate(char *buf, int len)
{
    printf("buf: %.*s\n", len - 1, buf);

    for (;;) {
        char temp = buf[0];
        for (int i = 0; i < len - 1; i++)
            buf[i] = buf[i + 1];
        buf[len - 1] = temp;

        usleep(50 * 1000);
        printf("\x1b[A"); // move cursor up one line
        printf("buf: %.*s\n", len - 1, buf);
    }
}

int ttySetCbreak(int fd, struct termios *current_tty)
{
    struct termios t;

    if (tcgetattr(fd, &t) == -1) {
        return -1;
    }

    if (current_tty != NULL) {
        *current_tty = t;
    }

    t.c_lflag |= ISIG;
    t.c_lflag &= ~(ICANON | ECHO);
    t.c_iflag &= ~ICRNL;

    t.c_cc[VMIN] = 1;
    t.c_cc[VTIME] = 0;

    if (tcsetattr(fd, TCSAFLUSH, &t) == -1) {
        return -1;
    }

    return 0;
}

int ttySetRaw(int fd, struct termios *current_tty)
{
    struct termios t;

    if (tcgetattr(fd, &t) == -1) {
        return -1;
    }

    if (current_tty != NULL) {
        *current_tty = t;
    }

    t.c_lflag &= ~(ICANON | ISIG | IEXTEN | ECHO);
    t.c_iflag &= ~(BRKINT | ICRNL | INLCR | IGNCR | IGNBRK | INPCK | ISTRIP | IXON | PARMRK);
    t.c_oflag &= ~OPOST;

    t.c_cc[VMIN] = 1;
    t.c_cc[VTIME] = 0;

    if (tcsetattr(fd, TCSAFLUSH, &t) == -1) {
        return -1;
    }

    return 0;
}

int log_file(const char *msg, int msg_len)
{
    int fd = open("./log", O_RDWR | O_CREAT | O_APPEND, 0644);
    if (fd == -1)
        return -1;

    time_t t = time(NULL);
    struct tm time_info;
    localtime_r(&t, &time_info);

    char timebuf[64];
    strftime(timebuf, sizeof(timebuf), "[%T]", &time_info);

    if (write(fd, timebuf, strlen(timebuf)) == -1) {
        close(fd);
        return -1;
    }

    if (write(fd, msg, msg_len) == -1) {
        close(fd);
        return -1;
    }

    close(fd);
    return 0;
}

static void handler(int sig)
{
    (void)sig;

    char *msg = "from SIGTSTP";
    if (log_file(msg, strlen(msg)) == -1) {
        perror("log_file");
        exit(EXIT_FAILURE);
    }

    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &usertp) == -1)
        errExit("tcsetattr");

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
        errExit("tcgetattr");

    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &usertp) == -1)
        errExit("tcsetattr");

    // Set SIGTSTP to Kernel default handler
    if (signal(SIGTSTP, SIG_DFL) == SIG_ERR)
        errExit("signal");

    // Send the SIGTSTP to the current process →
    raise(SIGTSTP);

    sigset_t tstp_mask, prev_mask;
    sigemptyset(&tstp_mask);
    sigaddset(&tstp_mask, SIGTSTP);
    if (sigprocmask(SIG_UNBLOCK, &tstp_mask, &prev_mask) == -1)
        errExit("sigprocmask unblock");

    // The following code runs after recevining the signal SIGCONT
    if (sigprocmask(SIG_SETMASK, &prev_mask, NULL) == -1)
        errExit("sigprocmask setmask");

    struct sigaction sa;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    sa.sa_handler = tstp_handler;
    if (sigaction(SIGTSTP, &sa, NULL) == -1)
        errExit("sigaction");

    if (tcgetattr(STDIN_FILENO, &usertp) == -1)
        errExit("tcgetattr");

    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &tp) == -1)
        errExit("tcsetattr");

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
            errExit("ttySetCbreak");

        // Handle following signals:
        // 1. SIGINT
        // 2. SIGQUIT
        // 3. SIGTSTP
        if (sigaction(SIGQUIT, NULL, &prev) == -1)
            errExit("sigaction SIGQUIT");
        if (prev.sa_handler != SIG_IGN)
            if (sigaction(SIGQUIT, &sa, NULL) == -1)
                errExit("sigaction SIGQUIT");

        if (sigaction(SIGINT, NULL, &prev) == -1)
            errExit("sigaction SIGINT");
        if (prev.sa_handler != SIG_IGN)
            if (sigaction(SIGINT, &sa, NULL) == -1)
                errExit("sigaction SIGINT");

        sa.sa_handler = tstp_handler;
        if (sigaction(SIGTSTP, NULL, &prev) == -1)
            errExit("sigaction SIGTSTP");
        if (prev.sa_handler != SIG_IGN)
            if (sigaction(SIGTSTP, &sa, NULL) == -1)
                errExit("sigaction SIGTSTP");
    }
    else {
        if (ttySetRaw(STDIN_FILENO, &usertp) == -1)
            errExit("ttySetRaw");
    }

    sa.sa_handler = handler;
    setbuf(stdout, NULL);

    int n;
    char ch;
    for (;;) {
        n = read(STDIN_FILENO, &ch, 1);
        if (n == 0)
            break;

        putchar(ch);

        if (ch == 27) {
            char *msg = "esc character received\n";
            log_file(msg, strlen(msg));
        }

        if (ch == 'q')
            break;
    }

    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &usertp) == -1)
        errExit("tcsetattr");

    return EXIT_SUCCESS;
}
