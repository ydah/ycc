#include <stdio.h>
#include <stdlib.h>

int foo() { return 5; }

int bar(int x) { return x; }

int baz(int x, int y, int z) { return x + y + z; }

void alloc4(int** ptr, int a, int b, int c, int d) {
    *ptr = malloc(sizeof(int) * 4);
    (*ptr)[0] = a;
    (*ptr)[1] = b;
    (*ptr)[2] = c;
    (*ptr)[3] = d;
}
