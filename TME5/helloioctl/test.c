#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

#include "request.h"

int main(){
    int file = open("/dev/hello", O_RDONLY);
    if (file < 0){
        perror("Cannot open\n");
        return 1;
    }
        
    if(ioctl(file, HELLO, msg)==-1)
        perror("Failed to system call ioctl in HELLO request\n");
    
    // printf("%s\n", msg);
    

    // if(ioctl(file, WHO, "beer")==-1)
    //     perror("Failed to system call ioctl in WHO request\n");
    

    // if(ioctl(file, HELLO, buf)==-1)
    //     perror("Failed to system call ioctl in HELLO request\n");
    
    // printf("%s\n", buf);
    

    close(file);

}