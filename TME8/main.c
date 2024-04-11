#include <sys/syscall.h>   // For SYS_xxx definitions
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

int main()
{
    int pid;
    int sig = SIGKILL; 
    printf("saisir le numero de pid:\n");
    scanf("%d",&pid);
    printf("%d\n", pid);
    
    long result = syscall(SYS_kill, pid, sig);

    if (result == -1) {
        perror("syscall kill failed");
        exit(-1);
    }

    exit(0);
}