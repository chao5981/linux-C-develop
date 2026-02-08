#include "sys/types.h"
#include "sys/stat.h"
#include "fcntl.h"
#include "unistd.h"
#include "stdio.h"
#include "errno.h"
#include "stdlib.h"

int main(void)
{
    int fd1;
    int res=0;

    /* O_APPEND的原理是每当调用write()时，就自动将地址偏移量调到末尾，从文件的末尾开始写入；因此，无论打开多少个文件，只要带有O_APPEND,那么每当对相应的文件描述符进行写入操作时，
    就自动从末尾写入；由此得知，当多次打开同一个文件时，若其中一个没有用O_APPEND,那么继续写的时候就会被覆盖 */

    /*dup2相对来说更加方便，他是复制一个相同的文件描述符，并可以自定义文件描述符的序号，正因为是复制的文件描述符，都指向一个相同的文件表，那么从中读出来的文件读写偏移量也是一样的*/

//    fd1=open("file1",O_CREAT|O_EXCL|O_RDWR,S_IRWXU);
    fd1=open("file1",O_CREAT|O_EXCL|O_RDWR|O_APPEND,S_IRWXU);

    if (fd1==-1)
    {
        perror("file1 create error");
        exit(-1);
    }
    int fd2;
//    fd2=open("file1",O_RDWR|O_APPEND,S_IRWXU);
    fd2=open("file1",O_RDWR|O_APPEND,S_IRWXU);
    if (fd2==-1)
    {
        perror("file1 open again error");
        exit(-1);
    }
    char buff1[4]={'a','b','c','d'};
    char buff2[4]={'q','w','e','r'};
        for(int i=0;i<2;i++)
        {
            res=write(fd1,buff1,sizeof(buff1));
            if(res==-1)
            {
                perror("write fd1 error");
                goto err1;
            }
            res=write(fd2,buff2,sizeof(buff2));
            if(res==-1)
            {
                perror("write fd2 error");
                goto err1;
            }
        }
    res=lseek(fd1,0,SEEK_SET);
    if(res==-1)
    {
        perror("lseek file1 error");
        goto err1;
    }
   
    char readbuff[16]={0};
    res=read(fd1,readbuff,sizeof(readbuff));
    if(res==-1)
    {
        perror("read file1 error");
        goto err1;
    }  
    printf("%s\n",readbuff);
    int fd3=dup2(fd1,100);
    if(fd3==-1)
    {
        perror("dup2 error");
        goto err1;
    }
    printf("old fd:%d,new fd:%d",fd1,fd3);
    close(fd3);
    res=0;
    goto err1;
err1:
    close(fd1);
    close(fd2);
    exit(res);
}