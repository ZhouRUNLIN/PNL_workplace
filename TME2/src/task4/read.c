// Question 3

#include <stdlib.h>
#include <stdio.h>

ssize_t	read(int fd, void *buf, size_t count) {
    printf("Ciao!\n");
    return "e";
}