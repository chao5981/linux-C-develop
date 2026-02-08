/* 如果是在.c文件创建文件且没有给予任何权限（包括所有者权限），则无法对文件进行读写操作*/


#include "sys/types.h"
#include "sys/stat.h"
#include "fcntl.h"
#include "unistd.h"
#include "stdio.h"
#include "errno.h"
#include "stdlib.h"

int main(void)
{
    int fd1,fd2,fd3,fd4;
    int res;
    char buff[4]={'a','b','c','d'};
    fd1=open("file1_txt",O_RDWR|O_CREAT|O_EXCL,S_IRWXU);
    if(fd1==-1)
    {
        perror("create file1_txt error");
    }
    res=write(fd1,buff,sizeof(buff));
    if(res==-1)
    {
        perror("write file1_txt error");
    }
    fd2=open("file2_txt",O_RDWR|O_CREAT|O_EXCL,S_IRWXU|S_IRWXG);
    if(fd2==-1)
    {
        perror("create file2_txt error");
    }
    res=write(fd2,buff,sizeof(buff));
    if(res==-1)
    {
        perror("write file2_txt error");
    }
    fd3=open("file3_txt",O_RDWR|O_CREAT|O_EXCL,S_IRWXU|S_IRWXG|S_IRWXO);
    if(fd3==-1)
    {
        perror("create file3_txt error");
    }
    res=write(fd3,buff,sizeof(buff));
    if(res==-1)
    {
        perror("write file3_txt error");
    }
    fd4=open("file4_txt",O_RDWR|O_CREAT|O_EXCL);
    if(fd4==-1)
    {
        perror("create file3_txt error");
    }
    res=write(fd4,buff,sizeof(buff));
    if(res==-1)
    {
        perror("write file4_txt error");
    }
    exit(res);
    close(fd1);
    close(fd2);
    close(fd3);
    close(fd4);

}