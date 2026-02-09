#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include "stdlib.h"
#include "string.h"

/* 关于内核缓冲用的比较少，一般是需要强制保存一些文件才用到，并且还用的是文件标识符，还需要互转 */

int main(void)
{
    int res=0;
    FILE *fp1=NULL;
    fp1=fopen("file1","w+");
    if(fp1==NULL)
    {
        perror("open file1 error");
        exit(-1);
    }

    //
    int fd2=open("file2",O_CREAT|O_EXCL|O_RDWR,S_IRWXU);
    if(fd2==-1)
    {
        perror("open file2 error");
        fclose(fp1);
        exit(-1);
    }

    //下述俩个是文件标识符和文件类型指针互转
    int fd1=fileno(fp1);
    if(fd1==-1)
    {
        perror("file1 fileno error");
        res=-1;
        goto err1;
    }
    //用fdopen时是不会把转换的同时再次打开文件的，但是要注意指定的方式要和文件描述符打开的一致
    FILE *fp2=fdopen(fd2,"r+");
    if(fp2==NULL)
    {
        perror("file2 fdopen error");
        res=-1;
        goto err1;
    }
    printf("fd1:%d fd2:%d\n",fd1,fd2);

    char writebuff[10];
    memset(writebuff,'a',sizeof(writebuff));
    res=write(fd1,writebuff,sizeof(writebuff));
    if(res==-1)
    {
        perror("write file1 error");
        goto err1;
    }

    //提前把文件I/O内核缓冲写入磁盘中
    //用 fsync()将参数 fd 所指文件的内容数据和元数据写入磁盘，只有在对磁盘设备的写入操作完成之后，fsync()函数才会返回
    //fdatasync()与 fsync()类似，不同之处在于 fdatasync()仅将参数 fd 所指文件的内容数据写入磁盘，并不包括文件的元数据,只有在对磁盘设备的写入操作完成之后，fdatasync()函数才会返回
    //sync()会将所有文件 I/O 内核缓冲区中的文件内容数据和元数据全部更新到磁盘设备中,即刷新所有文件 I/O 内核缓冲区
    //还有一种方法是在调用 open()函数时，指定 O_DSYNC 标志，其效果类似于在每个 write()调用之后调用 fdatasync()函数进行数据同步
    //或者调用 open()函数时，指定 O_SYNC 标志，其效果类似于在每个 write()调用之后调用 fsync()函数进行数据同步

    //补充：元数据这个概念，元数据并不是文件内容本身的数据，而是一些用于记录文件属性相关的数据信息，譬如文件大小、时间戳、权限等等信息，这里统称为文件的元数据，这些信息也是存储在磁盘设备中的，

    //一般不建议随随便便用内核缓冲函数，对性能要求很大
    fsync(fd1);
    res=0;
    goto err1;

err1:
    fclose(fp1);
    close(fd2);
    exit(res);
}