#include <unistd.h> 
#include <sys/syscall.h> 
#include <sys/types.h> 
#include <stdio.h> 
#define __NR_alcall 437
int main(int argc, char *argv[]) 
{
       char process[1024*4];
       int length = 1;
       syscall(__NR_alcall, length, process);
       printf("%s", process); 
       return 0; 
}
