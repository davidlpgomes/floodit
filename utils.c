#include "utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

char* ansi_bgs[16] = {
    "\x1b[40m",
    "\x1b[41m",
    "\x1b[42m",
    "\x1b[43m",
    "\x1b[44m",
    "\x1b[45m",
    "\x1b[46m",
    "\x1b[47m",
    "\x1b[100m",
    "\x1b[101m",
    "\x1b[102m",
    "\x1b[103m",
    "\x1b[104m",
    "\x1b[105m",
    "\x1b[106m",
    "\x1b[107m",
};

void test_alloc(void* p, char* name) {
    if (p)
        return;

    fprintf(stderr, "Error: could not alloc %s\n", name);
    exit(0);
}

void clear_terminal() {
    printf("\e[1;1H\e[2J");

    return;
}

int min(int a, int b) {
    if (a < b)
        return a;

    return b;
}

int max(int a, int b) {
    if (a > b)
        return a;

    return b;
}

double timestamp() {
    struct timespec tp;
    clock_gettime(CLOCK_MONOTONIC_RAW, &tp);

    return (double) (tp.tv_sec + tp.tv_nsec * 1.0e-9);
}
