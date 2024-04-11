#include <sys/syscall.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

int main() {
    const char* addr_str = "0xffffffffa0002740";
    unsigned long addr;
    addr = strtoull(addr_str, NULL, addr);
    printf("PID of main: %d\n", getpid());
    syscall(452, 0xffffffffa0002740, 4);
    return 0;
}