#include "stdio.h"

#include "sys/types.h"
#include "sys/stat.h"
#include "fcntl.h"
#include "unistd.h"

int main(void)
{
    int fd1,fd2,fd3;
    off_t off1,off2;
    ssize_t read1,write1;
    char readbuff[1024];
    /* (1) */
    fd1=open("document.txt",O_RDONLY);
    if(fd1==-1){
        printf("error 001\n");
    }
    fd2=open("dest_file.txt",O_CREAT|O_EXCL,S_IRWXU|S_IRGRP|S_IROTH);
    if(fd2==-1){
        printf("error 002\n");
    }
    fd2=open("dest_file.txt",O_WRONLY);
    if(fd2==-1){
        printf("error 0021\n");
    }   
    off1=lseek(fd1,500,SEEK_SET);
    if(off1==-1){
        printf("error 003\n");
    }
    read1=read(fd1,readbuff,1024);
    if(read1==-1){
        printf("error 004\n");
    }
    else{
        printf("%s",readbuff);
    }
    off2=lseek(fd2,0,SEEK_SET);
    if(off2==-1){
        printf("error 005\n");
    }
    write1=write(fd2,readbuff,sizeof(readbuff));
    if(write1==-1){
        printf("error 006\n");
    }
    /* (2) */
    fd3=open("text_file.txt",O_RDONLY);
    if(fd3==-1){
        printf("the file don`t exist\n");
        }
    else if(fd3==2){
        printf("the file exist\n");
    }
    close(fd1);
    close(fd2);
    close(fd3);
    /* (3) */
    int fd4;
    char buff1=0x00;
    char buff2=0xff;
    fd4=open("new_file.bin",O_CREAT|O_EXCL|O_WRONLY,S_IRWXU|S_IRGRP|S_IROTH);
    if(fd4==-1){
        printf("error 007\n");
        return 0;
    }
    for(int i=0;i<500;i++){
        write(fd4,&buff1,1);
    }
    for(int i=0;i<1024;i++){
        write(fd4,&buff2,1);
    }
    close(fd4);
    /* (4) */
    int fd5;
    fd5=open("document.txt",O_RDONLY);
    off_t off3;
    off3=lseek(fd5,0,SEEK_END);
    printf("size=%ld\n",off3);
    close(fd5);
    return 0;
}