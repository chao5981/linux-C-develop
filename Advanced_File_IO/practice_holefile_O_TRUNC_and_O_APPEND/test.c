/* 一个进程内多次 open 打开同一个文件，那么会得到多个不同的文件描述符 fd，同理在关闭文件的
时候也需要调用 close 依次关闭各个文件描述符。*/


/* 一个进程内多次 open 打开同一个文件，在内存中并不会存在多份动态文件。 */

/* 当 open 函数同时携带了 O_APPEND 和 O_TRUNC 两个标志时，文件会先被清空，但后续的写入操作依然会强制追加到文件末尾（而非从头写），相当于创建了空洞文件 */


#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include "string.h"
#include "stdlib.h"

int main(void)
{   
    /* practice of hole file */
    int fd1;
    off_t off1;
    char buff[1024];
    char readbuff[4096];
    int res;
    fd1=open("hole_file.txt",O_RDWR);
    if(fd1==-1){
        perror("open hole_file.txt error");
        exit(-1);
    }
    lseek(fd1,4096,SEEK_SET);
    memset(buff,'a',1024);
    res=write(fd1,buff,sizeof(buff));
    if(res==-1)
    {
        perror("write error");
        goto err;
    }
    res=lseek(fd1,0,SEEK_SET);
    if(res==-1)
    {
        perror("lseek error");
        goto err;
    }            
    res=read(fd1,readbuff,4096);
    if(res==-1)
    {
        perror("read error");
        goto err;
    }
    printf("readbuff is %s\n",readbuff);
    res=lseek(fd1,0,SEEK_END);
    if(res==-1)
    {
        perror("lseek 002 error");
        goto err;
    }
    printf("the size of hole_file is %d\n",res);

    /* try to use O_TRUNC */
    int fd2;
    fd2=open("new_file.txt",O_CREAT|O_EXCL|O_RDWR,S_IRWXU|S_IRWXG|S_IRWXO);
    if(fd2==-1)
    {
        perror("open new_file.txt error");
        exit(-1);
    }
    for(int i=0;i<4;i++)
    {
        res=write(fd2,buff,sizeof(buff));
        if(res==-1)
        {
            perror("write new_file error");
            goto err2;
        }
    }
    res=lseek(fd2,0,SEEK_END);
    if(res==-1)
    {
        perror("new_file lseek error");
        goto err2;
    }
    printf("the size of new_file is %d\n",res);
    close(fd2);
    fd2=open("new_file.txt",O_WRONLY|O_RDONLY|O_TRUNC);
    if(fd2==-1)
    {
        perror("new file open again error");
        exit(-1);
    }
    res=lseek(fd2,0,SEEK_SET);
    if(res==-1)
    {
        perror("new_file lseek again error");
        goto err2;
    }
    printf("the size of new file after use O_TRUNC:%d\n",res);

    /* try to use O_APPEND */
    int fd3;
    fd3=open("last_file.txt",O_CREAT|O_EXCL|O_RDWR,S_IRWXU|S_IRWXG|S_IRWXO);
    if(fd3==-1)
    {
        perror("open last_file.txt error");
        exit(-1);
    }
    char writelastfilebuff[4];
    memset(writelastfilebuff,'b',sizeof(writelastfilebuff));
    res=write(fd3,writelastfilebuff,sizeof(writelastfilebuff));
    if(res==-1)
    {
        perror("write last_file error");
        goto err3;
    }
    close(fd3);
    fd3=open("last_file.txt",O_RDWR|O_APPEND);
    if(fd3==-1)
    {
        perror("last_file open again error");
    }
    /* O_APPEND can`t change the position offset of read ,but write can change it*/
    char readlastfilebuff[8]={0};
    res=read(fd3,readlastfilebuff,8);
    if(res==-1)
    {
        perror("read last_file after using O_APPEND error");
        goto err3;
    }
    printf("%d the context of last file after using O_APPEND is %s\n",res,readlastfilebuff);

    res=lseek(fd3,0,SEEK_SET);
    if(res==-1)
    {
        perror("lseek last_file error");
    }
    memset(writelastfilebuff,'a',sizeof(writelastfilebuff));
    res=write(fd3,writelastfilebuff,sizeof(writelastfilebuff));
    if(res==-1)
    {
        perror("write last_file again error");
        goto err3;
    }

    char readlastfilebuffafterwrite[8]={0};
    res=read(fd3,readlastfilebuffafterwrite,8);
    if(res==-1)
    {
        perror("read last_file after using write() error");
        goto err3;
    }
    printf("%d the context of last file after using write() is %s\n",res,readlastfilebuffafterwrite);

    goto err3;
err:
    close(fd1);
    exit(res);

err2:
    close(fd1);
    close(fd2);
    exit(res);

err3:
    close(fd1);
    close(fd2);
    close(fd3);
    exit(res);

}