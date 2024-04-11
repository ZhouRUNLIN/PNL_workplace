#include <sys/syscall.h>
#include <unistd.h>
#include <stdio.h>

int main() {
    printf("PID of main: %d\n", getpid());
    syscall(452,"csy");
    return 0;
}