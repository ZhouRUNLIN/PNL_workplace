#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

#include "taskmonitor.h"

int main(){
    int file = open("/dev/taskmonitor", O_RDWR);
    if (file < 0){
        perror("Cannot open\n");
        return 1;
    }
        
    if(ioctl(file, TM_GET, 0)==-1)
        perror("Failed to system call ioctl in TM_GET request\n");
    
    if(ioctl(file, TM_STOP, 0)==-1)
        perror("Failed to system call ioctl in TM_STOP request\n");

    printf("Stopped\n");
    sleep(10);
    printf("Resumed\n");
    
    if(ioctl(file, TM_START, 0)==-1)
        perror("Failed to system call ioctl in TM_START request\n");    
    
    printf("Stopped\n");
    sleep(5);
    printf("Resumed\n");

    if(ioctl(file, TM_PID, 2)==-1)
        perror("Failed to system call ioctl in TM_PID request\n");
    close(file);

}