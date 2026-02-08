#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

int function1(void);
int function2(void);

int main(void)
{
    int fd1;
    /* try to use perror */
    fd1=open("document",O_RDONLY);
    if(fd1==-1){
        perror("open error");
    }
    /* the difference between return and exit */
    function1();
    printf("the sentence after return\n");
    function2();
    printf("the sentence after exit\n");
    exit(0);
}

int function1(void)
{
    int a=1;
    return 0;
}

int function2(void)
{
    int a=2;
    exit(0);
}