
/* 对于阻塞式读取某些文件，可能会出现因为一个文件没有准备好而出现堵塞其他文件的情况 */
/* 而对于非阻塞式读取，又会出现大量占用CPU的情况，对于程序很危险 */
/* IO多路复用可以解决这个问题，用使用 select()或 poll()轮训，但这样也会占有CPU，特别是文件多的时候 */
/* 采取异步IO能解决这些问题，但是默认的SIGIO是非实时信号，不支持排队 */
/* 并且不知道文件描述符发生了什么事件 */


/* 于是采取这个优化的异步IO */

#include "sys/stat.h"
#include "fcntl.h"
#include "stdio.h"
#include "stdlib.h"
#include "fcntl.h"
#include "unistd.h"
#include "signal.h"


#define MOUSE "/dev/input/event3"


static int fd;

void io_handler(int sig,siginfo_t *info,void *ucontext);

int main(void)
{
    int flag;
    struct sigaction act;
    /* 以非阻塞的形式打开文件 */
    fd=open(MOUSE,O_RDONLY|O_NONBLOCK);
    if (fd==-1)
    {
        /* code */
        perror("open mouse file error");
        exit(-1);
    }
    
    printf("possess is runnning\n");


    /* 允许开启异步IO模式 */
    //O_ASYNC没办法在open的时候置位
    flag=fcntl(fd,F_GETFL);
    flag|=O_ASYNC;
    fcntl(fd,F_SETFL,flag);

    //设置异步IO的所有者
    fcntl(fd,F_SETOWN,getpid());

    //指定实时信号SIGRTMIN作为异步IO的通讯信号
    fcntl(fd,__F_SETSIG,SIGRTMIN);

    /* 为实时信号 SIGRTMIN 注册信号处理函数 */
    act.sa_flags=SA_RESTART|SA_SIGINFO;
    act.sa_sigaction=io_handler;
    sigemptyset(&act.sa_mask);
    if(sigaction(SIGRTMIN,&act,NULL)==-1)
    {
        perror("create sigaction error");
        exit(-1);
    }

    while(1)
    {
        sleep(1);
    }
    
}

void io_handler(int sig,siginfo_t *info,void *ucontext)
{
    static int loop=5;
    char buff[100];
    int res=0;

    if(sig!=SIGRTMIN)
    {
        return;
    }
    
    // 传递给信号处理函数的 siginfo_t 结构体中与之相关的字段如下：
    // ⚫ si_signo：引发处理函数被调用的信号。这个值与信号处理函数的第一个参数一致。
    // ⚫ si_fd：表示发生异步 I/O 事件的文件描述符；
    // ⚫ si_code：表示文件描述符 si_fd 发生了什么事件，读就绪态、写就绪态或者是异常事件等。
    // ⚫ si_band：是一个位掩码，其中包含的值与系统调用 poll()中返回的 revents 字段中的值相同。如表
    // 13.4.1 所示，si_code 中可能出现的值与 si_band 中的位掩码有着一一对应关系。

    //POLL_IN：可读取数据
    //POLL_OUT：可写入数据
    //POLL_MSG：不使用
    //POLL_ERR：I/O 错误
    //POLL_PRI：可读取高优先级数据
    //POLL_HUP：出现宕机
    if(info->si_code==POLL_IN)
    {
        res=read(fd,buff,sizeof(buff));
        if(res>0)
        {
            printf("鼠标: 成功读取<%d>个字节数据\n", res);
        }
        loop--;
        if(loop<=0)
        {
            close(fd);
            exit(0);
        }
    }
    
}