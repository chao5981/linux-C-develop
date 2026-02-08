#include "sys/types.h"
#include "sys/stat.h"
#include "fcntl.h"
#include "unistd.h"
#include "stdio.h"
#include "errno.h"
#include "stdlib.h"
#include "string.h"

int main(void)
{
    /* try to use pwrite and pread */
    int fd1;
    int res=0;
    /* 其实O_EXCL和O_APPEND也属于原子操作 */
    fd1=open("file1",O_CREAT|O_EXCL|O_RDWR,S_IRWXU);
    if(fd1==-1)
    {
        perror("create file1 error");
        exit(-1);
    }
    char writebuff[10],readbuff[10];
    memset(writebuff,'a',sizeof(writebuff));
    res=pwrite(fd1,writebuff,sizeof(writebuff),10);
    if(res==-1)
    {
        perror("pwrite file1 error");
    }
    res=lseek(fd1,0,SEEK_CUR);
    if(res==-1)
    {
        perror("lseek file1 error");
    }
    //pwrite和pread都不会改变原有文件的偏移量
    printf("file1 position offset is %d",res);

    close(fd1);
    exit(res);
}