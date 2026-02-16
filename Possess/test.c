#include "stdio.h"
#include "stdlib.h"
#include "signal.h"
#include "sys/types.h"
#include "unistd.h"
#include "errno.h"
#include "sys/stat.h"
#include "fcntl.h"
#include "signal.h"
#include "time.h"
#include "sys/wait.h"

void sigchild_handler(int sig);
int create_Deamon(void);
void daemon_process(void);

int main(void)
{
    struct sigaction sa;
    sa.sa_handler=sigchild_handler;
    sa.sa_flags=SA_RESTART;
    if(sigaction(SIGCHLD,&sa,NULL)==-1)
    {
        perror("set SIGCHID error");
        exit(-1);
    }

    printf("\nparent possess is running\n");

    /* 创建一个新的进程 */
    //子进程拥有父进程数据段、堆、栈等副本，共享代码段，文件操作共享
    //在父进程返回子进程的PID，在子进程中返回0
    int child1_pid=fork();
    if(child1_pid==-1)
    {
        perror("create child possess error");
    }
    else if (child1_pid==0)
    {
        printf("\nchild possess is running\n");
        /* 获取进程所在进程组 */
        if(getpgid(child1_pid)!=child1_pid)
        {
            /* 加入或创建进程组 */
            //setpgid()函数将参数 pid 指定的进程的进程组 ID 设置为参数 gpid。如果这两个参数相等（pid==gpid），
            // 则由 pid 指定的进程变成为进程组的组长进程，创建了一个新的进程；如果参数 pid 等于 0，则使用调用者
            // 的进程 ID；另外，如果参数 gpid 等于 0，则创建一个新的进程组，由参数 pid 指定的进程作为进程组组长
            // 进程。
            if(setpgid(child1_pid,0)==-1)
            {
                perror("set possess group error");
                exit(-1);
            }
            printf("\n child possess group is created \n");
        }
        //exec类函数，让当下进程执行另外一个程序（或是一个新的程序，或是一些命令）
        //没有返回值（别看他的类型是int），执行完后子进程自动exit
        execlp("ls","ls","-l","/home",NULL);
        printf("execlp error\n");
        exit(-1);
    }
    int child2_pid=fork();
    if(child2_pid==-1)
    {
        perror("create child2 possess error");
        exit(-1);
    }
    else if (child2_pid==0)
    {
        int res=0;
        //创建守护进程
        res=create_Deamon();
        if(res==-1)
        {
            printf("\n!create deamon possess error!\n");
        }
        daemon_process();
        exit(res);
    }
    printf("\nparent pocess ready sleep\n");
    pause();
    printf("\nparent pocess ready sleep\n");
    pause();
    printf("\nparent pocess ready back\n");
    exit(0);
}


int create_Deamon(void)
{
    //创建子进程，父进程退出
    int child3_pid=fork();
    if(child3_pid==-1)
    {
        perror("create child3_pid error");
        return -1;
    }
    else if(child3_pid>0)
    {
        printf("\nready to create deamon possess\n");
        exit(0);
    }
    //创建新会话
    if(setsid()<0)
    {
        perror("setsid error");
        return -1;
    }
    //再次fork并父进程推出，防止会话首领再次获取终端
    int child4_pid=fork();
    if(child4_pid==-1)
    {
        perror("fork again error");
        return -1;
    }
    else if (child4_pid>0)
    {
        exit(0);
    }
    //将工作目录更改为根目录
    if(chdir("/tmp")==-1)
    {
        perror("change dir error");
        return -1;
    }
    //重设文件权限掩码 umask
    umask(0);
    //关闭所有不再需要的文件描述符
    for(int i=0;i<sysconf(_SC_OPEN_MAX);i++)
    {
        close(i);
    }
    //将文件描述符号为 0、1、2 定位到/dev/null
    open("/dev/null",O_RDWR);
    dup(0);
    dup(0);
    //忽略 SIGCHLD 信号
    signal(SIGCHLD,SIG_IGN);
    return 0;

}

void daemon_process(void)
{
    FILE *fp=fopen("daemon.log","a");
    if(fp==NULL)
    {
        exit(-1);
    }
     for (int i = 0; i < 4; i++) {
        time_t now = time(NULL);
        char *time_str = ctime(&now);
        fprintf(fp, "守护进程(%d)运行中，当前时间：%s", getpid(), time_str);
        fflush(fp); // 强制刷新缓冲区
        sleep(5);
    }
    fclose(fp);
    exit(0);
}

void sigchild_handler(int sig)
{
    printf("\nreceive SIGCHID signal\n");
    /* 收尸 */
    //wnohang：非阻塞式
    while(waitpid(-1,NULL,WNOHANG)>0)
    {
        continue;
    }
}
