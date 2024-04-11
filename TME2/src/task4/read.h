#include <stdlib.h>
#include <stdio.h>

#ifdef __DARWIN_ALIAS_C
    ssize_t read(int, void *, size_t) __DARWIN_ALIAS_C(read);
#else
    ssize_t read(int, void *, size_t);
#endif

