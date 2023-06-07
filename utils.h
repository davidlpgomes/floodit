#ifndef __UTILS__
#define __UTILS__

#define ANSI_COLOR_RESET   "\x1b[0m"

extern char* ansi_bgs[16];

void test_alloc(void* p, char* name);

void clear_terminal();

int min(int a, int b);

int max(int a, int b);

double timestamp();

#endif
