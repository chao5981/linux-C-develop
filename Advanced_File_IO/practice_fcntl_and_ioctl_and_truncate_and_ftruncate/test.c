#include "sys/types.h"
#include "sys/stat.h"
#include "fcntl.h"
#include "unistd.h"
#include "stdio.h"
#include "errno.h"
#include "string.h"
#include "stdlib.h"

int main(void)
{
    int res=0;
    int fd1;
    fd1=open("file1",O_CREAT|O_EXCL|O_RDWR,S_IRWXU);
    if(fd1==-1)
    {
        perror("create file1 error");
        exit(-1);
    }
    /* try to use fcntl and ioctl */
    //fcntl()和ioctl()是linux系统调用的百宝箱，可以执行非常多的操作。ioctl主要用于一般用于操作特殊文件或硬件外设；而fcntl(例更加常用一些，如
    //复制文件描述符（cmd=F_DUPFD 或 cmd=F_DUPFD_CLOEXEC）； 
    //获取/设置文件描述符标志（cmd=F_GETFD 或 cmd=F_SETFD）；   //fd的标识，很少用
    //获取/设置文件状态标志（cmd=F_GETFL 或 cmd=F_SETFL）；     //文件的标识，例如只读，只写
    //获取/设置异步 IO 所有权（cmd=F_GETOWN 或 cmd=F_SETOWN）；  
    //获取/设置记录锁（cmd=F_GETLK 或 cmd=F_SETLK）；

    int fd2=fcntl(fd1,F_DUPFD);
    if(fd2==-1)
    {
        perror("fcntl error");
        goto err1;
    }
    printf("fd1=%d,fd2=%d\n",fd1,fd2);
    
    //fcntl还可以复制文件描述符的同时，还可以输入第三个参数去修改文件描述符的系数（类似与dup2）
    int fd3=fcntl(fd1,F_DUPFD,100);
    if(fd3==-1)
    {
        perror("fcntl error");
        goto err1;
    }
    printf("fd1=%d,fd3=%d\n",fd1,fd2);

    //fcntl在修改文件状态标志时，，只有 O_APPEND、O_ASYNC、O_DIRECT、O_NOATIME 以及 O_NONBLOCK 这些标志可以被修改
    int flag=fcntl(fd1,F_GETFL);
    if(flag==-1)
    {
        perror("fcntl get file state error");
        res=-1;
        goto err1;
    }
    printf("file1 state is 0x%x\n",flag);
    res=fcntl(fd1,F_SETFL,flag|O_APPEND);
    if(res==-1)
    {
        perror("fcntl set file state error");
        goto err1;
    }
    flag=fcntl(fd1,F_GETFL);
    if(flag==-1)
    {
        perror("fcntl get file state again error");
        res=-1;
        goto err1;
    }
    printf("file1 state again is 0x%x\n",flag);

    /* try to use truncate and ftruncate */
    //truncate和ftruncate都是文件裁减函数，将文件裁减成指定大小，前者需要指定文件路径path，后者需要Open后得到文件描述符；
    int fd4=open("file2",O_CREAT|O_EXCL|O_RDWR,S_IRWXU);
    if(fd4==-1)
    {
        perror("create file2 error");
        goto err1;
    }
    int fd5=open("file3",O_CREAT|O_EXCL|O_RDWR,S_IRWXU);
    if(fd5==-1)
    {
        perror("create file3 error");
        goto err1;
    }
    char writebuff[1024];
    memset(writebuff,'a',sizeof(writebuff));
    res=write(fd4,writebuff,sizeof(writebuff));
    if(res==-1)
    {
        perror("write fd4 error");
        goto err1;

    }
    res=write(fd5,writebuff,sizeof(writebuff));
    if(res==-1)
    {
        perror("write fd5 error");
        goto err1;
    }
    res=lseek(fd4,0,SEEK_CUR);
    if(res==-1)
    {
        perror("lseek fd4 error");
        goto err1;
    }
    printf("fd4 size is %d\n",res);
    res=lseek(fd5,0,SEEK_CUR);
    if(res==-1)
    {
        perror("lseek fd5 error");
        goto err1;
    }
    printf("fd5 size is %d\n",res);

    res=ftruncate(fd4,512);
    if(res==-1)
    {
        perror("cut fd4 error");
        goto err1;
    }
    res=lseek(fd4,0,SEEK_END);
    if(res==-1)
    {
        perror("lseek fd4 again error");
        goto err1;
    }
    printf("fd4 size after cut is %d\n",res);

   res=truncate("file3",128);
    if(res==-1)
    {
        perror("cut fd5 error");
        goto err1;
    }
    res=lseek(fd5,0,SEEK_END);
    if(res==-1)
    {
        perror("lseek fd5 again error");
        goto err1;
    }
    printf("fd5 size after cut is %d\n",res);


err1:
    close(fd1);
    close(fd2);
    close(fd3);
    close(fd4);
    close(fd5);
    exit(res);
}